#include <cstdlib>
#include<random>  
#include<time.h>  
#include<iostream>  
#include "util.h"
#include <math.h>
#include"fpcache.hpp"
#define WINDOW 10

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

void uniAccess(LRUStack& lru, 
	FPCache&fpcahe, 
	std::vector<Transaction>&transactions, 
	std::vector<Transaction>&temptrans,
	size_t M,
	float rate) {
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
	while (temptrans.size()> WINDOW) {
		for (size_t index = 0; index < WINDOW; index++)
		{
			slice.push_back(temptrans.at(index));
			temptrans.erase(temptrans.begin() + index);
			//temptrans.at(index).shrink_to_fit();
		}
		tempslice = slice;//保存信息，在访问过程中slice的数据会消失
		//我们得到了一个含有一组事务的容器slice，下面随机取出这个容器中的事务项
		while (!slice.empty())
		{
			

			int index = radmGen(0, slice.size() - 1, 1);//选取容器中的一个事务
			if (slice.at(index).size() > 0) {			//如果事务中项的数量不为空

				int itmIndex = radmGen(0, slice.at(index).size() - 1, 1);//选取事务中的一个项

				lru.access(slice.at(index).at(itmIndex));//模拟访问
				fpcahe.access(slice.at(index).at(itmIndex));//模拟访问

				slice.at(index).erase(slice.at(index).begin() + itmIndex);//将此项从该事务中删除
				//slice.at(index).shrink_to_fit();

				if (slice.at(index).empty())
				{
					sliceCounter++;

					finishedCounter++;
					thenext--;
					if (thenext == 0) {//该采样了
						samplingTrans.push_back(Transaction(tempslice.at(index)));
						thenext = sampTheNext(2, rate, sampCounter, M);
						sampCounter++;
					}
					if (thenext == 0) {
						thenext = 1;
					}
					//保持一致性
					slice.erase(slice.begin() + index);
					//slice.shrink_to_fit();
					tempslice.erase(tempslice.begin() + index);
					//tempslice.shrink_to_fit();

					if (sampCounter == M * rate)//采样完了
					{
						sampCounter = 0;
						thenext = 1;
						/*************/
						
						fpcahe.runFPAnalyse(samplingTrans, patterns);
						fpcahe.sortPatternsBySup(sortedPatterns, patterns);
						fpcahe.procPattern(sortedPatterns,fpcahe.getHighCorrCache().getShadowCache(),fpcahe.getLowCorrCache().getShadowCache());
						fpcahe.cacheOrganize();

						/***********************************************************/
						cout << "ACC_NUM:" << fpcahe.stateACC()
							<< " HIT_NUM:" << fpcahe.stateHIT()
							<< " PAGE_FAULT_NUM:" << fpcahe.stateFault()
							<< "	hit ratio:" << ((float)fpcahe.stateHIT() / fpcahe.stateACC()) * 100 << "%" 
							<<" sample number:"<<samplingTrans.size() << endl;
						samplingTrans.clear();
						/**********************************************************/
						/*************/
						continue;
					}
					if (sliceCounter == M) {
						sliceCounter = 0;
						sampCounter = 0;
						thenext = 1;
						/*************/
						//std::set<Pattern> patterns;
						//std::vector<Pattern> sortedPatterns;
						fpcahe.runFPAnalyse(samplingTrans, patterns);
						fpcahe.sortPatternsBySup(sortedPatterns, patterns);
						fpcahe.procPattern(sortedPatterns, fpcahe.getHighCorrCache().getShadowCache(), fpcahe.getLowCorrCache().getShadowCache());
						fpcahe.cacheOrganize();

						/***********************************************************/
						cout << "FPC:	ACC_NUM:" << fpcahe.stateACC()
							<< " HIT_NUM:" << fpcahe.stateHIT()
							<< " PAGE_FAULT_NUM:" << fpcahe.stateFault()
							<< "	hit ratio:" << ((float)fpcahe.stateHIT() / fpcahe.stateACC()) * 100 << "%" 
							<< " sample number:" << samplingTrans.size() <<"............" <<((double)finishedCounter/totalsize)*100<<"%"<<endl;
						cout<< "LRU:	ACC_NUM:" << lru.stateACC()
							<< " HIT_NUM:" << lru.stateHIT()
							<< " PAGE_FAULT_NUM:" << lru.stateFault()
							<< "	hit ratio:" << ((float)lru.stateHIT() / lru.stateACC()) * 100 << "%" << endl;
						samplingTrans.clear();
						/**********************************************************/
						/*************/
						continue;
					}
				}
			};
		}
	}

	//if (counter >= fpCache.getMaxLogSize())
	//{
	//	此括号内为计算和调整fpcache 的代码块
		//vector<Pattern> sortedPatterns;
		//fpCache.runFPAnalyse(fpCache.getLog(), patterns);
		//fpCache.sortPatternsBySup(sortedPatterns, patterns);
		//fpCache.procPattern(sortedPatterns,
		//	fpCache.getHighCorrCache().getShadowCache(),
		//	fpCache.getLowCorrCache().getShadowCache());
		//fpCache.cacheOrganize();

		///***********************************************************/
		//cout << "ACC_NUM:" << fpCache.stateACC()
		//	<< " HIT_NUM:" << fpCache.stateHIT()
		//	<< " PAGE_FAULT_NUM:" << fpCache.stateFault()
		//	<< "	hit ratio:" << ((float)fpCache.stateHIT() / fpCache.stateACC()) * 100 << "%" << endl;
		///**********************************************************/
		//counter = 0;
		//blankCounter = 0;
	//}

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
