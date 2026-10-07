// Filename: browser_host.c

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <exdisp.h>
#include <mshtml.h>
#include <stdio.h>
#include <stdlib.h>
#include <ole2.h>

typedef struct {
    IOleInPlaceSiteVtbl* lpVtbl;
    IOleClientSiteVtbl*  lpClientVtbl;
    LONG                 refCount;
    HWND                 hwndParent;
    IOleObject*          pOleObject;
} CBrowserStorage;

static HRESULT STDMETHODCALLTYPE Storage_QueryInterface(IOleInPlaceSite* This, REFIID riid, void** ppvObject);
static ULONG STDMETHODCALLTYPE Storage_AddRef(IOleInPlaceSite* This);
static ULONG STDMETHODCALLTYPE Storage_Release(IOleInPlaceSite* This);

static HRESULT STDMETHODCALLTYPE Storage_GetWindow(IOleInPlaceSite* This, HWND* phwnd) {
    *phwnd = ((CBrowserStorage*)This)->hwndParent;
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE Storage_ContextSensitiveHelp(IOleInPlaceSite* This, BOOL fEnterMode) { UNREFERENCED_PARAMETER(This); UNREFERENCED_PARAMETER(fEnterMode); return E_NOTIMPL; }
static HRESULT STDMETHODCALLTYPE Storage_CanInPlaceActivate(IOleInPlaceSite* This) { UNREFERENCED_PARAMETER(This); return S_OK; }
static HRESULT STDMETHODCALLTYPE Storage_OnInPlaceActivate(IOleInPlaceSite* This) { UNREFERENCED_PARAMETER(This); return S_OK; }
static HRESULT STDMETHODCALLTYPE Storage_OnUIActivate(IOleInPlaceSite* This) { UNREFERENCED_PARAMETER(This); return S_OK; }

static HRESULT STDMETHODCALLTYPE Storage_GetWindowContext(IOleInPlaceSite* This, IOleInPlaceFrame** ppFrame, IOleInPlaceUIWindow** ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo) {
    CBrowserStorage* pThis = (CBrowserStorage*)This;
    GetClientRect(pThis->hwndParent, lprcPosRect);
    lprcPosRect->top += 50; // Leave room for toolbar
    GetClientRect(pThis->hwndParent, lprcClipRect);
    *ppFrame = NULL;
    *ppDoc = NULL;
    lpFrameInfo->cb = sizeof(OLEINPLACEFRAMEINFO);
    lpFrameInfo->fMDIApp = FALSE;
    lpFrameInfo->hwndFrame = pThis->hwndParent;
    lpFrameInfo->haccel = NULL;
    lpFrameInfo->cAccelEntries = 0;
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE Storage_Scroll(IOleInPlaceSite* This, SIZE scrollExtant) { UNREFERENCED_PARAMETER(This); UNREFERENCED_PARAMETER(scrollExtant); return E_NOTIMPL; }
static HRESULT STDMETHODCALLTYPE Storage_OnUIDeactivate(IOleInPlaceSite* This, BOOL fUndoable) { UNREFERENCED_PARAMETER(This); UNREFERENCED_PARAMETER(fUndoable); return S_OK; }
static HRESULT STDMETHODCALLTYPE Storage_OnInPlaceDeactivate(IOleInPlaceSite* This) { UNREFERENCED_PARAMETER(This); return S_OK; }
static HRESULT STDMETHODCALLTYPE Storage_DiscardUndoState(IOleInPlaceSite* This) { UNREFERENCED_PARAMETER(This); return S_OK; }
static HRESULT STDMETHODCALLTYPE Storage_DeactivateAndUndo(IOleInPlaceSite* This) { UNREFERENCED_PARAMETER(This); return S_OK; }
static HRESULT STDMETHODCALLTYPE Storage_OnPosRectChange(IOleInPlaceSite* This, LPCRECT lprcPosRect) {
    CBrowserStorage* pThis = (CBrowserStorage*)This;
    IOleInPlaceObject* pInPlaceObj = NULL;
    if (pThis->pOleObject && SUCCEEDED(pThis->pOleObject->lpVtbl->QueryInterface(pThis->pOleObject, &IID_IOleInPlaceObject, (void**)&pInPlaceObj))) {
        RECT rc = *lprcPosRect;
        rc.top += 50;
        pInPlaceObj->lpVtbl->SetObjectRects(pInPlaceObj, &rc, &rc);
        pInPlaceObj->lpVtbl->Release(pInPlaceObj);
    }
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE Storage_QueryInterface(IOleInPlaceSite* This, REFIID riid, void** ppvObject) {
    CBrowserStorage* pThis = (CBrowserStorage*)This;
    if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IOleWindow) || IsEqualIID(riid, &IID_IOleInPlaceSite)) {
        *ppvObject = This;
        Storage_AddRef(This);
        return S_OK;
    }
    if (IsEqualIID(riid, &IID_IOleClientSite)) {
        *ppvObject = &(pThis->lpClientVtbl);
        Storage_AddRef(This);
        return S_OK;
    }
    *ppvObject = NULL;
    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE Storage_AddRef(IOleInPlaceSite* This) {
    CBrowserStorage* pThis = (CBrowserStorage*)This;
    return InterlockedIncrement(&pThis->refCount);
}

static ULONG STDMETHODCALLTYPE Storage_Release(IOleInPlaceSite* This) {
    CBrowserStorage* pThis = (CBrowserStorage*)This;
    LONG res = InterlockedDecrement(&pThis->refCount);
    if (res == 0) {
        free(pThis);
    }
    return res;
}

static HRESULT STDMETHODCALLTYPE Client_QueryInterface(IOleClientSite* This, REFIID riid, void** ppvObject) {
    CBrowserStorage* pThis = (CBrowserStorage*)((BYTE*)This - sizeof(void*));
    return Storage_QueryInterface((IOleInPlaceSite*)pThis, riid, ppvObject);
}
static ULONG STDMETHODCALLTYPE Client_AddRef(IOleClientSite* This) {
    CBrowserStorage* pThis = (CBrowserStorage*)((BYTE*)This - sizeof(void*));
    return Storage_AddRef((IOleInPlaceSite*)pThis);
}
static ULONG STDMETHODCALLTYPE Client_Release(IOleClientSite* This) {
    CBrowserStorage* pThis = (CBrowserStorage*)((BYTE*)This - sizeof(void*));
    return Storage_Release((IOleInPlaceSite*)pThis);
}
static HRESULT STDMETHODCALLTYPE Client_SaveObject(IOleClientSite* This) { UNREFERENCED_PARAMETER(This); return E_NOTIMPL; }
static HRESULT STDMETHODCALLTYPE Client_GetMoniker(IOleClientSite* This, DWORD dwAssign, DWORD dwWhichMoniker, IMoniker** ppmk) { UNREFERENCED_PARAMETER(This); UNREFERENCED_PARAMETER(dwAssign); UNREFERENCED_PARAMETER(dwWhichMoniker); *ppmk = NULL; return E_NOTIMPL; }
static HRESULT STDMETHODCALLTYPE Client_GetContainer(IOleClientSite* This, IOleContainer** ppContainer) { UNREFERENCED_PARAMETER(This); *ppContainer = NULL; return E_NOINTERFACE; }
static HRESULT STDMETHODCALLTYPE Client_ShowObject(IOleClientSite* This) { UNREFERENCED_PARAMETER(This); return S_OK; }
static HRESULT STDMETHODCALLTYPE Client_OnShowWindow(IOleClientSite* This, BOOL fShow) { UNREFERENCED_PARAMETER(This); UNREFERENCED_PARAMETER(fShow); return S_OK; }
static HRESULT STDMETHODCALLTYPE Client_RequestNewObjectLayout(IOleClientSite* This) { UNREFERENCED_PARAMETER(This); return E_NOTIMPL; }

static IOleInPlaceSiteVtbl OleInPlaceSiteMethods = { Storage_QueryInterface, Storage_AddRef, Storage_Release, Storage_GetWindow, Storage_ContextSensitiveHelp, Storage_CanInPlaceActivate, Storage_OnInPlaceActivate, Storage_OnUIActivate, Storage_GetWindowContext, Storage_Scroll, Storage_OnUIDeactivate, Storage_OnInPlaceDeactivate, Storage_DiscardUndoState, Storage_DeactivateAndUndo, Storage_OnPosRectChange };
static IOleClientSiteVtbl OleClientSiteMethods = { Client_QueryInterface, Client_AddRef, Client_Release, Client_SaveObject, Client_GetMoniker, Client_GetContainer, Client_ShowObject, Client_OnShowWindow, Client_RequestNewObjectLayout };

void EnsureBrowserEmulation(void) {
    HKEY hKey;
    WCHAR appName[MAX_PATH];
    GetModuleFileNameW(NULL, appName, MAX_PATH);
    WCHAR* pExe = wcsrchr(appName, L'\\');
    if (pExe) pExe++; else pExe = appName;

    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        DWORD value = 11001;
        RegSetValueExW(hKey, pExe, 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
        RegCloseKey(hKey);
    }
}

void NavigateFile(IWebBrowser2* pBrowser) {
    if (!pBrowser)
        return;

    WCHAR path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);

    WCHAR* p = wcsrchr(path, L'\\');
    if (p)
        *(p + 1) = 0;

    WCHAR url[MAX_PATH + 64];
    swprintf_s(url, MAX_PATH + 64, L"file:///%spage.html", path);

    for (int i = 0; url[i]; i++) {
        if (url[i] == L'\\') url[i] = L'/';
    }

    BSTR b = SysAllocString(url);
    if (b) {
        VARIANT vaFlags, vaTargetFrameName, vaPostData, vaHeaders;
        VariantInit(&vaFlags); VariantInit(&vaTargetFrameName); VariantInit(&vaPostData); VariantInit(&vaHeaders);

        pBrowser->lpVtbl->Navigate(pBrowser, b, &vaFlags, &vaTargetFrameName, &vaPostData, &vaHeaders);
        SysFreeString(b);
    }
}

BOOL CreateBrowser(HWND hwndParent, IWebBrowser2** ppBrowser, IOleObject** ppOleObject) {
    CBrowserStorage* pStorage = (CBrowserStorage*)malloc(sizeof(CBrowserStorage));
    if (!pStorage) return FALSE;

    pStorage->lpVtbl = &OleInPlaceSiteMethods;
    pStorage->lpClientVtbl = &OleClientSiteMethods;
    pStorage->refCount = 1;
    pStorage->hwndParent = hwndParent;
    pStorage->pOleObject = NULL;

    HRESULT hr = CoCreateInstance(&CLSID_WebBrowser, NULL, CLSCTX_INPROC_SERVER, &IID_IOleObject, (void**)ppOleObject);
    if (FAILED(hr)) {
        free(pStorage);
        return FALSE;
    }

    pStorage->pOleObject = *ppOleObject;

    hr = (*ppOleObject)->lpVtbl->SetClientSite(*ppOleObject, (IOleClientSite*)&(pStorage->lpClientVtbl));
    if (FAILED(hr)) {
        (*ppOleObject)->lpVtbl->Release(*ppOleObject);
        *ppOleObject = NULL;
        free(pStorage);
        return FALSE;
    }

    Storage_Release((IOleInPlaceSite*)pStorage);

    hr = (*ppOleObject)->lpVtbl->QueryInterface(*ppOleObject, &IID_IWebBrowser2, (void**)ppBrowser);
    if (FAILED(hr)) return FALSE;

    RECT rc;
    GetClientRect(hwndParent, &rc);
    rc.top += 50;
    hr = (*ppOleObject)->lpVtbl->DoVerb(*ppOleObject, OLEIVERB_INPLACEACTIVATE, NULL, (IOleClientSite*)&(pStorage->lpClientVtbl), 0, hwndParent, &rc);
    if (FAILED(hr)) return FALSE;

    (*ppBrowser)->lpVtbl->put_Silent(*ppBrowser, VARIANT_TRUE);
    return TRUE;
}