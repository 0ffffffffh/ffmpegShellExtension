// ffmpegShellCtrl.h : Declaration of the CffmpegShellCtrl

#pragma once
#include "resource.h"       // main symbols



#include "..\..\ffmpegShell_i.h"
#include <Shobjidl.h>


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

using namespace ATL;


// CffmpegShellCtrl

class ATL_NO_VTABLE CffmpegShellCtrl :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CffmpegShellCtrl, &CLSID_ffmpegShellCtrl>,
	public IDispatchImpl<IffmpegShellCtrl, &IID_IffmpegShellCtrl, &LIBID_ffmpegShellLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IShellExtInit,
	public IContextMenu
{
public:
	CffmpegShellCtrl()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FFMPEGSHELLCTRL)


BEGIN_COM_MAP(CffmpegShellCtrl)
	COM_INTERFACE_ENTRY(IffmpegShellCtrl)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IContextMenu)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease();

public:

	STDMETHOD(Initialize)(PCIDLIST_ABSOLUTE pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID);
	
	STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu,UINT idCmdFirst,UINT idCmdLast,UINT uFlags);
	
	STDMETHOD(InvokeCommand)(CMINVOKECOMMANDINFO *pici);
	
	STDMETHOD(GetCommandString)(UINT_PTR idCmd,UINT uType,UINT *pReserved, __out_awcount(!(uType & GCS_UNICODE), cchMax)  LPSTR pszName,UINT cchMax);


};

OBJECT_ENTRY_AUTO(__uuidof(ffmpegShellCtrl), CffmpegShellCtrl)
