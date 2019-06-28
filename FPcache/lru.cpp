#include "lru.hpp"

LRUItem::LRUItem(Item _item)
{
	pre = nullptr;
	item=_item;
	next = nullptr;

}
LRUStack::LRUStack():ACC_NUM(0),
HIT_NUM(0),
PAGE_FAULT_NUM(0), 
maxszie(0),
stacksize(0),
root(nullptr),
tail(nullptr) {}
LRUStack::LRUStack(size_t _maxsize) :
	ACC_NUM(0),
	HIT_NUM(0),
	PAGE_FAULT_NUM(0),
	maxszie(_maxsize),
	stacksize(0),
	root(nullptr),
	tail(nullptr)
{
	ptrvec.clear();
}
void LRUStack::setMaxSize(size_t _maxsize) {
	maxszie = _maxsize;
}
bool LRUStack::isFull()
{
	return (maxszie== stacksize);
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
			ptrvec.erase(temp->item);
			stacksize--;
			free(temp);
			return true;
		}
		else if (stacksize == 1)//tail 前结点不存在（链表只有一个结点）
		{
			root = tail = nullptr;
			//root = temp->pre;
			//tail = temp->pre;
			ptrvec.erase(temp->item);
			stacksize--;
			free(temp);
			return true;
		}
	}
}

bool LRUStack::evict(Item _it)
{
	if (ptrvec.find(_it) != ptrvec.end())//item存在于LRU链表中
	{
		if (ptrvec[_it] != nullptr) {
			if (ptrvec[_it] == root) {//若为头结点
				root = (*ptrvec[_it]).next;
				if ((*ptrvec[_it]).next)//如果不是最后一个结点
					(*ptrvec[_it]).next->pre = nullptr;//就让下一个结点指向前一个节点的指针为null
				else//是最后一个结点
					tail = nullptr;//

				free(ptrvec[_it]);//释放结点
			}
			else if (ptrvec[_it] == tail)//若为尾结点
			{
				if (ptrvec[_it]->pre) {//不是最后一个结点
					tail = ptrvec[_it]->pre;
					ptrvec[_it]->pre->next = nullptr;
				}
				else//最后一个结点
				{
					root = nullptr;
					tail = nullptr;
				}
				free(ptrvec[_it]);
			}
			else if (ptrvec[_it] != this->end())//中间结点
			{
				ptrvec[_it]->pre->next = ptrvec[_it]->next;
				ptrvec[_it]->next->pre = ptrvec[_it]->pre;
				free(ptrvec[_it]);
			}
			ptrvec.erase(_it);
			stacksize--;
			return true;
		}
	}
	else {//并不存在于链表中
		return false;
	}
}

bool LRUStack::pageFault(Item _item)
{
	PAGE_FAULT_NUM++;
	if (isFull()){//满，驱逐
		if (!evict())
			return false;

		LRUItem *nitem = new LRUItem(_item);//申请新节点
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
		
		ptrvec[_item] = nitem;
		stacksize++;
		return true;
	}
	else if (isEmpty()) {//列表为空+1
		LRUItem* item = new LRUItem(_item);

		if (!item)
			return false;

		//item->next = tail;
		/******************/
		item->next = nullptr;
		item->pre = nullptr;
		/******************/
		tail = item;
		root = item;

		ptrvec[_item]= item;
		stacksize++;
		return true;
	}
	else {//不空，不满。
		LRUItem *nitem = new LRUItem(_item);
		if (!nitem)
			return false;

		root->pre = nitem;
		nitem->next = root;
		root = nitem;
		nitem->pre = nullptr;

		ptrvec[_item] = nitem;
		stacksize++;
		return true;
	}
}
void LRUStack::stateReset()
{
	ACC_NUM = 0;
	HIT_NUM = 0;
	PAGE_FAULT_NUM = 0;
}
bool LRUStack::access(Item _item)
{
	ACC_NUM++;
	if (_item!="")
	{
		auto it=ptrvec.find(_item);
		if (it!= ptrvec.end()) {//命中
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
			pageFault(_item);
			return true;
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





