#include "lowCache.h"

lowCache::lowCache(uint64_t _maxszie, cacheType _type):fpCache(_maxszie, _type)
{
	
}

bool lowCache::access(Entry entry)
{
	if (ShadowCache.find(entry.item)!= ShadowCache.end()) {
		//if (cache.find(entry.item)!= cache.end())
		if (ShadowCache.find(entry.item)->second== 1)
		{
			cache.touch(entry.item);
			return true;
		}
		else
		{
			cache.inseart(entry);
			auto it = ShadowCache.begin();
			while(it!= ShadowCache.end()) {
				if (cache.find(it->first)!=cache.end()) {
					it->second = 1;
				}
				else
				{
					it->second = 0;
				}
				it++;
			}
			return false;
		}
	}
	else
	{
		return false;
	}
}


int lowCache::findItemState(Item _item)
{
	if (lowCache::ShadowCache.find(_item) != lowCache::ShadowCache.end()) {//could in this cache
		if (cache.find(_item) != cache.end())
		{
			return 1;//should and actually
		}
		else
		{
			return 2;//could but not.
		}
	}
	else
	{
		return 0;
	}
}
void lowCache::orgnaize()
{
	while (cache.getCacheSize() > cache.getMaxSize())
	{
		Item evicteditem = (cache.end()->item);
		if (!cache.evict(1)) { 
			ShadowCache.find(evicteditem)->second = 0;
			break; 
		};
	}
	if (ShadowCache.empty())
	{
		return;
	}
	auto it = ShadowCache.begin();
	while (it != ShadowCache.end())
	{
		it->second = 0;
		//在cache中发现
		if (cache.find(it->first) != cache.end())
		{
			it->second = 1;
		}
		it++;
	}
	LRUItem* itt = cache.begin();
	while (itt!=cache.end())
	{
		if (ShadowCache.find(itt->item)== ShadowCache.end())
		{
			if (cache.evict(itt->item)) {
				itt= cache.begin();
			};
		}
		itt=itt->next;
	}
}
bool lowCache::evictCacheItem(Item _item)
{
	if (cache.getCacheSize() > 0) {

		if (cache.find(_item) != cache.end()&&ShadowCache.find(_item)!=ShadowCache.end())
		{
			cache.evict(_item);
			ShadowCache.find(_item)->second = 0;
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}
