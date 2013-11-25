#ifndef _SHARED_PTR_
#define _SHARED_PTR_

class Reference
{
private:
	unsigned int refCount;//reference count
public:
	Reference(){ refCount = 0;};
	unsigned int AddRef() {return ++refCount;};//increase reference count
	unsigned int Release() //decrease reference count
	{
		if(refCount == 0) 
			return 0 ;
		return --refCount;
	};
};


/*---------SharedPointer-------------*/
template<class T>
class SharedPtr
{
private:
	bool isArray;//this pointer points to an array?
	T * ptr;//the real pointer
	Reference* ref;
public:
	SharedPtr();
	SharedPtr(const T * ptr,bool isArray);//<ptr> must points to dynamic allocated mem block
	SharedPtr(const SharedPtr<T>& sptr2);//copy constructor
	~SharedPtr();

	T& operator *();
	T& operator [](unsigned int index);
	T* operator ->();
	SharedPtr<T>& operator = (const SharedPtr<T>& sptr2);

	bool operator == (const SharedPtr<T>& sptr2){return ptr == sptr2.ptr;}
	bool operator != (const SharedPtr<T>& sptr2){return ptr != sptr2.ptr;};
	bool operator == (const T* ptr2){return ptr == ptr2;}
	bool operator != (const T* ptr2){return ptr != ptr2;};

	SharedPtr<T>& ToNull();
};

template <class T>
SharedPtr<T>::SharedPtr()
{
	ptr = NULL;
	ref = new Reference();
	ref->AddRef();
}

template <class T>
SharedPtr<T>::SharedPtr(const T *ptr,bool isArray)
{
	this->isArray = isArray;
	this->ptr = (T*)ptr;
	ref = new Reference();
	ref->AddRef();
}

template <class T>
SharedPtr<T>::SharedPtr(const SharedPtr<T> &sptr2)
{
	this->isArray = sptr2.isArray;
	this->ptr = sptr2.ptr;
	this->ref = sptr2.ref;
	ref->AddRef();
}


template <class T>
SharedPtr<T>::~SharedPtr()
{
	if(ref->Release() == 0)
	{
		if(ptr)
		{
			if(isArray)
				delete[] ptr;
			else
				delete ptr;
		}
		delete ref;
	}
}

template <class T>
T& SharedPtr<T>::operator *()
{
	return *ptr;
}

template <class T>
T& SharedPtr<T>::operator [](unsigned int index)
{
	if(!isArray)
		return *ptr;
	return ptr[index];
}

template <class T>
T* SharedPtr<T>::operator ->()
{
	return ptr;
}

template <class T>
SharedPtr<T>& SharedPtr<T>::operator =(const SharedPtr<T> &sptr2)
{
	if(this!=&sptr2)//not self assign
	{
		if(ref->Release() == 0)
		{
			if(ptr)
			{
				if(isArray)
					delete[] ptr;
				else
					delete ptr;
			}
			delete ref;
		}
		this->isArray = sptr2.isArray;
		this->ptr = sptr2.ptr;
		this->ref = sptr2.ref;
		this->ref->AddRef();
	}
	return *this;
}

template <class T>
SharedPtr<T>& SharedPtr<T>::ToNull()
{
	if(ref->Release() == 0)
	{
		if(ptr)
		{
			if(isArray)
				delete[] ptr;
			else
				delete ptr;
		}
		delete ref;
	}
	ptr = NULL;
	ref = new Reference();
	ref->AddRef();

	return *this;
}

#endif