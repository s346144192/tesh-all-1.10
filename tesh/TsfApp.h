#pragma once
#define _CRT_SECURE_NO_DEPRECATE
#include<windows.h>
#include<WinUser.h>
#include<msctf.h>
#include<stdio.h>
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) \
   if(x != NULL)        \
   {                    \
      x->Release();     \
      x = NULL;         \
   }
#endif

#ifndef SAFE_ARRAY_DELETE
#define SAFE_ARRAY_DELETE(x) \
   if(x != NULL)             \
   {                         \
      delete[] x;            \
      x = NULL;              \
   }
#endif

struct TsfApp : public ITfUIElementSink, public ITfInputProcessorProfileActivationSink
{
	// IUnknown  
	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	// ITfUIElementSink  
	//   WM_IME_COMPOSITION、WM_IME_NOTIFY消息的替代者  
	STDMETHODIMP BeginUIElement(DWORD dwUIElementId, BOOL* pbShow);
	STDMETHODIMP UpdateUIElement(DWORD dwUIElementId);
	STDMETHODIMP EndUIElement(DWORD dwUIElementId);

	// ITfInputProcessorProfileActivationSink  
	//   WM_INPUTLANGUAGECHANGED消息的替代者  
	STDMETHODIMP OnActivated(DWORD dwProfileType, LANGID langid, REFCLSID clsid, REFGUID catid,
		REFGUID guidProfile, HKL hkl, DWORD dwFlags);

	LONG _cRef;

	DWORD m_dwUIElementSinkCookie;
	DWORD m_dwAlpnSinkCookie;

	ITfThreadMgrEx* m_pThreadMgrEx;
	ITfInputProcessorProfiles* m_pProfiles;
	ITfInputProcessorProfileMgr* _pProfileMgr;

	HRESULT SetupSinks();
	HRESULT ReleaseSinks();

	wchar_t* GetCurrentIMEName();
	BOOL GetIMEName(TF_LANGUAGEPROFILE& langtip, wchar_t* name, size_t length);
	BOOL SetDefaultIME(wchar_t* name);
	wchar_t* GetDefaultIMEName();
	wchar_t* NextIMEName(BOOL changeime);
	size_t GetIMEMaxCount();
};
