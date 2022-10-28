#define _CRT_SECURE_NO_DEPRECATE
#include "TsfApp.h"
#include <assert.h>
#include <locale.h>

HRESULT TsfApp::SetupSinks()
{
	CoInitialize(NULL);
	HRESULT hr;

	hr = CoCreateInstance(CLSID_TF_ThreadMgr,
		NULL,
		CLSCTX_INPROC_SERVER,
		__uuidof(ITfThreadMgrEx),
		(void**)&m_pThreadMgrEx);

	if (FAILED(hr))
	{
		return hr;
	}

	TfClientId cid;
	//TF_TMAE_UIELEMEN0TENABLEDONLY 如不处理设置0,不然候选框不见了。
	if (FAILED(hr = m_pThreadMgrEx->ActivateEx(&cid,0 /*TF_TMAE_UIELEMEN0TENABLEDONLY*/)))
	{
		return hr;
	}

	ITfSource* srcTm;
	if (SUCCEEDED(hr = m_pThreadMgrEx->QueryInterface(__uuidof(ITfSource), (void**)&srcTm)))
	{
		
		//srcTm->AdviseSink(__uuidof(ITfUIElementSink), (ITfUIElementSink*)this, &m_dwUIElementSinkCookie);
		srcTm->AdviseSink(__uuidof(ITfInputProcessorProfileActivationSink), (ITfInputProcessorProfileActivationSink*)this, &m_dwAlpnSinkCookie);
		srcTm->Release();
	}
	else
		return hr;
	hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfiles, (LPVOID*)&m_pProfiles);

	if (FAILED(hr))
		return hr;

	m_pProfiles->QueryInterface(IID_ITfInputProcessorProfileMgr, (void**)&_pProfileMgr);
	return S_OK;
}

HRESULT TsfApp::ReleaseSinks()
{
	CoInitialize(NULL);
	HRESULT hr;

	hr = CoCreateInstance(CLSID_TF_ThreadMgr,
		NULL,
		CLSCTX_INPROC_SERVER,
		__uuidof(ITfThreadMgrEx),
		(void**)&m_pThreadMgrEx);

	if (FAILED(hr))
	{
		return hr;
	}
	ITfSource* srcTm;
	if (SUCCEEDED(hr = m_pThreadMgrEx->QueryInterface(__uuidof(ITfSource), (void**)&srcTm)))
	{

		//srcTm->UnadviseSink(m_dwUIElementSinkCookie);
		srcTm->UnadviseSink(m_dwAlpnSinkCookie);
		srcTm->Release();
	}
	else
		return hr;
	return 0;
}

STDAPI TsfApp::QueryInterface(REFIID riid, void** ppvObj)
{
	if (ppvObj == NULL)
		return E_INVALIDARG;

	*ppvObj = NULL;
	if (IsEqualIID(riid, IID_IUnknown))
		*ppvObj = reinterpret_cast<IUnknown*>(this);
	else if (IsEqualIID(riid, __uuidof(ITfUIElementSink)))
		*ppvObj = (ITfUIElementSink*)this;
	else if (IsEqualIID(riid, __uuidof(ITfInputProcessorProfileActivationSink)))
		*ppvObj = (ITfInputProcessorProfileActivationSink*)this;
	else if (IsEqualIID(riid, __uuidof(ITfLanguageProfileNotifySink)))
		*ppvObj = (ITfLanguageProfileNotifySink*)this;

	if (*ppvObj)
	{
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDAPI_(ULONG) TsfApp::AddRef()
{
	return ++_cRef;
}

STDAPI_(ULONG) TsfApp::Release()
{
	LONG cr = --_cRef;

	assert(_cRef >= 0);

	if (_cRef == 0)
		delete this;

	return cr;
}

STDAPI TsfApp::BeginUIElement(DWORD dwUIElementId, BOOL* pbShow)
{
	/* 设置为FALSE表示隐藏UI */
	*pbShow = FALSE;
	return S_OK;
}

STDAPI TsfApp::UpdateUIElement(DWORD dwUIElementId)
{
	ITfUIElementMgr* lpMgr = NULL;
	ITfCandidateListUIElement* lpCandUI = NULL;
	ITfReadingInformationUIElement* lpReading = NULL;
	ITfUIElement* pElement = NULL;

	/* 获取候选词或组合词 */
	if (SUCCEEDED(m_pThreadMgrEx->QueryInterface(IID_ITfUIElementMgr, (void**)&lpMgr)))
	{
		if (SUCCEEDED(lpMgr->GetUIElement(dwUIElementId, &pElement)))
		{
			if (SUCCEEDED(pElement->QueryInterface(IID_ITfCandidateListUIElement, (void**)&lpCandUI)))
			{
				BSTR _sss;
				lpCandUI->GetString(0, &_sss);
				SysFreeString(_sss);
				SAFE_RELEASE(lpCandUI);
			}

			if (SUCCEEDED(pElement->QueryInterface(IID_ITfReadingInformationUIElement, (void**)&lpReading)))
			{
				BSTR _sss;
				lpReading->GetString(&_sss);
				SysFreeString(_sss);

				SAFE_RELEASE(lpReading);
			}

			SAFE_RELEASE(pElement);
		}
		SAFE_RELEASE(lpMgr);
	}

	return S_OK;
}

STDAPI TsfApp::EndUIElement(DWORD dwUIElementId)
{
	ITfDocumentMgr* pDocMgr = NULL;
	ITfContext* pContex = NULL;
	ITfContextView* pContexView = NULL;

	HWND hActiveHwnd = NULL;

	if (SUCCEEDED(m_pThreadMgrEx->GetFocus(&pDocMgr)))
	{
		if (SUCCEEDED(pDocMgr->GetTop(&pContex)))
		{
			if (SUCCEEDED(pContex->GetActiveView(&pContexView)))
			{
				pContexView->GetWnd(&hActiveHwnd);
				SAFE_RELEASE(pContexView);
			}
			SAFE_RELEASE(pContex);
		}
		SAFE_RELEASE(pDocMgr);
	}

	if (NULL != hActiveHwnd)
	{
		SendMessageW(hActiveHwnd, WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 0);
	}

	return S_OK;
}

STDAPI TsfApp::OnActivated(DWORD dwProfileType, LANGID langid, REFCLSID clsid, REFGUID catid,
	REFGUID guidProfile, HKL hkl, DWORD dwFlags)
{
	bool bActive = (dwFlags & TF_IPSINK_FLAG_ACTIVE);
	if (!bActive)
		return S_OK;

	if (dwProfileType & TF_PROFILETYPE_INPUTPROCESSOR)
	{
		/* 输入法是 TIP */

		printf("TIP输入法 [%08X]\n", (unsigned int)hkl);
	}
	else if (dwProfileType & TF_PROFILETYPE_KEYBOARDLAYOUT)
	{
		/* 当前输入法是键盘布局或IME */

		printf("HKL/IME %08X\n", (unsigned int)hkl);
	}
	else
	{
		/* 不可能走到这里 */
		printf("dwProfileType unknown!!!\n");
	}
	wchar_t* c = GetCurrentIMEName();
	setlocale(LC_ALL, "");
	printf("输入法名称：%ls\n", c);
	return S_OK;
}

/* 获取HKL的显示名字
*/
bool GetLayoutName(const wchar_t* kl, wchar_t* nm)
{
	long lRet;
	HKEY hKey;
	static wchar_t tchData[64];
	DWORD dwSize;
	wchar_t keypath[200];

	wsprintfW(keypath, L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts\\%s", kl);

	lRet = RegOpenKeyExW(
		HKEY_LOCAL_MACHINE,
		keypath,
		0,
		KEY_QUERY_VALUE,
		&hKey
	);

	if (lRet == ERROR_SUCCESS)
	{
		dwSize = sizeof(tchData);
		lRet = RegQueryValueExW(
			hKey,
			L"Layout Text",
			NULL,
			NULL,
			(LPBYTE)tchData,
			&dwSize
		);
	}

	RegCloseKey(hKey);

	if (lRet == ERROR_SUCCESS && wcslen(nm) < 64)
	{
		wcscpy(nm, tchData);
		return true;
	}

	return false;
}

/* 获取当前输入法名字
*/
wchar_t* TsfApp::GetCurrentIMEName()
{
	static wchar_t _LastTipName[64];

	ZeroMemory(_LastTipName, sizeof(_LastTipName));

	TF_INPUTPROCESSORPROFILE tip;
	_pProfileMgr->GetActiveProfile(GUID_TFCAT_TIP_KEYBOARD, &tip);

	if (tip.dwProfileType == TF_PROFILETYPE_INPUTPROCESSOR)
	{
		BSTR bstrImeName = NULL;
		m_pProfiles->GetLanguageProfileDescription(tip.clsid, tip.langid, tip.guidProfile, &bstrImeName);
		if (wcslen(bstrImeName) < 64)
			wcscpy(_LastTipName, bstrImeName);

		SysFreeString(bstrImeName);
	}
	else if (tip.dwProfileType == TF_PROFILETYPE_KEYBOARDLAYOUT)
	{
		static wchar_t klnm[KL_NAMELENGTH];
		if (GetKeyboardLayoutNameW(klnm))
		{
			GetLayoutName(klnm, _LastTipName);
		}
	}

	return _LastTipName;
}

BOOL TsfApp::GetIMEName(TF_LANGUAGEPROFILE& langtip, wchar_t* name, size_t length) {
	BSTR bstrImeName = NULL;
	HRESULT ret = m_pProfiles->GetLanguageProfileDescription(langtip.clsid, langtip.langid, langtip.guidProfile, &bstrImeName);
	wcscpy_s(name, length, bstrImeName);
	SysFreeString(bstrImeName);
	return !ret;
}
wchar_t* TsfApp::NextIMEName(BOOL changeime) {
	static wchar_t _SavedTipName[64];

	IEnumTfLanguageProfiles *ppEnum;
	TF_INPUTPROCESSORPROFILE active_tip;
	TF_LANGUAGEPROFILE langtip;
	TF_LANGUAGEPROFILE _first_tip;
	ULONG pcFetch;
	BOOL first = TRUE;
	BOOL active_pos = FALSE;
	HKL hkl = GetKeyboardLayout(0);
	WORD langid = LOWORD(hkl);
	HRESULT ret = m_pProfiles->EnumLanguageProfiles(langid, &ppEnum);
	BSTR bstrImeName = NULL;
	_pProfileMgr->GetActiveProfile(GUID_TFCAT_TIP_KEYBOARD, &active_tip);
	ZeroMemory(_SavedTipName, sizeof(_SavedTipName));
	while (S_OK == ppEnum->Next(1, &langtip, &pcFetch)) {
		if (first) {
			memcpy_s(&_first_tip, sizeof(_first_tip), &langtip, sizeof(langtip));
			first = FALSE;
		}
		if (active_pos) {
			GetIMEName(langtip, _SavedTipName, _countof(_SavedTipName));
			if (changeime) {
				m_pProfiles->ActivateLanguageProfile(langtip.clsid, langtip.langid, langtip.guidProfile);
				m_pProfiles->SetDefaultLanguageProfile(langtip.langid, langtip.clsid, langtip.guidProfile);

			}
			return _SavedTipName;
		}
		if (!active_pos)
		{
			active_pos = (langtip.clsid == active_tip.clsid);
		}

	}
	GetIMEName(_first_tip, _SavedTipName, _countof(_SavedTipName));
	if (changeime) {
		m_pProfiles->ActivateLanguageProfile(_first_tip.clsid, _first_tip.langid, _first_tip.guidProfile);
		m_pProfiles->SetDefaultLanguageProfile(_first_tip.langid, _first_tip.clsid, _first_tip.guidProfile);

	}
	return _SavedTipName;
}

BOOL TsfApp::SetDefaultIME(wchar_t* name) {
	if (!name) { return FALSE; }
	IEnumTfLanguageProfiles *ppEnum;
	TF_LANGUAGEPROFILE langtip;
	wchar_t _tip_name[64] = { 0 };
	ULONG pcFetch;
	HKL hkl = GetKeyboardLayout(0);
	WORD langid = LOWORD(hkl);
	HRESULT ret = m_pProfiles->EnumLanguageProfiles(langid, &ppEnum);
	while (S_OK == ppEnum->Next(1, &langtip, &pcFetch)) {
		GetIMEName(langtip, _tip_name, _countof(_tip_name));
		if (wcscmp(_tip_name, name) == 0) {

			return S_OK == (m_pProfiles->ActivateLanguageProfile(langtip.clsid, langtip.langid, langtip.guidProfile)
				& m_pProfiles->SetDefaultLanguageProfile(langtip.langid, langtip.clsid, langtip.guidProfile));
		}
	}
	return FALSE;
}
wchar_t* TsfApp::GetDefaultIMEName() {
	static wchar_t _DefaultTipName[64];
	IEnumTfLanguageProfiles *ppEnum;
	TF_LANGUAGEPROFILE langtip;
	ULONG pcFetch;
	HKL hkl = GetKeyboardLayout(0);
	WORD langid = LOWORD(hkl);
	HRESULT ret = m_pProfiles->EnumLanguageProfiles(langid, &ppEnum);
	ZeroMemory(_DefaultTipName, sizeof(_DefaultTipName));
	while (S_OK == ppEnum->Next(1, &langtip, &pcFetch)) {

		if (S_OK == m_pProfiles->GetDefaultLanguageProfile(langtip.langid, langtip.catid, &langtip.clsid, &langtip.guidProfile))
		{
			GetIMEName(langtip, _DefaultTipName, _countof(_DefaultTipName));
			break;
		}
	}
	return _DefaultTipName;
}
size_t TsfApp::GetIMEMaxCount() {
	IEnumTfLanguageProfiles *ppEnum;
	TF_LANGUAGEPROFILE langtip;
	ULONG pcFetch;
	HKL hkl = GetKeyboardLayout(0);
	WORD langid = LOWORD(hkl);
	HRESULT ret = m_pProfiles->EnumLanguageProfiles(langid, &ppEnum);
	size_t count = 0;
	while (S_OK == ppEnum->Next(1, &langtip, &pcFetch)) {
		count++;
	}
	return count;
}