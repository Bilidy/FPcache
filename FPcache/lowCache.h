#pragma once
#include "cache.h"
class lowCache :public fpCache
{
public:
	lowCache(uint64_t _maxszie, cacheType _type);
	bool evictCacheItem(Item _item);
	bool access(Entry entry);
	int findItemState(Item _item);
	void orgnaize();

};

