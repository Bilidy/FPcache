// tools.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <iostream>  




#include "transactions.h"
#include <set>

#include "util.h"

using Item = std::string;
using Transaction = std::vector<Item>;

#define ARGS_BUFFER_SIZE 101
#define COMM_BUFFER_SIZE 11
#define COMM_ARGS_BUFFER_SIZE 91
#define DEFAULT_SUPPORT 0.02;



void get_args(char* _comm_args_buffer, std::set<pair<string, string>>& args) {
	size_t i = 0;
	std::set<size_t> posOfArgs;
	string argstr;
	argstr = (string)_comm_args_buffer;
	while (_comm_args_buffer[i])
	{
		if ('-' == _comm_args_buffer[i] && _comm_args_buffer[i + 1])
		{
			posOfArgs.insert(i);
		}
		i++;
	}

	auto it = posOfArgs.begin();
	std::set<string> argstrs;
	size_t proPos = 0;
	while (it != posOfArgs.end())
	{

		string arg = argstr;
		arg = arg.substr(0, (*it) - proPos);
		if (arg != "")
		{
			argstrs.insert(arg);
		}
		argstr = argstr.substr((*it) - proPos, argstr.size());
		proPos = *it;
		it++;
		//args.insert(_comm_args_buffer[(*it)], _comm_args_buffer[(*it)+1]);
	}
	if (argstr != "")
	{
		argstrs.insert(argstr);
	}

	auto ite = argstrs.begin();
	while (ite != argstrs.end())
	{
		string argStr;
		string praStr;
		size_t splitPos = (*ite).find_first_of(' ');
		size_t endPos = (*ite).find_last_not_of(' ');
		if (endPos > splitPos)
		{
			argStr = (*ite).substr(0, splitPos);
			praStr = (*ite).substr(splitPos + 1, endPos - splitPos);
			pair<string, string> pair(argStr, praStr);
			args.insert(pair);
		}
		else
		{
			argStr = (*ite).substr(0, splitPos);
			praStr = "";
			pair<string, string> pair(argStr, praStr);
			args.insert(pair);
		}
		ite++;
	}
}
//string to real number
int stor(string numstr, size_t& num) {
	if (numstr.size() > 0)
	{
		auto it = numstr.end();
		int weight = 0;
		num = 0;
		while (it != numstr.begin())
		{
			it--;
			if ('0' <= (*it) && (*it) <= '9') {
				num += ((*it) - 48) * (pow(10, weight));
				weight++;
			}
			else
			{
				return 0;
			}
		}
	}
	return 1;
}
int get_command(char* _args_buffer, uint32_t _arg_buffersize, char* _comm_buffer, uint32_t _comm_buffersize, char* _comm_args_buffer, uint32_t _comm_args_buffersize)
{

	if (_arg_buffersize <= 0 || _comm_buffersize <= 0) {
		return 0;
	}
	if ((_args_buffer == NULL) || (_comm_buffer == NULL))
	{
		return 0;
	}
	size_t commindex = 0;
	for (commindex = 0; (commindex < _arg_buffersize )&&( _args_buffer[commindex] != 0); commindex++)
	{
		if (_args_buffer[commindex] == ' ') {
			break;
		}
	}
	if (0 == commindex) {
		return 0;
	}
	if (commindex > COMM_BUFFER_SIZE - 1)
	{
		printf("out of command buffer\n");
		return 0;
	}
	else
	{
		memcpy(_comm_buffer, _args_buffer, commindex);
	}
	size_t count=0,comm_args_index = 0;
	for (comm_args_index = commindex+1; (comm_args_index < _arg_buffersize) && (_args_buffer[comm_args_index] != 0); comm_args_index++){
		count++;
	}
	if (count == 0) {
	}
	else if (count > COMM_ARGS_BUFFER_SIZE - 1)
	{
		printf("out of command args buffer\n");
		return 0;
	}
	else
	{
		memcpy(_comm_args_buffer, _args_buffer + commindex + 1, count);
	}
	return 1;
}

double supWet = DEFAULT_SUPPORT;
string filepath;

std::vector<Transaction> _transactions;
std::vector<Transaction> _temptransactions;

size_t room;
size_t highSizeWeight = 0.2;
size_t lowSizeWeight = 0.2;
size_t lruSizeWeight = 0.6;
size_t logSize = 1000;
size_t samplenum = 1000;
size_t blankSize = 1000;
double samplingRate = 0.1;
double alpha = 0.7;

bool needRebuild = true;

void drive_machine() {
	bool stop = false;
	char args_buffer[ARGS_BUFFER_SIZE];
	char comm_buffer[COMM_BUFFER_SIZE];
	char comm_args_buffer[COMM_ARGS_BUFFER_SIZE];

	while (!stop)
	{
		printf(">");
		memset(args_buffer, 0, ARGS_BUFFER_SIZE);
		memset(comm_buffer, 0, COMM_BUFFER_SIZE);
		memset(comm_args_buffer, 0, COMM_ARGS_BUFFER_SIZE);
		std::cin.getline(args_buffer, ARGS_BUFFER_SIZE );

		if (!get_command(args_buffer, ARGS_BUFFER_SIZE, comm_buffer, COMM_BUFFER_SIZE, comm_args_buffer, COMM_ARGS_BUFFER_SIZE)) {
			printf("bad args\n");
			continue;
		};

		if (0 == strcmp(comm_buffer, "trans"))
		{
			printf("trans... args:%s\n", comm_args_buffer);
			transactions(comm_args_buffer);
			printf("trans... over\n");
		}
		if (0 == strcmp(comm_buffer, "fpcache")) {

			printf("fpcache> args:%s\n", comm_args_buffer);
			std::set<pair<string, string>> args;
			get_args(comm_args_buffer, args);

			auto it = args.begin();
			while (it!= args.end())
			{
				//-s 最小支持度参数，获取最小支持度。系统默认为2。
				if ("-s" == (*it).first) {//support
					//cout << "support:"<<(*it).second << endl;
					if (supWet=atof((*it).second.c_str())) {
						cout << "fpcache> support weight:" << supWet << endl;
						break;
					}
					else
					{
						cout << "fpcache> please check the parameter:" << (*it).first << endl;
						break;
					}
					//cout << "fpcache> default support:" << sup << endl;
				}
				//获取文件的路径filepath，文件里是transactions,若和前次文件路径相同则不做重建工作
				else if ("-p" == (*it).first) {
					
					if (filepath == (*it).second)
					{
						needRebuild = false;
					}
					else
					{
						filepath = (*it).second;
						needRebuild = true;
					}
					cout <<"fpcache> file path:"<< (*it).second << endl;
					//transactions((*it).second,_transactions);
				}
				//指定缓存方式的缓存大小。
				else if ("-r" == (*it).first) {
									
					if (stor((*it).second, room)) {
						cout << "fpcache> room:" << room << endl;
					}
					else
					{
						cout << "fpcache> please check the parameter:"<< (*it).first << endl;
					}

				}
				else if ("-H" == (*it).first) {
					if (stor((*it).second, highSizeWeight)) {
						cout << "fpcache> high weight:" << highSizeWeight << endl;
					}
					else
					{
						cout << "fpcache> please check the parameter:" << (*it).first << endl;
					}
				}
				else if ("-L" == (*it).first) {
					if (stor((*it).second, lowSizeWeight)) {
						cout << "fpcache> low weight:" << lowSizeWeight << endl;
					}
					else
					{
						cout << "fpcache> please check the parameter:" << (*it).first << endl;
					}
				}
				else if ("-U" == (*it).first) {
					if (stor((*it).second, lruSizeWeight)) {
						cout << "fpcache> LRU weight:" << lruSizeWeight << endl;
					} 
					else
					{
						cout << "fpcache> please check the parameter:" << (*it).first << endl;
					}
				}
				else if ("-m" == (*it).first) {
					if (stor((*it).second, samplenum)) {
						cout << "fpcache> sample size:" << samplenum << endl;
					}
					else
					{
						cout << "fpcache> please check the parameter:" << (*it).first << endl;
					}
				}
				else if (("-R" == (*it).first)) {
					if (samplingRate=atof((*it).second.c_str())) {
						cout << "fpcache> sampling rate:" << samplingRate << endl;
					}
					else
					{
						cout << "fpcache> please check the parameter:" << (*it).first << endl;
					}
				}
				else if (("-a" == (*it).first)) {
					if (alpha = atof((*it).second.c_str())) {
						cout << "fpcache> alpha:" << alpha << endl;
					}
					else
					{
						cout << "fpcache> please check the parameter:" << (*it).first << endl;
					}
				}
				it++;
			}
/******************************************************************************************************************************/
/******************************************************************************************************************************/
			/*
				fpcache -r 50 -p transactions.txt -s 15 -H 3 -L 1 -U 6 -m 1000 -R 0.1
				fpcache -r 1000 -p kosarak.dat -s 15 -H 5 -L 0 -U 5 -m 1000 -R 0.1
				fpcache -p kosarak.dat -H 3 -L 0 -U 7 -m 1000 -R 0.1 -r 1000 -s 0.01 -a 1.6
				fpcache -p kosarak.dat -H 5 -L 0 -U 5 -m 1000 -R 0.1 -r 500 -s 0.008 -a 1.3
				fpcache -r 800 -p kosarak.dat -s 0.02 -H 3 -L 0 -U 7 -m 1000 -R 0.1 -a 2
				fpcache -r 200 -p T40I10D100K.dat -s 21 -H 2 -L 0 -U 8 -m 1000 -R 0.1
				fpcache -r 200 -p retail.dat -s 21 -H 2 -L 0 -U 8 -m 1000 -R 0.1 
				fpcache -r 200 -p T10I4D100K.dat -s 20 -H 2 -L 0 -U 8 -m 1000 -R 0.1
				T10I4D100K
				T40I10D100K
				retail
			*/
			if (needRebuild)//需重建
			{
				transactions(filepath, _transactions);
				if (0 == _transactions.size())
				{
					cout << "fpcache> transactions can't be read ,please check the file:" << filepath << endl;
				}
			}


			LRUStack lruStack(room);
			FPCache fpCache(room,highSizeWeight, lowSizeWeight,lruSizeWeight);
			ARCCache accCache(room);


			fpCache.setMinSupport(supWet*samplenum);
			fpCache.setMinSupportWet(supWet);
			//std::set<Pattern> patterns;
			fpCache.setMaxLogSize(logSize);

			std::vector<Transaction> temptrans;

			int64_t counter = 0;
			int64_t blankCounter = 0;

			uniAccess(lruStack, fpCache, accCache, _transactions, temptrans, samplenum/samplingRate, samplingRate,alpha);


			//auto transIt = _transactions.begin();
			//while (transIt != _transactions.end())
			//{
			//	auto itemsIt = (*transIt).begin();
			//	while (itemsIt != (*transIt).end())
			//	{
			//		lruStack.access(*itemsIt);
			//		fpCache.access(*itemsIt);
			//		itemsIt++;
			//	}
			//	
			//	if (blankSize> blankCounter){
			//		blankCounter++;
			//	}
			//	else{
			//		fpCache.appendLogTrans(*transIt);
			//		counter++;
			//	}
			//	
			//*********************************************************************/
			//	if (counter >= fpCache.getMaxLogSize())
			//	{
			//		//此括号内为计算和调整fpcache 的代码块
			//		vector<Pattern> sortedPatterns;
			//		fpCache.runFPAnalyse(fpCache.getLog(),patterns);
			//		fpCache.sortPatternsBySup(sortedPatterns, patterns);
			//		fpCache.procPattern(sortedPatterns,
			//			fpCache.getHighCorrCache().getShadowCache(),
			//			fpCache.getLowCorrCache().getShadowCache());
			//		fpCache.cacheOrganize();
			//		/***********************************************************/
			//		cout << "ACC_NUM:" << fpCache.stateACC()
			//			<< " HIT_NUM:" << fpCache.stateHIT()
			//			<< " PAGE_FAULT_NUM:" << fpCache.stateFault()
			//			<< "	hit ratio:" << ((float)fpCache.stateHIT() / fpCache.stateACC()) * 100 << "%" << endl;
			//		/**********************************************************/
			//		counter = 0;
			//		blankCounter = 0;
			//	}
			//********************************************************************/
			//	transIt++;
			//}

			cout <<"Total:\n"<< fpCache << endl;
			printf("args:	%s\n", comm_args_buffer);
			cout << lruStack << endl;
			cout << "ARC:	hit ratio:"; accCache.getHitRatio(); cout << endl;
			
			lruStack.flush();
		}
		else if (0 == strcmp(comm_buffer, "exit")) {
			printf("exit...\n");
			stop= true;
		}
		else
		{
			printf("%s bad command\n",comm_buffer);
		}
	}
}



int main()
{
	drive_machine();
}
