#include "Stdafx.h"
#include "Memory.h"
#include "LinkedList.h"
#include "RefCount.h"


#ifndef __FILELIST__
#define __FILELIST__

typedef enum tagOBJECTTYPE
{
	File,
	Folder
}OBJECTTYPE;

#define OPL_FULLPATH	0
#define OPL_NAME		1
#define OPL_EXTENSION	2

#define PAS_NONE					0x00000000
#define PAS_OBJECTNAME				0x00000001
#define PAS_EXTENSION				0x00000002


#define FL_FLAG_QUOTE_SURROUNDED	0x00000001
#define FL_FLAG_ZERO_BUFFER			0x00000002

typedef struct
{
	wchar objectFullPath[MAX_PATH];
	wchar objectName[196];
	wchar objectExtension[64];
	uint4 objectPartLengths[3];
	OBJECTTYPE type;
	bool permissionOk;
}FILEPATHITEM;


static bool FlCheckDirectoryPermission(LPCWSTR File, OBJECTTYPE type)
{
	HANDLE Filehandle;

	Filehandle = CreateFileW(File,
							GENERIC_WRITE,
							0,
							NULL,
							OPEN_EXISTING,
							type == Folder ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL,
							NULL);


	if (Filehandle == INVALID_HANDLE_VALUE && GetLastError() == ERROR_ACCESS_DENIED)
		return false;

	CloseHandle(Filehandle);

	return true;

}

static BOOL FlParseObject(wnstring item, FILEPATHITEM *node)
{
	DWORD attrib;
	wchar *p;
	int count=0,len=0;

	if (item == NULL || node == NULL)
		return FALSE;

	len = wcslen(item);

	p = (wchar *)(item + len);

	attrib = GetFileAttributes(item);

	if (attrib & FILE_ATTRIBUTE_DIRECTORY)
		node->type = Folder;
	else if (attrib & FILE_ATTRIBUTE_NORMAL)
		node->type = File;


	if (node->type == Folder)
		goto PassExtensionDetect;

	while (*(--p) != L'.')
	{
		if (*p == L'\\')
		{
			wcsncpy(node->objectName,p+1,count);
			node->objectPartLengths[OPL_NAME] = count;
			count = 0;
			goto PassObjectDetect;
		}

		count++;
	}

	if (count != -1)
	{
		wcsncpy(node->objectExtension,p+1,count);
		node->objectPartLengths[OPL_EXTENSION] = count;
	}

PassExtensionDetect:
	count = 0;

	while (*(--p) != L'\\')
		count++;

	wcsncpy(node->objectName,p+1,count);
	node->objectPartLengths[OPL_NAME] = count;

PassObjectDetect:
	node->objectPartLengths[OPL_FULLPATH] = p - item;
	wcsncpy(node->objectFullPath,item,node->objectPartLengths[OPL_FULLPATH]);

	node->permissionOk = FlCheckDirectoryPermission(item,node->type);

	return TRUE;
}


static FILEPATHITEM *FlCloneNode(FILEPATHITEM *node)
{
	FILEPATHITEM *clone = ALLOCOBJECT(FILEPATHITEM);

	if (!clone)
		return NULL;

	RtlCopyMemory(clone,node,sizeof(FILEPATHITEM));
	
	return clone;
}

static VOID FlFreeClonedNode(FILEPATHITEM *node)
{
	if (node)
		FREEOBJECT(node);
}

static BOOL FlSetNodePart(FILEPATHITEM *node, LPCWSTR str, DWORD type)
{
	ULONG strLen = 0;
	ULONG maxCch=0;
	LPWSTR partPtr;

	switch (type)
	{
	case OPL_FULLPATH:
		{
			partPtr = node->objectFullPath;
			maxCch = sizeof(node->objectFullPath) / sizeof(wchar);
		}
		break;
	case OPL_NAME:
		{
			partPtr = node->objectName;
			maxCch = sizeof(node->objectName) / sizeof(wchar);
		}
		break;
	case OPL_EXTENSION:
		{
			partPtr = node->objectExtension;
			maxCch = sizeof(node->objectExtension) / sizeof(wchar);
		}
		break;
	default:
		return FALSE;
	}

	if (str == NULL)
	{
		RtlZeroMemory(partPtr, maxCch * sizeof(wchar));
		node->objectPartLengths[type] = 0;
		return TRUE;
	}


	strLen = wcslen(str);

	if (strLen >= maxCch)
		return FALSE;

	RtlZeroMemory(partPtr,maxCch * sizeof(wchar));
	wcsncpy(partPtr,str,strLen);
	node->objectPartLengths[type] = strLen;

	return TRUE;
}


static UINT FlGeneratePathString2(FILEPATHITEM *item, LPWSTR formatBuf, UINT cchMax, DWORD flag, DWORD appendStyle,LPCWSTR appendString)
{
	BOOL noExt=item->type == Folder;
	UINT writtenBytes=0;
	UINT extraRequiredLen=2; //default 2 for \ seperators

	*formatBuf = 0;

	//Check for extension state of file object.
	if (item->type == File && item->objectPartLengths[OPL_EXTENSION] == 0)
		noExt = TRUE;

	if (flag & FL_FLAG_QUOTE_SURROUNDED)
		extraRequiredLen += 2;

	if ((item->objectPartLengths[0] +
		item->objectPartLengths[1] +
		item->objectPartLengths[2] + 
		extraRequiredLen) > cchMax)
	{
		return 0;
	}

	if (flag & FL_FLAG_ZERO_BUFFER)
		RtlZeroMemory(formatBuf,sizeof(WCHAR) * cchMax);

	if (flag & FL_FLAG_QUOTE_SURROUNDED)
	{
		*formatBuf = L'"';
		writtenBytes = 1;

	}

	//Write first "DIRECTORY\FILENAME" 
	writtenBytes += wsprintfW(formatBuf+writtenBytes,L"%s\\%s",item->objectFullPath,item->objectName);

	if (!noExt)
	{
		//Write appendstring with style.
		switch (appendStyle)
		{
		case PAS_OBJECTNAME: //FILENAME|APPENDSTRING.EXT
			writtenBytes += wsprintfW(formatBuf+writtenBytes,L"%s.%s",appendString,item->objectExtension);
			break;
		case PAS_EXTENSION: //FILENAME.APPENDSTRING|EXT
			writtenBytes += wsprintfW(formatBuf+writtenBytes,L".%s%s",item->objectExtension,appendString);
			break;
		case PAS_NONE: //FILENAME.EXT
			writtenBytes += wsprintfW(formatBuf+writtenBytes,L".%s",item->objectExtension);
			break;
		}
	}
	else if (appendStyle != PAS_NONE)
	{
		//FILENAME|APPENDSTRING
		writtenBytes += wsprintfW(formatBuf+writtenBytes,L"%s",appendString);
	}

	if (flag & FL_FLAG_QUOTE_SURROUNDED)
	{
		*(formatBuf+writtenBytes) = L'"';
		writtenBytes++;
	}

	return writtenBytes;
}

static UINT FlGeneratePathString(FILEPATHITEM *item, LPWSTR formatBuf, UINT cchMax, DWORD appendStyle,LPCWSTR appendString)
{
	return FlGeneratePathString2(item,formatBuf,cchMax,0,appendStyle,appendString);
}

static UINT FlMakeFilenameWithParts(LPWSTR buffer, UINT cchMax, LPWSTR directory, LPWSTR name, LPWSTR extension)
{
	FILEPATHITEM *item = ALLOCOBJECT(FILEPATHITEM);
	UINT fnameLen=0;

	if (!item)
		return 0;

	if (buffer == NULL)
		return 0;

	FlSetNodePart(item,directory,OPL_FULLPATH);
	FlSetNodePart(item,name, OPL_NAME);
	FlSetNodePart(item,extension,OPL_EXTENSION);

	fnameLen = FlGeneratePathString(item,buffer,cchMax,PAS_NONE,NULL);

	FREEOBJECT(item);

	return fnameLen;
}

class FilePathItem
{
private:
	FILEPATHITEM item;
	
	uint4 GetPathLength()
	{
		uint4 *opl = this->item.objectPartLengths;
		////Parts lengths + '\' + '.'
		return opl[0] + opl[1] + opl[2] + 2;
	}

	uint4 GetPathBufferSize()
	{
		return sizeof(wchar) * (GetPathLength() + 1);
	}

public:
	static FilePathItem *Parse(wnstring ws)
	{
		FilePathItem *fpi = new FilePathItem();

		if (FlParseObject(ws,&fpi->item))
			return fpi;

		return NULL;
	}

	FilePathItem()
	{

	}

	~FilePathItem()
	{
	}

	bool SetExtension(wnstring newExtension)
	{
		return (bool)FlSetNodePart(&this->item,(LPCWSTR)newExtension,OPL_EXTENSION);
	}

	bool SetPath(wnstring path)
	{
		return (bool)FlSetNodePart(&this->item,(LPCWSTR)path,OPL_FULLPATH);
	}

	bool SetName(wnstring name)
	{
		return (bool)FlSetNodePart(&this->item,(LPCWSTR)name,OPL_NAME);
	}

	uint4 GeneratePathString(wnstring *buffer)
	{
		return GeneratePathString(buffer,PAS_NONE);
	}

	uint4 GeneratePathString(wnstring *buffer, uint4 appendStyle)
	{
		return GeneratePathString(buffer,appendStyle,NULL);
	}

	uint4 GeneratePathString(wnstring *buffer, uint4 appendStyle, wnstring appendString)
	{
		if (!buffer)
			return 0;

		*buffer = (wnstring)MemoryAlloc(GetPathBufferSize(),TRUE);

		return FlGeneratePathString(&this->item,*buffer,GetPathLength(),appendStyle,appendString);
	}

	uint4 GeneratePathString2(wnstring *buffer, uint4 cchMax)
	{
		return GeneratePathString2(buffer,cchMax);
	}

	uint4 GeneratePathString2(wnstring *buffer, uint4 cchMax, uint4 appendStyle)
	{
		return GeneratePathString2(buffer,cchMax,appendStyle,NULL);
	}

	uint4 GeneratePathString2(wnstring *buffer, uint4 cchMax, uint4 appendStyle, wnstring appendString)
	{
		if (!buffer)
			return 0;

		if (!(*buffer))
			return 0;

		return FlGeneratePathString(&this->item,*buffer,cchMax,appendStyle,appendString);
	}
};


class FileList : public RefCount
{
private:
	LinkedList<FILEPATHITEM *> *objectList;
	FILEPATHITEM *longestItem;

	bool InternalAdd(wnstring item)
	{
		FILEPATHITEM *newItem=NULL;

		if (item == NULL)
			return false;

		newItem = ALLOCOBJECT(FILEPATHITEM);

		if (newItem == NULL)
			return false;

		FlParseObject(item,newItem);

		if (longestItem == NULL)
			longestItem = newItem;
		else if (newItem->objectPartLengths[OPL_NAME] > longestItem->objectPartLengths[OPL_NAME])
			longestItem = newItem;

		return this->objectList->Insert(newItem);
	}

	void InternalClear()
	{
		this->objectList->Clear();
	}

protected:
	void OnReleased()
	{
		DPRINT("Released");
		InternalClear();
	}

public:

	FileList(void)
	{
		this->objectList = new LinkedList<FILEPATHITEM *>();
		longestItem = NULL;
	}

	~FileList(void)
	{
		delete this->objectList;
	}

	uint4 Count() const
	{
		return this->objectList->GetCount();
	}

	bool Add(wnstring item)
	{
		return InternalAdd(item);
	}

	void Clear()
	{
		InternalClear();
	}



	FILEPATHITEM *GetLongestObject() const
	{
		return longestItem;
	}

	FILEPATHITEM *GetLastObject() const
	{
		return this->objectList->End()->GetValue();
	}

	LinkedList<FILEPATHITEM *> *GetList() const
	{
		return this->objectList;
	}
};


#endif //__FILELIST__