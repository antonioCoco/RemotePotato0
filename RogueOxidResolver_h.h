

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0622 */
/* at Tue Jan 19 04:14:07 2038
 */
/* Compiler settings for RogueOxidResolver.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0622 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __RogueOxidResolver_h_h__
#define __RogueOxidResolver_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IRemUnknown_FWD_DEFINED__
#define __IRemUnknown_FWD_DEFINED__
typedef interface IRemUnknown IRemUnknown;

#endif 	/* __IRemUnknown_FWD_DEFINED__ */


#ifndef __IRemUnknown2_FWD_DEFINED__
#define __IRemUnknown2_FWD_DEFINED__
typedef interface IRemUnknown2 IRemUnknown2;

#endif 	/* __IRemUnknown2_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_RogueOxidResolver_0000_0000 */
/* [local] */ 

typedef GUID CLSID;

typedef GUID IID;

typedef MIDL_uhyper ID;

typedef MIDL_uhyper OXID;

typedef MIDL_uhyper OID;

typedef MIDL_uhyper SETID;

typedef GUID IPID;

typedef GUID CID;

typedef const GUID *REFIPID;

#define	MAX_REQUESTED_INTERFACES	( 0x8000 )

#define	MAX_REQUESTED_PROTSEQS	( 0x8000 )

typedef struct tagCOMVERSION
    {
    unsigned short MajorVersion;
    unsigned short MinorVersion;
    } 	COMVERSION;

typedef struct tagORPC_EXTENT
    {
    GUID id;
    unsigned long size;
    /* [size_is] */ byte data[ 1 ];
    } 	ORPC_EXTENT;

typedef struct tagORPC_EXTENT_ARRAY
    {
    unsigned long size;
    unsigned long reserved;
    /* [unique][size_is][size_is] */ ORPC_EXTENT **extent;
    } 	ORPC_EXTENT_ARRAY;

typedef struct tagORPCTHIS
    {
    COMVERSION version;
    unsigned long flags;
    unsigned long reserved1;
    CID cid;
    /* [unique] */ ORPC_EXTENT_ARRAY *extensions;
    } 	ORPCTHIS;

typedef struct tagORPCTHAT
    {
    unsigned long flags;
    /* [unique] */ ORPC_EXTENT_ARRAY *extensions;
    } 	ORPCTHAT;

typedef struct tagDUALSTRINGARRAY
    {
    unsigned short wNumEntries;
    unsigned short wSecurityOffset;
    /* [size_is] */ unsigned short aStringArray[ 1 ];
    } 	DUALSTRINGARRAY;


enum tagCPFLAGS
    {
        CPFLAG_PROPAGATE	= 0x1,
        CPFLAG_EXPOSE	= 0x2,
        CPFLAG_ENVOY	= 0x4
    } ;
typedef struct tagMInterfacePointer
    {
    unsigned long ulCntData;
    /* [size_is] */ byte abData[ 1 ];
    } 	MInterfacePointer;

typedef /* [unique] */ MInterfacePointer *PMInterfacePointer;

typedef struct tagErrorObjectData
    {
    DWORD dwVersion;
    DWORD dwHelpContext;
    IID iid;
    /* [string][unique] */ wchar_t *pszSource;
    /* [string][unique] */ wchar_t *pszDescription;
    /* [string][unique] */ wchar_t *pszHelpFile;
    } 	ErrorObjectData;



extern RPC_IF_HANDLE __MIDL_itf_RogueOxidResolver_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_RogueOxidResolver_0000_0000_v0_0_s_ifspec;

#ifndef __IObjectExporter_INTERFACE_DEFINED__
#define __IObjectExporter_INTERFACE_DEFINED__

/* interface IObjectExporter */
/* [unique][uuid] */ 

/* [idempotent] */ error_status_t ResolveOxid( 
    /* [in] */ handle_t hRpc,
    /* [in] */ OXID *pOxid,
    /* [in] */ unsigned short cRequestedProtseqs,
    /* [size_is][ref][in] */ unsigned short arRequestedProtseqs[  ],
    /* [ref][out] */ DUALSTRINGARRAY **ppdsaOxidBindings,
    /* [ref][out] */ IPID *pipidRemUnknown,
    /* [ref][out] */ DWORD *pAuthnHint);

/* [idempotent] */ error_status_t SimplePing( 
    /* [in] */ handle_t hRpc,
    /* [in] */ SETID *pSetId);

/* [idempotent] */ error_status_t ComplexPing( 
    /* [in] */ handle_t hRpc,
    /* [out][in] */ SETID *pSetId,
    /* [in] */ unsigned short SequenceNum,
    /* [in] */ unsigned short cAddToSet,
    /* [in] */ unsigned short cDelFromSet,
    /* [size_is][unique][in] */ OID AddToSet[  ],
    /* [size_is][unique][in] */ OID DelFromSet[  ],
    /* [out] */ unsigned short *pPingBackoffFactor);

/* [idempotent] */ error_status_t ServerAlive( 
    /* [in] */ handle_t hRpc);

/* [idempotent] */ error_status_t ResolveOxid2( 
    /* [in] */ handle_t hRpc,
    /* [in] */ OXID *pOxid,
    /* [in] */ unsigned short cRequestedProtseqs,
    /* [size_is][ref][in] */ unsigned short arRequestedProtseqs[  ],
    /* [ref][out] */ DUALSTRINGARRAY **ppdsaOxidBindings,
    /* [ref][out] */ IPID *pipidRemUnknown,
    /* [ref][out] */ DWORD *pAuthnHint,
    /* [ref][out] */ COMVERSION *pComVersion);

/* [idempotent] */ error_status_t ServerAlive2( 
    /* [in] */ handle_t hRpc,
    /* [ref][out] */ COMVERSION *pComVersion,
    /* [ref][out] */ DUALSTRINGARRAY **ppdsaOrBindings,
    /* [ref][out] */ DWORD *pReserved);



extern RPC_IF_HANDLE IObjectExporter_v0_0_c_ifspec;
extern RPC_IF_HANDLE IObjectExporter_v0_0_s_ifspec;
#endif /* __IObjectExporter_INTERFACE_DEFINED__ */

/* interface __MIDL_itf_RogueOxidResolver_0000_0001 */
/* [local] */ 

typedef struct tagSTDOBJREF
    {
    unsigned long flags;
    unsigned long cPublicRefs;
    OXID oxid;
    OID oid;
    IPID ipid;
    } 	STDOBJREF;

typedef struct tagREMQIRESULT
    {
    HRESULT hResult;
    STDOBJREF std;
    } 	REMQIRESULT;

typedef struct tagREMINTERFACEREF
    {
    IPID ipid;
    unsigned long cPublicRefs;
    unsigned long cPrivateRefs;
    } 	REMINTERFACEREF;

typedef /* [disable_consistency_check] */ REMQIRESULT *PREMQIRESULT;

typedef /* [disable_consistency_check] */ MInterfacePointer *PMInterfacePointerInternal;



extern RPC_IF_HANDLE __MIDL_itf_RogueOxidResolver_0000_0001_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_RogueOxidResolver_0000_0001_v0_0_s_ifspec;

#ifndef __IRemUnknown_INTERFACE_DEFINED__
#define __IRemUnknown_INTERFACE_DEFINED__

/* interface IRemUnknown */
/* [uuid][object] */ 


EXTERN_C const IID IID_IRemUnknown;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000131-0000-0000-C000-000000000046")
    IRemUnknown : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RemQueryInterface( 
            /* [in] */ REFIPID ripid,
            /* [in] */ unsigned long cRefs,
            /* [in] */ unsigned short cIids,
            /* [size_is][in] */ IID *iids,
            /* [size_is][size_is][out] */ PREMQIRESULT *ppQIResults) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemAddRef( 
            /* [in] */ unsigned short cInterfaceRefs,
            /* [size_is][in] */ REMINTERFACEREF InterfaceRefs[  ],
            /* [size_is][out] */ HRESULT *pResults) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemRelease( 
            /* [in] */ unsigned short cInterfaceRefs,
            /* [size_is][in] */ REMINTERFACEREF InterfaceRefs[  ]) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IRemUnknownVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRemUnknown * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRemUnknown * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRemUnknown * This);
        
        HRESULT ( STDMETHODCALLTYPE *RemQueryInterface )( 
            IRemUnknown * This,
            /* [in] */ REFIPID ripid,
            /* [in] */ unsigned long cRefs,
            /* [in] */ unsigned short cIids,
            /* [size_is][in] */ IID *iids,
            /* [size_is][size_is][out] */ PREMQIRESULT *ppQIResults);
        
        HRESULT ( STDMETHODCALLTYPE *RemAddRef )( 
            IRemUnknown * This,
            /* [in] */ unsigned short cInterfaceRefs,
            /* [size_is][in] */ REMINTERFACEREF InterfaceRefs[  ],
            /* [size_is][out] */ HRESULT *pResults);
        
        HRESULT ( STDMETHODCALLTYPE *RemRelease )( 
            IRemUnknown * This,
            /* [in] */ unsigned short cInterfaceRefs,
            /* [size_is][in] */ REMINTERFACEREF InterfaceRefs[  ]);
        
        END_INTERFACE
    } IRemUnknownVtbl;

    interface IRemUnknown
    {
        CONST_VTBL struct IRemUnknownVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRemUnknown_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IRemUnknown_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IRemUnknown_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IRemUnknown_RemQueryInterface(This,ripid,cRefs,cIids,iids,ppQIResults)	\
    ( (This)->lpVtbl -> RemQueryInterface(This,ripid,cRefs,cIids,iids,ppQIResults) ) 

#define IRemUnknown_RemAddRef(This,cInterfaceRefs,InterfaceRefs,pResults)	\
    ( (This)->lpVtbl -> RemAddRef(This,cInterfaceRefs,InterfaceRefs,pResults) ) 

#define IRemUnknown_RemRelease(This,cInterfaceRefs,InterfaceRefs)	\
    ( (This)->lpVtbl -> RemRelease(This,cInterfaceRefs,InterfaceRefs) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IRemUnknown_INTERFACE_DEFINED__ */


#ifndef __IRemUnknown2_INTERFACE_DEFINED__
#define __IRemUnknown2_INTERFACE_DEFINED__

/* interface IRemUnknown2 */
/* [uuid][object] */ 


EXTERN_C const IID IID_IRemUnknown2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000143-0000-0000-C000-000000000046")
    IRemUnknown2 : public IRemUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RemQueryInterface2( 
            /* [in] */ REFIPID ripid,
            /* [in] */ unsigned short cIids,
            /* [size_is][in] */ IID *iids,
            /* [size_is][out] */ HRESULT *phr,
            /* [size_is][out] */ PMInterfacePointerInternal *ppMIF) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IRemUnknown2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRemUnknown2 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRemUnknown2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRemUnknown2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *RemQueryInterface )( 
            IRemUnknown2 * This,
            /* [in] */ REFIPID ripid,
            /* [in] */ unsigned long cRefs,
            /* [in] */ unsigned short cIids,
            /* [size_is][in] */ IID *iids,
            /* [size_is][size_is][out] */ PREMQIRESULT *ppQIResults);
        
        HRESULT ( STDMETHODCALLTYPE *RemAddRef )( 
            IRemUnknown2 * This,
            /* [in] */ unsigned short cInterfaceRefs,
            /* [size_is][in] */ REMINTERFACEREF InterfaceRefs[  ],
            /* [size_is][out] */ HRESULT *pResults);
        
        HRESULT ( STDMETHODCALLTYPE *RemRelease )( 
            IRemUnknown2 * This,
            /* [in] */ unsigned short cInterfaceRefs,
            /* [size_is][in] */ REMINTERFACEREF InterfaceRefs[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *RemQueryInterface2 )( 
            IRemUnknown2 * This,
            /* [in] */ REFIPID ripid,
            /* [in] */ unsigned short cIids,
            /* [size_is][in] */ IID *iids,
            /* [size_is][out] */ HRESULT *phr,
            /* [size_is][out] */ PMInterfacePointerInternal *ppMIF);
        
        END_INTERFACE
    } IRemUnknown2Vtbl;

    interface IRemUnknown2
    {
        CONST_VTBL struct IRemUnknown2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRemUnknown2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IRemUnknown2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IRemUnknown2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IRemUnknown2_RemQueryInterface(This,ripid,cRefs,cIids,iids,ppQIResults)	\
    ( (This)->lpVtbl -> RemQueryInterface(This,ripid,cRefs,cIids,iids,ppQIResults) ) 

#define IRemUnknown2_RemAddRef(This,cInterfaceRefs,InterfaceRefs,pResults)	\
    ( (This)->lpVtbl -> RemAddRef(This,cInterfaceRefs,InterfaceRefs,pResults) ) 

#define IRemUnknown2_RemRelease(This,cInterfaceRefs,InterfaceRefs)	\
    ( (This)->lpVtbl -> RemRelease(This,cInterfaceRefs,InterfaceRefs) ) 


#define IRemUnknown2_RemQueryInterface2(This,ripid,cIids,iids,phr,ppMIF)	\
    ( (This)->lpVtbl -> RemQueryInterface2(This,ripid,cIids,iids,phr,ppMIF) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IRemUnknown2_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_RogueOxidResolver_0000_0003 */
/* [local] */ 

#define	MIN_ACTPROP_LIMIT	( 1 )

#define	MAX_ACTPROP_LIMIT	( 10 )

typedef struct _customREMOTE_REQUEST_SCM_INFO
    {
    DWORD ClientImpLevel;
    /* [range] */ unsigned short cRequestedProtseqs;
    /* [size_is] */ unsigned short *pRequestedProtseqs;
    } 	customREMOTE_REQUEST_SCM_INFO;

typedef struct _customREMOTE_REPLY_SCM_INFO
    {
    OXID Oxid;
    DUALSTRINGARRAY *pdsaOxidBindings;
    IPID ipidRemUnknown;
    DWORD authnHint;
    COMVERSION serverVersion;
    } 	customREMOTE_REPLY_SCM_INFO;

typedef struct tagInstantiationInfoData
    {
    CLSID classId;
    DWORD classCtx;
    DWORD actvflags;
    long fIsSurrogate;
    /* [range] */ DWORD cIID;
    DWORD instFlag;
    /* [size_is] */ IID *pIID;
    DWORD thisSize;
    COMVERSION clientCOMVersion;
    } 	InstantiationInfoData;

typedef struct tagLocationInfoData
    {
    /* [string] */ wchar_t *machineName;
    DWORD processId;
    DWORD apartmentId;
    DWORD contextId;
    } 	LocationInfoData;

typedef struct tagActivationContextInfoData
    {
    long clientOK;
    long bReserved1;
    DWORD dwReserved1;
    DWORD dwReserved2;
    MInterfacePointer *pIFDClientCtx;
    MInterfacePointer *pIFDPrototypeCtx;
    } 	ActivationContextInfoData;

typedef struct tagCustomHeader
    {
    DWORD totalSize;
    DWORD headerSize;
    DWORD dwReserved;
    DWORD destCtx;
    /* [range] */ DWORD cIfs;
    CLSID classInfoClsid;
    /* [size_is] */ CLSID *pclsid;
    /* [size_is] */ DWORD *pSizes;
    DWORD *pdwReserved;
    } 	CustomHeader;

typedef struct tagPropsOutInfo
    {
    /* [range] */ DWORD cIfs;
    /* [size_is] */ IID *piid;
    /* [size_is] */ HRESULT *phresults;
    /* [size_is] */ MInterfacePointer **ppIntfData;
    } 	PropsOutInfo;

typedef struct tagSecurityInfoData
    {
    DWORD dwAuthnFlags;
    COSERVERINFO *pServerInfo;
    DWORD *pdwReserved;
    } 	SecurityInfoData;

typedef struct tagScmRequestInfoData
    {
    DWORD *pdwReserved;
    customREMOTE_REQUEST_SCM_INFO *remoteRequest;
    } 	ScmRequestInfoData;

typedef struct tagScmReplyInfoData
    {
    DWORD *pdwReserved;
    customREMOTE_REPLY_SCM_INFO *remoteReply;
    } 	ScmReplyInfoData;

typedef struct tagInstanceInfoData
    {
    /* [string] */ wchar_t *fileName;
    DWORD mode;
    MInterfacePointer *ifdROT;
    MInterfacePointer *ifdStg;
    } 	InstanceInfoData;

typedef /* [public] */ 
enum __MIDL___MIDL_itf_RogueOxidResolver_0000_0003_0001
    {
        SPD_FLAG_USE_CONSOLE_SESSION	= 0x1,
        SPD_FLAG_USE_DEFAULT_AUTHN_LVL	= 0x2
    } 	SPD_FLAGS;

typedef struct tagSpecialPropertiesData
    {
    unsigned long dwSessionId;
    long fRemoteThisSessionId;
    long fClientImpersonating;
    long fPartitionIDPresent;
    DWORD dwDefaultAuthnLvl;
    GUID guidPartition;
    DWORD dwPRTFlags;
    DWORD dwOrigClsctx;
    DWORD dwFlags;
    DWORD Reserved1;
    unsigned __int64 Reserved2;
    DWORD Reserved3[ 5 ];
    } 	SpecialPropertiesData;

typedef struct tagSpecialPropertiesData_Alternate
    {
    unsigned long dwSessionId;
    long fRemoteThisSessionId;
    long fClientImpersonating;
    long fPartitionIDPresent;
    DWORD dwDefaultAuthnLvl;
    GUID guidPartition;
    DWORD dwPRTFlags;
    DWORD dwOrigClsctx;
    DWORD dwFlags;
    DWORD Reserved3[ 8 ];
    } 	SpecialPropertiesData_Alternate;



extern RPC_IF_HANDLE __MIDL_itf_RogueOxidResolver_0000_0003_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_RogueOxidResolver_0000_0003_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


