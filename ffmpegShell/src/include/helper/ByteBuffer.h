#pragma once
#include "stdafx.h"
#include "Memory.h"

typedef void (*BUFFER_CONSUMER_METHOD)(byte *, int);

class ByteBuffer
{
private:
	byte *buffer;
	int bufSize;
	int bufIndex;
	
	BUFFER_CONSUMER_METHOD consumerCallback;

	void Erase(int pos, int count)
	{
		this->bufIndex -= ffhelper::DataManipHelper<byte>::EraseBlock(this->buffer,this->bufIndex,pos,count);
	}

	void Init(int bufferSize)
	{
		this->bufSize = bufferSize;
		this->bufIndex = 0;
		this->buffer = ALLOCARRAY(byte,this->bufSize);
	}

public:

	ByteBuffer(int bufferSize, BUFFER_CONSUMER_METHOD consumerCb)
	{
		Init(bufferSize);
		this->consumerCallback = consumerCb;
	}

	ByteBuffer(int bufferSize)
	{
		Init(bufferSize);
		this->consumerCallback = NULL;
	}

	~ByteBuffer(void)
	{
		FREEOBJECT(this->buffer);
	}

	void Clear()
	{
		this->Erase(0,this->bufIndex);
	}

	void Remove(int pos, int count)
	{
		Erase(pos,count);
	}

	bool Write(byte *data, int length)
	{
		int remain = this->bufSize - this->bufIndex;

		if (length > remain)
		{
			if (this->consumerCallback != NULL)
				this->consumerCallback(this->buffer,length-remain);

			Erase(0,length-remain);
		}

		memcpy(this->buffer+this->bufIndex,data,length);
		this->bufIndex += length;
		return true;
	}

	int Read(byte *dataPtr,int pos, int count, bool eraseAfterRead)
	{
		if (pos < 0)
			pos = this->bufIndex - count;
		else
		{
			if (pos+count >= this->bufIndex-1)
				count = this->bufIndex-pos;
		}

		memcpy(dataPtr,this->buffer+pos,count);

		if (eraseAfterRead)
			Erase(pos,count);

		return count;
	}

	int CopyBuffer(byte *natBuf, int size)
	{
		int copySize = size <= this->bufIndex ? size : this->bufIndex;
		memcpy(natBuf,this->buffer,copySize);
		return copySize;
	}

	byte *GetBuffer(int *availableForWrite)
	{
		if (availableForWrite != NULL)
		{
			*availableForWrite = this->bufSize - this->bufIndex;;
			return this->buffer + this->bufIndex;
		}

		return this->buffer;
	}

	byte *GetBuffer()
	{
		return this->GetBuffer(NULL);
	}

	byte *GetWritableBuffer()
	{
		return this->buffer + this->bufIndex;
	}

	int GetSize() const
	{
		return this->bufSize;
	}

	int GetLength() const
	{
		return this->bufIndex;
	}

	int GetRemainSize() const
	{
		return this->bufSize - this->bufIndex;
	}

	void SetWrittenSize(int size)
	{
		this->bufIndex += size;
	}

	byte ByteAt(int pos) const
	{
		if (pos < 0 || pos > this->bufSize-1)
			return (byte)-1;
		return this->buffer[pos];
	}

	byte operator[](int index) 
	{
		return ByteAt(index);
	}

};

