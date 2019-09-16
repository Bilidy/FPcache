#include "RR.h"
RR::RR(uint64_t _size): maxSize(_size), cache(_size),ACC_NUM(0),HIT_NUM(0),PAGE_FAULT_NUM(0)
{
	sizeInByte = 0;
	itemNum = 0;
}

RR::~RR()
{
}

bool RR::isEnough(Entry entry)
{
	return (maxSize-sizeInByte>=entry.size);
}

bool RR::isFull()
{
	return (sizeInByte == maxSize);
}

bool RR::find(Entry entry)
{
	return cache.find(entry.item)!=cache.end();
}

bool RR::isEmpty()
{
	return (sizeInByte == 0);
}

bool RR::evict()
{
	if (!isEmpty())
	{
		int idx = radmGen(0, cache.getItemNum() - 1, 1);
		LRUStack::iterator it = cache.find(idx);
		if (it!=cache.end())
		{
			cache.evict((*it).item);
			sizeInByte -= (*it).size;
			return true;
		}
	}
	return false;
}

void RR::setMaxCacheSize(uint64_t maxcachesize)
{
	maxSize = maxcachesize;
}

uint64_t RR::size()
{
	return sizeInByte;
}

uint64_t RR::getItemNumber()
{
	return cache.getItemNum();
}

void RR::access(Entry entry)
{
	ACC_NUM++;
	if (find(entry))
	{
		HIT_NUM++;
		return;
	}
	else
	{
		PAGE_FAULT_NUM++;
		while (!isEnough(entry))
		{
			if(!evict(1))return;
		}
		cache.inseart(entry);
	}
	return;
}

uint64_t RR::stateACC()
{
	return ACC_NUM;
}

uint64_t RR::stateHIT()
{
	return HIT_NUM;
}

uint64_t RR::stateFault()
{
	return PAGE_FAULT_NUM;
}

void RR::stateReset()
{
	ACC_NUM = 0;
	HIT_NUM = 0;
	PAGE_FAULT_NUM = 0;
}

bool RR::evict(uint64_t num)
{
	for (size_t i = 0; i < num; i++)
	{
		if(!evict())
			return false;
	}
	return true;
}
