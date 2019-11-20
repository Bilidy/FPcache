#include "MQCache.h"
#include "common.h"

MQCache::MQCache(int size) :list_(size)
{
}

void MQCache::access(Entry entry)
{
	cacheRequest++;
	auto v = list_.Peek(stoi(entry.item));
	if (IS_VALID(v)) {
		cacheHit++;
		list_.Get(stoi(entry.item));
	}
	else {
		cacheMiss++;
		list_.Add(stoi(entry.item), entry.size);
	}
}

void MQCache::getHitRatio()
{
	list_.Print(true);
}

uint64_t MQCache::getHit()
{
	return cacheHit;
}

uint64_t MQCache::getAcc()
{
	return cacheRequest;
}

uint64_t MQCache::getMis()
{
	return cacheRequest - cacheHit;
}
