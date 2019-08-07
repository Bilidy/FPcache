#include "fpcache.hpp"
#include<iostream>

FPCache::FPCache(size_t _maxszie, float _highScaleWeight, float _lruScaleWeight) :
	highCorrCache(0, true), lruCache(0), maxszie(_maxszie), highScaleWeight(_highScaleWeight),
	lruScaleWeight(_lruScaleWeight), highCorrCacheMaxSize(0),
	lruCacheMaxSize(0), minimum_support_threshold(0), maxLogSize(0), min_sup_wet(0.02)
{
	float sum=_highScaleWeight + _lruScaleWeight;
	size_t highCacheSize = 0;
	size_t lruCacheSize = 0;
	if (sum) {
		highCacheSize = _maxszie * (_highScaleWeight) / sum;
		lruCacheSize = _maxszie * (_lruScaleWeight) / sum;
	}

	highCorrCache.setMaxCacheSize(highCacheSize);
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
	
	shadowCache tempScache;

	while (patterns.size() && sortedPatterns .size()<=2000&& (tempScache.size() < highCorrCache.getMaxCacheSize()))
	{
		MaxSupport = 0;
		auto tempit = patterns.end();
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
				}
				its++;
			}
		}
		if (tempit != patterns.end()) {
			patterns.erase(tempit);
		}
		//82.499%
		//skewtest -p kosarak.dat -w retail.dat -H 3 -L 0 -U 7 -m 1000 -R 0.1 -r 1000 -s 0.008 -a 0.3 会报错？？？调查
		//已解决
	}
}

bool FPCache::procPattern(std::vector<Pattern>& patterns, shadowCache& _shadowHigh)
{
	_shadowHigh.clear();
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
	return true;
}

bool FPCache::getCacheDelta(shadowCache& _shadowHigh,cacheDelta& _inHighCache, cacheDelta& _outHighCache)
{
	return false;
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

//将Item调入HighCorrCache，在此之前要将其从lru中抽取出来。
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

bool FPCache::evictHighCorrCacheItem()
{
	return false;
}


bool FPCache::resizeHighCorrCache()
{
	return false;
}


bool FPCache::resizeLRU()
{
	size_t lrusize = maxszie - highCorrCache.getCacheSize();
	if (lrusize<lruCache.getCacheSize())
	{
		if (lruCache.evict(lruCache.getCacheSize() - lrusize)) {
			lruCache.setMaxSize(lrusize);
			return true;
		}
		return false;
	}
	lruCache.setMaxSize(lrusize);
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
	lruCache.access(_item);
}

bool FPCache::isEmpty()
{
	return 0 == highCorrCache.getCacheSize() +
		lruCache.getCacheSize();
}

bool FPCache::isFull()
{
	return highCorrCache.getCacheSize() + 
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
void FPCache::cacheOrganize()
{
	highCorrCache.orgnaize();
	LRUStack::iterator it = highCorrCache.getCache().begin();
	while (it != highCorrCache.getCache().end())
	{
		if (lruCache.find((*it).item) != lruCache.end()) {
			lruCache.evict((*it).item);
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

