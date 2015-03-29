#pragma once

#include "stdafx.h"
#include "Memory.h"
#include "Types.h"

//http://msdn.microsoft.com/en-us/library/windows/desktop/hh706898(v=vs.85).aspx


typedef BOOL ( WINAPI *PWOA)(
  _In_      VOID volatile *Address,
  _In_      PVOID CompareAddress,
  _In_      SIZE_T AddressSize,
  _In_opt_  DWORD dwMilliseconds
);

static HMODULE kernel32;
static bool isWin8;
static PWOA pfnWaitOnAddress;

extern wchar ge_ModuleDirW[MAX_PATH];

#define CONVERT_TO_SIZE(x) ((x) = sizeof(T) * (x))

#define HDT_DEFAULT		0x00000000
#define HDT_DATE		0x00000002
#define HDT_TIME		0x00000004

#define mbtowc_h(s) ffhelper::Helper::AnsiToWideString((anstring)s)

#define wctomb_h(s) ffhelper::Helper::WideToAnsiString((wnstring)s)

extern HINSTANCE ge_ModuleInstance;

struct stringComparer
{
public:
	bool operator() (const wchar *x, const wchar *y) const
	{
		return wcscmp(x,y) < 0;
	}

};

namespace ffhelper
{
	class Helper
	{
	public:
		static void InitializeHelper()
		{
			OSVERSIONINFOW osVer;

			GetVersionExW(&osVer);

			isWin8 = osVer.dwMajorVersion == 6 && osVer.dwMinorVersion >= 2;


			if (isWin8)
			{
				kernel32 = LoadLibraryW(L"kernel32.dll");
		
				if (kernel32)
				{
					pfnWaitOnAddress = (PWOA)GetProcAddress(kernel32,"WaitOnAddress");
				}
			}
		}

		static void UninitializeHelper()
		{
			if (isWin8)
				FreeLibrary(kernel32);
		}

		static wnstring PathJoin(wnstring base, wnstring fileName)
		{
			uint4 baseLen,fileNameLen,newLen;
			wnstring newPath;

			if (base == NULL || fileName == NULL)
				return NULL;

			baseLen = wcslen(base);
			fileNameLen = wcslen(fileName);

			newPath = ALLOCSTRINGW(baseLen + fileNameLen);

			if (newPath == NULL)
				return NULL;

			if (*(base + baseLen-1) == L'\\')
				baseLen--;

			wcsncpy(newPath,base,baseLen);

			if (*fileName != L'\\')
				wcscat(newPath,L"\\");
				
			wcscat(newPath,fileName);

			return newPath;
		}

		static wnstring MakeAppPath(wnstring fileName)
		{
			uint4 size;
			wnstring appPath=NULL;
			wnstring modulePath = NULL;
			
			if (fileName == NULL)
				return NULL;

			modulePath = GetModulePath();

			size = wcslen(modulePath);
			size += wcslen(fileName);
			appPath = ALLOCSTRINGW(size);

			wcscpy(appPath,modulePath);
			
			wcscat(appPath,fileName + ((int)(*fileName == L'\\')));
			
			return (wnstring)appPath;
		}

		static wnstring GetModulePath()
		{
			uint4 len;
			wchar *ptr;
			ptr = ge_ModuleDirW;

			if (*ptr != 0)
				return (wnstring)ge_ModuleDirW;

			len = (uint4)GetModuleFileNameW((HMODULE)ge_ModuleInstance,ge_ModuleDirW,MAX_PATH);
			ptr += len;
			
			while (*ptr != L'\\')
			{
				*ptr-- = 0;
			}

			return (wnstring)ge_ModuleDirW;
		}

		static wnstring StringLastIndexOf(wnstring str, wchar chr)
		{
			wnstring ws;

			if (!str)
				return NULL;

			ws = str + wcslen(str);

			while (ws != str && *ws-- != chr);

			if (ws == str)
			{
				if (*ws == chr)
					return str;
				return NULL;
			}

			return ws+1;
		}

		static wnstring AnsiToWideString(anstring str)
		{
			uint4 slen;
			wnstring wstr;

			if (!str)
				return NULL;

			slen = lstrlenA((LPCSTR)str);

			wstr = (wnstring)ALLOCSTRINGW(slen);

			if (!wstr)
				return NULL;

			if (MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(LPCSTR)str,slen,wstr,slen) == slen)
				return wstr;

			FREEARRAY(wstr);
			return NULL;
		}

		static anstring WideToAnsiString(wnstring str)
		{
			uint4 slen;
			anstring astr;

			if (!str)
				return NULL;

			slen = lstrlenW((LPCWSTR)str);

			astr = (anstring)ALLOCSTRINGA(slen);

			if (!astr)
				return NULL;

			if (WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK,(LPCWSTR)str,slen,astr,slen,NULL,NULL) == slen)
				return astr;

			FREESTRING(astr);
			return NULL;
		}

		static wnstring StringIndexOf(wnstring str, wchar chr)
		{
			if (!str)
				return NULL;

			return wcschr(str,chr);
		}

		static void DelayExecution(uint4 milliseconds)
		{
			uint4 p=0,v=0;

			if (isWin8)
			{
				if (!milliseconds)
					milliseconds = 1;
				else if (milliseconds == INFINITE)
					milliseconds = 0;

				pfnWaitOnAddress(&p,&v,sizeof(uint4),milliseconds);
			}
			else 
				SleepEx(milliseconds,FALSE);
		}

		static void GetCurrentDateTimeString(wnstring buffer, uint4 maxCch, uint4 flag)
		{
			int4 wl;
			SYSTEMTIME currTime;
			GetLocalTime(&currTime);

			wl = GetDateFormatW(LOCALE_SYSTEM_DEFAULT,DATE_SHORTDATE,&currTime,NULL,buffer,maxCch);
			buffer[wl-1] = L' ';
			GetTimeFormatW(LOCALE_SYSTEM_DEFAULT,0,&currTime,NULL,buffer+wl,maxCch-wl);
		}
	};

	template <class T>
	class DataManipHelper
	{
	public:
		static void ShiftBlock(T *data, uint4 index, uint4 shiftCount, uint4 dataLen, bool fillZero, bool toLeft)
		{
			const int4 m[2] = {1,-1};
			byte *startPtr = NULL,*destPtr=NULL;
			int4 shiftSize, moveSize;

			if (!data)
				return;
			
			shiftSize = sizeof(T) * shiftCount;

			startPtr = (byte *)(data + index);
			destPtr = startPtr + (shiftSize * m[toLeft]);

			moveSize = (sizeof(T) * (dataLen - index)) + shiftSize ;

			memmove(destPtr,startPtr,moveSize);

			if (fillZero)
				memset(startPtr,0,abs(destPtr - startPtr));
		}

		static uint4 EraseBlock(T *data, uint4 filledLen, uint4 location, uint4 count)
		{
			byte *byteData = (byte *)data;
			
			CONVERT_TO_SIZE(filledLen);
			CONVERT_TO_SIZE(location);
			CONVERT_TO_SIZE(count);

			memset(data+location,0,count);
		
			if (location+count < filledLen) 
			{
				memmove(data+location,data+location+count,filledLen-(location+count));
				memset(data+(filledLen-count),0,count);
			}

			
			return count / sizeof(T);
		}


		static void ShiftBlockToRight(T *data, uint4 index, uint4 shiftCount, uint4 dataLen, bool fillZero)
		{
			DataManipHelper<T>::ShiftBlock(data,index,shiftCount,dataLen,fillZero,false);
		}

		static void ShiftBlockToLeft(T *data, uint4 index, uint4 shiftCount, uint4 dataLen,bool fillZero)
		{
			DataManipHelper<T>::ShiftBlock(data,index,shiftCount,dataLen,fillZero,true);
		}

		static void Copy(T *dataBlock, uint4 copyPos, T *source, uint4 count)
		{
			byte *bytePtr;

			CONVERT_TO_SIZE(copyPos);
			CONVERT_TO_SIZE(count);

			bytePtr = ((byte *)dataBlock) + copyPos;
			memcpy(bytePtr,(void *)source,count);
		}

	};

	
}