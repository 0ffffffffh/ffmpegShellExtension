#pragma once

#include "stdafx.h"
#include "Memory.h"

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

#define CONVERT_TO_SIZE(x) ((x) = sizeof(T) * (x))

#define HDT_DEFAULT		0x00000000
#define HDT_DATE		0x00000002
#define HDT_TIME		0x00000004

#define mbtowc_h(s) ffhelper::Helper::AnsiToWideString((astring)s)

#define wctomb_h(s) ffhelper::Helper::WideToAnsiString((wstring)s)

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

		static wstring StringLastIndexOf(wstring str, wchar chr)
		{
			wstring ws;

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

		static wstring AnsiToWideString(astring str)
		{
			uint4 slen;
			wstring wstr;

			if (!str)
				return NULL;

			slen = lstrlenA((LPCSTR)str);

			wstr = (wstring)ALLOCSTRINGW(slen);

			if (!wstr)
				return NULL;

			if (MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(LPCSTR)str,slen,wstr,slen) == slen)
				return wstr;

			FREEARRAY(wstr);
			return NULL;
		}

		static astring WideToAnsiString(wstring str)
		{
			uint4 slen;
			astring astr;

			if (!str)
				return NULL;

			slen = lstrlenW((LPCWSTR)str);

			astr = (astring)ALLOCSTRINGA(slen);

			if (!astr)
				return NULL;

			if (WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK,(LPCWSTR)str,slen,astr,slen,NULL,NULL) == slen)
				return astr;

			FREESTRING(astr);
			return NULL;
		}

		static wstring StringIndexOf(wstring str, wchar chr)
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

		static void GetCurrentDateTimeString(wstring buffer, uint4 maxCch, uint4 flag)
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
	private:
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

	public:
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