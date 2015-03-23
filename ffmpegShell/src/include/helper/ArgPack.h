#pragma once
#include "stdafx.h"

#define ALLOCPACKET(size) MemoryAlloc(size,true)

#define FREEPACKET(pack) MemoryFree(pack)

#define _PACKIO_READ	0
#define _PACKIO_WRITE	1

int4 __forceinline _PACKETIO(int4 ioType, vptr packet, int4 typeSize, int4 offset, byte *val)
{
	vptr psrc,pdst;

	byte *pack = (byte *)packet;

	pack += offset;

	DPRINT("packetio=%p\n",val);

	switch (ioType)
	{
	case _PACKIO_READ:
		pdst = val;
		psrc = pack;
		break;
	case _PACKIO_WRITE:
		pdst = pack;
		psrc = val;
		break;
	}

	memcpy(pdst,psrc,typeSize);

	return offset + typeSize;
}


#define WRITEPACKET(packet, type, offset, val) _PACKETIO(_PACKIO_WRITE,packet,sizeof(type), offset,((byte *)val))

#define READPACKET(packet, type, offset, val) _PACKETIO(_PACKIO_READ,packet,sizeof(type),offset,((byte *)val))

