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

		if (tail->pre) {//tailǰ������
			tail->pre->next = tail->next;
			tail = tail->pre;
			ptrvec.erase(temp->item);
			stacksize--;
			free(temp);
			return true;
		}
		else if (stacksize == 1)//tail ǰ��㲻���ڣ�����ֻ��һ����㣩
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
	if (ptrvec.find(_it) != ptrvec.end())//item������LRU������
	{
		if (ptrvec[_it] != nullptr) {
			if (ptrvec[_it] == root) {//��Ϊͷ���
				root = (*ptrvec[_it]).next;
				if ((*ptrvec[_it]).next)//����������һ�����
					(*ptrvec[_it]).next->pre = nullptr;//������һ�����ָ��ǰһ���ڵ��ָ��Ϊnull
				else//�����һ�����
					tail = nullptr;//

				free(ptrvec[_it]);//�ͷŽ��
			}
			else if (ptrvec[_it] == tail)//��Ϊβ���
			{
				if (ptrvec[_it]->pre) {//�������һ�����
					tail = ptrvec[_it]->pre;
					ptrvec[_it]->pre->next = nullptr;
				}
				else//���һ�����
				{
					root = nullptr;
					tail = nullptr;
				}
				free(ptrvec[_it]);
			}
			else if (ptrvec[_it] != this->end())//�м���
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
	else {//����������������
		return false;
	}
}

bool LRUStack::pageFault(Item _item)
{
	PAGE_FAULT_NUM++;
	if (isFull()){//��������
		if (!evict())
			return false;

		LRUItem *nitem = new LRUItem(_item);//�����½ڵ�
		if (!nitem)
			return false;
		if (root)//���ں������
		{
			root->pre = nitem;
			nitem->next = root;
			nitem->pre = nullptr;
			root = nitem;
		}
		else {//�����ں������
			nitem->next = nullptr;
			nitem->pre = nullptr;
			root = nitem;
			tail = nitem;
		}
		
		ptrvec[_item] = nitem;
		stacksize++;
		return true;
	}
	else if (isEmpty()) {//�б�Ϊ��+1
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
	else {//���գ�������
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
		if (it!= ptrvec.end()) {//����
			HIT_NUM++;
			if ((*it).second == root) {//����ͷ�ڵ㣬����������
				return true;
			}
			if ((*it).second == tail) {//����β���
				
				if ((*it).second->pre) {//����ǰ����㣬����˵������ֻ��һ�����
					(*it).second->pre->next = nullptr;
					//(*it).second->pre->next = (*it).second->next;
					tail = (*it).second->pre;
					(*it).second->pre = nullptr;
					(*it).second->next = root;
					root->pre = (*it).second;
					root = (*it).second;
					return true;
				}
				else {//������ǰ����㣬����˵ֻ��һ����㣬���账��
					return true;
				}
			}
			else//�����������
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





