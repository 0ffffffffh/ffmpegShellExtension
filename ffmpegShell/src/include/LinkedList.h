#pragma once

#include "Synch.h"
#include "Memory.h"

typedef void (*NODE_ITEM_DISPOSER)(void *object);


template <class T>
class LinkedListNode
{
private:
	T value;
	LinkedListNode<T> *next;
	LinkedListNode<T> *prev;

	void Init()
	{
		this->prev = this->next = NULL;
		this->value = NULL;
	
	}
public:
	LinkedListNode()
	{
		Init();
	}

	~LinkedListNode()
	{
	}

	LinkedListNode<T>* Next() const
	{
		return this->next;
	}

	LinkedListNode<T>* Previous() const
	{
		return this->prev;
	}

	bool HasPrevious() const
	{
		return this->prev != NULL;
	}

	bool HasNext() const
	{
		return this->next != NULL;
	}

	T *GetPointer() const
	{
		return &this->value;
	}

	T GetValue() const
	{
		return this->value;
	}

	template <class T> friend class LinkedList;
};



template <class T>
class LinkedList
{
private:
	LinkedListNode<T> *headNode;
	LinkedListNode<T> *tailNode;
	SPINLOCK synchLock;
	NODE_ITEM_DISPOSER disposer;
	int count;

	void InsertInternal(LinkedListNode<T> *node)
	{
		AcquireSpinLock(&this->synchLock);

		if (this->headNode==NULL)
		{
			this->headNode = this->tailNode = node;
			this->count = 1;
		}
		else
		{
			this->tailNode->next = node;
			node->prev = this->tailNode;
			this->tailNode = node;
			this->count++;
		}

		ReleaseSpinLock(&this->synchLock);
	}

public:

	LinkedList()
	{
		InitializeSynch(&this->synchLock);

		this->headNode = this->tailNode = NULL;
		this->count=0;
		this->disposer = NULL;
	}

	~LinkedList(void)
	{
		Clear();
	}

	void Clear()
	{
		while (this->count > 0)
		{
			Remove(this->headNode);
		}
	}

	bool Insert(T val)
	{
		if (val == NULL)
			return false;

		LinkedListNode<T> *node = new LinkedListNode<T>();
		node->value = val;
		InsertInternal(node);
		return true;
	}

	
	void Remove(LinkedListNode<T> *node)
	{
		if (!node)
			return;

		AcquireSpinLock(&this->synchLock);

		if (this->count == 1)
		{
			this->headNode = this->tailNode = NULL;
		}
		else if (node == this->headNode)
		{
			node->next->prev = NULL;
			this->headNode = node->next;
		}
		else if (node == this->tailNode)
		{
			this->tailNode = node->prev;
			this->tailNode->next = NULL;
		}
		else
		{
			node->prev->next = node->next;
			node->next->prev = node->prev;
		}


		this->count -= 1;

		ReleaseSpinLock(&this->synchLock);

		if (this->disposer != NULL)
			this->disposer(node->value);
		else
			FREEOBJECT(node->value); //call default disposer

		delete node;
	}

	LinkedListNode<T> *Begin() const
	{
		return this->headNode;
	}

	LinkedListNode<T> *End() const
	{
		return this->tailNode;
	}

	LinkedListNode<T> *At(int4 index)
	{
		LinkedListNode<T> *node;

		if (index >= this->GetCount())
			return NULL;

		node = this->Begin();

		while (index-- > 0)
			node = node->Next();

		return node;
	}

	int GetCount() const
	{
		return this->count;
	}

	void SetDisposer(NODE_ITEM_DISPOSER disposer)
	{
		this->disposer = disposer;
	}
};

