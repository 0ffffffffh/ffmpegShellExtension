#ifndef _DEBUG_DEF_
#define _DEBUG_DEF_

typedef enum tagPrefix
{
	UnknownPrefix = 0,
	Info = 2,
	Warning = 4,
	Error = 8
}PREFIX;

#endif

#ifdef INCLUDE_DEBUGGING_ROUTINES

void DbDebugPrintEx(LPCSTR format, PREFIX prefix, ...);

void DbDebugPrint(LPCSTR format, ...);

VOID __fastcall DbSetDebugBreak(BOOL Active);

VOID __fastcall DbDebugBreak();

#define DPRINT(str,...) DbDebugPrint(str,__VA_ARGS__)

#define DPRINTEX(str,prefix,...) DbDebugPrintEx(str,prefix,__VA_ARGS__)

#define Win32Error() DbDebugPrint("Win32 error: 0x%x, at %s",GetLastError(), __FUNCTION__)

#define CURRENTROUTINE() DPRINT("Entered %s()",__FUNCTION__)

#define DBPRINT_CURR(str, ...) DbDebugPrint(__FILE__ " -> " __FUNCTION__ " at %d | " str, __LINE__, __VA_ARGS__)

#define ASSERT(exp) { \
	char __assertbuf[256]={0}; \
	if (!(exp)) { \
		sprintf(__assertbuf,"Assertion failed in %s file at line %d. Function: %s, Expression: %s", __FILE__, __LINE__,__FUNCTION__,#exp); \
		DbDebugPrint(__assertbuf); \
	} \
}



#else

#define DbDebugPrintEx(format,prefix,...)

#define DbDebugPrint(format,...)

#define DbSetDebugBreak(active)

#define DbDebugBreak()

#define DPRINT(format,...)

#define Win32Error()

#define CURRENTROUTINE()

#define ASSERT(exp) TRUE

#endif
