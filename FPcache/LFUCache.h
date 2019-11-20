#pragma once
#pragma once
#include "lfu.h"
#include "common.h"
using namespace std;

class LFUCache
{
public:
	LFUCache(int size);
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
	LFU_Cache<string, string> lfuCache;
	uint32_t cacheHit = 0;
	uint32_t cacheRequest = 0;
	uint32_t cacheMiss = 0;
	uint64_t time = 0;
	uint64_t seqnum = 0;
	uint64_t avgtime = 0;
};
