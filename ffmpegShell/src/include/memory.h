#pragma once

#include "Types.h"

#define ALLOCARRAY(type,len) ((type *)MemoryAlloc(sizeof(type) * (len),true))

#define ALLOCOBJECT(type) ((type *)MemoryAlloc(sizeof(type),true))

#define REALLOCOBJECT(type, oldsize, howmuch,mem) MemoryReAlloc(oldsize,howmuch,&mem)

#define FREEOBJECT(obj) MemoryFree(obj)

#define FREEARRAY(arr) FREEOBJECT(arr)

#define ALLOCSTRINGW(len) ALLOCARRAY(wchar,(len+1))

#define ALLOCSTRINGA(len) ALLOCARRAY(achar,(len+1))

#define FREESTRING(str) FREEARRAY(str)

//Object size in byte
#define OSIB(type, count) ( sizeof(type) * (count) )

//Object count from byte
#define OCFB(type, size) ((size) / sizeof(type) )


vptr MemoryAlloc(ulong size, bool zeroFill);

bool MemoryReAlloc(ulong oldSize, ulong extendAmount, vptr *memoryBlock);

void MemoryFree(vptr memBlock);

