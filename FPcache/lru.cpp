#include "lru.hpp"


LRUItem::LRUItem(Item _item,uint64_t _size)
{
	pre = nullptr;
	item=_item;
	size = _size;
	next = nullptr;
}
LRUItem::LRUItem(Entry entry)
{
	pre = nullptr;
	item = entry.item;
	size = entry.size;
	next = nullptr;
}
LRUStack::LRUStack():ACC_NUM(0),
HIT_NUM(0),
PAGE_FAULT_NUM(0), 
maxsize(0),
stacksize(0),
root(nullptr),
tail(nullptr) {}
LRUStack::LRUStack(size_t _maxsize) :
	ACC_NUM(0),
	HIT_NUM(0),
	PAGE_FAULT_NUM(0),
	maxsize(_maxsize),
	stacksize(0),
	root(nullptr),
	tail(nullptr)
{
	ptrvec.clear();
}
void LRUStack::setMaxSize(size_t _maxsize) {
	maxsize = _maxsize;
	//std::cout << "after" << maxszie << std::endl;
}

bool LRUStack::isFull()
{
	return (maxsize == stacksize);
}
bool LRUStack::isEnough(Entry entry) {
	return (maxsize - stacksize) >= entry.size;
}

bool LRUStack::isEmpty()
{
	return (stacksize==0);
}



bool LRUStack::evict()
{
	if(!tail)//tail is a nullpter
		return false;

	else
	{
		LRUItem* temp= tail;

		if (tail->pre) {//tail前结点存在
			tail->pre->next = tail->next;
			tail = tail->pre;

			stacksize -= ptrvec[temp->item]->size;
			ptrvec.erase(temp->item);
			
			free(temp);
			return true;
		}
		else if (ptrvec.size() == 1)//tail 前结点不存在（链表只有一个结点）
		{
			root = tail = nullptr;
			//root = temp->pre;
			//tail = temp->pre;
			stacksize -= ptrvec[temp->item]->size;
			ptrvec.erase(temp->item);
			free(temp);
			return true;
		}
	}
}

bool LRUStack::evict(Item item)
{
	if (ptrvec.find(item) != ptrvec.end())//item存在于LRU链表中
	{
		if (ptrvec[item] != nullptr) {
			if (ptrvec[item] == root) {//若为头结点
				root = (*ptrvec[item]).next;
				if ((*ptrvec[item]).next)//如果不是最后一个结点
					(*ptrvec[item]).next->pre = nullptr;//就让下一个结点指向前一个节点的指针为null
				else//是最后一个结点
					tail = nullptr;//

				stacksize -= ptrvec[item]->size;
				free(ptrvec[item]);//释放结点
				ptrvec.erase(item);
			}
			else if (ptrvec[item] == tail)//若为尾结点
			{
				if (ptrvec[item]->pre) {//不是最后一个结点
					tail = ptrvec[item]->pre;
					ptrvec[item]->pre->next = nullptr;
				}
				else//最后一个结点
				{
					root = nullptr;
					tail = nullptr;
				}

				stacksize -= ptrvec[item]->size;
				free(ptrvec[item]);
				ptrvec.erase(item);
			}
			else if (ptrvec[item] != this->end())//中间结点
			{
				ptrvec[item]->pre->next = ptrvec[item]->next;
				ptrvec[item]->next->pre = ptrvec[item]->pre;

				stacksize -= ptrvec[item]->size;
				free(ptrvec[item]);
				ptrvec.erase(item);
			}
			
			return true;
		}
	}
	else {//并不存在于链表中
		return false;
	}
}

bool LRUStack::evict(int _num)
{
	if (_num>ptrvec.size())
	{
		return false;
	}
	for (size_t i = 0; i < _num; i++)
	{
		evict();
	}
	return true;
}

bool LRUStack::inseart(Entry entry) {
	if (!isEnough(entry)){//满，驱逐
		while (!isEnough(entry))
		{
			if (!evict())
				return false;
		}

		LRUItem *nitem = new LRUItem(entry);//申请新节点
		if (!nitem)
			return false;
		if (root)//存在后续结点
		{
			root->pre = nitem;
			nitem->next = root;
			nitem->pre = nullptr;
			root = nitem;
		}
		else {//不存在后续结点
			nitem->next = nullptr;
			nitem->pre = nullptr;
			root = nitem;
			tail = nitem;
		}

		ptrvec[entry.item] = nitem;
		stacksize+= entry.size;
		return true;
	}
	else if (isEmpty()) {//列表为空+1
		LRUItem* item = new LRUItem(entry);

		if (!item)
			return false;

		//item->next = tail;
		/******************/
		item->next = nullptr;
		item->pre = nullptr;
		/******************/
		tail = item;
		root = item;

		ptrvec[entry.item] = item;
		stacksize+= entry.size;
		return true;
	}
	else {//不空，不满。
		LRUItem *nitem = new LRUItem(entry);
		if (!nitem)
			return false;

		root->pre = nitem;
		nitem->next = root;
		root = nitem;
		nitem->pre = nullptr;

		ptrvec[entry.item] = nitem;
		stacksize+= entry.size;
		return true;
	}
}
bool LRUStack::pageFault(Entry entry)
{
	PAGE_FAULT_NUM++;
	if (!isEnough(entry)){//满，驱逐
		//
		while (!isEnough(entry))
		{
			if (!evict())
				return false;
		}
		LRUItem *nitem = new LRUItem(entry);//申请新节点
		if (!nitem)
			return false;
		if (root)//存在后续结点
		{
			root->pre = nitem;
			nitem->next = root;
			nitem->pre = nullptr;
			root = nitem;
		}
		else {//不存在后续结点
			nitem->next = nullptr;
			nitem->pre = nullptr;
			root = nitem;
			tail = nitem;
		}
		
		ptrvec[entry.item] = nitem;
		stacksize= stacksize+entry.size;
		return true;
	}
	else if (isEmpty()) {//列表为空+1
		LRUItem* item = new LRUItem(entry);

		if (!item)
			return false;

		//item->next = tail;
		/******************/
		item->next = nullptr;
		item->pre = nullptr;
		/******************/
		tail = item;
		root = item;

		ptrvec[entry.item] = item;
		stacksize+= entry.size;
		return true;
	}
	else {//不空，不满。
		LRUItem *nitem = new LRUItem(entry);
		if (!nitem)
			return false;

		root->pre = nitem;
		nitem->next = root;
		root = nitem;
		nitem->pre = nullptr;

		ptrvec[entry.item] = nitem;
		stacksize+=entry.size;
		return true;
	}
}
void LRUStack::stateReset()
{
	ACC_NUM = 0;
	HIT_NUM = 0;
	PAGE_FAULT_NUM = 0;
}
bool LRUStack::access(Entry entry)
{
	ACC_NUM++;
	if (entry.item != "")
	{
		auto it = ptrvec.find(entry.item);
		if (it != ptrvec.end()) {//命中
			HIT_NUM++;
			if ((*it).second == root) {//命中头节点，不用做处理
				return true;
			}
			if ((*it).second == tail) {//命中尾结点

				if ((*it).second->pre) {//存在前驱结点，或者说并不是只有一个结点
					(*it).second->pre->next = nullptr;
					//(*it).second->pre->next = (*it).second->next;
					tail = (*it).second->pre;
					(*it).second->pre = nullptr;
					(*it).second->next = root;
					root->pre = (*it).second;
					root = (*it).second;
					return true;
				}
				else {//不存在前驱结点，或者说只有一个结点，无需处理
					return true;
				}
			}
			else//命中其他结点
			{
				(*it).second->pre->next = (*it).second->next;
				(*it).second->next->pre = (*it).second->pre;

				root->pre = (*it).second;
				(*it).second->pre = nullptr;
				(*it).second->next = root;
				root->pre = (*it).second;
				root = it->second;
				return true;
			}//LRUItem Item=(*(*it).second);			
		}
		else {
			pageFault(entry);
			return true;
		}
	}
	else
		return false;
}
//bool LRUStack::access(Item _item)
//{
//	ACC_NUM++;
//	if (_item!="")
//	{
//		auto it=ptrvec.find(_item);
//		if (it!= ptrvec.end()) {//命中
//			HIT_NUM++;
//			if ((*it).second == root) {//命中头节点，不用做处理
//				return true;
//			}
//			if ((*it).second == tail) {//命中尾结点
//				
//				if ((*it).second->pre) {//存在前驱结点，或者说并不是只有一个结点
//					(*it).second->pre->next = nullptr;
//					//(*it).second->pre->next = (*it).second->next;
//					tail = (*it).second->pre;
//					(*it).second->pre = nullptr;
//					(*it).second->next = root;
//					root->pre = (*it).second;
//					root = (*it).second;
//					return true;
//				}
//				else {//不存在前驱结点，或者说只有一个结点，无需处理
//					return true;
//				}
//			}
//			else//命中其他结点
//			{
//				(*it).second->pre->next = (*it).second->next;
//				(*it).second->next->pre = (*it).second->pre;
//
//				root->pre = (*it).second;
//				(*it).second->pre = nullptr;
//				(*it).second->next = root;
//				root->pre = (*it).second;
//				root = it->second;
//				return true;
//			}//LRUItem Item=(*(*it).second);			
//		}
//		else {
//			pageFault(_item);
//			return true;
//		}
//	}
//	else
//		return false;
//}
bool LRUStack::touch(Item item)
{
	if (item != "")
	{
		auto it = ptrvec.find(item);
		if (it != ptrvec.end()) {
			if ((*it).second == root) {//头节点，不用做处理
				return true;
			}
			if ((*it).second == tail) {//尾结点

				if ((*it).second->pre) {//存在前驱结点，或者说并不是只有一个结点
					(*it).second->pre->next = nullptr;
					//(*it).second->pre->next = (*it).second->next;
					tail = (*it).second->pre;
					(*it).second->pre = nullptr;
					(*it).second->next = root;
					root->pre = (*it).second;
					root = (*it).second;
					return true;
				}
				else {//不存在前驱结点，或者说只有一个结点，无需处理
					return true;
				}
			}
			else//命中其他结点
			{
				(*it).second->pre->next = (*it).second->next;
				(*it).second->next->pre = (*it).second->pre;

				root->pre = (*it).second;
				(*it).second->pre = nullptr;
				(*it).second->next = root;
				root->pre = (*it).second;
				root = it->second;
				return true;
			}//LRUItem Item=(*(*it).second);			
		}
		else {
			//pageFault(entry);
			return false;
		}
	}
	else
		return false;
}
uint64_t LRUStack::stateACC()
{
	return ACC_NUM;
}

uint64_t LRUStack::stateHIT()
{
	return HIT_NUM;
}

uint64_t LRUStack::stateFault()
{
	return PAGE_FAULT_NUM;
}

void LRUStack::flush()
{
	LRUItem* pPtr = root;
	LRUItem* tempPtr = nullptr;
	while (pPtr)
	{
		tempPtr = pPtr->next;
		free(pPtr);
		pPtr=tempPtr;
		stacksize--;
	}
}

size_t LRUStack::getCacheSize()
{
	return stacksize;
}

size_t LRUStack::getMaxSize()
{
	return maxsize;
}

LRUStack::iterator  LRUStack::find(Item _item) {
	auto it = this->begin();
	while (it != this->end()) {
		if ((*it).item != _item) {
			
		}
		else
		{
			return it;
		}
		it = it->next;
	}
	return it;
}





