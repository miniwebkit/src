// kdgui.cpp : Defines the entry point for the application.
//

using namespace WTF;
using namespace WTF::Unicode;

#include "stdafx.h"
//#include "kdgui.h"
#include "resource.h"
//#include "KWebPage.h"
#include "../include/kdguiapi.h"
#include <shlwapi.h>

#include "../include/npruntime.h"

// #include "F:/app_prj/Gui2/SonicUIDemo/SonicUI_src/SonicUI/include/injecttool.h"
// #pragma comment(lib, "F:/app_prj/Gui2/SonicUIDemo/SonicUI_src/SonicUI/lib/InjectTool.lib")
#pragma comment(lib, "shlwapi.lib")

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
extern WCHAR szTitle[MAX_LOADSTRING];
extern WCHAR szWindowClass[MAX_LOADSTRING];
//WebCore::KWebPage* g_pKdGUI;

KdPagePtr g_pKdGUIPage;
HWND g_hWnd;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE hInstance, int nCmdShow, LPTSTR lpCmdLine);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

#include "Hook.h"

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    MSG msg;
    HACCEL hAccelTable;

    //+++
    HMODULE hMod = NULL;
    //     hMod = GetModuleHandle(_T("msvcr80d.dll"));
    //     DWORD oldmalloc = (DWORD)GetProcAddress(hMod, "malloc");
    // 
    //     g_pOldMalloc = ReplaceFuncAndCopy((LPVOID)oldmalloc, MyMalloc);
    //     g_pOldRealloc = ReplaceFuncAndCopy(GetProcAddress(hMod, "realloc"), MyRealloc);
    //     g_pOldFree = ReplaceFuncAndCopy((LPVOID)(oldmalloc + 0xF30), MyFree);
    // 
    //     hMod = GetModuleHandle(_T("kernel32.dll"));
    //     g_pOldHeapAlloc = ReplaceFuncAndCopy(GetProcAddress(hMod, "HeapAlloc"), MyHeapAlloc);
    //     g_pOldHeapFree = ReplaceFuncAndCopy(GetProcAddress(hMod, "HeapFree"), MyHeapFree);
    //     g_pOldHeapReAlloc = ReplaceFuncAndCopy(GetProcAddress(hMod, "HeapReAlloc"), MyHeapReAlloc);

    hMod = GetModuleHandle(_T("ntdll.dll"));
    //g_pOldZwAllocateVirtualMemory = ReplaceFuncAndCopy(GetProcAddress(hMod, "ZwAllocateVirtualMemory"), MyZwAllocateVirtualMemory);
    //g_pOldZwFreeVirtualMemory = ReplaceFuncAndCopy(GetProcAddress(hMod, "ZwFreeVirtualMemory"), MyZwFreeVirtualMemory);
    //g_pOldFree = ReplaceFuncAndCopy(GetProcAddress(hMod, "_free_dbg_nolock"), MyFree);
    //--

    // Initialize global strings
//     LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
//     LoadString(hInstance, IDC_KDGUI, szWindowClass, MAX_LOADSTRING);
//     MyRegisterClass(hInstance);

    KdInitThread();

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow, lpCmdLine))
        return FALSE;

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDI_TESTKDGUI));

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            //g_pKdGUI->TimerFired();
        }
    }

    return (int) msg.wParam;
}

static bool GetUrlDataFromFile(
    /*in*/ LPCWSTR Url,
    /*in*/ LPWSTR lpPath
    )
{
    HANDLE        hFile        = NULL;
    DWORD         bytesReaded  = 0;
    UINT          DataSize     = 8;
    bool          bRet         = false;
    LARGE_INTEGER FileSize     = {0};

    hFile = CreateFileW(Url, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    if(!hFile || INVALID_HANDLE_VALUE == hFile)
    { return false; }

    bRet = GetFileSizeEx(hFile, &FileSize);
    if (!bRet || (0 == FileSize.HighPart && 0 == FileSize.LowPart))
    {goto Exit0;}

    memset(lpPath, 0, sizeof(WCHAR) * MAX_PATH);
    if(!::ReadFile(hFile, lpPath, FileSize.LowPart, &bytesReaded, NULL))
    {goto Exit0;}

    bRet = true;

Exit0:
    if (hFile)
    {::CloseHandle(hFile);}

    return bRet;
}

static void ConvPath(/* in out */ LPWSTR lpPath)
{
    int len = wcslen(lpPath);
    for (int i = 0; i < len; ++i) {
        if (L'\\' == lpPath[i])
            lpPath[i] = L'/';
    }
}

void CallScript() {
    NPIdentifier funcID = NPN_GetStringIdentifier("callback_hello");
    NPVariant result;

    NPVariant vArgs[1];
    STRINGZ_TO_NPVARIANT("Testaaaa", vArgs[0]);
    KdInvokeScript(g_pKdGUIPage, funcID, vArgs, 1, &result);

    NPN_ReleaseVariantValue(&result);
}

bool _NPInvokeFunction (NPObject *npobj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result) {
    NPString callName = NPVARIANT_TO_STRING(args[0]);
    if (0 == strnicmp(callName.UTF8Characters, "ShowHtml", callName.UTF8Length)) {
        OutputDebugStringW(L"ShowHtml");
    }
    return true;
}

static LRESULT __stdcall _PostCallback (
    KdPagePtr pKdPagePtr,
    void* pMainContext,
    void* pPageContext,
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    bool* needContinue
    ) 
{
    RECT Rect = {20, 20, 100, 100};
    if (WM_NCDESTROY == message) {
        ExitProcess(0);
        return 0;
    } else if (WM_PAINT == message) {
    } else if (WM_KEYDOWN == message) {
    } else if (WM_KEYUP == message) {
        CallScript();
    }

    return 0;
}

void CreateKdPage() {

}

static BOOL InitGUI(HINSTANCE hInstance, LPWSTR lpCmdLine)
{
    WCHAR szPath[MAX_PATH + 1] = {0};
    WCHAR szModPath[MAX_PATH + 1] = {0};
    LPWSTR pPath = L"";

//     if (3 > wcslen(lpCmdLine))
//         return FALSE;

    GetModuleFileNameW(hInstance, szModPath, MAX_PATH);

    pPath = lpCmdLine;
    pPath++;

    if (0 != wcsnicmp(pPath, L"file:///", 8)) {
        PathRemoveFileSpecW(szModPath);
        wcscpy(szPath, szModPath);
        PathAppendW(szPath, L"main.txt");
        GetUrlDataFromFile(szPath, szPath);
        if (!PathMatchSpecW(szPath, L"*:*")) {
            GetModuleFileNameW(hInstance, szModPath, MAX_PATH);
            PathRemoveFileSpecW(szModPath);
            PathAppendW(szModPath, szPath);
            pPath = szModPath;
        } else {
            pPath = szPath;
        }

    } else {
        wcscpy(szPath, pPath);
        pPath = szPath;
        pPath[wcslen(pPath) - 1] = 0;
    }

    ConvPath(pPath);

    KdPageInfo PageInfo = {0};

    PageInfo.dwStyle = WS_OVERLAPPEDWINDOW; // WS_POPUP;
    PageInfo.dwExStyle = WS_EX_APPWINDOW /*| WS_EX_LAYERED*/;

    PageInfo.X = 500;
    PageInfo.Y = 200;
    PageInfo.nWidth = 400;
    PageInfo.nHeight = 520;
    //PageInfo.DebugInfo.port = 1234;

    g_pKdGUIPage = KdCreateRealWndAttachedWebPage(0, &PageInfo, 0, true);
    KdRegisterMsgHandle(g_pKdGUIPage, 0, _PostCallback);
    KdLoadPageFormUrl(g_pKdGUIPage, pPath);
    //KdRegisterScriptInit(g_pKdGUIPage, _ScriptInitCallback);
    //KdRegisterPaintCallback(g_pKdGUIPage, KdPagePaintCallback);
    KdRegisterScriptCallback(g_pKdGUIPage, _NPInvokeFunction);
    HWND g_hWnd = KdGetHWNDFromPagePtr(g_pKdGUIPage);

    return TRUE;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, LPWSTR lpCmdLine)
{
    if (!InitGUI(hInstance, lpCmdLine))
        return FALSE;

    return TRUE;
}
