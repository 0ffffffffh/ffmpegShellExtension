// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

#undef _MERGE_PROXYSTUB

#if defined(_MSC_VER)
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE (1)
#endif
#pragma warning(disable : 4996)
#endif

#pragma warning (disable : 4003)

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#include "targetver.h"

#define _ATL_APARTMENT_THREADED

#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit


#include <comsvcs.h>

#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW
#define INCLUDE_DEBUGGING_ROUTINES

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#include "Debug.h"
#include "Types.h"
#include "helper\Helper.h"

#ifdef _DEBUG
	#define NOTIMPLEMENTED_R(Defrettype) DbDebugPrint("%s, Not implemented yet.",__FUNCTION__); return Defrettype
	#define NOTIMPLEMENTED() DbDebugPrint("%s, not implemented yet.",__FUNCTION__)
#else
	#define NOTIMPLEMENTED_R(Defrettype) MessageBoxW(__FUNCTION__ " not implemented yet. Ret: " #Defrettype); return Defrettype
	#define NOTIMPLEMENTED() MessageBoxW(__FUNCTION__ " not implemented yet.")
#endif

#define FORWARDED extern

void DmProtectVirtualSpace();

void DmUnprotectVirtualSpace();

//#undef INCLUDE_DEBUGGING_ROUTINES