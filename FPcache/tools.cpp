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
#include"common.h"

using Item = std::string;
using Transaction = std::vector<Item>;

#define ARGS_BUFFER_SIZE 201
#define COMM_BUFFER_SIZE 21
#define COMM_ARGS_BUFFER_SIZE 191
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
string args2fn(string comd,std::set<pair<string, string>> args) {

	char windowsize[10];
	string rt = comd+"_t"+ _itoa(WINDOW, windowsize,10)+ "_stay"+ _itoa(STAY, windowsize, 10);
	auto it = args.begin();
	while (it!= args.end())
	{
		switch ((*it).first[1])
		{
		case 't':
			rt += "_type" + (*it).second;
			break;
		case 'm':
			rt += "_m" + (*it).second;
			break;
		case 'R':
			rt += "_R" + (*it).second;
			break;
		case 'H':
			rt += "_H" + (*it).second;
			break;
		case 'U':
			rt += "_U" + (*it).second;
			break;
		case 'r':
			rt += "_r" + (*it).second;
			break;
		case 's':
			rt += "_s" + (*it).second;
			break;
		case 'a':
			rt += "_a" + (*it).second;
			break;
		default:
			break;
		}
		it++;
	}
	auto p = rt.begin();
	while (p!= rt.end())
	{
		if (*p=='.')
		{
			rt.erase(p);
		}
		else
		{
			p++;
		}
	}
	return rt;
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

double skew_supWet = DEFAULT_SUPPORT;
string skew_filepath;
string skew_w_filepath;

std::vector<Transaction> skew_transactions;
std::vector<Transaction> skew_w_transactions;
std::vector<Transaction> skew_temptransactions;

size_t skew_room;
size_t skew_highSizeWeight = 0.3;
size_t skew_lruSizeWeight = 0.7;
size_t skew_logSize = 1000;
size_t skew_samplenum = 1000;
size_t skew_blankSize = 1000;
string defaultOutputName = "defaultOutput.csv";
double skew_samplingRate = 0.1;
double skew_alpha = 0.7;
int TYPE=2;

bool skew_needRebuild = true;
bool skew_w_needRebuild = true;


size_t skew_jump_low;
size_t skew_jump_high;



double supWet = DEFAULT_SUPPORT;
string filepath;

std::vector<Transaction> _transactions;
std::vector<Transaction> _temptransactions;

size_t room;
size_t highSizeWeight = 0.3;
size_t lruSizeWeight = 0.7;
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
					}
					else
					{
						cout << "fpcache> please check the parameter:" << (*it).first << endl;
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
				else if (("-t" == (*it).first)) {
					if (TYPE = atoi((*it).second.c_str())) {
						cout << "fpcache> TYPE:" << TYPE << endl;
					}
					else
					{
						cout << "fpcache> please check the parameter:" << (*it).first << endl;
					}
				}
				it++;
			}
			defaultOutputName = args2fn(comm_buffer, args) + ".csv";
			cout << "fpcache> result output:" << defaultOutputName << endl;
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

				fpcache -p P12.lis -H 3 -L 0 -U 7 -m 10000 -R 0.1 -r 100000 -s 0.01 -a 1.6
				fpcache -p P4.lis -H 3 -L 0 -U 7 -m 1000 -R 0.1 -r 100000 -s 0.002 -a 0.7
				fpcache -p P4.lis -H 3 -L 0 -U 7 -m 1000 -R 0.1 -r 10000 -s 0.0005 -a 0.7
				fpcache -r 200 -p retail.dat -s 0.008 -H 2 -L 0 -U 8 -m 1000 -R 0.1
				fpcache -p kosarak.dat -H 2 -L 0 -U 8 -m 1000 -R 0.1 -r 1000 -s 0.008 -a 1.2 -o type2_fp_m1000_R01_r1000_s0008_a12
				fpcache -p kosarak.dat -H 2 -L 0 -U 8 -m 1000 -R 0.1 -r 1000 -s 0.008 -a 1.2 -o type1_fp_m1000_R01_r1000_s0008_a12
				T10I4D100K
				T40I10D100K
				retail
				

				fpcache -p kosarak.dat -H 2 -U 8 -m 1000 -R 0.1 -r 4194304 -s 0.008 -a 0.7 -t 3
				fpcache -p kosarak.dat -H 2 -U 8 -m 1000 -R 0.1 -r 4194304 -s 0.008 -a 1.2 -t 3
				fpcache -p kosarak.dat -H 2 -U 8 -m 1000 -R 0.1 -r 4194304 -s 0.008 -a 1.7 -t 3

				fpcache -p kosarak.dat -H 2 -U 8 -m 1000 -R 0.1 -r 8388608 -s 0.008 -a 0.7 -t 3
				fpcache -p kosarak.dat -H 2 -U 8 -m 1000 -R 0.1 -r 8388608 -s 0.008 -a 1.2 -t 3
				fpcache -p kosarak.dat -H 2 -U 8 -m 1000 -R 0.1 -r 8388608 -s 0.008 -a 1.7 -t 3

				fpcache -p kosarak.dat -H 2 -U 8 -m 1000 -R 0.1 -r 16777216 -s 0.008 -a 0.7 -t 3
				fpcache -p kosarak.dat -H 2 -U 8 -m 1000 -R 0.1 -r 16777216 -s 0.008 -a 1.2 -t 3
				fpcache -p kosarak.dat -H 2 -U 8 -m 1000 -R 0.1 -r 16777216 -s 0.008 -a 1.7 -t 3
				
				fpcache -p kosarak.dat -H 2 -U 8 -m 1000 -R 0.1 -r 33554432 -s 0.008 -a 0.7 -t 3
				fpcache -p kosarak.dat -H 2 -U 8 -m 1000 -R 0.1 -r 33554432 -s 0.008 -a 1.2 -t 3
				fpcache -p kosarak.dat -H 2 -U 8 -m 1000 -R 0.1 -r 33554432 -s 0.008 -a 1.7 -t 3

				fpcache -p kosarak.dat -H 2 -U 8 -m 1000 -R 0.1 -r 67108864 -s 0.008 -a 0.7 -t 3
				fpcache -p kosarak.dat -H 2 -U 8 -m 1000 -R 0.1 -r 67108864 -s 0.008 -a 1.2 -t 3
				fpcache -p kosarak.dat -H 2 -U 8 -m 1000 -R 0.1 -r 67108864 -s 0.008 -a 1.7 -t 3

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
			FPCache fpCache(room,highSizeWeight,lruSizeWeight);
			ARCCache accCache(room);

			fpCache.setMinSupport(ceil(supWet*samplenum));
			fpCache.setMinSupportWet(supWet);
			//std::set<Pattern> patterns;
			fpCache.setMaxLogSize(logSize);

			std::vector<Transaction> temptrans;

			int64_t counter = 0;
			int64_t blankCounter = 0;

			uniAccess(lruStack, fpCache, accCache, _transactions
				, temptrans, samplenum/samplingRate
				, samplingRate,alpha,defaultOutputName,
				TYPE
			);

			cout <<"Total:\n"<< fpCache << endl;
			printf("args:	%s\n", comm_args_buffer);
			cout << lruStack << endl;
			cout << "ARC:	hit ratio:"; accCache.getHitRatio(); cout << endl;
			
			lruStack.flush();
		}
		
		if (0 == strcmp(comm_buffer, "skewtest")) {
			printf("skewtest> args:%s\n", comm_args_buffer);
			std::set<pair<string, string>> args;
			get_args(comm_args_buffer, args);

			auto it = args.begin();
			while (it != args.end())
			{
				//-s 最小支持度参数，获取最小支持度。系统默认为2。
				if ("-s" == (*it).first) {//support
					//cout << "support:"<<(*it).second << endl;
					if (skew_supWet = atof((*it).second.c_str())) {
						cout << "skewtest> support weight:" << skew_supWet << endl;
					}
					else
					{
						cout << "skewtest> please check the parameter:" << (*it).first << endl;
					}
					//cout << "fpcache> default support:" << sup << endl;
				}
				//获取文件的路径filepath，文件里是transactions,若和前次文件路径相同则不做重建工作
				else if ("-p" == (*it).first) {

					if (skew_filepath == (*it).second)
					{
						skew_needRebuild = false;
					}
					else
					{
						skew_filepath = (*it).second;
						skew_needRebuild = true;
					}
					cout << "skewtest> file path:" << (*it).second << endl;
				}
				else if ("-w" == (*it).first) {

					if (skew_w_filepath == (*it).second)
					{
						skew_w_needRebuild = false;
					}
					else
					{
						skew_w_filepath = (*it).second;
						skew_w_needRebuild = true;
					}
					cout << "skewtest> workload file path:" << (*it).second << endl;
				}
				//指定缓存方式的缓存大小。
				else if ("-r" == (*it).first) {

					if (stor((*it).second, skew_room)) {
						cout << "skewtest> room:" << skew_room << endl;
					}
					else
					{
						cout << "skewtest> please check the parameter:" << (*it).first << endl;
					}

				}
				else if ("-H" == (*it).first) {
					if (stor((*it).second, skew_highSizeWeight)) {
						cout << "skewtest> high weight:" << skew_highSizeWeight << endl;
					}
					else
					{
						cout << "skewtest> please check the parameter:" << (*it).first << endl;
					}
				}
				else if ("-U" == (*it).first) {
					if (stor((*it).second, skew_lruSizeWeight)) {
						cout << "skewtest> LRU weight:" << skew_lruSizeWeight << endl;
					}
					else
					{
						cout << "skewtest> please check the parameter:" << (*it).first << endl;
					}
				}
				else if ("-m" == (*it).first) {
					if (stor((*it).second, skew_samplenum)) {
						cout << "skewtest> sample size:" << skew_samplenum << endl;
					}
					else
					{
						cout << "skewtest> please check the parameter:" << (*it).first << endl;
					}
				}
				else if (("-R" == (*it).first)) {
					if (skew_samplingRate = atof((*it).second.c_str())) {
						cout << "skewtest> sampling rate:" << skew_samplingRate << endl;
					}
					else
					{
						cout << "skewtest> please check the parameter:" << (*it).first << endl;
					}
				}
				else if (("-a" == (*it).first)) {
					if (skew_alpha = atof((*it).second.c_str())) {
						cout << "skewtest> alpha:" << skew_alpha << endl;
					}
					else
					{
						cout << "skewtest> please check the parameter:" << (*it).first << endl;
					}
				}
				else if (("-t" == (*it).first)) {
					if (TYPE = atoi((*it).second.c_str())) {
						cout << "skewtest> TYPE:" << TYPE << endl;
					}
					else
					{
						cout << "skewtest> please check the parameter:" << (*it).first << endl;
					}
				}
				it++;
			}
			defaultOutputName = args2fn(comm_buffer, args) + ".csv";
			cout << "fpcache> result output:" << defaultOutputName << endl;
			/*
				skewtest -r 50 -p transactions.txt -w retail.dat -s 15 -H 3 -U 6 -m 1000 -R 0.1
				skewtest -r 1000 -p kosarak.dat -w retail.dat -s 15 -H 5 -U 5 -m 1000 -R 0.1
				skewtest -p kosarak.dat -w retail.dat -H 3-U 7 -m 1000 -R 0.1 -r 1000 -s 0.01 -a 1.6
				skewtest -p kosarak.dat -w retail.dat -H 5 -U 5 -m 1000 -R 0.1 -r 500 -s 0.008 -a 1.3
				skewtest -r 800 -p kosarak.dat -w retail.dat -s 0.02 -H 3 -U 7 -m 1000 -R 0.1 -a 2
				skewtest -r 200 -p T40I10D100K.dat -w retail.dat -s 21 -H 2 -U 8 -m 1000 -R 0.1
				skewtest -r 200 -p retail.dat -w retail.dat -s 21 -H 2 -U 8 -m 1000 -R 0.1
				skewtest -r 200 -p T10I4D100K.dat -w retail.dat -s 20 -H 2 -U 8 -m 1000 -R 0.1

				skewtest -p kosarak.dat -w retail.dat -H 3 -U 7 -m 1000 -R 0.1 -r 1000 -s 0.01 -a 1.6
				skewtest -p kosarak.dat -w retail.dat -H 3 -U 7 -m 1000 -R 0.5 -r 400 -s 0.01 -a 1.6

				skewtest -p T40I10D100K.dat -w retail.dat -H 3 -U 7 -m 1000 -R 0.5 -r 400 -s 0.01 -a 1.6
				skewtest -p kosarak.dat -w retail.dat -H 3 -U 7 -m 1000 -R 0.5 -r 400 -s 0.01 -a 1.6
				skewtest -p P12.lis -w retail.dat -H 3 -U 7 -m 10000 -R 0.1 -r 100000 -s 0.01 -a 1.6
				skewtest -p kosarak.dat -w retail.dat -H 3 -U 7 -m 1000 -R 0.1 -r 1000 -s 0.008 -a 1.7 -o m1000_R01_r1000_s0008_a17
				skewtest -p kosarak.dat -w retail.dat -H 2 -U 8 -m 2000 -R 0.2 -r 500 -s 0.008 -a 1.7 -o type2_t64_sk_H2_U8_m2000_R02_r500_s0008_a17
				skewtest -p kosarak.dat -w retail.dat -H 2 -U 8 -m 1000 -R 0.1 -r 400 -s 0.008 -a 1.2 -o type2_t16_sk_H2_U8_m1000_R01_r400_s0008_a17
				
				skewtest -p kosarak.dat -w retail.dat -H 2 -U 8 -m 2000 -R 0.2 -r 500 -s 0.008 -a 1.7

				skewtest -p kosarak.dat -w retail.dat -H 2 -U 8 -m 1000 -R 0.1 -r 4194304 -s 0.008 -a 0.7 -t 3
				skewtest -p kosarak.dat -w retail.dat -H 2 -U 8 -m 1000 -R 0.1 -r 4194304 -s 0.008 -a 1.2 -t 3
				skewtest -p kosarak.dat -w retail.dat -H 2 -U 8 -m 1000 -R 0.1 -r 4194304 -s 0.008 -a 1.7 -t 3

				skewtest -p kosarak.dat -w retail.dat -H 2 -U 8 -m 1000 -R 0.1 -r 8388608 -s 0.008 -a 0.7 -t 3
				skewtest -p kosarak.dat -w retail.dat -H 2 -U 8 -m 1000 -R 0.1 -r 8388608 -s 0.008 -a 1.2 -t 3
				skewtest -p kosarak.dat -w retail.dat -H 2 -U 8 -m 1000 -R 0.1 -r 8388608 -s 0.008 -a 1.7 -t 3

				skewtest -p kosarak.dat -w retail.dat -H 2 -U 8 -m 1000 -R 0.1 -r 16777216 -s 0.008 -a 0.7 -t 3
				skewtest -p kosarak.dat -w retail.dat -H 2 -U 8 -m 1000 -R 0.1 -r 16777216 -s 0.008 -a 1.2 -t 3
				skewtest -p kosarak.dat -w retail.dat -H 2 -U 8 -m 1000 -R 0.1 -r 16777216 -s 0.008 -a 1.7 -t 3

				skewtest -p kosarak.dat -w retail.dat -H 2 -U 8 -m 1000 -R 0.1 -r 33554432 -s 0.008 -a 0.7 -t 3
				skewtest -p kosarak.dat -w retail.dat -H 2 -U 8 -m 1000 -R 0.1 -r 33554432 -s 0.008 -a 1.2 -t 3
				skewtest -p kosarak.dat -w retail.dat -H 2 -U 8 -m 1000 -R 0.1 -r 33554432 -s 0.008 -a 1.7 -t 3

				skewtest -p kosarak.dat -w retail.dat -H 2 -U 8 -m 1000 -R 0.1 -r 67108864 -s 0.008 -a 0.7 -t 3
				skewtest -p kosarak.dat -w retail.dat -H 2 -U 8 -m 1000 -R 0.1 -r 67108864 -s 0.008 -a 1.2 -t 3
				skewtest -p kosarak.dat -w retail.dat -H 2 -U 8 -m 1000 -R 0.1 -r 67108864 -s 0.008 -a 1.7 -t 3

			*/
			if (skew_needRebuild)//需重建
			{

				if (split(skew_filepath, '.')[1] == "lis") {
					transactions(skew_filepath, skew_transactions,1);
				}
				else{
					transactions(skew_filepath, skew_transactions);
				}
				
				skew_jump_high  = radmGen(0, skew_transactions.size() - 1, 1);
				skew_jump_low = radmGen(0, skew_jump_high, 1);
				cout << "skewtest> skew_jump_low:"<< skew_jump_low << " skew_jump_high:" << skew_jump_high << endl;
				//skewWorkLoad(skew_transactions, skew_jump_low, skew_jump_high);
				if (0 == skew_transactions.size())
				{
					cout << "skewtest> transactions can't be read ,please check the file:" << skew_filepath << endl;
				}
			}
			if (skew_w_needRebuild)
			{
				wtransactions(skew_w_filepath, skew_w_transactions);
			}

			LRUStack lruStack(skew_room);
			FPCache fpCache(skew_room, skew_highSizeWeight, skew_lruSizeWeight);
			ARCCache accCache(skew_room);

			fpCache.setMinSupport(skew_supWet*skew_samplenum);
			fpCache.setMinSupportWet(skew_supWet);
			fpCache.setMaxLogSize(skew_logSize);

			std::vector<Transaction> temptrans;

			int64_t counter = 0;
			int64_t blankCounter = 0;

			uniAccess(lruStack, fpCache, accCache,
					skew_transactions,
					skew_w_transactions,
					temptrans,
					skew_samplenum / skew_samplingRate,
					skew_samplingRate,
					skew_alpha,
					skew_jump_low,
					skew_jump_high,
					defaultOutputName,
					TYPE
				);
			

			cout << "Total:\n" << fpCache << endl;
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
