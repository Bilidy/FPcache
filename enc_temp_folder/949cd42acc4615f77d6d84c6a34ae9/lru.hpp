#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>

using Item = std::string;

struct LRUItem 
{
private:
	
public:
	LRUItem* pre;
	Item item;
	LRUItem* next;
	LRUItem(Item item);
};
struct LRUStack
{

	struct iterator
	{
	private:
		LRUItem * ptr;
	public:
		iterator() :ptr(nullptr) {};
		iterator(LRUItem * ptr) :ptr(ptr) {};
		LRUItem operator*() { return *ptr; };
		iterator operator++(int) {
			iterator temp = *this;
			ptr = ptr->next;
			return temp;
		};
		bool operator!=(iterator _it) {
			return (ptr!= _it.ptr);
		}
		bool operator==(iterator _it) {
			return (ptr == _it.ptr);
		}
		
	};
private:
	uint64_t ACC_NUM;
	uint64_t HIT_NUM;
	uint64_t PAGE_FAULT_NUM;

	size_t maxszie;//最大大小限制
	size_t stacksize;

	LRUItem* root;//头指针交于智能指针托管
	LRUItem* tail;//尾指针
	std::map<Item, LRUItem*> ptrvec;	
public:

	LRUStack();
	LRUStack(size_t _maxsize);

	void setMaxSize(size_t _maxsize);
	bool access(Item item);

	uint64_t stateACC();
	uint64_t stateHIT();
	uint64_t stateFault();

	void flush();

	size_t getCacheSize();

	iterator find(Item _item);

	LRUItem * begin() { return root; };
	LRUItem * end() { return nullptr; };

	//iterator find(Item _item);
	bool evict(Item _it);
	bool evict(int _num);
	friend std::ostream& operator<<(std::ostream&os, LRUStack&lru) {
		os << "LRU:	ACC:" << lru.stateACC()
			<< " HIT:" << lru.stateHIT()
			<< " FAULT:" << lru.stateFault()
			<< "	hit ratio:" << ((float)lru.stateHIT() / lru.stateACC()) * 100 << "%" ;
		return os;
	}
private:

	bool isFull();
	bool isEmpty();

	bool evict();

	bool pageFault(Item item);

	void stateReset();
};