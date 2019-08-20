#include <cstdlib>
#include<random>  
#include<time.h>  
#include<iostream>  
#include "util.h"
#include <math.h>
#include"fpcache.hpp"
#include <stdlib.h>
#include <fstream>
#include <windows.h>
#define WINDOW 256

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
int radmGen(size_t low, size_t high, int) {

	static std::default_random_engine e;
	std::uniform_int_distribution<unsigned> u(low, high);
	/*
	std::default_random_engine random(time(NULL));
	std::uniform_int_distribution<int> dis1(low, high);
	*/
	return u(e);
}
double radmGen(size_t low, size_t high, double) {
	std::default_random_engine random(time(NULL));
	std::uniform_real_distribution<double> dis2(0.0, 1.0);
	return dis2(random);
}
struct lastState
{
	uint64_t lastStateAcc = 0;
	uint64_t lastStateHit = 0;
	uint64_t lastStateMis = 0;
};
void uniAccess(LRUStack& lru, FPCache&fpcahe, ARCCache&accache,
	std::vector<Transaction>&transactions,
	std::vector<Transaction>&temptrans,
	size_t M,
	float rate,
	float a,
	string outputfile)
{
	temptrans.clear();
	temptrans = transactions;
	std::vector<Transaction> samplingTrans;
	std::vector<Transaction> slice;
	std::vector<Transaction> tempslice;
	int finishedCounter = 0;
	int totalsize = transactions.size();
	int sliceCounter = 0;
	int sampCounter = 0;
	int thenext = 1;
	std::set<Pattern> patterns;
	std::vector<Pattern> sortedPatterns;

	lastState FPClast;
	lastState LRUlast;
	lastState ARClast;


	int streamNO = 1;
	ofstream oFile;

	int recordcounter = 0;
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
			int index = radmGen(0, slice.size() - 1, 1);//选取容器中的一个事务
			if (slice.at(index).size() > 0) {			//如果事务中项的数量不为空

				int itmIndex = radmGen(0, slice.at(index).size() - 1, 1);//选取事务中的一个项

				lru.access(slice.at(index).at(itmIndex));//模拟访问
				fpcahe.access(slice.at(index).at(itmIndex));//模拟访问
				accache.ARCreference(stoi(slice.at(index).at(itmIndex)));

				recordcounter++;
				if (recordcounter == 10000)
				{
					oFile.open(outputfile, ios::out | ios::app);
					oFile << streamNO++ << ","
						<< ((float)(fpcahe.stateHIT() - FPClast.lastStateHit)) / (fpcahe.stateACC() - FPClast.lastStateAcc) * 100 << ","
						<< ((float)(lru.stateHIT() - LRUlast.lastStateHit)) / ((lru.stateACC() - LRUlast.lastStateAcc)) * 100 << ","
						<< ((float)(accache.getHit() - ARClast.lastStateHit)) / ((accache.getAcc() - ARClast.lastStateAcc)) * 100 << endl;
					oFile.close();
					FPClast.lastStateAcc = fpcahe.stateACC();
					FPClast.lastStateHit = fpcahe.stateHIT();
					FPClast.lastStateMis = fpcahe.stateFault();
					LRUlast.lastStateAcc = lru.stateACC();
					LRUlast.lastStateHit = lru.stateHIT();
					LRUlast.lastStateMis = lru.stateFault();
					ARClast.lastStateAcc = accache.getAcc();
					ARClast.lastStateHit = accache.getHit();
					ARClast.lastStateMis = accache.getMis();
					recordcounter = 0;
				}


				slice.at(index).erase(slice.at(index).begin() + itmIndex);//将此项从该事务中删除
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
				//	if (sampCounter == M * rate || (slice.size() == 0)) {
						sliceCounter = 0;
						sampCounter = 0;
						thenext = 1;
						/***********************************************************/
						DWORD StartTime = ::GetTickCount();
						fpcahe.setMinSupport(ceil(fpcahe.getMinSupportWet()*(M*rate + samplingTrans.size()) / 2));
						fpcahe.runFPAnalyse(samplingTrans, patterns);
						//fpcahe.sortPatternsBySup(sortedPatterns, patterns);
						fpcahe.procPattern(sortedPatterns, fpcahe.getHighCorrCache().getShadowCache());
						fpcahe.cacheOrganize();
						DWORD EndTime = ::GetTickCount();
						cout << "	*耗时" << EndTime - StartTime << "ms" << endl;
						/***********************************************************/

						cout << "FPC:	ACC:" << fpcahe.stateACC()
							<< " HIT:" << fpcahe.stateHIT()
							<< " FAULT:" << fpcahe.stateFault()
							<< "	hit ratio:" << ((float)(fpcahe.stateHIT())) / (fpcahe.stateACC()) * 100 << "%	"
							<< fpcahe.getHighCorrCache().getCacheSize() << "/" << fpcahe.getHighCorrCache().getMaxSize()
							<< "	sample num:" << samplingTrans.size()
							<< "	【" << ((double)finishedCounter / totalsize) * 100 << "%】" << endl;
						cout << "LRU:	ACC:" << lru.stateACC()
							<< " HIT:" << lru.stateHIT()
							<< " FAULT:" << lru.stateFault()
							<< "	hit ratio:" << ((float)(lru.stateHIT())) / ((lru.stateACC())) * 100 << "%" << endl;
						cout << "ARC:	ACC:" << accache.getAcc()
							<< " HIT:" << accache.getHit()
							<< " FAULT:" << accache.getMis()
							<< "	hit ratio:" << ((float)(accache.getHit()) / (accache.getAcc())) * 100 << "%" << endl;
						cout << endl;

						/*oFile.open(outputfile, ios::out | ios::app);
						oFile << streamNO++ << ","
							<< ((float)(fpcahe.stateHIT() - FPClast.lastStateHit)) / (fpcahe.stateACC() - FPClast.lastStateAcc) * 100 << ","
							<< ((float)(lru.stateHIT() - LRUlast.lastStateHit)) / ((lru.stateACC() - LRUlast.lastStateAcc)) * 100 << ","
							<< ((float)(accache.getHit() - ARClast.lastStateHit)) / ((accache.getAcc() - ARClast.lastStateAcc)) * 100 << endl;
						oFile.close();
						FPClast.lastStateAcc = fpcahe.stateACC();
						FPClast.lastStateHit = fpcahe.stateHIT();
						FPClast.lastStateMis = fpcahe.stateFault();
						LRUlast.lastStateAcc = lru.stateACC();
						LRUlast.lastStateHit = lru.stateHIT();
						LRUlast.lastStateMis = lru.stateFault();
						ARClast.lastStateAcc = accache.getAcc();
						ARClast.lastStateHit = accache.getHit();
						ARClast.lastStateMis = accache.getMis();*/
						samplingTrans.clear();
						continue;
					}
				}
			}
		}
	}
}

void uniAccess(LRUStack& lru, FPCache&fpcahe, ARCCache&accache,
	std::vector<Transaction>&transactions,
	std::vector<Transaction>&w_transactions,
	std::vector<Transaction>&temptrans,
	size_t M,
	float rate,
	float a,
	size_t skew_jump_low,
	size_t skew_jump_high,
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
	std::vector<Pattern> sortedPatterns;

	lastState FPClast;
	lastState LRUlast;
	lastState ARClast;


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
			int index = radmGen(0, slice.size() - 1, 1);//选取容器中的一个事务
			if (slice.at(index).size() > 0) {			//如果事务中项的数量不为空

				int itmIndex = radmGen(0, slice.at(index).size() - 1, 1);//选取事务中的一个项
				
				
				if ((skew_jump_low <= finishedCounter) && (skew_jump_high >= finishedCounter))
				{
					lru.access(slice.at(index).at(itmIndex));//模拟访问
					fpcahe.access(slice.at(index).at(itmIndex));//模拟访问
					accache.ARCreference(stoi(slice.at(index).at(itmIndex)));
					//hashtable[slice.at(index).at(itmIndex)].dis = 0;
					
				}
				else {
					lru.access(slice.at(index).at(itmIndex));//模拟访问
					fpcahe.access(slice.at(index).at(itmIndex));//模拟访问
					accache.ARCreference(stoi(slice.at(index).at(itmIndex)));					
				}

				if (hashtable.find(slice.at(index).at(itmIndex)) == hashtable.end())
				{
					metadata m;
					Item item=slice.at(index).at(itmIndex);
					//srand(atoi(item.c_str()));
					m.size = radmGen(1024,1024*50,1);
					m.accnum = 1;
					m.lastacc = 0;
					m.lastdis = lru.stateACC() - m.lastacc;
					m.dis = lru.stateACC() - m.lastacc;
					m.weidis = 0.5*m.lastdis + 0.5*m.dis;
					m.lastdis = m.weidis;
					hashtable[slice.at(index).at(itmIndex)] = m;
				}
				else
				{
					hashtable[slice.at(index).at(itmIndex)].accnum++;
					hashtable[slice.at(index).at(itmIndex)].dis = lru.stateACC() - hashtable[slice.at(index).at(itmIndex)].lastacc;
					hashtable[slice.at(index).at(itmIndex)].weidis = 0.8*hashtable[slice.at(index).at(itmIndex)].lastdis + 0.2*hashtable[slice.at(index).at(itmIndex)].dis;
					hashtable[slice.at(index).at(itmIndex)].lastdis = hashtable[slice.at(index).at(itmIndex)].weidis;
					hashtable[slice.at(index).at(itmIndex)].lastacc = lru.stateACC();
				}

				recordcounter++;
				if (recordcounter==20000)
				{
					oFile.open(outputfile, ios::out | ios::app);
					oFile << streamNO++ << ","
						<< ((float)(fpcahe.stateHIT() - FPClast.lastStateHit)) / (fpcahe.stateACC() - FPClast.lastStateAcc) * 100 << ","
						<< ((float)(lru.stateHIT() - LRUlast.lastStateHit)) / ((lru.stateACC() - LRUlast.lastStateAcc)) * 100 << ","
						<< ((float)(accache.getHit() - ARClast.lastStateHit)) / ((accache.getAcc() - ARClast.lastStateAcc)) * 100 << endl;
					oFile.close();
					FPClast.lastStateAcc = fpcahe.stateACC();
					FPClast.lastStateHit = fpcahe.stateHIT();
					FPClast.lastStateMis = fpcahe.stateFault();
					LRUlast.lastStateAcc = lru.stateACC();
					LRUlast.lastStateHit = lru.stateHIT();
					LRUlast.lastStateMis = lru.stateFault();
					ARClast.lastStateAcc = accache.getAcc();
					ARClast.lastStateHit = accache.getHit();
					ARClast.lastStateMis = accache.getMis();
					recordcounter = 0;
				}


				slice.at(index).erase(slice.at(index).begin() + itmIndex);//将此项从该事务中删除
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
					//slice.shrink_to_fit();
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
						case 1:
							fpcahe.sortPatternsBySup(sortedPatterns, patterns);
							break;
						case 2:
							fpcahe.valuatePatterns(patterns, hashtable, valuatedpatterns);
							fpcahe.sortPatternsByVal(sortedPatterns, valuatedpatterns);
							break;
						default:
							fpcahe.sortPatternsBySup(sortedPatterns, patterns);
							break;
						}
						fpcahe.procPattern(sortedPatterns, fpcahe.getHighCorrCache().getShadowCache());
						fpcahe.cacheOrganize();
						DWORD EndTime = ::GetTickCount();
						cout <<streamNO <<":"<<recordcounter<< "	*耗时" << EndTime - StartTime << "ms" <<"	number of filted Patterns: "<< sortedPatterns.size() << endl;
						/***********************************************************/

						cout << "FPC:	ACC:" << fpcahe.stateACC()
							<< " HIT:" << fpcahe.stateHIT()
							<< " FAULT:" << fpcahe.stateFault()
							<< "	hit ratio:" << ((float)(fpcahe.stateHIT())) / (fpcahe.stateACC()) * 100 << "%	"
							<< fpcahe.getHighCorrCache().getCacheSize() << "/" << fpcahe.getHighCorrCache().getMaxSize()
							<< "	sample num:" << samplingTrans.size()
							<< "	【" << ((double)finishedCounter / totalsize) * 100 << "%】" << endl;
						cout << "LRU:	ACC:" << lru.stateACC()
							<< " HIT:" << lru.stateHIT()
							<< " FAULT:" << lru.stateFault()
							<< "	hit ratio:" << ((float)(lru.stateHIT())) / ((lru.stateACC())) * 100 << "%" << endl;
						cout << "ARC:	ACC:" << accache.getAcc()
							<< " HIT:" << accache.getHit()
							<< " FAULT:" << accache.getMis()
							<< "	hit ratio:" << ((float)(accache.getHit()) / (accache.getAcc())) * 100 << "%" << endl;
						cout << endl;

						/*oFile.open(outputfile, ios::out | ios::app);
						oFile << streamNO++ << ","
							<< ((float)(fpcahe.stateHIT() - FPClast.lastStateHit)) / (fpcahe.stateACC() - FPClast.lastStateAcc) * 100 << ","
							<< ((float)(lru.stateHIT() - LRUlast.lastStateHit)) / ((lru.stateACC() - LRUlast.lastStateAcc)) * 100 << ","
							<< ((float)(accache.getHit() - ARClast.lastStateHit)) / ((accache.getAcc() - ARClast.lastStateAcc)) * 100 << endl;
						oFile.close();
						FPClast.lastStateAcc = fpcahe.stateACC();
						FPClast.lastStateHit = fpcahe.stateHIT();
						FPClast.lastStateMis = fpcahe.stateFault();
						LRUlast.lastStateAcc = lru.stateACC();
						LRUlast.lastStateHit = lru.stateHIT();
						LRUlast.lastStateMis = lru.stateFault();
						ARClast.lastStateAcc = accache.getAcc();
						ARClast.lastStateHit = accache.getHit();
						ARClast.lastStateMis = accache.getMis();*/
						samplingTrans.clear();
						continue;
					}					
				}
			}
		}
	}
}

int sampTheNext(float a, float rate, size_t curr ,size_t M) {// M=m/r 
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
			/*
			char buffer[10]{0};
			itoa(atoi((*it).c_str())+20000, buffer,10);
			(*it) = buffer;
			*/
			(*it) = "10000" + (*it);
			it++;
		}
		
	}
	

}