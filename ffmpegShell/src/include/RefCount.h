#pragma once

#include "stdafx.h"
#include "Types.h"
#include "Synch.h"

class RefCount
{
private:
	uint4 refCount;
	SPINLOCK lock;
	bool released;
	
protected:
	virtual void OnReleased()
	{

	}

public:

	RefCount(void)
	{
		InitializeSynch(&this->lock);
		this->released = false;
		this->refCount = 0;
	}

	~RefCount(void)
	{
	}

	bool IsReleased() const
	{
		return this->released;
	}

	bool IsReferenced() const
	{
		return this->refCount > 0;
	}

	void AddRef() 
	{
		AcquireSpinLock(&this->lock);

		this->refCount++;

		if (this->refCount == 1)
			this->released=false;

		ReleaseSpinLock(&this->lock);
	}

	void Release()
	{
		AcquireSpinLock(&this->lock);

		if (this->refCount == 0)
			goto releaseLock;

		this->refCount--;

		if (this->refCount == 0)
		{
			this->released=true;
			OnReleased();
		}

releaseLock:
		ReleaseSpinLock(&this->lock);
	}
};

