#include "Stdafx.h"
#include "Memory.h"

static UCHAR DbBreakActive = 
#ifdef _DEBUG //initially set in debug mode
1
#else
0
#endif

;


#ifdef INCLUDE_DEBUGGING_ROUTINES

LPCSTR DbPrefInfo = "ffShell-Info: ";
LPCSTR DbPrefWarn = "ffShell-Warn: ";
LPCSTR DbPrefErr = "ffShell-Error: ";

void __debugPrint(LPCSTR format, va_list argList)
{
	char buffer[1024];
	uint4 p;
	
	memset(buffer,0,sizeof(buffer));

	p = _vsprintf_p(buffer,sizeof(buffer),format,argList);

	va_end(argList);

	strcat(buffer+p,"\r\n");

	OutputDebugStringA(buffer);
}

LPCSTR DbGetPrefixString(PREFIX prefix)
{
	switch (prefix)
	{
	case Info:
		return DbPrefInfo;
	case Warning:
		return DbPrefWarn;
	case Error:
		return DbPrefErr;

	}

	return NULL;
}

BOOL DbDebugSetFilter(PREFIX Px)
{
	NOTIMPLEMENTED_R(FALSE);
}

PREFIX DbDebugGetFilter()
{
	NOTIMPLEMENTED_R(UnknownPrefix);
}

void DbDebugPrintEx(LPCSTR format, PREFIX prefix, ...)
{
	va_list argList;
	char buf[1024];

	if (prefix == Unknown)
	{
		va_start(argList,prefix);
		__debugPrint(format,argList);
		va_end(argList);
		return;
	}

	memset(buf,0,sizeof(buf));

	sprintf(buf,"%s : %s",DbGetPrefixString(prefix),format);
	
	va_start(argList,prefix);

	__debugPrint(buf,argList);

	va_end(argList);
}

void DbDebugPrint(LPCSTR format,...)
{
	va_list argList;

	va_start(argList,format);

	__debugPrint(format,argList);
	
	va_end(argList);
}

void  __fastcall DbSetDebugBreak(BOOL Active)
{
#ifdef _DEBUG
	DbBreakActive=Active;
#else
	(Active);
#endif
}

void __fastcall DbDebugBreak()
{
	if (DbBreakActive)
		DebugBreak();
}

#endif