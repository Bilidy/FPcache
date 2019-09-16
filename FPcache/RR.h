#include "lru.hpp"
#ifndef RR_H_
#define RR_H_


class RR
{
public:

	void setMaxCacheSize(uint64_t maxcachesize);
	uint64_t size();
	uint64_t getItemNumber();
	void access(Entry entry);

	uint64_t stateACC();
	uint64_t stateHIT();
	uint64_t stateFault();
	void stateReset();

	RR(uint64_t size);
	~RR();

private:

	uint64_t ACC_NUM;
	uint64_t HIT_NUM;
	uint64_t PAGE_FAULT_NUM;

	uint64_t maxSize;
	uint64_t sizeInByte;
	uint64_t itemNum;
	LRUStack cache;
	friend std::ostream& operator<<(std::ostream&os, RR&rr) {
		os << "RR :	ACC:" << rr.stateACC()
			<< " HIT:" << rr.stateHIT()
			<< " FAULT:" << rr.stateFault()
			<< "	hit ratio:" << ((float)rr.stateHIT() / rr.stateACC()) * 100 << "%";
		return os;
	}
	bool evict(uint64_t num);
	bool isEnough(Entry entry);
	bool isFull();
	bool find(Entry entry);
	bool isEmpty();
	bool evict();
}; 

#endif // RR_H_