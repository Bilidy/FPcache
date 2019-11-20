#include "LFUCache.h"

LFUCache::LFUCache(int size):lfuCache(size)
{
}

bool LFUCache::access(Entry entry)
{
	cacheRequest++;
	if (lfuCache.find(entry.item, cacheRequest) == nullptr) {
		lfuCache.put(entry.item, entry.item, cacheRequest);
		cacheMiss++;
		return false;
	}
	else
		cacheHit++;
	return true;
}

void LFUCache::getHitRatio()
{
	std::cout << "	All Read: " << cacheRequest << "	Hit:" << cacheHit << "	" << 100.0 * cacheHit / cacheRequest << "%" << std::endl;
}

uint64_t LFUCache::getHit()
{
	return cacheHit;
}

uint64_t LFUCache::getAcc()
{
	return cacheRequest;
}

uint64_t LFUCache::getMis()
{
	return cacheMiss;
}
void LFUCache::timeINC(uint64_t t)
{
	time += t;
}

void LFUCache::seqnumINC()
{
	seqnum++;
}

uint64_t LFUCache::getAvgtime()
{

	return time / seqnum;
}

void LFUCache::restAvgtime()
{
	time = 0;
	seqnum = 0;
	avgtime = 0;
}
