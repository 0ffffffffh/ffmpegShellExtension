#pragma once

#include "stdafx.h"
#include "FileReadWrite.h"
#include <varargs.h>
#include <map>

typedef struct
{
	wnstring	langFormatString;
	wnstring	formattedStringBuffer;
	int4		formattedStringBufferSize;
	bool		isDefault;
}__langItemInfo;

typedef std::map<wnstring, __langItemInfo *, stringComparer> LangKeyValueList;

class LanguageManager;

LanguageManager* gs_LanMan = NULL;

const byte UNICODE_MARK[] = {0xff,0xfe};

class LanguageManager
{
private:
	FileReadWrite *fileIo;
	LangKeyValueList *langList;
	wnstring langFileContent;

	
	bool GetString(wnstring key, __langItemInfo **langItem)
	{
		LangKeyValueList::iterator it;

		it = this->langList->find(key);

		if (it == this->langList->end())
			return false;

		if (langItem != NULL)
			*langItem = it->second;
		
		return true;
	}

	void InsertLanguageString(wnstring key, wnstring string, bool defaultValue)
	{
		bool justUpdate=false;
		int4 strLen;
		__langItemInfo *langItem = NULL;

		if (key == NULL || string == NULL)
			return;

		if (GetString(key,&langItem))
		{
			if (!langItem->isDefault || defaultValue)
			{
				return;
			}

			FREESTRING(langItem->formattedStringBuffer);
			justUpdate = true;
		}

		langItem = ALLOCOBJECT(__langItemInfo);

		langItem->isDefault = defaultValue;
		langItem->langFormatString = string;

		strLen = wcslen(string);

		//double it the size to make sure enough memory for formatted string
		langItem->formattedStringBufferSize = strLen * 2; 

		//pre-allocate formatted buffer size for lang item
		langItem->formattedStringBuffer = 
			ALLOCSTRINGW(langItem->formattedStringBufferSize);


		if (!justUpdate)
			this->langList->insert(std::make_pair(key,langItem));
	}


	void InitializeWithDefaults()
	{
		InsertLanguageString(L"FSL_MN_SETTINGS",L"Settings",true);
		InsertLanguageString(L"FSL_MN_COMPILE",L"Compile preset",true);
		InsertLanguageString(L"FSL_MN_ABOUT",L"About",true);
		InsertLanguageString(L"FSL_MN_SHOW_MEDIA_INFO",L"Show media info",true);

		InsertLanguageString(L"FSL_UI_SETTING_TITLE",L"Settings",true);
		InsertLanguageString(L"FSL_UI_SETTING_FFMPEG_DIR_STATIC",L"ffmpeg binary directory:",true);
		InsertLanguageString(L"FSL_UI_SETTING_CURRENT_LANG",L"Language:",true);
		InsertLanguageString(L"FSL_UI_SETTING_BROWSE_BUTTON",L"Browse",true);
		
		//etc.
	}

	void ParseLanguageContent()
	{
		wchar *pstr = this->langFileContent;
		wchar *temp=pstr,*key = NULL,*val = NULL;

		while (*pstr != NULL)
		{
			if (*pstr == L'=')
			{
				key = temp;
				temp = pstr+1;
				*pstr = 0;
			}
			else if (*pstr == L'\n')
			{
				if (*(pstr-1) == '\r')
					*(pstr-1) = 0;

				val = temp;
				temp = pstr+1;

				InsertLanguageString(key,val,false);

				key = val = NULL;

				*pstr = 0; 
			}
			else if (*pstr == L'\r')
				*pstr = 0;

			pstr++;
		}
	}

public:
	LanguageManager()
	{
		this->fileIo = NULL;
		this->langFileContent = NULL;

		this->langList = new LangKeyValueList();
	}

	~LanguageManager()
	{
		if (this->fileIo != NULL)
			delete this->fileIo;

		//TODO: release langItem objects
		if (this->langList != NULL)
			delete this->langList;

		FREESTRING(this->langFileContent);
	}

	static bool Initialize(wnstring langFile)
	{
		byte buffer[128];

		LanguageManager *lanMan = NULL;
		
		if (gs_LanMan != NULL && langFile == NULL)
			goto exitSuccess;
		
		lanMan = new LanguageManager();

		lanMan->InitializeWithDefaults();

		if (langFile == NULL)
			goto exitSuccess;

		lanMan->fileIo = new FileReadWrite(langFile,OpenForRead,OpenExisting);

		if (!lanMan->fileIo->Open())
		{
			delete lanMan;
			return false;
		}

		if (lanMan->fileIo->Read(buffer,0,2))
		{
			if (buffer[0] != UNICODE_MARK[0] ||
				buffer[1] != UNICODE_MARK[1])
			{
				delete lanMan;
				return false;
			}
		}

		//Release previously loaded language resources
		if (gs_LanMan != NULL)
		{
			delete gs_LanMan;
			gs_LanMan = NULL;
		}

		lanMan->langFileContent = ALLOCSTRINGW(lanMan->fileIo->GetLength());

		lanMan->fileIo->Read(
			(byte *)lanMan->langFileContent,
			-1,lanMan->fileIo->GetLength() - sizeof(UNICODE_MARK));


		lanMan->InitializeWithDefaults();
		lanMan->ParseLanguageContent();

	exitSuccess:
		gs_LanMan = lanMan;

		return true;
	}

	static void Destory()
	{
		if (gs_LanMan == NULL)
			return;

		delete gs_LanMan;
		gs_LanMan = NULL;
	}

#define _FormatV _vswprintf_p

public:

	void Format(wnstring dest, int4 destSize, wnstring key,...)
	{
		__langItemInfo *langItem=NULL;
		va_list va;
		
		va_start(va,key);

		if (!GetString(key,&langItem))
		{
			va_end(va);
			//eki eki eki 
			memset(dest,'\0u\0G',sizeof(wchar) * destSize);
			return;
		}

		_FormatV(dest,destSize,langItem->langFormatString,va);

		va_end(va);
	}

	wnstring Format2(wnstring key, ...)
	{
		__langItemInfo *langItem=NULL;
		va_list va;
		
		va_start(va,key);

		if (!GetString(key,&langItem))
		{
			va_end(va);
			return NULL;
		}

		if (langItem->formattedStringBuffer == NULL)
			return NULL;

		memset(
			langItem->formattedStringBuffer,0,
			langItem->formattedStringBufferSize * sizeof(wchar));

		_FormatV(
			langItem->formattedStringBuffer,
			langItem->formattedStringBufferSize,
			langItem->langFormatString,va);

		va_end(va);

		return langItem->formattedStringBuffer;
	}


};