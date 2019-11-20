#include "LRUKCache.h"
LRUKCache::LRUKCache(int size,int k) :lrukCache(size, 0.5*size, 0.2 * size,k)
{
}

bool LRUKCache::access(Entry entry)
{
	cacheRequest++;
	if (lrukCache.find(entry.item, cacheRequest) == nullptr) {
		lrukCache.put(entry.item, entry.item, cacheRequest);
		cacheMiss++;
		return false;
	}
	else
		cacheHit++;
	return true;
		
}

void LRUKCache::getHitRatio()
{
	std::cout << "	All Read: " << cacheRequest << "	Hit:" << cacheHit << "	" << 100.0 * cacheHit / cacheRequest << "%" << std::endl;
}

uint64_t LRUKCache::getHit()
{
	return cacheHit;
}

uint64_t LRUKCache::getAcc()
{
	return cacheRequest;
}

uint64_t LRUKCache::getMis()
{
	return cacheMiss;
}
void LRUKCache::timeINC(uint64_t t)
{
	time += t;
}

void LRUKCache::seqnumINC()
{
	seqnum++;
}

uint64_t LRUKCache::getAvgtime()
{

	return time / seqnum;
}

void LRUKCache::restAvgtime()
{
	time = 0;
	seqnum = 0;
	avgtime = 0;
}
