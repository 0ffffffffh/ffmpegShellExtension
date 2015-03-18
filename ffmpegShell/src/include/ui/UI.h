

#ifndef __UI__
#define __UI__

#include "Synch.h"

typedef struct tagPreCreateWindowInfo
{
	BOOL centered;
	SIZE size;
}PRECREATEWINDOWINFO;

typedef INT_PTR (CALLBACK* UIDLGPROC)(HWND, UINT, WPARAM, LPARAM,PVOID);
typedef VOID (*PRECREATEWINDOWEVENT)(HWND hwnd, PRECREATEWINDOWINFO *pci);
typedef VOID (*UIAFTEREXITDISPOSER)();

typedef struct tagUiResult
{
	HWND hwnd;
	UIDLGPROC dlgProc;
	UIAFTEREXITDISPOSER uiDisposer;
	HANDLE uiThread;
	HANDLE uiEvent;
	INT_PTR result;
	UINT dlgResourceId;
	HWND parentWnd;
	VOID *param;
	BOOL isUiOutside;
	BOOL isRunning;
}UIOBJECT;

#define SetTextW(hwnd,idc,wstr) ::SetWindowTextW(::GetDlgItem(hwnd,idc),wstr)
#define SetDlgCaptionW(wnd,wstr) ::SetWindowTextW(wnd,wstr)

#define GetControlHwnd(ui,item) ::GetDlgItem(ui->hwnd,item)

BOOL UiIsRunning(UIOBJECT *ui);

VOID UiRegisterDisposer(UIOBJECT *uiObject, UIAFTEREXITDISPOSER disposer);

UIOBJECT *UiCreateDialog(UIDLGPROC dlgProc, HWND parentWnd, UINT dialogResourceId, VOID *param, PRECREATEWINDOWINFO *pci,PRECREATEWINDOWEVENT creationEvent);

VOID UiDestroyDialog(UIOBJECT *ui);

#endif //__UI__