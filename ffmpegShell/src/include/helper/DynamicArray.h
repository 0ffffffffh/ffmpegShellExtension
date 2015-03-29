#pragma once

#include "stdafx.h"
#include "Memory.h"


template <class T>
class DynamicArray
{
public:
	typedef int4 (*DYNAMIC_ARRAY_SORT_COMPARER)(T v1,T v2);
private:
	T *								array;
	uint4							size;
	uint4							index;
	DYNAMIC_ARRAY_SORT_COMPARER		cmpFunc;

	uint4 GetRemain() const
	{
		return this->size - this->index;
	}

	bool Extend(uint4 amount)
	{
		T* tmp;
		uint4 remain;

		if (!this->array)
		{
			this->array = (T *)MemoryAlloc(OSIB(T,amount),TRUE);
			
			if (!this->array)
				return false;

			this->size = amount;
			return true;
		}

		remain = this->size - this->index;

		if (remain > 4)
			return true;
		
		if (amount < 8)
			amount = 8;

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

	void Init(uint4 initSize,DYNAMIC_ARRAY_SORT_COMPARER cmpFunc)
	{
		this->cmpFunc=cmpFunc;
		this->array = NULL;
		this->index = 0;
		Extend(initSize);
	}

	int4 SortedAdd(T val, uint4 low, uint4 high)
	{
		int4 mid;
		int4 insertIndex=-1;

		T midVal;

		if (low == high)
		{
			this->array[this->index++] = val;
			return this->index-1;
		}
		
		while (low <= high)
		{
			mid = (low + high) >> 1;
			
			midVal = this->array[mid];

			//val < midVal
			if (this->cmpFunc(val,midVal)<0)
			{
				if (mid-1 < 0 || this->cmpFunc(val,this->array[mid-1])>0)
				{
					insertIndex = mid;
					break;
				}

				high = mid;
			}
			else if (this->cmpFunc(val,midVal)>0)
			{
				if ((mid == this->index-1) || this->cmpFunc(val,this->array[mid+1])<0)
				{
					insertIndex = mid+1;
					break;
				}

				low = mid;
			}
			else
			{
				insertIndex = mid;
				break;
			}
		}

		if (insertIndex>-1)
		{

			//shift required?
			if (insertIndex < this->index)
			{
				if (!Extend((this->index - insertIndex) + 8))
					return -1;

				ffhelper::DataManipHelper<T>::ShiftBlock(this->array,insertIndex,1,this->index,false,false);
			}
			else
			{
				if (!Extend(1))
					return -1;

			}

			this->index++;

			this->array[insertIndex] = val;

			return insertIndex;
		}

		return -1;
	}

public:

	DynamicArray(uint4 initialSize, DYNAMIC_ARRAY_SORT_COMPARER cmpFunc)
	{
		Init(initialSize,cmpFunc);
	}

	DynamicArray(uint4 initialSize)
	{
		Init(initialSize,NULL);
	}

	DynamicArray(DYNAMIC_ARRAY_SORT_COMPARER cmpFunc)
	{
		Init(10,cmpFunc);
	}

	DynamicArray()
	{
		Init(10, NULL);
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

		if (this->cmpFunc != NULL)
			return SortedAdd(item,0,this->index);

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

