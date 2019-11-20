#pragma once
#include <list>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include "common.h"
#include "mq.h"

using namespace std;

class MQCache
{
public:
	MQCache(int size);
	void access(Entry entry);
	void getHitRatio();
	uint64_t getHit();
	uint64_t getAcc();
	uint64_t getMis();

private:
	MQ list_;
	int cacheHit = 0;
	int cacheRequest = 0;
	int cacheMiss = 0;
};

#pragma once
