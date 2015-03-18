#pragma once

#include "stdafx.h"

#if defined(ARCH_X86)
typedef volatile ULONG SPINLOCK;
#elif defined(ARCH_X64)
typedef volatile ULONGLONG SPINLOCK;
#endif

#define InitializeSynch(obj) *(obj) = 0L


extern "C" { 
	void AcquireSpinLock(SPINLOCK *lock); 
	void ReleaseSpinLock(SPINLOCK *lock); 
	bool TestSpinLock(SPINLOCK *lock);
	
	void SpinWait(
#if defined(ARCH_X64)
		ULONGLONG
#else
		ULONG 
#endif
		spinCount);

}


