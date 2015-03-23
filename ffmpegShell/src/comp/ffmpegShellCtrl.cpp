#include "stdafx.h"
#include "ffmpegShellCtrl.h"
#include "ui\MenuMgr.h"
#include "Debug.h"
#include "FileList.h"
#include "ui\UI.h"
#include "MenuHandlers.h"

//File system object list.
FileList					*g_fileObjectList=NULL;
HMENU						g_contextShellMenu=NULL;
MENUCONTAINER				*g_menu;
UIOBJECT					*g_uiObj;
HWND						g_activeHwnd=NULL;
FILEPATHITEM				*g_selectedFile;
bool						g_presetsLoaded=FALSE;

VOID AfterUiDestoryDisposerRoutine()
{
	CURRENTROUTINE();
	delete g_fileObjectList;
	g_fileObjectList = NULL;
}

STDMETHODIMP CffmpegShellCtrl::Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj,HKEY hkeyProgID)
{
	HRESULT Hr;
	FORMATETC Fetc;
	STGMEDIUM stgMedData;
	UINT fileCount;
	wchar fileTemp[MAX_PATH];
	wnstring compiledPresetPath;
	UINT i;

	Fetc.cfFormat = CF_HDROP;
	Fetc.ptd = 0;
	Fetc.dwAspect = DVASPECT_CONTENT;
	Fetc.lindex = -1;
	Fetc.tymed = TYMED_HGLOBAL;

	Hr = pdtobj->GetData(&Fetc,&stgMedData);

	if (!SUCCEEDED(Hr))
	{
		DbDebugBreak();
		return E_FAIL;
	}

	compiledPresetPath = ffhelper::Helper::MakeAppPath(L"preset.cpf");

	g_presetsLoaded = PtLoadPreset(compiledPresetPath);

	FREESTRING(compiledPresetPath);

	DbDebugPrint("fileObjectList = %p",g_fileObjectList);

	if (g_fileObjectList == NULL)
		g_fileObjectList = new FileList();
	
	fileCount = DragQueryFileW((HDROP)stgMedData.hGlobal,0xFFFFFFFF,NULL,0);

	DbDebugPrint("Selected Object Count=%d",fileCount);


	if (fileCount > 2)
	{
		ReleaseStgMedium(&stgMedData);
		return E_FAIL;
	}

	for (i=0;i<fileCount;i++)
	{
		DragQueryFileW((HDROP)stgMedData.hGlobal,i,fileTemp,MAX_PATH);
		g_fileObjectList->Add(fileTemp);

		if (!wcsicmp(g_fileObjectList->GetLastObject()->objectExtension,L"exe") ||
			!wcsicmp(g_fileObjectList->GetLastObject()->objectExtension,L"dll"))
		{
			ReleaseStgMedium(&stgMedData);
			delete g_fileObjectList;
			g_fileObjectList = NULL;
			return E_FAIL;
		}
	}

	ReleaseStgMedium(&stgMedData);

	return S_OK;
}

STDMETHODIMP CffmpegShellCtrl::QueryContextMenu(HMENU hmenu, UINT indexMenu,UINT idCmdFirst,UINT idCmdLast,UINT uFlags)
{	
	MENUCONTAINER *child;
	FILEPATHITEM *filePath;
	LinkedList<PRESET *> *matchedPresets;
	
	g_menu = MeCreateMenuContainer(idCmdFirst,idCmdLast,uFlags);

	filePath = g_fileObjectList->GetLastObject();

	if (filePath != NULL)
	{
		if (!wcsicmp(filePath->objectExtension,L"pst"))
		{
			MeAddItem(g_menu,MenuHandlers::CompilePresetHandler,filePath,L"Compile preset");
		}
		else
		{
			if (!g_presetsLoaded)
			{
				MeDeleteMenuContainer(g_menu);
				return E_FAIL;
			}

			matchedPresets = PtGetPresetsByExtension((wchar *)filePath->objectExtension);
			
			if (matchedPresets != NULL)
			{
				for (LinkedListNode<PRESET *> *node = matchedPresets->Begin();
					node != NULL;
					node = node->Next())
				{
					MeAddItem(g_menu,MenuHandlers::StartConvertingOperation,filePath,(wnstring)node->GetValue()->name);
					MeAddItem(g_menu,MenuHandlers::ShowMediaInformations,filePath,L"Show Video Info");
				}
			}
		}
	}

	return MeActivateMenu(g_menu,hmenu);
}
	
STDMETHODIMP CffmpegShellCtrl::InvokeCommand(CMINVOKECOMMANDINFO *pici)
{
	BOOL success=FALSE;
	uint4 offset;

	CURRENTROUTINE();

	if (!HIWORD(pici->lpVerb))
	{
		offset = LOWORD(pici->lpVerb);
		MeInvokeHandler(g_menu,offset);
	}
	
	DPRINT("invoking : %x",pici->lpVerb);

	return success ? S_OK : S_FALSE;
}

static wchar psz[] = L"DFTVRB";
	
STDMETHODIMP CffmpegShellCtrl::GetCommandString(UINT_PTR idCmd,UINT uType,UINT *pReserved, __out_awcount(!(uType & GCS_UNICODE), cchMax)  LPSTR pszName,UINT cchMax)
{
	USES_CONVERSION;
	UINT Verb;
	LPCWSTR Str = psz;//change to dynamic

	BOOL IsUnicode = ((uType & GCS_UNICODE) == GCS_UNICODE);

	CURRENTROUTINE();

	Verb = IsUnicode ? GCS_VERBW : GCS_VERBA;

	if ((uType & Verb) == Verb)
	{
		if (Str != NULL)
		{
			if (IsUnicode)
				lstrcpynW((LPWSTR)pszName,Str,cchMax);
			else
				lstrcpynA(pszName,W2CA(Str),cchMax);
		}
		else
			return E_INVALIDARG;
	}
	else
		return E_INVALIDARG;

	return S_OK;
}

void CffmpegShellCtrl::FinalRelease()
{
	CURRENTROUTINE();
}

