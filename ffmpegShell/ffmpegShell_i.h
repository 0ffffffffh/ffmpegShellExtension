

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Mon Mar 23 03:08:03 2015
 */
/* Compiler settings for ffmpegShell.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __ffmpegShell_i_h__
#define __ffmpegShell_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IffmpegShellCtrl_FWD_DEFINED__
#define __IffmpegShellCtrl_FWD_DEFINED__
typedef interface IffmpegShellCtrl IffmpegShellCtrl;
#endif 	/* __IffmpegShellCtrl_FWD_DEFINED__ */


#ifndef __ffmpegShellCtrl_FWD_DEFINED__
#define __ffmpegShellCtrl_FWD_DEFINED__

#ifdef __cplusplus
typedef class ffmpegShellCtrl ffmpegShellCtrl;
#else
typedef struct ffmpegShellCtrl ffmpegShellCtrl;
#endif /* __cplusplus */

#endif 	/* __ffmpegShellCtrl_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IffmpegShellCtrl_INTERFACE_DEFINED__
#define __IffmpegShellCtrl_INTERFACE_DEFINED__

/* interface IffmpegShellCtrl */
/* [unique][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IffmpegShellCtrl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("07D8B9D9-E267-4B75-9C1C-B06FB89BEFAE")
    IffmpegShellCtrl : public IDispatch
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IffmpegShellCtrlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IffmpegShellCtrl * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IffmpegShellCtrl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IffmpegShellCtrl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IffmpegShellCtrl * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IffmpegShellCtrl * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IffmpegShellCtrl * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IffmpegShellCtrl * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } IffmpegShellCtrlVtbl;

    interface IffmpegShellCtrl
    {
        CONST_VTBL struct IffmpegShellCtrlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IffmpegShellCtrl_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IffmpegShellCtrl_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IffmpegShellCtrl_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IffmpegShellCtrl_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IffmpegShellCtrl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IffmpegShellCtrl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IffmpegShellCtrl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IffmpegShellCtrl_INTERFACE_DEFINED__ */



#ifndef __ffmpegShellLib_LIBRARY_DEFINED__
#define __ffmpegShellLib_LIBRARY_DEFINED__

/* library ffmpegShellLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_ffmpegShellLib;

EXTERN_C const CLSID CLSID_ffmpegShellCtrl;

#ifdef __cplusplus

class DECLSPEC_UUID("F44E5F86-4BA1-4359-888E-2EB257343E29")
ffmpegShellCtrl;
#endif
#endif /* __ffmpegShellLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


