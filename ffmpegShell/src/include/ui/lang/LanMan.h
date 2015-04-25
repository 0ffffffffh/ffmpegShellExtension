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


static LanguageManager* gs_LanMan = NULL;

const byte _UNICODE_MARK[] = {0xff,0xfe};

#define LANGSTR(str,...) gs_LanMan->Format2(L##str,__VA_ARGS__)

class LanguageManager
{
private:
	LangKeyValueList *langList;
	wnstring langFileContent;

	void FreeLanguageListItemResources()
	{
		LangKeyValueList::iterator it;

		if (this->langList == NULL)
			return;

		for (it = this->langList->begin();
			it != this->langList->end();
			it++)
		{
			if (it->second->formattedStringBuffer != NULL)
				FREESTRING(it->second->formattedStringBuffer);

			FREEOBJECT(it->second);
		}
		
		delete this->langList;
	}

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
		InsertLanguageString(L"FSL_MSG_COMPILING",L"Compiling %s...\r\n",true);
		InsertLanguageString(L"FSL_MSG_COMPILED_SUCCESS",L"Preset compiled successfuly.\r\n",true);
		InsertLanguageString(L"FSL_MSG_COMPILE_FAILED",L"Preset could not compiled.\r\n",true);
		InsertLanguageString(L"FSL_MSG_ACQUIRING_MEDIA_INFO",L"Initializing and acquiring media info...",true);
		InsertLanguageString(L"FSL_MSG_NO_STREAM_DETECTED",L"There is no valid stream metadata for this file",true);

		InsertLanguageString(L"FSL_MSG_STARTING_FFMPEG_OP",L"Initiating ffmpeg process",true);
		InsertLanguageString(L"FSL_MSG_PROCESSING_TIME",L"Processing: %02d:%02d:%02d",true);
		InsertLanguageString(L"FSL_MSG_OPERATION_COMPLETED_SUCCESSFULY",L"Operation has been completed successfuly",true);
		InsertLanguageString(L"FSL_MSG_FFMPEG_EXITED_UNEXPECTED",L"Operation failed. ffmpeg exited unexpectedly.",true);
		InsertLanguageString(L"FSL_MSG_CANCELLED_BY_USER",L"Operation cancelled by user",true);

		InsertLanguageString(L"FSL_MN_SETTINGS",L"Settings",true);
		InsertLanguageString(L"FSL_MN_COMPILE",L"Compile preset",true);
		InsertLanguageString(L"FSL_MN_ABOUT",L"About",true);
		InsertLanguageString(L"FSL_MN_SHOW_MEDIA_INFO",L"Show media info",true);

		InsertLanguageString(L"FSL_UI_GEN_OK_BUTTON",L"Ok",true);

		InsertLanguageString(L"FSL_UI_SETTING_TITLE",L"Settings",true);
		InsertLanguageString(L"FSL_UI_SETTING_FFMPEG_DIR_STATIC",L"ffmpeg binary directory:",true);
		InsertLanguageString(L"FSL_UI_SETTING_CURRENT_LANG_STATIC",L"Language:",true);
		InsertLanguageString(L"FSL_UI_SETTING_BROWSE_BUTTON",L"Browse",true);
		
		InsertLanguageString(L"FSL_UI_COMPILE_TITLE",L"Compling",true);

		InsertLanguageString(L"FSL_UI_PSTVALUE_TITLE",L"User provided variable",true);
		InsertLanguageString(L"FSL_UI_PSTVALUE_VBIT_STATIC",L"Video bitrate:",true);
		InsertLanguageString(L"FSL_UI_PSTVALUE_ABIT_STATIC",L"Audio bitrate:",true);
		InsertLanguageString(L"FSL_UI_PSTVALUE_STARTTIME_STATIC",L"Start time (-ss):",true);
		InsertLanguageString(L"FSL_UI_PSTVALUE_DURLEN_STATIC",L"Duration/Length (-t,-to):",true);

		InsertLanguageString(L"FSL_UI_PROGRESSDLG_CANCEL_BUTTON",L"Cancel",true);


		InsertLanguageString(L"FSL_UI_MEDIAINFO_TITLE",L"Media information",true);
		InsertLanguageString(L"FSL_UI_MEDIAINFO_STREAMINDEX_STATIC",L"Stream Index:",true);
		InsertLanguageString(L"FSL_UI_MEDIAINFO_CODECNAME",L"Codec Name: %s",true);
		InsertLanguageString(L"FSL_UI_MEDIAINFO_CODECTYPE",L"Codec Type: %s",true);
		InsertLanguageString(L"FSL_UI_MEDIAINFO_CODECTAG",L"Codec Tag: %s",true);
		InsertLanguageString(L"FSL_UI_MEDIAINFO_ASPECTRAT",L"Aspect Ratio: %s",true);
		InsertLanguageString(L"FSL_UI_MEDIAINFO_DURATION",L"Duration: %02d:%02d:%02d",true);
		InsertLanguageString(L"FSL_UI_MEDIAINFO_SIZE",L"Width: %d, Height: %d",true);
		InsertLanguageString(L"FSL_UI_MEDIAINFO_BITRATE",L"Bitrate: %d",true);
		InsertLanguageString(L"FSL_UI_MEDIAINFO_NUMOFFRAMES",L"Number of frames: %d",true);
		InsertLanguageString(L"FSL_UI_MEDIAINFO_SAMPRATE",L"Sample rate: %d",true);
		InsertLanguageString(L"FSL_UI_MEDIAINFO_CHANNELS",L"Channels: %d",true);

		//etc.
	}

	void ParseLanguageContent()
	{
		wchar *pstr = this->langFileContent;
		wchar *temp=pstr,*key = NULL,*val = NULL;

		while (*pstr != 0)
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
		this->langFileContent = NULL;

		this->langList = new LangKeyValueList();
	}

	~LanguageManager()
	{
		FreeLanguageListItemResources();

		FREESTRING(this->langFileContent);
	}

	static void Initialize(wnstring langFile)
	{
		byte buffer[2];
		FileReadWrite *fileIo = NULL;
		LanguageManager *lanMan = NULL;
		
		if (gs_LanMan != NULL)
		{
			if (langFile == NULL || *langFile == 0)
				return;
		}

		lanMan = new LanguageManager();

		lanMan->InitializeWithDefaults();

		fileIo = new FileReadWrite(langFile,OpenForRead,OpenExisting);

		if (!fileIo->Open())
			goto exitSuccess;

		if (fileIo->Read(buffer,0,2))
		{
			if (buffer[0] != _UNICODE_MARK[0] ||
				buffer[1] != _UNICODE_MARK[1])
			{
				fileIo->Close();
				delete fileIo;

				MessageBoxW(NULL,
					L"Language file is not unicode format. File content ignored and loaded defaults",
					L"Incorrect language file format",
					MB_ICONWARNING | MB_OK);

				goto exitSuccess;
			}
		}

		//Release previously loaded language resources
		if (gs_LanMan != NULL)
		{
			LanguageManager::Destory();
		}

		lanMan->langFileContent = ALLOCSTRINGW(fileIo->GetLength());

		fileIo->Read(
			(byte *)lanMan->langFileContent,
			-1,fileIo->GetLength() - sizeof(_UNICODE_MARK));


		fileIo->Close();
		delete fileIo;

		lanMan->ParseLanguageContent();

exitSuccess:
		if (gs_LanMan == NULL)
			gs_LanMan = lanMan;
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