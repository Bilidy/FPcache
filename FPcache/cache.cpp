#include"cache.h"

fpCache::fpCache(uint64_t _maxszie, cacheType _type)
{

	maxszie = _maxszie;
	//size = 0;
	type = _type;
}

uint64_t fpCache::getCacheSize()
{
	return cache.size();
}

void fpCache::setMaxCacheSize(uint64_t _maxszie)
{
	maxszie= _maxszie;
}

uint64_t fpCache::getMaxCacheSize()
{
	return maxszie;
}

void fpCache::setShadowCache(shadowCache _ShadowCache)
{
	ShadowCache = _ShadowCache;
}
shadowCache& fpCache::getShadowCache()
{
	return ShadowCache;
}
FPcache& fpCache::getCache()
{
	return cache;
}
//_item ÊÇ·ñÔÚ
bool fpCache::isItemInCache(Item _item) {
	auto cit = cache.begin();
	while (cit!= cache.end())
	{
		if (_item==(*cit))
		{
			return true;
		}
		cit++;
	}
	return false;
}
bool fpCache::setCacheItem(Item _item)
{
	if (cache.size()<maxszie){
		if (ShadowCache.find(_item)!= ShadowCache.end())//should in cache
		{
			ShadowCache.find(_item)->second = 1;
			cache.push_back(_item);
			//size++;
			return true;
		}
		else{
			return false;
		}
	}
	else{
		return false;
	}
	
}
//
bool fpCache::evictCacheItem(Item _item)
{
	if (cache.size() > 0){
		auto it = cache.begin(); 
		while (it != cache.end()){
			if ((*it)== _item){
				cache.erase(it);
				//size--;
			}
		}
		if (cache.end()==it){
			return false;
		}
		return true;
	}
	return false;
}




int fpCache::findItemState(Item _item)
{
	auto it = ShadowCache.find(_item);
	if (it != ShadowCache.end()) {//should in this cache
		if (it->second == 1)//in this cache actually;
		{
			return 1;//should and actually
		}
		else if ((L_type == type) && (it->second == 0))//but it is not and the type of the cache is lowcorrcache
		{
			return 2; //should but not,set item in cache
		}
		else if ((H_type == type) && (it->second == 0)) {
			//item must in this cache ,but something wrong, maybe a bug.
			//setCacheItem(_item);
			return -1;
		}
	}
	return 0;//should not in this cache.
}

void fpCache::access(Item _item)
{
	//ACC_NUM++;
	//auto it = ShadowCache.find(_item);
	//if (it != ShadowCache.end()) {//should in this cache
	//	if (it->second == 1)//in this cache actually;
	//	{
	//		HIT_NUM++;
	//		return true;
	//	}
	//	else if ((L_type == type) &&( it->second == 0))//but it is not and the type of the cache is lowcorrcache
	//	{
	//		PAGE_FAULT_NUM++;
	//		if (setCacheItem(_item)){
	//			return true;
	//		}
	//		else
	//		{
	//			return false;
	//		}
	//	}
	//	else if((H_type == type) && (it->second == 0)){//something wrong, maybe a bug.
	//		setCacheItem(_item);
	//		return true;
	//	}
	//}
	//return false;//should not in this cache
}

void fpCache::clear()
{
	cache.clear();
}

