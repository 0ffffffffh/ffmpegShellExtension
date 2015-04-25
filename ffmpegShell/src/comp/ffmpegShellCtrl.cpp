#pragma once

#include "stdafx.h"
#include "ffmpegShellCtrl.h"
#include "ui\MenuMgr.h"
#include "Debug.h"
#include "FileList.h"
#include "ui\UI.h"
#include "MenuHandlers.h"
#include "Settings.h"
#include "helper\ArgPack.h"
#include "ui\lang\LanMan.h"

#define FSLF_NONE				0
#define FSLF_SETTINGS_LOADED	1
#define FSLF_PRESETS_LOADED		2
#define FSLF_LANGUAGE_LOADED	4

//File system object list.
FileList*					g_fileObjectList=NULL;
HMENU						g_contextShellMenu=NULL;
MENUCONTAINER*				g_menu;
UIOBJECT*					g_uiObj;
HWND						g_activeHwnd=NULL;
FILEPATHITEM*				g_selectedFile;
DWORD						g_loadFlag=FSLF_NONE;
FORWARDED SETTINGS			g_settings;
FORWARDED LanguageManager*	gs_LanMan;


STDMETHODIMP CffmpegShellCtrl::Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj,HKEY hkeyProgID)
{
	HRESULT Hr;
	FORMATETC Fetc;
	STGMEDIUM stgMedData;
	UINT fileCount;
	wchar fileTemp[MAX_PATH];
	wnstring compiledPresetPath;
	UINT i;
	
	if (! (g_loadFlag & FSLF_SETTINGS_LOADED))
	{
		if (IntLoadSettings())
			g_loadFlag |= FSLF_SETTINGS_LOADED;
	}

	if (! (g_loadFlag & FSLF_LANGUAGE_LOADED) )
	{
		LanguageManager::Initialize(g_settings.langFilename);
		g_loadFlag |= FSLF_LANGUAGE_LOADED;
	}

	if (pdtobj == NULL)
		return S_OK; //Its a directory background handler. simply return ok

	Fetc.cfFormat = CF_HDROP;
	Fetc.ptd = 0;
	Fetc.dwAspect = DVASPECT_CONTENT;
	Fetc.lindex = -1;
	Fetc.tymed = TYMED_HGLOBAL;

	Hr = pdtobj->GetData(&Fetc,&stgMedData);

	if (!SUCCEEDED(Hr))
	{
		DBPRINT_CURR("Hresult: %lu",Hr);
		return E_FAIL;
	}

	compiledPresetPath = ffhelper::Helper::MakeAppPath(L"presets.cpf");

	if (! (g_loadFlag & FSLF_PRESETS_LOADED) )
	{
		if (PtLoadPreset(compiledPresetPath))
			g_loadFlag |= FSLF_PRESETS_LOADED;
	}

	FREESTRING(compiledPresetPath);

	
	if (g_fileObjectList == NULL)
		g_fileObjectList = new FileList();

	if (g_fileObjectList->IsReferenced())
		g_fileObjectList->Release();

	g_fileObjectList->AddRef();
	
	fileCount = DragQueryFileW((HDROP)stgMedData.hGlobal,0xFFFFFFFF,NULL,0);

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
	MENUCONTAINER *child = NULL;
	FILEPATHITEM *filePath = NULL;
	LinkedList<PRESET *> *matchedPresets = NULL;
	vptr argPack;

	g_menu = MeCreateMenuContainer(idCmdFirst,idCmdLast,uFlags);

	if (g_fileObjectList != NULL)
	{
		filePath = g_fileObjectList->GetLastObject();
	}

	MeAddItem(g_menu,MenuHandlers::ShowSettings,NULL,LANGSTR("FSL_MN_SETTINGS"));

	if (filePath != NULL)
	{
		if (!filePath->objectPartLengths[OPL_EXTENSION])
			return MeActivateMenu(g_menu,hmenu);

		if (!wcsicmp(filePath->objectExtension,L"pst"))
		{
			MeAddItem(g_menu,MenuHandlers::CompilePresetHandler,filePath,LANGSTR("FSL_MN_COMPILE"));
		}
		else
		{
			if (!( g_loadFlag & FSLF_PRESETS_LOADED))
			{
				return MeActivateMenu(g_menu,hmenu);
			}

			matchedPresets = PtGetPresetsByExtension((wchar *)filePath->objectExtension);
			
			
			if (matchedPresets != NULL)
			{
				MeAddItem(g_menu,MenuHandlers::ShowMediaInformations,filePath,LANGSTR("FSL_MN_SHOW_MEDIA_INFO"));

				if (matchedPresets->GetCount()>0)
					MeAddSeperator(g_menu);

				g_fileObjectList->AddRef();
				
				for (LinkedListNode<PRESET *> *node = matchedPresets->Begin();
					node != NULL;
					node = node->Next())
				{
					int4 packOffset=0;
					vptr nodePtr = node->GetValue();

					
					argPack = ALLOCPACKET(sizeof(PRESET *) + sizeof(FileList *));

					packOffset = WRITEPACKET(argPack,PRESET *,packOffset,&nodePtr);
					WRITEPACKET(argPack,FileList *,packOffset,&g_fileObjectList);


					MeAddItem2(g_menu,LongTimeHandler,MenuHandlers::StartConvertingOperation,argPack,(wnstring)node->GetValue()->name);
				}
			}
		}
	}

	MeAddSeperator(g_menu);

	MeAddItem(g_menu,MenuHandlers::About,NULL,LANGSTR("FSL_MN_ABOUT"));

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
	if (g_fileObjectList != NULL)
		g_fileObjectList->Release();
}

