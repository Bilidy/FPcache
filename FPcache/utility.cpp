
#include "util.h"
#include "common.h"
#include <sstream>


using std::cout;
using std::endl;
using std::cin;
double generateGaussianNoise(double mu, double sigma)
{
	const double epsilon = 2.2250738585072014e-308;
	const double two_pi = 2.0*3.14159265358979323846;

	static double z0, z1;
	static bool generate;
	generate = !generate;

	if (!generate)
		return z1 * sigma + mu;

	double u1, u2;
	do
	{
		u1 = rand() * (1.0 / RAND_MAX);
		u2 = rand() * (1.0 / RAND_MAX);
	} while (u1 <= epsilon);

	z0 = sqrt(-2.0 * log(u1)) * cos(two_pi * u2);
	z1 = sqrt(-2.0 * log(u1)) * sin(two_pi * u2);
	return z0 * sigma + mu;
}

struct lastState
{
	uint64_t lastStateAcc = 0;
	uint64_t lastStateHit = 0;
	uint64_t lastStateMis = 0;
};
void resetMetaAccnum(std::map<Item, metadata> &hashtable, float olddisWei,	float newdisWei) {
	auto it = hashtable.begin();
	while (it!= hashtable.end())
	{
		it->second.accnum = 1;
		/*************/
		it->second.lastdis = 1;//lru.stateACC() - m.lastacc;
		it->second.dis = 0;// lru.stateACC() - m.lastacc;
		it->second.weidis = olddisWei * it->second.lastdis + newdisWei * it->second.dis;
		/*************/
		it++;
	}
}
void recordPatternsItem(std::vector<Pattern>&sortedPatterns,string outputfile) {
	auto PatternIt = sortedPatterns.begin();
	ofstream oFile;
	oFile.open("pat_" + outputfile, ios::out | ios::app);
	std::map<Item,int> items;
	while (PatternIt != sortedPatterns.end())
	{
		auto ItemIt = (*PatternIt).first.begin();
		while (ItemIt != (*PatternIt).first.end())
		{
			items[*ItemIt] = 0; 
			ItemIt++;
			/*oFile << atoi((*ItemIt).c_str());
			if ((ItemIt++) != (*PatternIt).first.end())
			{
				oFile << ",";
			}*/
		}
		PatternIt++;
	}
	auto item = items.begin();
	while (item!= items.end())
	{
		oFile << atoi((*item).first.c_str());
		if ((item++) != items.end())
		{
			oFile << ",";
		}
	}
	oFile << endl;
	oFile.close();
}
void uniAccess(LRUStack& lru, FPCache&fpcahe, ARCCache&accache,RR&random,LIRSCache&lirsCache, MQCache& mqCache, LRUKCache&lrukCache, LFUCache&lfuCache,
	std::vector<Transaction>&transactions,
	std::vector<Transaction>&temptrans,
	size_t M,
	float rate,
	float a,

	float olddisWei,
	float newdisWei,

	string outputfile,
	int _type)
{
	temptrans.clear();
	temptrans = transactions;
	std::vector<Transaction> samplingTrans;
	std::vector<Transaction> slice;
	std::vector<Transaction> tempslice;
	std::vector<vector<uint64_t>> timeArray(C_NUM);

	int finishedCounter = 0;
	int totalsize = transactions.size();
	int sliceCounter = 0;
	int sampCounter = 0;
	int thenext = 1;
	std::set<Pattern> patterns;
	std::vector<Pattern> sortedPatterns;
	std::vector<valuatedPattern> valuatedpatterns;
	std::vector<valuatedPattern> valuatedpatterns2;

	//ofstream oFileFPCHit;
	//ofstream oFileFPCMiss;

	ofstream oFileInfo;
	//ofstream oFileLRUKMiss;

	lastState FPClast;
	lastState LRUlast;
	lastState ARClast;
	lastState RRlast;
	lastState LIRSlast;
	lastState LRUKlast;
	lastState LFUlast;

	uint32_t time = 1;
	int streamNO = 1;
	ofstream oFile;
	std::map<Item, metadata> hashtable;
	int recordcounter = 0;

	//oFileFPCHit.open("fpc_Hit_" + outputfile, ios::out | ios::app);
	//oFileFPCMiss.open("fpc_Mis_" + outputfile, ios::out | ios::app);
	
	//oFileLRUKMiss.open("LRUK_Mis_" + outputfile, ios::out | ios::app);
	for (size_t index = 0; index < WINDOW; index++)
	{
		std::vector<Transaction>::iterator beginIt = temptrans.begin();
		//int idx = radmGen(0, temptrans.size() - 1, 1);
		int idx = 0;
		slice.push_back(temptrans.at(idx));
		for (size_t i = 0; i < C_NUM; i++)
		{
			timeArray.at(i).push_back(0);
		}
		std::vector<Transaction>::iterator it = beginIt + idx;
		temptrans.erase(it);
	}
	while (temptrans.size() > 0) {
		tempslice = slice;//保存信息，在访问过程中slice的数据会消失
		//我们得到了一个含有一组事务的容器slice，下面随机取出这个容器中的事务项
		while (!slice.empty())
		{
			extern uint16_t TimeSlice;
			int timeslice = TimeSlice;
			//int index = radmGen(0, slice.size() - 1, 1);//选取容器中的一个事务
			int index = 0;
			while (timeslice-- != 0) {			//如果事务中项的数量不为空
				if (index >= slice.size())
				{
					break;
				}

				Item item = (*slice.at(index).begin());
				slice.at(index).erase(slice.at(index).begin());

				//在hashtable中插入元数据
				if (hashtable.find(item) == hashtable.end())
				{
					metadata m;
					//srand(atoi(item.c_str()));
					//m.size = ((uint64_t)abs(generateGaussianNoise(0, 35))+1)*1024;
					m.size = 1;
					m.accnum = 1;
					m.lastacc = lru.stateACC();
					m.lastdis = 1;//lru.stateACC() - m.lastacc;
					m.dis = 0;// lru.stateACC() - m.lastacc;
					m.weidis = olddisWei * m.lastdis + newdisWei * m.dis;
					hashtable[item] = m;
				}
				else
				{
					/*newdisWei = 1.0 / hashtable[item].lastdis;
					olddisWei = 1.0 - newdisWei;*/
					hashtable[item].accnum++;
					hashtable[item].dis = lru.stateACC() - hashtable[item].lastacc;
					hashtable[item].weidis = olddisWei * hashtable[item].lastdis + newdisWei * hashtable[item].dis;
					hashtable[item].lastdis = hashtable[item].weidis;
					hashtable[item].lastacc = lru.stateACC();
				}

				Entry entry;
				entry.item = item;
				entry.size = hashtable[item].size;
				/*****************************************************************/
				
				if (lru.access(entry)) {
					timeArray.at(LRU_POS).at(index) += MEM_ACC_TIME;
					//oFileFPCHit << fpcahe.stateACC() << "," << entry.item << endl;
				}
				else {
					timeArray.at(LRU_POS).at(index) += DIK_ACC_TIME;
					//oFileFPCMiss << fpcahe.stateACC() << "," << entry.item << endl;
				}//模拟访问
				/*****************************************************************/
				if (fpcahe.access(entry)) {
					timeArray.at(FPC_POS).at(index) += MEM_ACC_TIME;
					//oFileFPCHit << fpcahe.stateACC() << "," << entry.item << endl;
				}
				else{
					timeArray.at(FPC_POS).at(index) += DIK_ACC_TIME;
					//oFileFPCMiss<< fpcahe.stateACC() << "," << entry.item << endl;
				}//模拟访问
				/*****************************************************************/
				if (accache.access(entry)) {
					timeArray.at(ARC_POS).at(index) += MEM_ACC_TIME;
					//oFileFPCHit << accache.getAcc() << "," << entry.item << endl;
				}
				else {
					timeArray.at(ARC_POS).at(index) += DIK_ACC_TIME;
					//oFileFPCMiss << accache.getAcc() << "," << entry.item << endl;
				}//模拟访问
				/*****************************************************************/
				if (random.access(entry)) {
					timeArray.at(RR_POS).at(index) += MEM_ACC_TIME;
					//oFileFPCHit << random.stateACC() << "," << entry.item << endl;
				}
				else {
					timeArray.at(RR_POS).at(index) += DIK_ACC_TIME;
					//oFileFPCMiss << random.stateACC() << "," << entry.item << endl;
				}//模拟访问
				/*****************************************************************/
				if (lirsCache.access(entry)) {
					timeArray.at(LIRS_POS).at(index) += MEM_ACC_TIME;
					//oFileFPCHit << lirsCache.getAcc() << "," << entry.item << endl;
				}
				else {
					timeArray.at(LIRS_POS).at(index) += DIK_ACC_TIME;
					//oFileFPCMiss << lirsCache.getAcc() << "," << entry.item << endl;
				}//模拟访问
				/*****************************************************************/
				if (lrukCache.access(entry)) {
					timeArray.at(LRUK_POS).at(index) += MEM_ACC_TIME;
					//oFileLRUKHit << lrukCache.getAcc() << "," << entry.item << endl;

				}
				else {
					timeArray.at(LRUK_POS).at(index) += DIK_ACC_TIME;
					//oFileLRUKMiss << lrukCache.getAcc() << "," << entry.item << endl;
				}//模拟访问
				/*****************************************************************/
				if (lfuCache.access(entry)) {
					timeArray.at(LFU_POS).at(index) += MEM_ACC_TIME;
					//oFileLRUKHit << lfuCache.getAcc() << "," << entry.item << endl;

				}
				else {
					timeArray.at(LFU_POS).at(index) += DIK_ACC_TIME;
					//oFileLRUKMiss << lfuCache.getAcc() << "," << entry.item << endl;
				}//模拟访问
				/*****************************************************************/
				recordcounter++;
				if (recordcounter == RECORDE_STEP)
				{
					oFile.open(outputfile, ios::out | ios::app);
					oFile << streamNO++ << ","
						<< ((float)(fpcahe.stateHIT() - FPClast.lastStateHit)) / (fpcahe.stateACC() - FPClast.lastStateAcc) * 100 << ","
						<< ((float)(lru.stateHIT() - LRUlast.lastStateHit)) / ((lru.stateACC() - LRUlast.lastStateAcc)) * 100 << ","
						<< ((float)(random.stateHIT() - RRlast.lastStateHit)) / ((random.stateACC() - RRlast.lastStateAcc)) * 100 << ","
						<< ((float)(accache.getHit() - ARClast.lastStateHit)) / ((accache.getAcc() - ARClast.lastStateAcc)) * 100 << ","
						<< ((float)(lirsCache.getHit() - LIRSlast.lastStateHit)) / ((lirsCache.getAcc() - LIRSlast.lastStateAcc)) * 100 << ","
						<< ((float)(lrukCache.getHit() - LRUKlast.lastStateHit)) / ((lrukCache.getAcc() - LRUKlast.lastStateAcc)) * 100 << ","
						<< ((float)(lfuCache.getHit() - LFUlast.lastStateHit)) / ((lfuCache.getAcc() - LFUlast.lastStateAcc)) * 100 << ","
						<< ((float)(fpcahe.stateHIT() - FPClast.lastStateHit)) / (fpcahe.stateACC() - FPClast.lastStateAcc) * 100 - ((float)(lru.stateHIT() - LRUlast.lastStateHit)) / ((lru.stateACC() - LRUlast.lastStateAcc)) * 100
						<< endl;
					oFile.close();
					FPClast.lastStateAcc = fpcahe.stateACC();
					FPClast.lastStateHit = fpcahe.stateHIT();
					FPClast.lastStateMis = fpcahe.stateFault();
					LRUlast.lastStateAcc = lru.stateACC();
					LRUlast.lastStateHit = lru.stateHIT();
					LRUlast.lastStateMis = lru.stateFault();
					RRlast.lastStateAcc = random.stateACC();
					RRlast.lastStateHit = random.stateHIT();
					RRlast.lastStateMis = random.stateFault();
					ARClast.lastStateAcc = accache.getAcc();
					ARClast.lastStateHit = accache.getHit();
					ARClast.lastStateMis = accache.getMis();
					LIRSlast.lastStateAcc = lirsCache.getAcc();
					LIRSlast.lastStateHit = lirsCache.getHit();
					LIRSlast.lastStateMis = lirsCache.getMis();
					LRUKlast.lastStateAcc = lrukCache.getAcc();
					LRUKlast.lastStateHit = lrukCache.getHit();
					LRUKlast.lastStateMis = lrukCache.getMis();
					LFUlast.lastStateAcc = lfuCache.getAcc();
					LFUlast.lastStateHit = lfuCache.getHit();
					LFUlast.lastStateMis = lfuCache.getMis();
					
					recordcounter = 0;
				}


				//slice.at(index).erase(slice.at(index).begin() + itmIndex);//将此项从该事务中删除
				//slice.at(index).shrink_to_fit();

				if (slice.at(index).empty())
				{
					sliceCounter++;
					finishedCounter++;
					thenext--;
					if (thenext == 0) {//该采样了
						samplingTrans.push_back(Transaction(tempslice.at(index)));
						thenext = sampTheNext(a, rate, sampCounter, M);
						sampCounter++;
					}
					if (thenext == 0) {
						thenext = 1;
					}
					//保持一致性
					slice.erase(slice.begin() + index);
					//slice.shrink_to_fit();
					tempslice.erase(tempslice.begin() + index);

					fpcahe.timeINC(timeArray.at(FPC_POS).at(index)); fpcahe.seqnumINC();
					accache.timeINC(timeArray.at(ARC_POS).at(index)); accache.seqnumINC();
					lru.timeINC(timeArray.at(LRU_POS).at(index)); lru.seqnumINC();
					random.timeINC(timeArray.at(RR_POS).at(index)); random.seqnumINC();
					lirsCache.timeINC(timeArray.at(LIRS_POS).at(index)); lirsCache.seqnumINC();
					lrukCache.timeINC(timeArray.at(LRUK_POS).at(index)); lrukCache.seqnumINC();
					lfuCache.timeINC(timeArray.at(LFU_POS).at(index)); lfuCache.seqnumINC();
					for (size_t i = 0; i < C_NUM; i++)
					{
						timeArray.at(i).erase(timeArray.at(i).begin() + index);
					}
					//添加一个事务进入slice
					if (temptrans.size() > 0) {
						std::vector<Transaction>::iterator beginIt = temptrans.begin();
						int idx = radmGen(0, temptrans.size() - 1, 1);
						slice.push_back(temptrans.at(idx));
						for (size_t i = 0; i < C_NUM; i++)
						{
							timeArray.at(i).push_back(0);
						}
						tempslice.push_back(temptrans.at(idx));
						std::vector<Transaction>::iterator it = beginIt + idx;
						temptrans.erase(it);
					}
					if (sliceCounter == M || (slice.size() == 0)) {
				//	if (sampCounter == M * rate || (slice.size() == 0)) {
						sliceCounter = 0;
						sampCounter = 0;
						thenext = 1;
						/***********************************************************/
						DWORD StartTime = ::GetTickCount();
						fpcahe.setMinSupport(ceil(fpcahe.getMinSupportWet()*(M*rate + samplingTrans.size()) / 2));
						fpcahe.runFPAnalyse(samplingTrans, patterns);
						int sizeofpatterns = patterns.size();
						DWORD EndTime = ::GetTickCount();
						DWORD runtime = EndTime - StartTime;
						//fpcahe.sortPatternsBySup(sortedPatterns, patterns);
						/*fpcahe.valuatePatterns(patterns, hashtable, valuatedpatterns);
						fpcahe.sortPatternsByDensity(sortedPatterns, valuatedpatterns);*/
						switch (_type)
						{
						case 1:
							fpcahe.sortPatternsBySup(sortedPatterns, patterns);
							break;
						case 2:
							StartTime = ::GetTickCount();
							fpcahe.valuatePatterns(patterns, hashtable, valuatedpatterns, valuatedpatterns2);
							EndTime = ::GetTickCount();
							runtime+= EndTime - StartTime;
							fpcahe.sortPatternsByDensity(sortedPatterns, hashtable, valuatedpatterns);
							break;
						case 3:
							StartTime = ::GetTickCount();
							fpcahe.valuatePatterns(patterns, hashtable, valuatedpatterns, valuatedpatterns2);
							EndTime = ::GetTickCount();
							runtime += EndTime - StartTime;
							fpcahe.sortPatternsByVal(sortedPatterns, hashtable, valuatedpatterns);
							break;
						default:
							fpcahe.sortPatternsBySup(sortedPatterns, patterns);
							break;
						}
						//recordPatternsItem(sortedPatterns, outputfile);
						fpcahe.procPattern(sortedPatterns, valuatedpatterns2, hashtable, fpcahe.getHighCorrCache().getShadowCache(), fpcahe.getLowCorrCache().getShadowCache());
						fpcahe.cacheOrganize(hashtable);
						resetMetaAccnum(hashtable,olddisWei,newdisWei);
						
						cout << streamNO << ":" << recordcounter << "	*耗时" << runtime << "ms" <<"	"<< sizeofpatterns <<"/"<< sortedPatterns.size() << endl;
						oFileInfo.open("Info_" + outputfile, ios::out | ios::app);
						oFileInfo<< runtime << "," << sortedPatterns.size() << endl;
						oFileInfo.close();
						/***********************************************************/

						cout << "FPC:	avgtime:"<< fpcahe .getAvgtime()<<"	ACC:" << fpcahe.stateACC()
							<< " HIT:" << fpcahe.stateHIT()
							//<< " FAULT:" << fpcahe.stateFault()
							<< "	hit ratio:" << ((float)(fpcahe.stateHIT())) / (fpcahe.stateACC()) * 100 << "%	"
							<< fpcahe.getHighCorrCache().getCacheSize() << "/" << fpcahe.getHighCorrCache().getMaxSize() << "(" << fpcahe.getHighItemNum() << ")"
							<< "	sample num:" << samplingTrans.size()
							<< "	【" << ((double)finishedCounter / totalsize) * 100 << "%】" << endl;
						cout << "LRU:	avgtime:" << lru.getAvgtime() << "	ACC:" << lru.stateACC()
							<< " HIT:" << lru.stateHIT()
							//<< " FAULT:" << lru.stateFault()
							<< "	hit ratio:" << ((float)(lru.stateHIT())) / ((lru.stateACC())) * 100 << "%" 
							<< "	delta:" << ((float)(fpcahe.stateHIT())) / (fpcahe.stateACC()) * 100- ((float)(lru.stateHIT())) / ((lru.stateACC())) * 100<<"%" << endl;
						cout << "RR :	avgtime:" << random.getAvgtime() << "	ACC:" << random.stateACC()
							<< " HIT:" << random.stateHIT()
							//<< " FAULT:" << random.stateFault()
							<< "	hit ratio:" << ((float)(random.stateHIT())) / ((random.stateACC())) * 100 << "%"
							<< "	delta:" << ((float)(fpcahe.stateHIT())) / (fpcahe.stateACC()) * 100 - ((float)(random.stateHIT())) / ((random.stateACC())) * 100 << "%" << endl;
						cout << "ARC:	avgtime:" << accache.getAvgtime()<<" "; accache.getHitRatio();
						cout << "LIRS:	avgtime:" << lirsCache.getAvgtime() << " "; lirsCache.getHitRatio();
						cout << "LRUK::	avgtime:" << lrukCache.getAvgtime() << " "; lrukCache.getHitRatio();
						cout << "LFU::	avgtime:" << lfuCache.getAvgtime() << " "; lfuCache.getHitRatio();
						fpcahe.restAvgtime();
						lru.restAvgtime();
						random.restAvgtime();
						accache.restAvgtime();
						lirsCache.restAvgtime();
						lrukCache.restAvgtime();
						lfuCache.restAvgtime();
						//cout << "MQ:	"; mqCache.getHitRatio();
						cout << endl;
						samplingTrans.clear();
						continue;
					}
				}
			}
		}
	}
	//oFileFPCHit.close();
	//oFileFPCMiss.close();
	//oFileLRUKHit.close();
	//oFileLRUKMiss.close();
}

void uniAccess(LRUStack& lru, FPCache&fpcahe, ARCCache&accache, RR&random,
	std::vector<Transaction>&transactions,
	std::vector<Transaction>&w_transactions,
	std::vector<Transaction>&temptrans,
	size_t M,
	float rate,
	float a,
	size_t skew_jump_low,
	size_t skew_jump_high,
	float olddisWei,
	float newdisWei,
	string outputfile,
	int _type
)
{
	temptrans.clear();
	temptrans = transactions;
	std::vector<Transaction>wtemptrans = w_transactions;
	std::vector<Transaction> samplingTrans;
	std::vector<Transaction> slice;
	std::vector<Transaction> tempslice;


	int finishedCounter = 0;
	int totalsize = transactions.size();
	int sliceCounter = 0;
	int sampCounter = 0;
	int thenext = 1;
	std::set<Pattern> patterns;
	std::vector<valuatedPattern> valuatedpatterns;
	std::vector<valuatedPattern> valuatedpatterns2;
	std::vector<Pattern> sortedPatterns;

	lastState FPClast;
	lastState LRUlast;
	lastState ARClast;
	lastState RRlast;


	int streamNO = 1;
	ofstream oFile;

	std::map<Item, metadata> hashtable;
	
	int recordcounter=0;
	for (size_t index = 0; index < WINDOW; index++)
	{
		std::vector<Transaction>::iterator beginIt = temptrans.begin();
		int idx = radmGen(0, temptrans.size() - 1, 1);
		slice.push_back(temptrans.at(idx));
		std::vector<Transaction>::iterator it = beginIt + idx;
		temptrans.erase(it);
	}
	while (temptrans.size()> 0) {
		tempslice = slice;//保存信息，在访问过程中slice的数据会消失
		//我们得到了一个含有一组事务的容器slice，下面随机取出这个容器中的事务项
		while (!slice.empty())
		{
			extern uint16_t TimeSlice;
			int timeslice = TimeSlice;
			int index = radmGen(0, slice.size() - 1, 1);//选取容器中的一个事务
			//if (slice.at(index).size() > 0) {			//如果事务中项的数量不为空
			while (timeslice-- != 0) {			//如果事务中项的数量不为空
				if (index >= slice.size())
				{
					break;
				}		//如果事务中项的数量不为空
				//int itmIndex = radmGen(0, slice.at(index).size() - 1, 1);//选取事务中的一个项
				Item item = (*slice.at(index).begin());
				slice.at(index).erase(slice.at(index).begin());
				
				//在hashtable中插入元数据
				if (hashtable.find(item) == hashtable.end())
				{
					metadata m;
					//srand(atoi(item.c_str()));
					if ((skew_jump_low <= finishedCounter) && (skew_jump_high >= finishedCounter)) {
						m.size = ((uint64_t)abs(generateGaussianNoise(8, 40)+0.001)) * 1024;
					}
					else{
						m.size = ((uint64_t)abs(generateGaussianNoise(0, 35)+1)) * 1024;
					}
					m.accnum = 1;
					m.lastacc = 0;
					m.lastdis = lru.stateACC() - m.lastacc;
					m.dis = lru.stateACC() - m.lastacc;
					m.weidis =1+ olddisWei * m.lastdis + newdisWei * m.dis;
					m.lastdis = m.weidis;
					hashtable[item] = m;
				}
				else{
					//newdisWei = 1.0 / hashtable[item].lastdis;
					//olddisWei = 1.0 - newdisWei;
					hashtable[item].accnum++;
					hashtable[item].dis = lru.stateACC() - hashtable[item].lastacc;
					hashtable[item].weidis = olddisWei * hashtable[item].lastdis + newdisWei *hashtable[item].dis;
					hashtable[item].lastdis = hashtable[item].weidis;
					hashtable[item].lastacc = lru.stateACC();
				}
				
				Entry entry;
				entry.item = item;
				entry.size = hashtable[item].size;

				if ((skew_jump_low <= finishedCounter) && (skew_jump_high >= finishedCounter)){
					lru.access(entry);//模拟访问
					fpcahe.access(entry);//模拟访问
					accache.access(entry);
					random.access(entry);
					//hashtable[slice.at(index).at(itmIndex)].dis = 0;
					
				}
				else {
					lru.access(entry);
					fpcahe.access(entry);//模拟访问
					accache.access(entry);
					random.access(entry);
				}

				recordcounter++;
				if (recordcounter== RECORDE_STEP)
				{
					oFile.open(outputfile, ios::out | ios::app);
					oFile << streamNO++ << ","
						<< ((float)(fpcahe.stateHIT() - FPClast.lastStateHit)) / (fpcahe.stateACC() - FPClast.lastStateAcc) * 100 << ","
						<< ((float)(lru.stateHIT() - LRUlast.lastStateHit)) / ((lru.stateACC() - LRUlast.lastStateAcc)) * 100 << ","
						<< ((float)(accache.getHit() - ARClast.lastStateHit)) / ((accache.getAcc() - ARClast.lastStateAcc)) * 100 << ","
						<< ((float)(random.stateHIT() - RRlast.lastStateHit)) / ((random.stateACC() - RRlast.lastStateAcc)) * 100 << ","
						<< ((float)(fpcahe.stateHIT() - FPClast.lastStateHit)) / (fpcahe.stateACC() - FPClast.lastStateAcc) * 100 - ((float)(lru.stateHIT() - LRUlast.lastStateHit)) / ((lru.stateACC() - LRUlast.lastStateAcc)) * 100 
						<< endl;
					oFile.close();

					FPClast.lastStateAcc = fpcahe.stateACC();
					FPClast.lastStateHit = fpcahe.stateHIT();
					FPClast.lastStateMis = fpcahe.stateFault();
					LRUlast.lastStateAcc = lru.stateACC();
					LRUlast.lastStateHit = lru.stateHIT();
					LRUlast.lastStateMis = lru.stateFault();
					RRlast.lastStateAcc = random.stateACC();
					RRlast.lastStateHit = random.stateHIT();
					RRlast.lastStateMis = random.stateFault();
					ARClast.lastStateAcc = accache.getAcc();
					ARClast.lastStateHit = accache.getHit();
					ARClast.lastStateMis = accache.getMis();

					recordcounter = 0;
				}


				//slice.at(index).erase(slice.at(index).begin() + itmIndex);//将此项从该事务中删除
				//slice.at(index).shrink_to_fit();

				if (slice.at(index).empty())
				{
					sliceCounter++;

					finishedCounter++;
					if (((skew_jump_low== finishedCounter)|| (skew_jump_high == finishedCounter))&&(wtemptrans.size()>0))
					{
						cout << streamNO << ":" << recordcounter << "##### workload changed ####"<<endl;
					}

					thenext--;

					if (thenext == 0) {//该采样了
						samplingTrans.push_back(Transaction(tempslice.at(index)));
						thenext = sampTheNext(a, rate, sampCounter, M);
						sampCounter++;
					}
					if (thenext == 0) {
						thenext = 1;
					}

					//保持一致性
					slice.erase(slice.begin() + index);
					tempslice.erase(tempslice.begin() + index);
					//添加一个事务进入slice
					if (wtemptrans.size() > 0 && (skew_jump_low <= finishedCounter) && (skew_jump_high >= finishedCounter)) {
						std::vector<Transaction>::iterator beginIt = wtemptrans.begin();
						int idx = radmGen(0, wtemptrans.size() - 1, 1);
						slice.push_back(wtemptrans.at(idx));
						tempslice.push_back(wtemptrans.at(idx));
						std::vector<Transaction>::iterator it = beginIt + idx;
						wtemptrans.erase(it);
						if (wtemptrans.size() == 0) {
							cout << streamNO << ":" << recordcounter<< "##### workload changed ####" << endl;
						}
					}
					else if (temptrans.size() > 0) {
						std::vector<Transaction>::iterator beginIt = temptrans.begin();
						int idx = radmGen(0, temptrans.size() - 1, 1);
						slice.push_back(temptrans.at(idx));
						tempslice.push_back(temptrans.at(idx));
						std::vector<Transaction>::iterator it = beginIt + idx;
						temptrans.erase(it);
					}
					if (sliceCounter == M|| (slice.size() == 0)){
					//if (sampCounter == M * rate || (slice.size()==0)) {
						sliceCounter = 0;
						sampCounter = 0;
						thenext = 1;
						/***********************************************************/
						DWORD StartTime = ::GetTickCount();
						fpcahe.setMinSupport(ceil(fpcahe.getMinSupportWet()*(M*rate+samplingTrans.size())/2));
						fpcahe.runFPAnalyse(samplingTrans, patterns);
						
						switch (_type)
						{
						case 1://type1使用支持度作为调入指标
							fpcahe.sortPatternsBySup(sortedPatterns, patterns);
							break;
						case 2://type2使用访问密度作为调入指标
							fpcahe.valuatePatterns(patterns, hashtable, valuatedpatterns, valuatedpatterns2);
							fpcahe.sortPatternsByDensity(sortedPatterns, hashtable, valuatedpatterns);
							break;
						case 3://type3使用value作为调入指标
							fpcahe.valuatePatterns(patterns, hashtable, valuatedpatterns, valuatedpatterns2);
							fpcahe.sortPatternsByVal(sortedPatterns, hashtable, valuatedpatterns);
							break;
						default:
							fpcahe.sortPatternsBySup(sortedPatterns, patterns);
							break;
						}
						fpcahe.procPattern(sortedPatterns, valuatedpatterns2, hashtable, fpcahe.getHighCorrCache().getShadowCache(),fpcahe.getLowCorrCache().getShadowCache());
						fpcahe.cacheOrganize(hashtable);
						DWORD EndTime = ::GetTickCount();
						cout <<streamNO <<":"<<recordcounter<< "	*耗时" << EndTime - StartTime << "ms" <<"	number of filted Patterns "<< sortedPatterns.size() << endl;
						/***********************************************************/

						cout << "FPC:	ACC:" << fpcahe.stateACC()
							<< " HIT:" << fpcahe.stateHIT()
							<< " FAULT:" << fpcahe.stateFault()
							<< "	hit ratio:" << ((float)(fpcahe.stateHIT())) / (fpcahe.stateACC()) * 100 << "%	"
							<< fpcahe.getHighCorrCache().getCacheSize() << "/" << fpcahe.getHighCorrCache().getMaxSize()<<"("<<fpcahe.getHighItemNum()<<")"
							<< "	sample num:" << samplingTrans.size()
							<< "	【" << ((double)finishedCounter / totalsize) * 100 << "%】" << endl;
						cout << "LRU:	ACC:" << lru.stateACC()
							<< " HIT:" << lru.stateHIT()
							<< " FAULT:" << lru.stateFault()
							<< "	hit ratio:" << ((float)(lru.stateHIT())) / ((lru.stateACC())) * 100 << "%"
							<< "	delta:" << ((float)(fpcahe.stateHIT())) / (fpcahe.stateACC()) * 100 - ((float)(lru.stateHIT())) / ((lru.stateACC())) * 100 << "%" << endl;
						cout << "RR :	ACC:" << random.stateACC()
							<< " HIT:" << random.stateHIT()
							<< " FAULT:" << random.stateFault()
							<< "	hit ratio:" << ((float)(random.stateHIT())) / ((random.stateACC())) * 100 << "%"
							<< "	delta:" << ((float)(fpcahe.stateHIT())) / (fpcahe.stateACC()) * 100 - ((float)(random.stateHIT())) / ((random.stateACC())) * 100 << "%" << endl;
						cout << "ARC:	ACC:" << accache.getAcc()
							<< " HIT:" << accache.getHit()
							<< " FAULT:" << accache.getMis()
							<< "	hit ratio:" << ((float)(accache.getHit()) / (accache.getAcc())) * 100 << "%" << endl;
						cout << endl;
						samplingTrans.clear();
						continue;
					}					
				}
			}
		}
	}
}

void uniAccess(LRUStack & lru, FPCache&fpcahe, std::vector<Transaction>& transactions, std::vector<Transaction>& temptrans, float olddis, float newdis, size_t M, float rate, float a, string outputfile, int _type)
{
	temptrans.clear();
	temptrans = transactions;
	std::vector<Transaction> samplingTrans;
	std::vector<Transaction> slice;
	std::vector<Transaction> tempslice;
	std::vector<Transaction> randmTrans;
	valuatedPattern spyTran;
	Transaction spyNotTran;
	int finishedCounter = 0;
	int totalsize = transactions.size();
	int sliceCounter = 0;
	int sampCounter = 0;
	int thenext = 1;
	std::set<Pattern> patterns;
	std::vector<Pattern> sortedPatterns;
	std::vector<valuatedPattern> valuatedpatterns;
	std::vector<valuatedPattern> valuatedpatterns2;

	lastState FPClast;
	lastState LRUlast;


	int streamNO = 1;
	int frameNum = 1;
	ofstream oFile;
	ofstream oFile2;
	ofstream oFileHit;
	ofstream oFileMiss;
	std::map<Item, metadata> hashtable;
	int recordcounter = 0;

	oFileHit.open("patens_Hit_" + outputfile, ios::out | ios::app);
	oFileMiss.open("patens_Mis_" + outputfile, ios::out | ios::app);
	for (size_t index = 0; index < WINDOW; index++)
	{
		std::vector<Transaction>::iterator beginIt = temptrans.begin();
		int idx = radmGen(0, temptrans.size() - 1, 1);
		slice.push_back(temptrans.at(idx));
		std::vector<Transaction>::iterator it = beginIt + idx;
		temptrans.erase(it);
	}
	while (temptrans.size() > 0) {
		tempslice = slice;//保存信息，在访问过程中slice的数据会消失
		//我们得到了一个含有一组事务的容器slice，下面随机取出这个容器中的事务项
		while (!slice.empty())
		{
			extern uint16_t TimeSlice;
			uint16_t timeslice = TimeSlice;//TIME_SLICE;
			int index = radmGen(0, slice.size() - 1, 1);//选取容器中的一个事务
			while (timeslice--!=0) {			//如果事务中项的数量不为空
				if (index>= slice.size())
				{
					break;
				}

				Item item= (*slice.at(index).begin());
				slice.at(index).erase(slice.at(index).begin());
				//在hashtable中插入元数据
				if (hashtable.find(item) == hashtable.end())
				{
					metadata m;
					//srand(atoi(item.c_str()));
					m.size = ((uint64_t)abs(generateGaussianNoise(0, 35) + 0.0001)) * 1024;
					m.accnum = 1;
					m.lastacc = 0;
					m.lastdis = 1;//lru.stateACC() - m.lastacc;
					m.dis = 0;//lru.stateACC() - m.lastacc;
					m.weidis = 1 + olddis * m.lastdis + newdis * m.dis;
					m.lastdis = m.weidis;
					hashtable[item] = m;
				}
				else
				{
					/*newdis = 1.0 / hashtable[item].lastdis;
					olddis = 1.0 - newdis;*/
					hashtable[item].accnum++;
					hashtable[item].dis = lru.stateACC() - hashtable[item].lastacc;
					hashtable[item].weidis = olddis * hashtable[item].lastdis + newdis * hashtable[item].dis;
					hashtable[item].lastdis = hashtable[item].weidis;
					hashtable[item].lastacc = lru.stateACC();
				}
				if (item == "333") {
					std::ostringstream buffer;
					buffer << newdis;
					string str = buffer.str();
					oFile.open("trace_"+ str +"_333.csv", ios::out | ios::app);
					oFile << hashtable[item].weidis<<","<< hashtable[item].dis << endl;
					oFile.close();
				}
				Entry entry;
				entry.item = item;
				entry.size = hashtable[item].size;


				if (lru.access(entry)) {
					oFileHit << lru.stateACC() <<","<<entry.item << endl;
				}
				else{
					oFileMiss << lru.stateACC() << "," << entry.item << endl;
				}


				recordcounter++;
				if (recordcounter == RECORDE_STEP)
				{
					oFile.open(outputfile, ios::out | ios::app);
					oFile << streamNO++ << ","
						<< ((float)(lru.stateHIT() - LRUlast.lastStateHit)) / ((lru.stateACC() - LRUlast.lastStateAcc)) * 100 
						<< endl;
					oFile.close();
					LRUlast.lastStateAcc = lru.stateACC();
					LRUlast.lastStateHit = lru.stateHIT();
					LRUlast.lastStateMis = lru.stateFault();
					recordcounter = 0;
				}

				if (slice.at(index).empty())
				{
					sliceCounter++;
					finishedCounter++;
					thenext--;
					if (thenext == 0) {//该采样了
						samplingTrans.push_back(Transaction(tempslice.at(index)));
						thenext = sampTheNext(a, rate, sampCounter, M);
						sampCounter++;
					}
					if (thenext == 0) {
						thenext = 1;
					}
					//保持一致性
					slice.erase(slice.begin() + index);
					//slice.shrink_to_fit();
					tempslice.erase(tempslice.begin() + index);
					//添加一个事务进入slice
					if (temptrans.size() > 0) {
						std::vector<Transaction>::iterator beginIt = temptrans.begin();
						int idx = radmGen(0, temptrans.size() - 1, 1);
						slice.push_back(temptrans.at(idx));
						tempslice.push_back(temptrans.at(idx));
						std::vector<Transaction>::iterator it = beginIt + idx;
						temptrans.erase(it);
					}
					if (sliceCounter == M || (slice.size() == 0)) {
						sliceCounter = 0;
						sampCounter = 0;
						thenext = 1;
						/***********************************************************/
						DWORD StartTime = ::GetTickCount();
						fpcahe.setMinSupport(ceil(fpcahe.getMinSupportWet()*(M*rate + samplingTrans.size()) / 2));
						fpcahe.runFPAnalyse(samplingTrans, patterns);
						fpcahe.valuatePatterns(patterns, hashtable, valuatedpatterns, valuatedpatterns2);

						if (spyTran.first.size() == 0)
						{
							int idx = radmGen(0, valuatedpatterns.size() - 1, 1);
							double min = 0.0;
							auto its = valuatedpatterns.begin();
							auto temp = its;
							min = its->second.val;
							while (its != valuatedpatterns.end()) {
								if (min> its->second.val)
								{
									min = its->second.val;
									temp = its;
								}
								its++;
							}
							spyTran=(*(temp));
						}
						else 
						{
							oFile.open("spy_Spatial_" + outputfile, ios::out | ios::app);
							oFile2.open("spy_Temporal_" + outputfile, ios::out | ios::app);
							auto beg = spyTran.first.begin();
							vector<metadata> randmselect;
							for (beg; beg!= spyTran.first.end(); beg++)
							{
								randmselect.push_back(hashtable[(*beg)]);
							}

							double meanSpatial = getSpatialMean(randmselect);
							double varSpatial = getSpatialVan(randmselect, meanSpatial);

							double meanTemporal = getTemporalMean(randmselect);
							double varTemporal = getTemporalVan(randmselect, meanTemporal);
							oFile << (uint64_t)meanSpatial << "," << (varSpatial / meanSpatial) * 100 << endl;
							oFile2 << (uint64_t)meanTemporal << "," << (varTemporal / meanTemporal) * 100 << endl;
							oFile.close();
							oFile2.close();
						}
						if (spyNotTran.size() == 0)
						{
							for (size_t i = 0; i < spyTran.first.size(); i++)
							{
								int idx = radmGen(0, hashtable.size() - 1, 1);
								auto its = hashtable.begin();
								for (size_t j = 0; j < idx; j++)
								{
									its++;
								}
								spyNotTran.push_back((*its).first);
							}
						}
						else
						{
							oFile.open("spyNot_Spatial_" + outputfile, ios::out | ios::app);
							oFile2.open("spyNot_Temporal_" + outputfile, ios::out | ios::app);
							auto beg = spyNotTran.begin();
							vector<metadata> randmselect;
							for (beg; beg != spyNotTran.end(); beg++)
							{
								randmselect.push_back(hashtable[(*beg)]);
							}

							double meanSpatial = getSpatialMean(randmselect);
							double varSpatial = getSpatialVan(randmselect, meanSpatial);

							double meanTemporal = getTemporalMean(randmselect);
							double varTemporal = getTemporalVan(randmselect, meanTemporal);
							oFile << (uint64_t)meanSpatial << "," << (varSpatial / meanSpatial) * 100 << endl;
							oFile2 << (uint64_t)meanTemporal << "," << (varTemporal / meanTemporal) * 100 << endl;
							oFile.close();
							oFile2.close();
						}

						auto ite = valuatedpatterns.begin();
						oFile.open("patens_Spatial_"+ outputfile, ios::out | ios::app);
						oFile2.open("frame"+ std::to_string(frameNum) +"_x", ios::out | ios::app);
						while (ite != valuatedpatterns.end())
						{
							oFile << (*ite).second.Spatial_mean << "," << ((double)(*ite).second.Spatial_var / (*ite).second.Spatial_mean) * 100 << "," << (*ite).second.sup <<","<<(*ite).second.val<< endl;
							oFile2 << ((double)(*ite).second.Spatial_var / (*ite).second.Spatial_mean) * 100 << endl;
							ite++;
						}
						oFile2.close();
						oFile.close();

						ite = valuatedpatterns.begin();
						oFile.open("patens_Temporal_"  + outputfile, ios::out | ios::app);
						oFile2.open("frame" + std::to_string(frameNum) + "_y", ios::out | ios::app);
						while (ite != valuatedpatterns.end())
						{
							oFile << (*ite).second.Temporal_mean << "," << ((double)(*ite).second.Temporal_var / (*ite).second.Temporal_mean) * 100 << "," << (*ite).second.accnum << endl;
							oFile2 << ((double)(*ite).second.Temporal_var / (*ite).second.Temporal_mean) * 100 << endl;
							ite++;
						}
						oFile2.close();
						oFile.close();
						frameNum++;
						/********************************************************************/
						ite = valuatedpatterns.begin();
						if (randmTrans.size() == 0)
						{
							while (ite != valuatedpatterns.end())
							{
								uint64_t accnum = 0;
								Transaction randmselect;
								for (size_t i = 0; i < (*ite).first.size(); i++)
								{
									int idx = radmGen(0, hashtable.size() - 1, 1);
									auto its = hashtable.begin();
									for (its; its != hashtable.end() && idx; its++, idx--);
									randmselect.push_back((*its).first);
								}
								randmTrans.push_back(randmselect);
								ite++;
							}
						}
						if (randmTrans.size() > 0)
						{
							oFile.open("notpatens_Spatial_" + outputfile, ios::out | ios::app);
							oFile2.open("notpatens_Temporal_" + outputfile, ios::out | ios::app);
							auto ite = randmTrans.begin();
							while (ite != randmTrans.end())
							{
								uint64_t accnum = 0;
								vector<metadata> randmselect;
								for (size_t index = 0; index < (*ite).size(); index++)
								{
									accnum += hashtable[(*ite).at(index)].accnum;
									randmselect.push_back(hashtable[(*ite).at(index)]);
								}
								ite++;

								double meanSpatial = getSpatialMean(randmselect);
								double varSpatial = getSpatialVan(randmselect, meanSpatial);

								double meanTemporal = getTemporalMean(randmselect);
								double varTemporal = getTemporalVan(randmselect, meanTemporal);


								oFile << (uint64_t)meanSpatial << "," << (varSpatial / meanSpatial) * 100 << "," << accnum << endl;
								oFile2 << (uint64_t)meanTemporal << "," << (varTemporal / meanTemporal) * 100 << "," << accnum << endl;

							}
							oFile.close();
							oFile2.close();
						}
						/**********************************************************************************/
						switch (_type)
						{
						case 1:
							fpcahe.sortPatternsBySup(sortedPatterns, patterns);
							break;
						case 2:
							fpcahe.sortPatternsByDensity(sortedPatterns, hashtable, valuatedpatterns);
							break;
						case 3:
							fpcahe.sortPatternsByVal(sortedPatterns, hashtable, valuatedpatterns);
							break; 
						default:
							fpcahe.sortPatternsBySup(sortedPatterns, patterns);
							break;
						}
						//resetMetaAccnum(hashtable, olddis, newdis);
						DWORD EndTime = ::GetTickCount();
						cout << streamNO << ":" << recordcounter << "	*耗时" << EndTime - StartTime << "ms" << "	number of filted Patterns " << sortedPatterns.size() << endl;
						/***********************************************************/

						cout << "LRU:	ACC:" << lru.stateACC()
							<< " HIT:" << lru.stateHIT()
							<< " FAULT:" << lru.stateFault()
							<< "	hit ratio:" << ((float)(lru.stateHIT())) / ((lru.stateACC())) * 100 << "%"<< endl;
						cout << endl;
						samplingTrans.clear();
						continue;
					}
				}
			}
		}
	}
	oFileHit.close();
	oFileMiss.close();
}

int sampTheNext(float a, float rate, size_t curr ,size_t M) {
	size_t samplenum = M * rate;
	double xstep = 1.0 / samplenum;
	double xcurrmap = (double)curr / samplenum;
	double xnextmap = (double)(curr+1) / samplenum;

	double ycurrmap = (1.0 - exp(a * log2(1.0 - (xcurrmap))));
	double ynextmap = (1.0 - exp(a * log2(1.0-(xnextmap))));

	int ycurr = M * (1.0 - exp(a * log2(1.0 - (xcurrmap))));
	int ynext = M * (1.0 - exp(a * log2(1.0-(xnextmap))));

	if ((1.0 - exp(a*log2(1.0 - (xnextmap))))>1){
		return M - ycurr;
	}
	return ynext - ycurr;
}
void skewWorkLoad(std::vector<Transaction>& skew_transactions,size_t skew_low, size_t skew_high) {
	for (size_t i = skew_low; i <= skew_high; i++)
	{
		Transaction::iterator it = skew_transactions[i].begin();
		while (it!= skew_transactions[i].end())
		{
			(*it) = "99999" + (*it);
			it++;
		}
	}
}