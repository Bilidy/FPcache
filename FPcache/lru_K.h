#pragma once

#include "defs.h"
#include "config.h"

#include <map>
#include <list>
#include <limits>
#include <cstdlib>
#include <utility>
#include <vector>
#include <iostream>
#include <functional>
#include <unordered_map>


#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

/*
    Cache has two strategies: LRU-K and LRU (for ambiguous cases)
*/

template <typename Key, typename Value>
class LRU_K_Cache {
    typedef std::list<std::pair<Key, Value>> LruList;
    typedef std::unordered_map<std::string, size_t> ContentSizes;
public:
    LRU_K_Cache() {};
    explicit LRU_K_Cache(size_t size, const size_t & learn_limit = 100,
                            const size_t & period = 1000, const size_t & history_len = 2) :
            cacheSize(size < 1 ? 1 : size),
            currentCacheSize(0)
    {
        correlated_reference_period = learn_limit;
        retained_information_period = period;
        this->history_len = history_len;
    }

    void prepare_cache() {
        return;
    }

    Value* find(const Key &key, const size_t & current_time = 0) {
        auto it = lookup.find(key);

        if (it == lookup.end()) {
            return nullptr;
        }

        // update LRU cache
        Value *value = &promote(it->second)->second;

        // update history information
        size_t &last_request = cid_last_request[key];
        std::vector<size_t> &history = cid_history[key];
        size_t oldest_request_time = history.back();

        if ((current_time - last_request) > correlated_reference_period) {
            /* a new, uncorreleated reference */
            size_t correl_period_of_refd_page = last_request - history[0];
            for (size_t i = (history.size() - 1); i >= 1; --i) {
                history[i] = history[i-1] + correl_period_of_refd_page;
            }
            history[0] = current_time;
            last_request = current_time;

            delete_value_from_multimap(oldest_request_cid_map, oldest_request_time, key);
            oldest_request_cid_map.insert(std::pair<size_t, Key>(history.back(), key));
            cid_oldest_request_map.insert(std::pair<Key, size_t>(key, history.back()));

        } else {
            /* a correlated reference */
            last_request = current_time;
        }

        return value;
    }

    Value* put(const Key &key, const Value &value, const size_t &current_time = 0) {
		contentSizes[key] = 1;
        size_t cidSize = contentSizes[key];
		//size_t cidSize = 1;
        if (cidSize > cacheSize)
            return nullptr;
        
        while ((getCacheSize() + cidSize) >  cacheSize) {
            std::vector<typename LruList::iterator> victims = find_victims(current_time);
            if (victims.size() == 0) {

                // if all elements in correlation period
                // delete elements by LRU-1 strategy
                makeSizeInvariant(cacheSize - cidSize, current_time);

            } else if (victims.size() == 1) {
                typename LruList::iterator it = victims[0];
                Key victim = it->first;
                if (evictionCallback) {
                    evictionCallback(victim, victim, current_time);
                }

                size_t victimSize = contentSizes[victim];
                currentCacheSize -= victimSize;

                lruList.erase(it);
                lookup.erase(victim);

            }
        }

        typename LruList::iterator addedIt = addCidToCache(key, value, current_time);
        return &addedIt->second;
    }

    bool erase(const Key &key) {
        auto it = lookup.find(key);

        if (it == lookup.end()) {
            return false;
        }

        size_t cidSize = contentSizes[key];
        currentCacheSize -= cidSize;

        lruList.erase(it->second);
        lookup.erase(it);

        std::vector<size_t> &history = cid_history[key];
        size_t oldest_request_time = history.back();
        delete_value_from_multimap(oldest_request_cid_map, oldest_request_time, key);
        return true;
    }

    void setEvictionCallback(std::function<void(const Key &,const Value &, const size_t & current_time)> callback) {
        evictionCallback = callback;
    }

    size_t size() const {
        return cacheSize;
    }

    size_t elementsCount() const {
        return lookup.size();
    }

    void setCacheSize(size_t size) {
        cacheSize = size;
        makeSizeInvariant(cacheSize);
    }

    const std::pair<Key, Value> *mruItem() const {
        return &lruList.back();
    }

    const std::pair<Key, Value> *lruItem() const {
        return &lruList.front();
    }

    ContentSizes getContentSizes() {
        return contentSizes;
    }

    size_t getCacheSize() {
        return currentCacheSize;
    }

    void addCidSize(std::string cid, size_t size) {
        ContentSizes::iterator it = contentSizes.find(cid);
        if (it != contentSizes.end() && it->second != size) {
            std::cout << "Another size for content. Was -> " <<  it->second
                << " now -> "<< size 
                << " for cid -> " << cid << std::endl;
        }

        contentSizes[cid] = size;
    }

    VecStr get_hot_content(const float &cache_hot_content) {
        VecStr hot_content;
        int curr_count = 0;
        int count = MAX((int)(cache_hot_content*elementsCount()), 1);
        typename std::list<std::pair<Key, Value>>::reverse_iterator it = 
                                            lruList.rbegin();
        for (; it != lruList.rend() && curr_count++ < count; ++it) {
            hot_content.push_back(it->first);
        }

        return hot_content;
    }

private:
    void makeSizeInvariant(size_t size, const size_t & current_time = 0) {
        while (getCacheSize() > size) {
            if (evictionCallback) {
                evictionCallback(lruList.front().first, 
                                 lruList.front().second, 
                                 current_time);
            }

            size_t cidSize = contentSizes[lruList.front().first];
            currentCacheSize -= cidSize;

            lookup.erase(lruList.front().first);

            lruList.pop_front();
        }
    }

    void delete_value_from_multimap(typename std::multimap<size_t, Key> &mmap,
                                    const size_t &key, 
                                    const Key &value) {
        std::pair <typename std::multimap<size_t, Key>::iterator, 
                   typename std::multimap<size_t, Key>::iterator > ret;
        ret = mmap.equal_range(key);
        typename std::multimap<size_t, Key>::iterator it;
        for (it = ret.first; it != ret.second; ++it) {
            if (it->second == value) {
                mmap.erase(it);
                break;
            }
        }
    }

    typename LruList::iterator addCidToCache(const Key & key, 
                                             const Value & value, 
                                             const size_t & current_time) {
        size_t cidSize = contentSizes[key];
        lruList.push_back(std::make_pair(key, value));
        auto addedIt = --lruList.end();
        lookup[key] = addedIt;
        currentCacheSize += cidSize;

        // update history for old cid
        // or add history for new cid 

        auto it = cid_history.find(key);
        if (it == cid_history.end()) {
            std::vector<size_t> v(history_len, 0);
            cid_history[key] = v;
            oldest_request_cid_map.insert(std::pair<size_t, Key>(0, key));;
            cid_oldest_request_map[key] = 0;
        } else {
            size_t oldest_request_time = cid_history[key].back();
            for (size_t i = (history_len - 1); i >= 1; --i) {
                cid_history[key][i] = cid_history[key][i-1];
            }
            delete_value_from_multimap(oldest_request_cid_map, oldest_request_time, key);
            oldest_request_cid_map.insert(std::pair<size_t, Key>(cid_history[key].back(), key));
            cid_oldest_request_map[key] = cid_history[key].back();
        }

        cid_history[key][0] = current_time;
        cid_last_request[key] = current_time;
        return addedIt;
    }

    std::vector<typename LruList::iterator> find_victims(const size_t & current_time) {
        // return null, one cid with minimal HIST(cid, k)
        std::vector<typename LruList::iterator> victims;
        
        typename LruList::iterator cid1, cid2;
        cid1 = cid2 = lruList.end();

        typename std::multimap<size_t, Key>::iterator it;
        for (it = oldest_request_cid_map.begin(); it != oldest_request_cid_map.end(); ++it) {
            Key cid = it->second;
            if (lookup.find(cid) == lookup.end())
                continue;

            size_t last_request = cid_last_request[cid];
            if ((current_time - last_request) <= correlated_reference_period)
                continue;

            // cid is out of corr_ref_period
            cid1 = lookup[cid];
            break;
        }

        if (cid1 != lruList.end()) {
            oldest_request_cid_map.erase(it);
            victims.push_back(cid1);
        }
        
        if (cid2 != lruList.end()) {
            oldest_request_cid_map.erase(it);
            victims.push_back(cid2);
        }

        return victims;
    }

    typename LruList::iterator promote(typename LruList::iterator it) {
        lruList.push_back(*it);

        auto addedIt = --lruList.end();
        lookup[it->first] = addedIt;

        lruList.erase(it);

        return addedIt;
    }

// private:
public:
    LruList lruList;
    std::unordered_map<Key, typename LruList::iterator> lookup;
    std::function<void(const Key &,const Value &, const size_t & current_time)> evictionCallback;


    size_t cacheSize;
    size_t currentCacheSize;
    size_t correlated_reference_period;
    size_t retained_information_period;
    
    // history_len is a parameter K in LRU-K
    size_t history_len;
    
    // time of last request to the content
    std::unordered_map<Key, size_t> cid_last_request;

    // history of requests to the cid
    // cid_history[0] - last request
    // cid_history[1] - penultimate request
    // and etc.
    std::unordered_map<Key, std::vector<size_t>> cid_history;

    // data structures for search cids with minimal oldest request
    std::multimap<size_t, Key> oldest_request_cid_map;
    std::unordered_map<Key, size_t> cid_oldest_request_map;

    ContentSizes contentSizes;
};
