#include "ARCCache.h"
#include "common.h"

ARCCache::ARCCache(int size):list_(size)
{
}

bool ARCCache::access(Entry entry)
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

void ARCCache::getHitRatio()
{
	list_.Print(true);
	
}

uint64_t ARCCache::getHit()
{
	return cacheHit;
}

uint64_t ARCCache::getAcc()
{
	return cacheRequest;
}

uint64_t ARCCache::getMis()
{
	return cacheRequest-cacheHit;
}

void ARCCache::timeINC(uint64_t t)
{
	time += t;
}

void ARCCache::seqnumINC()
{
	seqnum++;
}

uint64_t ARCCache::getAvgtime()
{
	
	return time / seqnum;
}

void ARCCache::restAvgtime()
{
	time = 0;
	seqnum = 0;
	avgtime = 0;
}
