#ifndef _GROWABLE_ARRAY_
#define _GROWABLE_ARRAY_

/*-------------------------------------------------
template class GrowableArray - implements an array
that has growable size
------------------------------------------------*/
template <class T>
class GrowableArray
{
public:

	GrowableArray();
	~GrowableArray();

	bool Add(const T& item);//add a copy of item to the end of array
	
	T* GetArray() {return elements;}
	T& operator[](int index);//member access
	T& operator[](unsigned short index);//member access
	T& operator[](unsigned int index);//member access
	operator T*() {return elements;}//casting operator
	unsigned int Size(){return size;}//current size
	void Clear();
protected:
	T* elements;
	unsigned int size;
};


template <class T>
GrowableArray<T>::GrowableArray()
{
	this->elements= 0;
	this->size=0;
}
template <class T>
GrowableArray<T>::~GrowableArray()
{
	Clear();
}

template <class T>
void GrowableArray<T>::Clear()
{
	if(elements)
	{
		free(elements);
		elements = NULL;
	}
	size = 0;
}


template <class T>
bool GrowableArray<T>::Add(const T& item)
{
	
	if(this->size % 10==0)//we use all available cached slots,so alloc more cache for additional items
	{
		unsigned int newMaxSize=this->size+10;//addition 10 slots for future use

		T* newPtr=(T*)realloc(elements,newMaxSize * sizeof(T));
		if(newPtr==NULL)
			return false;
		elements=newPtr;

	}
	memcpy(&elements[size] , &item , sizeof(T));

	size++;

	return true;
}
template <class T>
T& GrowableArray<T>::operator [](unsigned short index)
{
	if(index >=size)//out of range
	{
		return elements[size-1];//return last element
	}
	return elements[index];
}

template <class T>
T& GrowableArray<T>::operator [](unsigned int index)
{
	if(index >=size)//out of range
	{
		return elements[size-1];//return last element
	}
	return elements[index];
}

template <class T>
T& GrowableArray<T>::operator [](int index)
{
	if(index >=(int)size)//out of range
		return elements[size-1];//return last element
	if(index < 0)
		return elements[0];//return first element
	return elements[index];
}

#endif