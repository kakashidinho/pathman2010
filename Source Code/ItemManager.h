#ifndef _ITEM_MAN_
#define _ITEM_MAN_
#include <list>
#include "SharedPointer.h"

#define MAX_ID 0xcdcdcdcd
#define INVALID_ID 0xcdcdcdcd
#define ID_NOT_FOUND -2
/*-------------------------------------------------
template class ItemManager - manages list of items
------------------------------------------------*/
template <class T>
class ItemManager
{
public:
	/*------iterator class - for traversing through list of items in this manager---*/
	class Iterator
	{
		friend class ItemManager;
	private:
		SharedPtr<T>** slots;
		unsigned int currentPos;
		unsigned int maxPos;
	public:
		Iterator& operator ++();//prefix addition
		Iterator operator ++(int);//suffic addition

		Iterator& operator --();//prefix subtraction
		Iterator operator --(int);//suffix subtraction
		
		bool isAtBegin(){return (currentPos == 0);};
		bool isAtEnd(){return (currentPos == maxPos);};
		
		void Rewind(){currentPos = 0;};

		SharedPtr<T> operator->();
	};

	ItemManager();
	~ItemManager();
	/*
	add new item to list , pItem is pointer to item object that going to be added.pItem must not be an array
	Note: <pItem> must points to dynamc allocated memmory block,or else it will cause undefined behavior
	and that memory block must not be deallocated outside ItemManager class objects,because it will be
	dealloc inside itemManager object that contains it.
	Sharing item pointer between multiple ItemManager onjects is allowed
	*/
	bool AddItem(T* pItem,unsigned int *freeSlotID);
	int ReleaseSlot(unsigned int slotID);//release slot that has id <slotID>
	void ReleaseAllSlot();
	SharedPtr<T> GetItemPointer(unsigned int ID);//get pointer to item which has id <ID>
	unsigned int GetAllocSlot() {return allocSlots;}
	void GetIterator(Iterator& iterator);
protected:
	SharedPtr<T>** slots;//list of item slots
	std::list<int> reuseSlotIDs;//list id of reusable slots - slots  that used at least once before but released and ready for reuse  
	unsigned int allocSlots;//number of slots that allused at least once before , it will include reusable slots
};


template <class T>
ItemManager<T>::ItemManager()
{
	this->allocSlots= 0;
	this->slots=0;
}

template <class T>
ItemManager<T>::~ItemManager()
{
	ReleaseAllSlot();
}

template <class T>
void ItemManager<T>::ReleaseAllSlot()
{
	if(slots)
	{
		for (unsigned int i=0;i<allocSlots;++i)
		{
			if(slots[i])
			{
				delete slots[i];
				slots[i]=0;
			}
		}
		free(slots);
		slots=0;
	}
	allocSlots=0;
	reuseSlotIDs.clear();
}

template <class T>
int ItemManager<T>::ReleaseSlot(unsigned int ID)
{
	if(ID >=allocSlots || slots[ID]==NULL)
		return ID_NOT_FOUND;//invalid ID

	delete slots[ID];
	slots[ID]=NULL;

	this->reuseSlotIDs.push_back(ID);//mark this slot as reusable slot

	return 1;
}

template <class T>
bool ItemManager<T>::AddItem(T* pItem,unsigned int *freeSlotID)
{
	SharedPtr<T>** ppfreeSlot=NULL;

	if(this->reuseSlotIDs.size())//we have reusable slot
	{
		unsigned int ID=this->reuseSlotIDs.front();
		this->reuseSlotIDs.pop_front();
		if(freeSlotID)
			*freeSlotID = ID;
		ppfreeSlot = &slots[ID];
	}
	else
	{
		if(this->allocSlots % 10==0)//we use all available slots ,so alloc more space for additional items
		{
			unsigned int newMaxSize=this->allocSlots+10;
			if(newMaxSize > MAX_ID)//exceed max allowed number of slots
				return false;
			SharedPtr<T>**newPtr=(SharedPtr<T>**)realloc(slots,newMaxSize * sizeof(SharedPtr<T>*));
			if(newPtr==NULL)
				return false;
			slots=newPtr;

		}
		ppfreeSlot= &slots[allocSlots];
		if(freeSlotID)
			*freeSlotID = allocSlots;
		allocSlots++;
	}
	
	*ppfreeSlot = new SharedPtr<T>(pItem,false);

	return true;
}

template <class T>
SharedPtr<T> ItemManager<T>::GetItemPointer(unsigned int ID)
{
	if(ID >=allocSlots || slots[ID] == NULL)
	{
		SharedPtr<T> null;
		return null;//invalid ID,so return null pointer
	}
	return *slots[ID];
}

template <class T>
void ItemManager<T>::GetIterator(typename ItemManager<T>::Iterator & iterator)
{
	iterator.slots = slots;
	iterator.currentPos = 0;
	iterator.maxPos = allocSlots;
}

/*-----------iterator class------------*/
template <class T>
typename ItemManager<T>::Iterator ItemManager<T>::Iterator::operator ++(int i)
{
	Iterator preAdd = *this;
	while(currentPos < maxPos )
	{
		++currentPos;
		if(currentPos!=maxPos && slots[currentPos]!=NULL)
			break;
	}
	return preAdd;
}

template <class T>
typename ItemManager<T>::Iterator& ItemManager<T>::Iterator::operator ++()
{
	while(currentPos < maxPos )
	{
		++currentPos;
		if(currentPos!=maxPos && slots[currentPos]!=NULL)
			break;
	}
	return *this;
}

template <class T>
typename ItemManager<T>::Iterator ItemManager<T>::Iterator::operator --(int i)
{
	Iterator preSub = *this;
	while(currentPos > 0 )
	{
		if(slots[--currentPos]!=NULL)
			break;
	}
	return preSub;
}

template <class T>
typename ItemManager<T>::Iterator& ItemManager<T>::Iterator::operator --()
{
	while(currentPos > 0 )
	{
		if(slots[--currentPos]!=NULL)
			break;
	}
	return *this;
}

template<class T>
SharedPtr<T> ItemManager<T>::Iterator::operator->()
{
	if(currentPos == maxPos)
	{
		SharedPtr<T> null;
		return null;
	}
	return *slots[currentPos];
}
#endif