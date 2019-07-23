#include <map>
#include <vector>


using Item = std::string;
using FPcache = std::vector<Item>;
using shadowCache = std::map<Item, uint16_t>;
using cacheType = bool;


 #define H_type true
#define L_type false

struct fpCache {
private:

	uint64_t maxszie;
	//uint64_t size;

	FPcache cache;
	shadowCache ShadowCache;
	cacheType type;
public:

	fpCache(uint64_t _maxszie, cacheType _type);

	uint64_t getCacheSize();
	void setMaxCacheSize(uint64_t _maxszie);

	uint64_t getMaxCacheSize();

	void setShadowCache(shadowCache _ShadowCache);

	shadowCache & getShadowCache();

	FPcache & getCache();
	uint64_t getMaxSize();
	bool isItemInCache(Item _item);

	bool setCacheItem(Item _item);
	bool evictCacheItem(Item _item);

	void stateReset();

	int findItemState(Item _item);

	void access(Item _item);

	void clear();
};