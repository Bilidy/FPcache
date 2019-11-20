#pragma once
#include <list>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include "common.h"
#include "arc.h"

using namespace std;

class ARCCache
{
public:
	ARCCache(int size);
	bool access(Entry entry);
	void getHitRatio();
	uint64_t getHit();
	uint64_t getAcc();
	uint64_t getMis();
	void timeINC(uint64_t t);
	void seqnumINC();
	uint64_t getAvgtime();
	void restAvgtime();



private:
	ARC list_;
	int cacheHit=0; 
	int cacheRequest=0; 
	int cacheMiss = 0;
	uint64_t time = 0;
	uint64_t seqnum = 0;
	uint64_t avgtime = 0;
};

