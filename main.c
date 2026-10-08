// Filename: main.c

#pragma warning(disable: 4201)

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#include <exdisp.h>
#include <ole2.h>
#include <stdio.h>
#include "emoji_core.h"
#include "emoji_root.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")

#define ID_BTN_PREV       101
#define ID_BTN_NEXT       102
#define ID_CHK_FILTER     103
#define ID_LBL_PAGE       104

#define IDI_TRAY_ICON     1
#define WM_TRAYICON       (WM_APP + 1)
#define IDM_TRAY_RESTORE  201
#define IDM_TRAY_EXIT     202

int page_has_emoji(int page);

BOOL CreateBrowser(HWND hwndParent, IWebBrowser2** ppBrowser, IOleObject** ppOleObject);
void NavigateFile(IWebBrowser2* pBrowser);
void EnsureBrowserEmulation(void);

static IWebBrowser2* g_pBrowser = NULL;
static IOleObject*   g_pOleObject = NULL;

static int   g_current_page = 0; 
static int   g_filter_emoji = 1;
static HWND  g_hLblPage = NULL;
static HICON g_hAppIcon = NULL;

static void UpdatePageDisplay(void) {
    WCHAR buf[64];
    swprintf_s(buf, 64, L"Page %d / %d", g_current_page, (int)TOTAL_PAGES - 1);
    if (g_hLblPage) {
        SetWindowTextW(g_hLblPage, buf);
    }
}

static void RefreshPage(void) {
    WCHAR status[128];
    swprintf_s(status, 128, L"AVIS Neon Pager - Page %d (Range 0x%X - 0x%X)", 
        g_current_page, g_current_page * ITEMS_PER_PAGE, (g_current_page + 1) * ITEMS_PER_PAGE - 1);
    
    generate_page_html(g_current_page, g_filter_emoji, status);
    UpdatePageDisplay();

    if (g_pBrowser) {
        g_pBrowser->lpVtbl->Refresh(g_pBrowser);
    }
}

static void AddTrayIcon(HWND hwnd) {
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hwnd;
    nid.uID = IDI_TRAY_ICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = g_hAppIcon;
    wcscpy_s(nid.szTip, ARRAYSIZE(nid.szTip), L"Robo Rook's All of THEE Emoji Clipboard");
    Shell_NotifyIconW(NIM_ADD, &nid);
}

static void RemoveTrayIcon(HWND hwnd) {
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hwnd;
    nid.uID = IDI_TRAY_ICON;
    Shell_NotifyIconW(NIM_DELETE, &nid);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        EnsureBrowserEmulation();

        // Load custom icon from resources (ID 100)
        g_hAppIcon = LoadImageW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(100), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
        if (!g_hAppIcon) {
            g_hAppIcon = LoadIconW(NULL, IDI_APPLICATION);
        }
        // Set window icons
        SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)g_hAppIcon);
        SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hAppIcon);

        AddTrayIcon(hwnd);

        for (int i = 0; i < (int)TOTAL_PAGES; i++) {
            if (page_has_emoji(i)) {
                g_current_page = i;
                break;
            }
        }

        CreateWindowW(L"BUTTON", L"< Prev Emoji", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 10, 110, 30, hwnd, (HMENU)ID_BTN_PREV, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Next Emoji >", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 130, 10, 110, 30, hwnd, (HMENU)ID_BTN_NEXT, NULL, NULL);
        
        g_hLblPage = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, 250, 10, 150, 30, hwnd, (HMENU)ID_LBL_PAGE, NULL, NULL);
        
        HWND hChk = CreateWindowW(L"BUTTON", L"Emoji Only", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 410, 10, 110, 30, hwnd, (HMENU)ID_CHK_FILTER, NULL, NULL);
        SendMessageW(hChk, BM_SETCHECK, g_filter_emoji ? BST_CHECKED : BST_UNCHECKED, 0);

        RefreshPage();

        if (CreateBrowser(hwnd, &g_pBrowser, &g_pOleObject))
            NavigateFile(g_pBrowser);
        return 0;
    }

    case WM_TRAYICON: {
        if (LOWORD(lParam) == WM_RBUTTONUP || LOWORD(lParam) == WM_LBUTTONUP) {
            POINT pt;
            GetCursorPos(&pt);
            HMENU hMenu = CreatePopupMenu();
            AppendMenuW(hMenu, MF_STRING, IDM_TRAY_RESTORE, L"Show Window");
            AppendMenuW(hMenu, MF_STRING, IDM_TRAY_EXIT, L"Exit");
            
            SetForegroundWindow(hwnd);
            TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
            DestroyMenu(hMenu);
        }
        return 0;
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == IDM_TRAY_RESTORE) {
            ShowWindow(hwnd, SW_RESTORE);
            SetForegroundWindow(hwnd);
        }
        else if (id == IDM_TRAY_EXIT) {
            DestroyWindow(hwnd);
        }
        else if (id == ID_BTN_PREV) {
            int target_page = g_current_page - 1;
            while (target_page >= 0 && !page_has_emoji(target_page)) {
                target_page--;
            }
            if (target_page >= 0) {
                g_current_page = target_page;
                RefreshPage();
            }
        } 
        else if (id == ID_BTN_NEXT) {
            int target_page = g_current_page + 1;
            while (target_page < (int)TOTAL_PAGES && !page_has_emoji(target_page)) {
                target_page++;
            }
            if (target_page < (int)TOTAL_PAGES) {
                g_current_page = target_page;
                RefreshPage();
            }
        } 
        else if (id == ID_CHK_FILTER) {
            g_filter_emoji = !g_filter_emoji;
            RefreshPage();
        }
        return 0;
    }

    case WM_CLOSE:
        // Hide window to system tray instead of closing immediately
        ShowWindow(hwnd, SW_HIDE);
        return 0;

    case WM_SIZE: {
        if (g_pOleObject) {
            RECT rc = { 0, 50, LOWORD(lParam), HIWORD(lParam) };
            IOleInPlaceObject* pInPlaceObj = NULL;
            if (SUCCEEDED(g_pOleObject->lpVtbl->QueryInterface(g_pOleObject, &IID_IOleInPlaceObject, (void**)&pInPlaceObj))) {
                pInPlaceObj->lpVtbl->SetObjectRects(pInPlaceObj, &rc, &rc);
                pInPlaceObj->lpVtbl->Release(pInPlaceObj);
            }
        }
        return 0;
    }

    case WM_DESTROY:
        RemoveTrayIcon(hwnd);
        if (g_pBrowser) { 
            g_pBrowser->lpVtbl->Release(g_pBrowser); 
            g_pBrowser = NULL; 
        }
        if (g_pOleObject) { 
            g_pOleObject->lpVtbl->Close(g_pOleObject, OLECLOSE_NOSAVE); 
            g_pOleObject->lpVtbl->Release(g_pOleObject); 
            g_pOleObject = NULL; 
        }
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    UNREFERENCED_PARAMETER(hPrev); UNREFERENCED_PARAMETER(lpCmd);

    if (FAILED(OleInitialize(NULL)))
        return 0;

    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"IEHostWindow";
    wc.hIcon = LoadImageW(hInst, MAKEINTRESOURCEW(100), IMAGE_ICON, 32, 32, 0);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowW(wc.lpszClassName, L"Robo Rook's All of THEE Emoji", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 900, NULL, NULL, hInst, NULL);
    ShowWindow(hwnd, nShow);
    UpdateWindow(hwnd);
    
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    
    OleUninitialize();
    return (int)msg.wParam;
}