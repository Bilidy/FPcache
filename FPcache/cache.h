#include <map>
#include <vector>
#include "lru.hpp"


using Item = std::string;
using FPcache = std::vector<Item>;
using shadowCache = std::map<Item, uint16_t>;
using cacheType = bool;

struct fpCache {
private:

	uint64_t maxszie;
	//uint64_t size;

	//FPcache cache;
	shadowCache ShadowCache;
	shadowCache CacheOrgNum;
	cacheType type;

	LRUStack cache;

public:

	fpCache(uint64_t _maxszie, cacheType _type);

	uint64_t getCacheSize();
	void setMaxCacheSize(uint64_t _maxszie);

	uint64_t getMaxCacheSize();

	void setShadowCache(const shadowCache _ShadowCache);

	shadowCache & getShadowCache();

	LRUStack & getCache();
	uint64_t getMaxSize();
	bool isItemInCache(Item _item);

	bool setCacheItem(Entry entry);
	bool evictCacheItem(Item _item);
	void orgnaize(std::map<Item, metadata> &metadata_hashtable);

	int findItemState(Item _item);

	void access(Entry entry);

	void clear();
};