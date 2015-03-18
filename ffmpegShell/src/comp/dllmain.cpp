// dllmain.cpp : Implementation of DllMain.

#include "stdafx.h"
#include "resource.h"
#include "..\..\ffmpegShell_i.h"
#include "dllmain.h"
#include "xdlldata.h"
#include "LinkedList.h"

extern HINSTANCE ge_ModuleInstance=0;
extern wchar ge_ModuleDirW[MAX_PATH]={0};
CffmpegShellModule _AtlModule;


void DmProtectVirtualSpace()
{
	_AtlModule.Lock(); //Prevent unload dll by explorer.exe
}

void DmUnprotectVirtualSpace()
{
	_AtlModule.Unlock(); //Release module
}

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
#ifdef _MERGE_PROXYSTUB
	if (!PrxDllMain(hInstance, dwReason, lpReserved))
		return FALSE;
#endif


	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		ge_ModuleInstance = hInstance;
		ffhelper::Helper::InitializeHelper();
		break;
	case DLL_PROCESS_DETACH:
		ffhelper::Helper::UninitializeHelper();
		break;
	}
	
	return _AtlModule.DllMain(dwReason, lpReserved); 
}
