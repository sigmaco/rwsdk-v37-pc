// RwCmdLineMessage.cpp: implementation of the RwCmdLineMessage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RwCmdLineMessage.h"

//////////////////////////////////////////////////////////////////////
// Implementation of Rf3MessageEvents interface
//////////////////////////////////////////////////////////////////////

// The command line message output
STDMETHODIMP 
RwCmdLineMessage::AddMessage(BSTR strObject, BSTR strMessage, int nSeverity, int nVerbosity)
{
    if (0 == nSeverity)
    {
        wprintf(L"Error: ");
    }    

	if (NULL != strObject && L'\0' != *strObject)
    {
        wprintf(L"%s: ", strObject);
    }

	wprintf(L"%s\r\n", strMessage);

	return S_OK;
}

// Provide implementation for abstract base class
HRESULT STDMETHODCALLTYPE 
RwCmdLineMessage::QueryInterface(REFIID iid, void **ppvObject)
{
    if (iid == IID_IRf3MessageEvents)
    {
      m_dwRefCount++;
      *ppvObject = (void *)this;
      return S_OK;
    }
    if (iid == IID_IUnknown)
    {
      m_dwRefCount++;
      *ppvObject = (void *)this;
      return S_OK;
    }
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE 
RwCmdLineMessage::AddRef()
{
    m_dwRefCount++;
    return m_dwRefCount;
}

ULONG STDMETHODCALLTYPE 
RwCmdLineMessage::Release()
{
    ULONG l;
    l  = m_dwRefCount--;
    if ( 0 == m_dwRefCount)
       delete this;

    return l;
}


