#pragma once

#include "stdafx.h"
#include "Memory.h"

template <class T>
class DynamicArray
{
private:
	T *array;
	uint4 size;
	uint4 index;

	bool Extend(uint4 amount)
	{
		T* tmp;

		if (!this->array)
		{
			this->array = (T *)MemoryAlloc(OSIB(T,amount),TRUE);
			
			if (!this->array)
				return false;

			this->size = amount;
			return true;
		}

		if (!MemoryReAlloc(OSIB(T,this->size),OSIB(T,amount),(void **)&this->array))
		{
			tmp = (T *)MemoryAlloc(OSIB(T,this->size + amount),TRUE);

			if (!tmp)
				return false;

			memcpy(tmp,this->array,OSIB(T,this->size));
			MemoryFree(this->array);
			this->array = tmp;
		}

		this->size += amount;
		return true;
	}

	void Init(uint4 initSize)
	{
		this->array = NULL;
		this->index = 0;
		Extend(initSize);
	}

public:

	DynamicArray(uint4 initialSize)
	{
		Init(initialSize);
	}

	DynamicArray()
	{
		Init(10);
	}

	~DynamicArray(void)
	{
		MemoryFree(this->array);
	}

	T None;

	T & operator [](uint4 i)
	{
		if (i < this->size)
			return this->array[i];

		return this->None;
	}

	uint4 GetCount() const
	{
		return this->index;
	}

	int4 Add(T item)
	{
		if (this->index > this->size - 1)
		{
			if (!Extend((this->size * 10) / 100))
				return -1;
		}

		this->array[this->index++] = item;
		return this->index-1;
	}

	bool Remove(uint4 pos)
	{
		if (pos >= this->index)
			return false;

		if (pos+1 != this->index) 
		{
			ffhelper::DataManipHelper<T>::EraseBlock(this->array,this->index,pos,1);
		}

		this->index -= 1;

		return true;
	}

	void Clear()
	{
		memset(this->array,0,OSIB(T,this->index));
		this->index = 0;
	}

	void ReleaseMemory()
	{
		MemoryFree(this->array);
	}
};

