#pragma once
#include "lru.hpp"
#include "fptree.hpp"
#include"cache.h"

using shadowCache=std::map<Item,uint16_t>;
//using fpCache = std::map<Item, uint16_t>;
using cacheDelta=std::vector<Item>;

struct FPCache
{
private:

	uint64_t maxszie;
	uint64_t ACC_NUM;
	uint64_t HIT_NUM;
	uint64_t PAGE_FAULT_NUM;

	fpCache highCorrCache;
	//fpCache lowCorrCache;
	LRUStack lruCache;

	
	//shadowCache currHighCorrShadowCache;
	
	std::vector<Transaction> accLog;
	uint64_t maxLogSize;

	float highScaleWeight;//高关联度缓存占比
	float lruScaleWeight;// 无关联 LRU
	
	uint64_t highCorrCacheMaxSize;
	uint64_t lruCacheMaxSize;

	uint64_t minimum_support_threshold;
	double min_sup_wet;
	friend std::ostream& operator<<(std::ostream&os, FPCache&fpc) {
		os << "FPC:	ACC:" << fpc.stateACC()
			<< " HIT:" << fpc.stateHIT()
			<< " FAULT:" << fpc.stateFault()
			<< "	hit ratio:" << ((float)fpc.stateHIT() / fpc.stateACC()) * 100 << "%";
		return os;
	}

public:
	FPCache(size_t maxszie, float _highScaleWeight, float _lruScaleWeight);

	//run FP-Growth analyse
	bool runFPAnalyse(std::vector<Transaction> _accLog, std::set<Pattern>& patterns);
	
	void sortPatternsBySup(std::vector<Pattern>& sortedPatterns,std::set<Pattern>& patterns);

	//input:fp pattern 
	//output:high cache,low cache
	bool procPattern(std::vector<Pattern>& patterns, 
		shadowCache& _shadowHigh);

	//get the item lists that should be cached or evicted
	bool getCacheDelta(
		shadowCache& _shadowHigh, 
		cacheDelta& _inHighCache,
		cacheDelta& _outHighCache);


	//set the max size of access log
	void setMaxLogSize(uint64_t _logSize);
	uint64_t getMaxLogSize();

	//set the min support of FP-Growth
	void setMinSupport(uint64_t _minimum_support_threshold);
	void setMinSupportWet(double _min_sup_wet);
	float getMinSupportWet();

	//put item into cache
	bool setHighCorrCacheItem(Item _item);

	//evict item from cache
	bool evictHighCorrCacheItem();

	//risize cache dynamicly
	bool resizeHighCorrCache();
	bool resizeLRU();
	
	//access the Item
	void access(Item _item);

	//is cache empty/full?
	bool isEmpty();
	bool isFull();
	
	//is log full?
	bool logIsFull();

	void appendLogTrans(Transaction _trans);
	fpCache& getHighCorrCache();

	std::vector<Transaction> &getLog() {
		return accLog;
	}

	void cacheOrganize();

	uint64_t stateACC();

	uint64_t stateHIT();

	uint64_t stateFault();



};