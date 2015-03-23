#pragma once
#include "stdafx.h"
#include <Windows.h>
#include "helper\ByteBuffer.h"

typedef unsigned long OpenMode;
typedef unsigned long OpenType;

#define OpenForWrite GENERIC_WRITE
#define OpenForRead GENERIC_READ
#define OpenForReadWrite (GENERIC_WRITE | GENERIC_READ)

#define CreateAlways CREATE_ALWAYS
#define CreateNew CREATE_NEW
#define OpenAlways OPEN_ALWAYS
#define OpenExisting OPEN_EXISTING
#define TruncateExisting TRUNCATE_EXISTING

typedef enum seekType
{
	Begin,
	Current,
	End
}SeekType;

class FileReadWrite
{
private:
	HANDLE fileHandle;
	LARGE_INTEGER filePointer;
	wchar filePath[MAX_PATH];
	ByteBuffer *buffer;
	OpenMode mode;
	OpenType type;
	bool refreshBuffer;
	bool manageVirtualEof;
	int vfp;


	void GetNativeFilePointer()
	{
		LARGE_INTEGER pos = {0};
		SetFilePointerEx(this->fileHandle,pos,&this->filePointer,FILE_CURRENT);
	}

	long _Write(byte *buf, long offset, long count)
	{
		int written;

		if (WriteFile(this->fileHandle,(LPCVOID)buf,count,(LPDWORD)&written,NULL))
			return written;

		return 0;
	}

	long _Read(byte *buf, long offset, long count)
	{
		long readLen=0;
		
		if (offset > -1)
		{
			if (offset > this->filePointer.QuadPart)
				this->vfp = offset - this->filePointer.QuadPart;
			else
				Seek(offset,Begin);
		}

		if (this->refreshBuffer)
		{
			this->buffer->Clear();

			if (!ReadFile(this->fileHandle,(LPVOID)this->buffer->GetBuffer(),this->buffer->GetSize(),(LPDWORD)&readLen,NULL))
				return -1;

			GetNativeFilePointer();

			if (this->EndOfFile())
				this->manageVirtualEof=true;

			this->buffer->SetWrittenSize(readLen);
			this->refreshBuffer=false;
			this->vfp = 0;
		}

		readLen = this->buffer->Read(buf,this->vfp,count,false);
		this->vfp += readLen;

		return readLen;
	}

public:

	static bool FileExists(wnstring fileName)
	{
		if (GetFileAttributesW(fileName) == INVALID_FILE_ATTRIBUTES)
		{
			if (GetLastError() == ERROR_FILE_NOT_FOUND)
				return false; 
		}

		return true;
	}

	FileReadWrite(wnstring fileName, OpenMode openMode, OpenType openType)
	{
		this->mode = openMode;
		this->type = openType;
		memset(this->filePath,0,sizeof(this->filePath));
		wcsncpy(this->filePath,fileName,wcslen(fileName));
		this->buffer = new ByteBuffer(10 * 1024,NULL);
		this->refreshBuffer = false;
		this->vfp=0;
		this->manageVirtualEof=false;
		this->filePointer.QuadPart=0;
	}

	~FileReadWrite(void)
	{
		Close();
	}

	bool Open()
	{
		this->fileHandle = CreateFileW((LPCWSTR)this->filePath,(DWORD)this->mode,FILE_SHARE_READ,NULL,(DWORD)this->type,FILE_ATTRIBUTE_NORMAL,NULL);

		if (this->fileHandle == INVALID_HANDLE_VALUE)
			return false;

		this->refreshBuffer=true;
		return true;
	}

	void Close()
	{
		if (this->buffer != NULL)
			delete this->buffer;
		
		if (this->fileHandle != NULL)
			CloseHandle(this->fileHandle);

		this->buffer = NULL;
		this->fileHandle = NULL;
	}

	bool Seek(long pos, SeekType seekType)
	{
		LARGE_INTEGER p;
		p.QuadPart = pos;

		if (SetFilePointerEx(this->fileHandle,p,&this->filePointer,seekType))
		{
			this->refreshBuffer=true;
			this->manageVirtualEof=false;
			return true;
		}

		return false;
	}

	long Read(byte *buffer, long offset, long count)
	{
		if (!buffer)
			return 0;

		return _Read(buffer,offset,count);
	}

	int ReadByte()
	{
		byte b;

		if (Read(&b,-1,1))
			return b;

		return 0;
	}

	char ReadAnsiChar()
	{
		char chr;
		
		if (Read((byte *)&chr,-1,1) == 1)
			return chr;

		return '\0';
	}

	wchar ReadWideChar()
	{
		wchar wideChr;

		if (Read((byte *)&wideChr,-1,sizeof(wchar)) == sizeof(wchar))
			return wideChr;

		return L'';
	}

	long Write(byte *buffer, long offset, long count)
	{
		if (!buffer)
			return 0;

		return _Write(buffer,offset,count);
	}

	long Write(uint4 value, long offset)
	{
		return Write((byte *)&value,offset,sizeof(uint4));
	}

	bool EndOfFile() const
	{
		bool eof=false;
		byte byte;
		int read;

		if (this->manageVirtualEof)
			return this->vfp == this->buffer->GetLength();

		if (!ReadFile(this->fileHandle,&byte,1,(LPDWORD)&read,NULL))
			return false;

		if (read > 0)
		{
			SetFilePointerEx(this->fileHandle,this->filePointer,NULL,FILE_BEGIN);
			return false;
		}

		return true;
	}

	OpenType GetOpenType() const
	{
		return this->type;
	}

	OpenMode GetOpenMode() const
	{
		return this->mode;
	}

};

