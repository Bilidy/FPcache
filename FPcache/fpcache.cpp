#include "fpcache.hpp"

FPCache::FPCache(size_t _maxszie, float _highScaleWeight, float _lowScaleWeight, float _lruScaleWeight) :
	highCorrCache(0, true), lowCorrCache(0, false), lruCache(0), maxszie(_maxszie), highScaleWeight(_highScaleWeight),
	lowScaleWeight(_lowScaleWeight), lruScaleWeight(_lruScaleWeight), highCorrCacheMaxSize(0), lowCorrCacheMaxSize(0),
	lruCacheMaxSize(0), minimum_support_threshold(0), maxLogSize(0), highLowThreshold(0.2), min_sup_wet(0.02)
{
	float sum=_highScaleWeight + _lowScaleWeight + _lruScaleWeight;
	size_t highCacheSize = 0;
	size_t lowCacheSize = 0;
	size_t lruCacheSize = 0;
	if (sum) {
		highCacheSize = _maxszie * (_highScaleWeight) / sum;
		lowCacheSize = _maxszie * (_lowScaleWeight) / sum;
		lruCacheSize = _maxszie * (_lruScaleWeight) / sum;
	}

	highCorrCache.setMaxCacheSize(highCacheSize);
	lowCorrCache.setMaxCacheSize(lowCacheSize);
	//lruCache.setMaxSize(_maxszie - highCacheSize - lowCacheSize);
	lruCache.setMaxSize(_maxszie);

	ACC_NUM = 0;
	HIT_NUM = 0;
	PAGE_FAULT_NUM = 0;
}

bool FPCache::runFPAnalyse(std::vector<Transaction> _accLog,std::set<Pattern>& patterns)
{
	if (_accLog.size())
	{
		const FPTree fptree{ _accLog, minimum_support_threshold };
		patterns = fptree_growth(fptree);
		_accLog.clear();
		return true;
	}
	else
		return false;
}
//sort Patterns By Support form big to little
void FPCache::sortPatternsBySup(std::vector<Pattern>& sortedPatterns, std::set<Pattern>& patterns)
{
	sortedPatterns.clear();
	uint64_t MaxSupport = minimum_support_threshold;
	Pattern tempTrans;
	std::set<Pattern>::iterator tempit;
	shadowCache tempScache;

	while (patterns.size() && (tempScache.size() < highCorrCache.getMaxCacheSize()))
	{
		
		MaxSupport = minimum_support_threshold;
		auto it = patterns.begin();
		while (it != patterns.end())
		{
			if ((*it).second >= MaxSupport)
			{
				tempit = it;
				tempTrans = (*it);
				MaxSupport = (*it).second;
			}
			it++;
		}
		if (tempTrans.first.size() > 1)
		{
			sortedPatterns.push_back(tempTrans);
			auto its = tempTrans.first.begin();
			while (its != tempTrans.first.end())
			{
				if (tempScache.find(*its) == tempScache.end()) {
					tempScache.insert(std::pair<Item,uint64_t>((*its), 0));
					//printf("\n**********%d\n", tempScache.size());
				}
				//printf("%s ", (*its).c_str());
				its++;
			}
			//printf("\n");
		}
		patterns.erase(tempit);
	}
}

bool FPCache::procPattern(std::vector<Pattern>& patterns, shadowCache& _shadowHigh, shadowCache& _shadowLow)
{
	_shadowHigh.clear();
	_shadowLow.clear();
	auto it = patterns.begin();
	while (it != patterns.end() && _shadowHigh.size() < highCorrCache.getMaxCacheSize())
	{
		auto its = (*it).first.begin();
		while (its != (*it).first.end())
		{
			_shadowHigh[*its] = 0;
			its++;
		}
		it++;
	}
	if (it != patterns.end())
	{
		while (it != patterns.end() && _shadowLow.size() < lowCorrCache.getMaxCacheSize())
		{
			auto its = (*it).first.begin();
			while (its != (*it).first.end())
			{
				if (_shadowHigh.find(*its)== _shadowHigh.end())//����highcache��
				{
					_shadowLow[*its] = 0;
				}
				its++;
			}
			it++;
		}
	}
	else
	{
		_shadowLow.clear();
	}
	return true;
}

bool FPCache::getCacheDelta(shadowCache& _shadowHigh, shadowCache& _shadowLow,cacheDelta& _inHighCache, cacheDelta& _outHighCache, cacheDelta& _inLowCache, cacheDelta& _outLowCache)
{
	return false;
}

void FPCache::setHighCorrCacheMaxSize(uint64_t _HighCorrCacheSize)
{
}

void FPCache::setLowCorrCacheMaxSize(uint64_t _LowCorrCacheSize)
{
}

void FPCache::setMaxLogSize(uint64_t _logSize)
{
	maxLogSize =_logSize;
}

uint64_t FPCache::getMaxLogSize()
{
	return maxLogSize;
}

void FPCache::setMinSupport(uint64_t _minimum_support_threshold)
{
	minimum_support_threshold=_minimum_support_threshold;
}
void FPCache::setMinSupportWet(double _min_sup_wet)
{
	min_sup_wet = _min_sup_wet;
}
float FPCache::getMinSupportWet()
{
	return min_sup_wet;
}
//��Item����HighCorrCache���ڴ�֮ǰҪ�����lru�г�ȡ������
bool FPCache::setHighCorrCacheItem(Item _item)
{
	//LRUStack::iterator it = lruCache.begin();
	
	if (lruCache.find(_item) !=lruCache.end()) {
		if (lruCache.evict(_item))
		{
			if (highCorrCache.setCacheItem(_item)) {
				return true;
			};
		}
		return false;
	}
	else
	{
		highCorrCache.setCacheItem(_item);
		return true;
	}
}

bool FPCache::setLowCorrCacheItem(Item _item)
{
	if (lruCache.find(_item) != lruCache.end()) {
		if (lruCache.evict(_item))//�ȴ�lru���޳�
		{
			if (lowCorrCache.setCacheItem(_item)) {
				return true;
			};
		}
		return false;
	}
	else
	{
		if (lowCorrCache.setCacheItem(_item)) {
			return true;
		};
	}
}

bool FPCache::evictHighCorrCacheItem()
{
	return false;
}

bool FPCache::evictLowCorrCacheItem()
{
	return false;
}

bool FPCache::resizeHighCorrCache()
{
	return false;
}

bool FPCache::resizeLowCorrCache()
{
	return false;
}

bool FPCache::resizeLRU()
{
	size_t lrusize = maxszie - highCorrCache.getCacheSize()-lowCorrCache.getCacheSize();

	if (lrusize<lruCache.getCacheSize())
	{
		if (lruCache.evict(lruCache.getCacheSize() - lrusize)) {
			return true;
		}
		return false;
	}
	else
	{
		lruCache.setMaxSize(lrusize);
	}
	return true;
}

void FPCache::access(Item _item)
{
	ACC_NUM++;
	int stats=highCorrCache.findItemState(_item);
	switch (stats)
	{
	case 1:
		highCorrCache.access(_item);
		HIT_NUM++;
		return;
	default:
		// something wrong, maybe a bug.
		break;
	}
	stats = lowCorrCache.findItemState(_item);
	switch (stats)
	{
	case 1:
		lowCorrCache.access(_item);
		HIT_NUM++;
		return;
	case 2:
		lowCorrCache.setCacheItem(_item);
		PAGE_FAULT_NUM++;
		return;
	default:
		// something wrong, maybe a bug.
		break;
	}
	lruCache.access(_item);
}

bool FPCache::isEmpty()
{
	return 0 == highCorrCache.getCacheSize() +
		lowCorrCache.getCacheSize() +
		lruCache.getCacheSize();
}

bool FPCache::isFull()
{
	return highCorrCache.getCacheSize() + 
		lowCorrCache.getCacheSize() + 
		lruCache.getCacheSize() == maxszie;
}

bool FPCache::logIsFull()
{
	return accLog.size() == maxLogSize;
}

void FPCache::appendLogTrans(Transaction _trans)
{
	accLog.push_back(_trans);
}

fpCache & FPCache::getHighCorrCache()
{
	return highCorrCache;
}
fpCache & FPCache::getLowCorrCache()
{
	return lowCorrCache;
}
void FPCache::cacheOrganize()
{
	auto it = highCorrCache.getShadowCache().begin();
	//�����cache�е�items
	auto findcacheit = highCorrCache.getCache().begin();
	//����cache
	while (findcacheit != highCorrCache.getCache().end())
	{
		//ShadowCache�д������item
		if (highCorrCache.getShadowCache().find((*findcacheit)) != highCorrCache.getShadowCache().end())
		{
			highCorrCache.getShadowCache()[(*findcacheit)] = 1;
			findcacheit++;
		}
		else//ShadowCache�в��������item
		{
			//������������һ�����⣬�����������ָ������һ��Ԫ�أ��ڴ˴�erase����ô��ָ��end()
			//���ĵ������ԼӺ󽫵��´���
			//.erase()��������ֵ������Ԫ��ָ�룬����.erase(findcacheit)==findcacheit++��
			highCorrCache.getCache().erase(findcacheit);
			//findcacheit++;
		}
		
	}
	//�ڴ˱��shadowCache�Ѿ�������Low�����itemsΪ1
	it = lowCorrCache.getShadowCache().begin();
	while (it != lowCorrCache.getShadowCache().end())
	{
		if (lowCorrCache.isItemInCache(it->first)) {
			it->second = 1;
		}
		it++;
	}

	//����high cache
	it = highCorrCache.getShadowCache().begin();
	while (it!= highCorrCache.getShadowCache().end())
	{
		if ((*it).second == 0)
		{
			setHighCorrCacheItem((*it).first);
			(*it).second = 1;
		}
		it++;
	}
	it = lowCorrCache.getShadowCache().begin();
	
	//���low
	lowCorrCache.clear();
	while (it != lowCorrCache.getShadowCache().end())
	{
		//ԭ��������low�����е�
		if ((*it).second == 1) {
			setLowCorrCacheItem((*it).first);
		}
		//ԭ��������low���浫����lru�з�����
		if ((*it).second == 0&&lruCache.find((*it).first)!= lruCache.end())
		{
			//����
			setLowCorrCacheItem((*it).first);
			(*it).second = 1;
		}
		it++;
	}
	resizeLRU();
}
uint64_t FPCache::stateACC()
{
	return ACC_NUM;
}

uint64_t FPCache::stateHIT()
{
	return HIT_NUM+lruCache.stateHIT();
}

uint64_t FPCache::stateFault()
{
	return PAGE_FAULT_NUM + lruCache.stateFault();
}

