#include "Stdafx.h"
#include "Memory.h"

HANDLE MmProcessHeap=NULL;

vptr MemoryAlloc(ulong size, bool zeroFill)
{
	vptr memPtr;

	if (MmProcessHeap == NULL)
		MmProcessHeap = GetProcessHeap();

	memPtr = HeapAlloc(MmProcessHeap,zeroFill ? HEAP_ZERO_MEMORY : 0,size);

	return memPtr;
}

bool MemoryReAlloc(ulong oldSize, ulong extendAmount, vptr *memoryBlock)
{
	vptr Extended = NULL;

	if (MmProcessHeap == NULL || *memoryBlock == NULL)
	{
		*memoryBlock = MemoryAlloc(oldSize + extendAmount,true);
		return *memoryBlock != NULL;
	}

	Extended = HeapReAlloc(MmProcessHeap,0,*memoryBlock,oldSize + extendAmount);

	if (Extended == NULL)
		return FALSE;

	RtlZeroMemory((((byte *)Extended)+oldSize),extendAmount);

	*memoryBlock = Extended;

	return true;
}

void MemoryFree(vptr memBlock)
{
	if (!memBlock)
		return;

	HeapFree(MmProcessHeap,0,memBlock);
}