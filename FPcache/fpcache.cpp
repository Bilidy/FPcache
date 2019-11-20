#include "fpcache.hpp"
#include <iostream>

int FPCache::findItemState(Item item)
{
	if(highCorrCache.isItemInCache(item))
		return 1;
	if (lowCorrCache.isItemInCache(item))
		return 2;
	else
		return 3;
}

FPCache::FPCache(size_t _maxszie, float _highScaleWeight, float _lowScaleWeight, float _lruScaleWeight) :
	highCorrCache(0, true), lowCorrCache(0,true), lruCache(0), maxszie(_maxszie), highScaleWeight(_highScaleWeight), 
	lowScaleWeight(_lowScaleWeight),lruScaleWeight(_lruScaleWeight), 
	highCorrCacheMaxSize(0),lowCorrCacheMaxSize(0),lruCacheMaxSize(0),
	minimum_support_threshold(0), maxLogSize(0), min_sup_wet(0.02)
{
	float sum=_highScaleWeight+_lowScaleWeight + _lruScaleWeight;
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
void FPCache::valuatePatterns(std::set<Pattern>& patterns, std::map<Item, metadata>&metadata, std::vector<valuatedPattern>&valuated,std::vector<valuatedPattern>& valuated2)
{
	valuated.clear();
	std::set<Pattern>::iterator it = patterns.begin();
	while (it != patterns.end())
	{
		if ((*it).first.size() < 2) { it++; continue; }
		double sum = 0;//均值
		std::set<Item>::iterator its = (*it).first.begin();
		while (its != (*it).first.end())
		{
			sum += metadata[(*its)].weidis;
			//std::cout << metadata[(*its)].weidis<<",";
			its++;
		}
	//	std::cout << std::endl;

		double meanDis = sum / (*it).first.size();

		double vansum = 0;//方差
		its = (*it).first.begin();
		while (its != (*it).first.end())
		{
			uint64_t X= metadata[(*its)].weidis;
			vansum += (X- meanDis)*(X - meanDis);
			its++;
		}
		double variance = vansum / ((*it).first.size());

		uint64_t totalsize = 0;
		its = (*it).first.begin();
		while (its != (*it).first.end())
		{
			uint64_t X = metadata[(*its)].size;
			totalsize += X;
			its++;
		}

		uint64_t accnum = 0;
		its = (*it).first.begin();
		//std::cout << "{";
		while (its != (*it).first.end())
		{
			uint64_t X = metadata[(*its)].accnum;
			//std::cout << metadata[(*its)].accnum << ",";
			accnum += X;
			its++;
		}
		double accMean = accnum/ (*it).first.size();//计数均值

		double vanAccSum = 0;//访问计数方差
		its = (*it).first.begin();
		while (its != (*it).first.end())
		{
			uint64_t X = metadata[(*its)].accnum;
			vanAccSum += (X - accMean)*(X - accMean);
			its++;
		}
		double accVariance = vanAccSum / ((*it).first.size());//计数方差


		//std::cout << "}"<<std::endl;
		metaPattern m;

		m.Spatial_mean = meanDis;
		m.Temporal_mean = accMean;
		
		m.sup = (*it).second;

		m.Spatial_var = sqrt(variance);
		m.Temporal_var = sqrt(accVariance);

		m.size = totalsize;
		m.accnum = accnum;
		m.accden = ((double)accnum) / m.size;
		
		//if ((double)m.var / m.mean <1.0&&m.var!=0)
		if (m.Spatial_mean*m.Temporal_mean != 0&& (double)m.Spatial_var / m.Spatial_mean <1.0 && (double)m.Temporal_var / m.Temporal_mean <1.0)
		{
			valuatedPattern valuatedpattern;//{ {(*it).first} ,m };
			valuatedpattern.first = (*it).first;
			//m.val = (m.mean*m.mean)/m.var;
			//m.val = (m.sup * 10 * (m.mean / m.var)+(1.0/m.sup)*(10 * (m.mean / m.var) + m.sup )*(10 * (m.mean / m.var) + m.sup));

			double cv_x = (double)m.Spatial_var / m.Spatial_mean;
			double cv_y = (double)m.Temporal_var / m.Temporal_mean;

			m.val = (((cv_x) * (cv_y)) - 0.15 * (( cv_x) + (cv_y)) * ((cv_x) + (cv_y)));
			//m.val = (((cv_x) * (1.0 - cv_y)) - 0.15 * ((cv_x) + (cv_y)) * ((cv_x) + (cv_y)));
			extern double Threshold;
			extern double ProtectThreshold;
			if (m.val < (Threshold) && m.val>0)
			{
				valuatedpattern.second = m;
				valuated.push_back(valuatedpattern);
			}
			else if (m.val < ProtectThreshold && m.val>0) {
				valuatedpattern.second = m;
				valuated2.push_back(valuatedpattern);
			}
		}
		
		it++;
	}

}
//sort Patterns By Support form big to little
void FPCache::sortPatternsByDensity(std::vector<Pattern>& sortedPatterns, std::map<Item, metadata> &metadata_hashtable, std::vector<valuatedPattern>& patterns)
{
	sortedPatterns.clear();
	double MaxDensity;
	Pattern tempTrans;
	
	shadowCache tempScache;
	uint64_t sizesum = 0;

	//while (patterns.size() && sortedPatterns .size()<=2000&& (sizesum < highCorrCache.getMaxCacheSize()))
	while (patterns.size() && (sizesum < highCorrCache.getMaxCacheSize()))
	{
		
		MaxDensity = 0.0;
		auto tempit = patterns.end();
		auto it = patterns.begin();
		while (it != patterns.end())
		{
			if ((*it).second.accden >= MaxDensity)
			{
				tempit = it;
				tempTrans.first = (*it).first;
				tempTrans.second = (*it).second.accden;
				MaxDensity = (*it).second.accden;
			}
			it++;
		}
		if (tempTrans.first.size() > 1)//至少是2模式
		{
			sortedPatterns.push_back(tempTrans);
			auto its = tempTrans.first.begin();
			while (its != tempTrans.first.end())
			{
				if (tempScache.find(*its) == tempScache.end()) {
					tempScache.insert(std::pair<Item,uint64_t>((*its), 0));
					sizesum += metadata_hashtable[*its].size;
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
void FPCache::sortPatternsByVal(std::vector<Pattern>& sortedPatterns, std::map<Item, metadata>& metadata_hashtable, std::vector<valuatedPattern>& patterns)
{
	sortedPatterns.clear();
	double MinValue;
	Pattern tempTrans;

	shadowCache tempScache;
	uint64_t sizesum = 0;

	//while (patterns.size() && sortedPatterns.size() <= 20000 && (sizesum < highCorrCache.getMaxCacheSize()))
	while (patterns.size() && (sizesum < highCorrCache.getMaxCacheSize()))
	{

		MinValue = 5.0;
		auto tempit = patterns.end();
		auto it = patterns.begin();
		while (it != patterns.end())
		{
			if ((*it).second.val <= MinValue)
			{
				tempit = it;
				tempTrans.first = (*it).first;
				tempTrans.second = (*it).second.val;
				MinValue = (*it).second.val;
			}
			it++;
		}
		if (tempTrans.first.size() > 1)//至少是2模式
		{
			sortedPatterns.push_back(tempTrans);
			auto its = tempTrans.first.begin();
			while (its != tempTrans.first.end())
			{
				if (tempScache.find(*its) == tempScache.end()) {
					tempScache.insert(std::pair<Item, uint64_t>((*its), 0));
					sizesum += metadata_hashtable[*its].size;
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
void FPCache::sortPatternsBySup(std::vector<Pattern>& sortedPatterns, std::set<Pattern>& patterns)
{
	sortedPatterns.clear();
	uint64_t MaxSupport = minimum_support_threshold;
	Pattern tempTrans;

	shadowCache tempScache;
	uint64_t sizesum = 0;

	while (patterns.size() && (sizesum < highCorrCache.getMaxCacheSize()))
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
					tempScache.insert(std::pair<Item, uint64_t>((*its), 0));
					sizesum += tempit->second;
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
//规则内部的各项重用距离和min重用距离之差之和，访问频率和文件大小
bool FPCache::procPattern(std::vector<Pattern>& patterns, std::vector<valuatedPattern> & patterns2, std::map<Item, metadata> &metadata_hashtable, shadowCache& _shadowHigh, shadowCache& _shadowLow)
{
	_shadowHigh.clear();
	int sum = 0;
	auto it = patterns.begin();
	while (it != patterns.end() && sum < highCorrCache.getMaxCacheSize())
	{
		auto its = (*it).first.begin();
		while (its != (*it).first.end())
		{
			if (_shadowHigh.find(*its) == _shadowHigh.end()) {
				_shadowHigh[*its] = 0;
				sum += metadata_hashtable[*its].size;
			}
			its++;
		}
		it++;
	}
	sum = 0;
	auto itt = patterns2.begin();
	while (itt != patterns2.end() && sum < lowCorrCache.getMaxCacheSize())
	{
		auto its = (*itt).first.begin();
		while (its != (*itt).first.end())
		{
			if (_shadowLow.find(*its) == _shadowLow.end()) {
				_shadowLow[*its] = 0;
				sum += metadata_hashtable[*its].size;
			}
			its++;
		}
		itt++;
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
bool FPCache::setHighCorrCacheItem(Entry entry)
{
	//LRUStack::iterator it = lruCache.begin();
	
	if (lruCache.find(entry.item) !=lruCache.end()) {
		if (lruCache.evict(entry.item))
		{
			if (highCorrCache.setCacheItem(entry)) {
				return true;
			};
		}
		return false;
	}
	else
	{
		highCorrCache.setCacheItem(entry);
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
	size_t lrusize = maxszie - highCorrCache.getCacheSize()- lowCorrCache.getCacheSize();
	if (lrusize<lruCache.getCacheSize())
	{
		while (lrusize < lruCache.getCacheSize()) {
			if (!lruCache.evict(1))
				return false;
		}
		/*if (lruCache.evict(lruCache.getCacheSize() - lrusize)) {
			lruCache.setMaxSize(lrusize);
			return true;
		}
		return false;*/
	}
	lruCache.setMaxSize(lrusize);
	return true;
}

bool FPCache::access(Entry entry)
{
	ACC_NUM++;
	int stats=highCorrCache.findItemState(entry.item);
	switch (stats)
	{
	case 1://should in the cache and in the cache actually
		highCorrCache.access(entry);
		HIT_NUM++;
		return true;
	default:
		break;
	}
	stats = lowCorrCache.findItemState(entry.item);
	switch (stats)
	{
	case 1://should in the cache and in the cache actually
		lowCorrCache.access(entry);
		HIT_NUM++;
		return true;
	case 2://could in the cache but not
		lowCorrCache.access(entry);

		if (lruCache.find(entry.item) != lruCache.end()) {
			lruCache.evict(entry.item);
		}
		PAGE_FAULT_NUM++;
		return true;
	default:
		break;
	}
	resizeLRU();
	return lruCache.access(entry);
}

bool FPCache::isEmpty()
{
	return 0 == highCorrCache.getCacheSize() + lowCorrCache.getCacheSize()+
		lruCache.getCacheSize();
}

bool FPCache::isFull()
{
	return highCorrCache.getCacheSize() + lowCorrCache.getCacheSize() + 
		lruCache.getCacheSize() == maxszie;
}

bool FPCache::logIsFull()
{
	return accLog.size() == maxLogSize;
}

uint64_t FPCache::getHighItemNum()
{
	return highCorrCache.itemNumber();
}

void FPCache::appendLogTrans(Transaction _trans)
{
	accLog.push_back(_trans);
}

fpCache & FPCache::getHighCorrCache()
{
	return highCorrCache;
}
fpCache& FPCache::getLowCorrCache()
{
	// TODO: 在此处插入 return 语句
	return lowCorrCache;
}
void FPCache::cacheOrganize(std::map<Item, metadata> &metadata_hashtable)
{
	lowCorrCache.orgnaize();
	LRUStack::iterator it = lowCorrCache.getCache().begin();
	while (it != lowCorrCache.getCache().end())
	{
		if (lruCache.find((*it).item) != lruCache.end()) {
			lruCache.evict((*it).item);
		}
		it++;
	}
	highCorrCache.orgnaize(metadata_hashtable);
	it = highCorrCache.getCache().begin();
	while (it != highCorrCache.getCache().end())
	{
		if (lowCorrCache.isItemInCache((*it).item)) {
			lowCorrCache.evictCacheItem((*it).item);
		}
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

void FPCache::timeINC(uint64_t t)
{
	time += t;
}

void FPCache::seqnumINC()
{
	seqnum++;
}

uint64_t FPCache::getAvgtime()
{

	return time / seqnum;
}

void FPCache::restAvgtime()
{
	time = 0;
	seqnum = 0;
	avgtime = 0;
}

