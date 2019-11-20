#include "LIRSCache.h"
#include "common.h"

LIRSCache::LIRSCache(int size):list_(size)
{
}

bool LIRSCache::access(Entry entry)
{
	cacheRequest++;
	auto v = list_.Peek(stoi(entry.item));
	if (IS_VALID(v)) {
		cacheHit++;
		list_.Get(stoi(entry.item));
		return true;
	}
	else {
		cacheMiss++;
		list_.Add(stoi(entry.item), entry.size);
		return false;
	}
}

void LIRSCache::getHitRatio()
{
	list_.Print(true);
}

uint64_t LIRSCache::getHit()
{
	return cacheHit;
}

uint64_t LIRSCache::getAcc()
{
	return cacheRequest;
}

uint64_t LIRSCache::getMis()
{
	return cacheRequest - cacheHit;
}
void LIRSCache::timeINC(uint64_t t)
{
	time += t;
}

void LIRSCache::seqnumINC()
{
	seqnum++;
}

uint64_t LIRSCache::getAvgtime()
{

	return time / seqnum;
}

void LIRSCache::restAvgtime()
{
	time = 0;
	seqnum = 0;
	avgtime = 0;
}
