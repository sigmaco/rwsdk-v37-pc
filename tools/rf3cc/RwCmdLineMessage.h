// RwCmdLineMessage.h: interface for the RwCmdLineMessage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(RwCmdLineMessage_INCLUDED_)
#define RwCmdLineMessage_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Get the message events interface in the Rf3Translator
#include "RwRf3Translator.h"
#import "..\..\export\RwRf3Translator\RwRf3Translator.tlb" named_guids raw_interfaces_only

// A simple COM object that derives from IRf3MessageEvents and implements AddMessage.
class RwCmdLineMessage : 
	public IRf3MessageEvents
{
public:
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
	RwCmdLineMessage()
	{ 
		m_dwRefCount = 0;
	};

	virtual ~RwCmdLineMessage()	{ };

public:
//////////////////////////////////////////////////////////////////////
// Implementation of _IMessageEvents interface
//////////////////////////////////////////////////////////////////////
	STDMETHOD(AddMessage)(BSTR strObject, BSTR strMessage, int nSeverity, int nVerbosity);
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject);
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

public:
	DWORD m_dwRefCount;
};

#endif // !defined(RwCmdLineMessage_INCLUDED_)
