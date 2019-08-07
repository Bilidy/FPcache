#include"cache.h"

fpCache::fpCache(uint64_t _maxszie, cacheType _type):cache(_maxszie)
{

	maxszie = _maxszie;
	type = _type;
}

uint64_t fpCache::getCacheSize()
{
	return cache.getCacheSize();
}

void fpCache::setMaxCacheSize(uint64_t _maxszie)
{
	maxszie= _maxszie;
	cache.setMaxSize(maxszie);
}

uint64_t fpCache::getMaxCacheSize()
{
	return maxszie;
}

void fpCache::setShadowCache(const shadowCache _ShadowCache)
{
	ShadowCache = _ShadowCache;
}
shadowCache& fpCache::getShadowCache()
{
	return ShadowCache;
}
LRUStack& fpCache::getCache()
{
	return cache;
}
uint64_t fpCache::getMaxSize()
{
	return maxszie;
}
//_item �Ƿ���
bool fpCache::isItemInCache(Item _item) {
	auto cit = cache.begin();
	while (cit!= cache.end())
	{
		if (_item==(*cit).item)
		{
			return true;
		}
		cit++;
	}
	return false;
}

bool fpCache::setCacheItem(Item _item)
{
	if (cache.getCacheSize()<maxszie){
		if (ShadowCache.find(_item)!= ShadowCache.end())//should in cache
		{
			ShadowCache.find(_item)->second = 1;
			cache.inseart(_item);
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
	if (cache.getCacheSize() > 0){
		auto it = cache.begin(); 
		while (it != cache.end()){
			if ((*it).item == _item){
				cache.evict(it->item);
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

void fpCache::orgnaize()
{
	if (ShadowCache.empty())
	{
		return;
	}
	auto it = ShadowCache.begin();
	while (it != ShadowCache.end())
	{
		//δ��cache�з���
		if (cache.find(it->first)!=cache.end()) {
			cache.touch(it->first);
		}
		else//��cache�з���
		{
			cache.inseart(it->first);
			//�Ǿ��ڰ�
		}
		it++;
	}

}




int fpCache::findItemState(Item _item)
{
	auto it = cache.find(_item);
	if (it != cache.end()) {//should in this cache
		return 1;//should and actually
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
	cache.flush();
}

