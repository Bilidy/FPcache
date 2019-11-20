#pragma once
#include "lru.hpp"
#include "fptree.hpp"
#include "cache.h"
#include "lowCache.h"

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

	uint64_t time = 0;
	uint64_t seqnum = 0;
	uint64_t avgtime = 0;

	fpCache highCorrCache;
	lowCache lowCorrCache;
	LRUStack lruCache;

	
	//shadowCache currHighCorrShadowCache;
	
	std::vector<Transaction> accLog;
	uint64_t maxLogSize;

	float highScaleWeight;//高关联度缓存占比
	float lowScaleWeight;
	float lruScaleWeight;// 无关联 LRU
	
	uint64_t highCorrCacheMaxSize;
	uint64_t lowCorrCacheMaxSize;
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
	int findItemState(Item item);

public:
	FPCache(size_t maxszie, float _highScaleWeight, float _lowScaleWeight, float _lruScaleWeight);

	//run FP-Growth analyse
	bool runFPAnalyse(std::vector<Transaction> _accLog, std::set<Pattern>& patterns);

	void valuatePatterns(std::set<Pattern>& patterns, std::map<Item, metadata>&metadata, std::vector<valuatedPattern>&valuated, std::vector<valuatedPattern>& valuated2);
	
	void sortPatternsBySup(std::vector<Pattern>& sortedPatterns, std::set<Pattern>& patterns);
	void sortPatternsByDensity(std::vector<Pattern>& sortedPatterns, std::map<Item, metadata> &metadata_hashtable, std::vector<valuatedPattern>& patterns);
	void sortPatternsByVal(std::vector<Pattern>& sortedPatterns, std::map<Item, metadata>& metadata_hashtable, std::vector<valuatedPattern>& patterns);
	//void sortPatternsBySup(std::vector<Pattern>& sortedPatterns,std::set<Pattern>& patterns);

	//input:fp pattern 
	//output:high cache,low cache
	bool procPattern(std::vector<Pattern>& patterns, std::vector<valuatedPattern>& patterns2, std::map<Item, metadata> &metadata_hashtable,
		shadowCache& _shadowHigh, shadowCache& _shadowLow);

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
	bool setHighCorrCacheItem(Entry entry);

	//evict item from cache
	bool evictHighCorrCacheItem();

	//risize cache dynamicly
	bool resizeHighCorrCache();
	bool resizeLRU();
	
	//access the Item
	bool access(Entry entry);

	//is cache empty/full?
	bool isEmpty();
	bool isFull();
	
	//is log full?
	bool logIsFull();
	uint64_t getHighItemNum();

	void appendLogTrans(Transaction _trans);
	fpCache& getHighCorrCache();
	fpCache& getLowCorrCache();

	std::vector<Transaction> &getLog() {
		return accLog;
	}

	void cacheOrganize(std::map<Item, metadata> &metadata_hashtable);

	uint64_t stateACC();

	uint64_t stateHIT();

	uint64_t stateFault();

	void timeINC(uint64_t t);
	void seqnumINC();
	uint64_t getAvgtime();
	void restAvgtime();



};