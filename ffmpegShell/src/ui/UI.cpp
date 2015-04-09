#include "Stdafx.h"
#include "ui\UI.h"
#include "Memory.h"
#include "Synch.h"

#include <map>

using namespace std;

#pragma comment(lib,"Comctl32.lib")

#define SWP_FLAG_ONLY_RESIZE (SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOZORDER)
#define SWP_FLAG_RESIZE_AND_MOVE (SWP_NOREPOSITION | SWP_NOZORDER)

#ifdef _DEBUG

volatile UCHAR UiStepTraceEnd=0;

#endif

extern HINSTANCE ge_ModuleInstance;

typedef struct tagIntParam
{
	WORD intParamMagic;
	void *userParam;
	WINDOWCREATIONINFO *wci;
}INTPARAM;


map<HWND,UIOBJECT *> *gp_windowMap;

extern void DmProtectVirtualSpace();
extern void DmUnprotectVirtualSpace();

void __UiReleaseResources(UIOBJECT *ui);

UIOBJECT *__UiGetMappedUiObject(HWND hwnd)
{
	map<HWND,UIOBJECT *>::iterator iter;

	if (gp_windowMap == NULL)
		return NULL;

	iter = gp_windowMap->find(hwnd);

	if (iter == gp_windowMap->end())
		return NULL;

	return iter->second;
}

#define __UiIsMappedHwnd(hwnd) (__UiGetMappedUiObject(hwnd) != NULL)

BOOL __UiMapUiObject(UIOBJECT *uiObj)
{
	if (gp_windowMap == NULL)
	{
		gp_windowMap = new map<HWND, UIOBJECT *>();
	}

	if (__UiIsMappedHwnd(uiObj->hwnd))
		return FALSE;

	gp_windowMap->insert(make_pair(uiObj->hwnd,uiObj));
	return TRUE;
}

UIOBJECT *__UiUnmapHwnd(HWND hwnd)
{
	UIOBJECT *uiObj;
	map<HWND, UIOBJECT *>::iterator iter;

	iter = gp_windowMap->find(hwnd);

	if (iter == gp_windowMap->end())
		return NULL;

	uiObj = iter->second;

	gp_windowMap->erase(iter);

	return uiObj;
}

__forceinline PVOID _UiDecodeParamPointer(PVOID param)
{
	INTPARAM *intParam;

	if (param == NULL)
		return NULL;

	intParam = (INTPARAM *)param;

	if (intParam->intParamMagic == 0x4950)
	{
		return intParam->userParam;
	}

	return param;
}

INT_PTR CALLBACK _UiMainWndProc(HWND hwnd,UINT Msg, WPARAM wParam, LPARAM lParam)
{
	INITCOMMONCONTROLSEX icc;
	INT_PTR ret;
	UIOBJECT *uiObject = __UiGetMappedUiObject(hwnd);
	PVOID validParam = NULL;

	if (uiObject != NULL)
		validParam = _UiDecodeParamPointer(uiObject->param);

	switch (Msg)
	{
	case WM_INITDIALOG:
		{
			uiObject = (UIOBJECT *)lParam;
			uiObject->hwnd = hwnd;

			__UiMapUiObject(uiObject);

			validParam = _UiDecodeParamPointer(uiObject->param);

			icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
			icc.dwICC = ICC_STANDARD_CLASSES;
			InitCommonControlsEx(&icc);

			ret = uiObject->dlgProc(hwnd,Msg,wParam,lParam,validParam);

			return ret;
		}
		break;
	case WM_CLOSE:
		{
			ret = uiObject->dlgProc(hwnd,Msg,wParam,lParam,validParam);
			uiObject->result = 0;
			DestroyWindow(hwnd); //Dispatch WM_DESTROY
			return ret;
		}
		break;
	case WM_DESTROY:
		{
			uiObject->isUiOutside = FALSE;
			PostQuitMessage(0);
		}
		break;
	default:
		{
			if (!uiObject)
				return FALSE;
		}
	}


	return uiObject->dlgProc(hwnd,Msg,wParam,lParam,validParam);
}

//We need unicode resource
#undef RT_DIALOG
#define RT_DIALOG MAKEINTRESOURCEW(5)

LPCDLGTEMPLATEW IntUiGetDialogTemplate(UINT dlgResourceId)
{
	HMODULE module = (HMODULE)ge_ModuleInstance;
	HRSRC res;
	HGLOBAL templ;
	DWORD err;

	res = FindResourceW(module,MAKEINTRESOURCEW(dlgResourceId),RT_DIALOG);

	if (res == NULL)
	{
		Win32Error();
		return NULL;
	}

	templ = LoadResource(module,res);

	return (LPCDLGTEMPLATE)templ;
}

VOID IntUiResizeAndLocateWindow(HWND hwnd, PRECREATEWINDOWINFO *pci)
{
	SIZE referenceSize;
	RECT wndRc;
	UINT swpFlag = SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE;

	int sysScrWidth,sysScrHeight;

	sysScrWidth = GetSystemMetrics(SM_CXSCREEN);
	sysScrHeight = GetSystemMetrics(SM_CYSCREEN);

	if (pci->wri.flag & WRIF_SIZE)
	{
		swpFlag &= ~SWP_NOSIZE;
		referenceSize = pci->wri.size;
	}
	else
	{
		GetWindowRect(hwnd,&wndRc);

		referenceSize.cx = wndRc.right - wndRc.left;
		referenceSize.cy = wndRc.bottom - wndRc.top;
	}

	if (pci->wri.flag & WRIF_LOCATION)
	{
		swpFlag &= ~SWP_NOMOVE;
	}

	if (pci->wri.flag & WRIF_CENTER)
	{
		if (swpFlag & SWP_NOMOVE)
			swpFlag &= ~SWP_NOMOVE;

		pci->wri.point.x = (sysScrWidth / 2) - (referenceSize.cx / 2);
		pci->wri.point.y = (sysScrHeight / 2) - (referenceSize.cy / 2);
	}

	SetWindowPos(
		hwnd,NULL,
		pci->wri.point.x,
		pci->wri.point.y,
		referenceSize.cx,
		referenceSize.cy,
		swpFlag);

}

DWORD WINAPI IntUiWorker(VOID *Arg)
{
	MSG msg;
	INTPARAM *internParam;
	UIOBJECT *uiObj = (UIOBJECT *)Arg;
	WINDOWCREATIONINFO *wciPtr = NULL;
	
	uiObj->isUiOutside = TRUE;


	internParam = (INTPARAM *)uiObj->param;

	uiObj->hwnd = CreateDialogIndirectParamW(GetModuleHandle(NULL),
												IntUiGetDialogTemplate(uiObj->dlgResourceId),
												uiObj->parentWnd,
												(DLGPROC)_UiMainWndProc,
 												(LPARAM)uiObj);

	if (internParam->wci != NULL)
	{
		wciPtr = internParam->wci;
	}


	if (wciPtr)
	{
		if (wciPtr->pci != NULL)
			IntUiResizeAndLocateWindow(uiObj->hwnd,wciPtr->pci);
	}

	InterlockedExchangePointer((volatile PVOID *)&uiObj->param,internParam->userParam);

	FREEOBJECT(internParam);

	uiObj->isRunning = TRUE;
	SetEvent(uiObj->uiEvent);
	
	ShowWindow(uiObj->hwnd,SW_NORMAL);

#ifdef _DEBUG
	if (IsDebuggerPresent())
		UiStepTraceEnd=1;
#endif

	//Enter UI Message Loop

	while (GetMessageW(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	uiObj->isRunning = FALSE;

	__UiUnmapHwnd(uiObj->hwnd);

	//Ok. The Windows Explorer can unload our module now
	if (gp_windowMap->size() == 0)
	{
		DPRINT("Unprotecting virtual space");
		DmUnprotectVirtualSpace();
		CoFreeUnusedLibrariesEx(1000,0);
	}

	__UiReleaseResources(uiObj);

	return 0;
}


BOOL IntUiCreateDialog(UIOBJECT *uiObj)
{
	DWORD tid;
	
	uiObj->uiEvent = CreateEventW(NULL,FALSE,FALSE,NULL);
	
	
	if (uiObj->seperateThread)
	{
		uiObj->uiThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)IntUiWorker,uiObj,0,&tid);
	}
	else
	{
		uiObj->uiThread = GetCurrentThread();
		IntUiWorker(uiObj);
	}

	if (WaitForSingleObject(uiObj->uiEvent,30 * 1000) == WAIT_TIMEOUT)
	{

#ifdef _DEBUG
		if (IsDebuggerPresent())
		{
			//Stall until debug step finished
			while (!UiStepTraceEnd)
				SpinWait(20);
		}
#else
		DbDebugPrint("UI Wait object has timed out!!!");
		DbSetDebugBreak(TRUE); //Force enable 
		DbDebugBreak();
#endif
	}


	CloseHandle(uiObj->uiEvent);

	return uiObj->uiThread != NULL;
}

BOOL UiIsRunning(UIOBJECT *ui)
{
	return TRUE;
}

VOID UiRegisterDisposer(UIOBJECT *uiObject, UIAFTEREXITDISPOSER disposer)
{
	uiObject->uiDisposer = disposer;
}

UIOBJECT *UiCreateDialog(
	UIDLGPROC dlgProc, 
	HWND parentWnd, 
	UINT dialogResourceId,
	BOOL seperateThread,
	PVOID param, 
	WINDOWCREATIONINFO *wci)
{
	UIOBJECT *uiObject=NULL;
	INTPARAM *internParam;

	uiObject = ALLOCOBJECT(UIOBJECT);
	uiObject->dlgResourceId = dialogResourceId;
	uiObject->seperateThread = seperateThread;
	uiObject->parentWnd = parentWnd;
	
	internParam = ALLOCOBJECT(INTPARAM);

	internParam->intParamMagic = 0x4950;
	internParam->wci = wci;
	internParam->userParam = param;

	uiObject->param = internParam;
	uiObject->dlgProc = dlgProc;

	DmProtectVirtualSpace();

	if (IntUiCreateDialog(uiObject) == FALSE)
	{
		FREEOBJECT(uiObject);
		return NULL;
	}
	else
		DmUnprotectVirtualSpace();

	return uiObject;
}


void __UiReleaseResources(UIOBJECT *ui)
{
	if (ui->uiDisposer)
		ui->uiDisposer();

	FREEOBJECT(ui);
}

void UiDestroyDialog(UIOBJECT *ui)
{
	//Who called?
	if (ui->isUiOutside) 
	{
		//Hmm. This function called from outside of UIMgr
		//Post close message and wait WM_DESTROY
		PostMessage(ui->hwnd,WM_CLOSE,0,0); 
	}
}

