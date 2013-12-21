// kdgui.cpp : Defines the entry point for the application.
//

using namespace WTF;
using namespace WTF::Unicode;

#include "stdafx.h"
#include "kdgui.h"
#include "KWebPage.h"
#include "kdguiapi.h"
#include <shlwapi.h>

#include "F:/app_prj/Gui2/SonicUIDemo/SonicUI_src/SonicUI/include/injecttool.h"
#pragma comment(lib, "F:/app_prj/Gui2/SonicUIDemo/SonicUI_src/SonicUI/lib/InjectTool.lib")

#pragma comment(lib, "shlwapi.lib")

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
extern WCHAR szTitle[MAX_LOADSTRING];
extern WCHAR szWindowClass[MAX_LOADSTRING];
WebCore::KWebPage* g_pKdGUI;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE hInstance, int nCmdShow, LPTSTR lpCmdLine);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

//++

int g_WeolarSystemMem = 0;
int g_WeolarHeapMem = 0;
int g_VirtualMemory = 0;

void* g_pOldMalloc = 0;
void* g_pOldFree = 0;
void* g_pOldRealloc = 0;

void* g_pOldHeapAlloc = 0;
void* g_pOldHeapFree = 0;
void* g_pOldHeapReAlloc = 0;

void* g_pOldZwAllocateVirtualMemory = 0;
void* g_pOldZwFreeVirtualMemory = 0;

extern "C"  void * __cdecl MyMalloc (size_t nSize)
{
    void * retPoint = 0;
    g_WeolarSystemMem += nSize;

    __asm
    {
        //push [ebp + 0ch]
        push [ebp + 8h];
        call [g_pOldMalloc];
        mov [retPoint], eax
        add  esp,4;
    }
    return retPoint;
}

extern "C" void * __cdecl MyRealloc(
    void * pUserData,
    size_t nNewSize
    )
{
    void * retPoint = 0;
    int nDataSize = 0;
    if (pUserData) {
        nDataSize = *(int*)((byte*)pUserData - 0x10);
        g_WeolarSystemMem -= nDataSize;
    }
    
    g_WeolarSystemMem += nNewSize;
    __asm
    {
        push [ebp + 0ch]
        push [ebp + 8h];
        call [g_pOldRealloc];
        mov [retPoint], eax;
        add  esp,8;
    }
    return retPoint;
}

extern "C" void __cdecl MyFree(void * pUserData, int nBlockUse)
{
    int nDataSize = *(int*)((byte*)pUserData - 0x10);
    g_WeolarSystemMem -= nDataSize;
    __asm
    {
        push [ebp + 0ch]
        push [ebp + 8h];
        call [g_pOldFree];
        add  esp,8;
    }
}

typedef struct _HeapHead {
    DWORD dwSize;
    DWORD dwFlag;
}MyHeapHead;

#define LPMapMax 5000

class LPMap {
public:
    LPMap() {
        KeyArr = (LPVOID*)malloc(LPMapMax*sizeof(LPVOID));
        memset(KeyArr, -1, LPMapMax*sizeof(LPVOID));
        ValueArr = (DWORD*)malloc(LPMapMax*sizeof(DWORD));
    }

    int find(LPVOID Key) {
        BOOL bSuc;
        int ret = find1(Key, bSuc);
        if (bSuc)
        { return ret; }

        return -1;
    }

    // 
    int find1(LPVOID Key, BOOL& bSuc) {
        int i = 0;
        bSuc = FALSE;
        for (i = 0; i < LPMapMax; ++i) {
            if (KeyArr[i] == Key) {
                bSuc = TRUE;
                return i;
            }

            if ((LPVOID)-1 == KeyArr[i]) {
                bSuc = FALSE;
                return i;
            }
        }
        return i;
    }

    void insert(LPVOID Key, DWORD Value) {
        BOOL bSuc;
        if (NULL == Key || (LPVOID)-1 == Key)
        { return; }

        int it = find1(Key, bSuc);
        if (bSuc || LPMapMax <= it) {
            __asm int 3;
        }

        KeyArr[it] = Key;
        ValueArr[it] = Value;
    }

    void erase(int it) {
        for (int i = it; i < LPMapMax - 1 && (LPVOID)-1 != KeyArr[i]; ++i) {
            KeyArr[i] = KeyArr[i + 1];
            ValueArr[i] = ValueArr[i + 1];
        }
        if (LPMapMax - 1 == i)
        { KeyArr[i] = (LPVOID)-1;}
    }

    LPVOID* KeyArr;
    DWORD* ValueArr;
};

LPMap g_MapHead;

LPVOID
WINAPI
MyHeapAlloc(
    __in HANDLE hHeap,
    __in DWORD dwFlags,
    __in SIZE_T dwBytes
    )
{
//     MyHeapHead* retPoint = 0;
//     int nDataSize = 0;
//     g_WeolarHeapMem += dwBytes;
//     dwBytes += sizeof(MyHeapHead);
//     __asm
//     {
//         push [dwBytes]
//         push [dwFlags];
//         push [hHeap];
//         call [g_pOldHeapAlloc];
//         mov [retPoint], eax;
//     }
//     retPoint->dwSize = dwBytes - sizeof(MyHeapHead);
//     retPoint->dwFlag = 0x12345678;
// 
//     return retPoint + 1;
    
    LPVOID retPoint;
    g_WeolarHeapMem += dwBytes;
    __asm
    {
        push [dwBytes]
        push [dwFlags];
        push [hHeap];
        call [g_pOldHeapAlloc];
        mov [retPoint], eax;
    }

    //g_MapHead.insert(retPoint, dwBytes);
    return retPoint;

}

BOOL
WINAPI
MyHeapFree(
    __inout HANDLE hHeap,
    __in    DWORD dwFlags,
    __deref LPVOID lpMem
    )
{
//     BOOL bRet = FALSE;
//     MyHeapHead* retPoint = (MyHeapHead*)lpMem;
//     int nDataSize = 0;
// 
//     if (retPoint && 0x12345678 == retPoint->dwFlag)
//     { nDataSize = retPoint->dwSize; }
// 
//     g_WeolarHeapMem -= nDataSize;
// 
//     __asm
//     {
//         push [lpMem]
//         push [dwFlags];
//         push [hHeap];
//         call [g_pOldHeapFree];
//         mov [bRet], eax;
//     }
// 
//     return bRet;

    BOOL bRet = FALSE;
    int Size = 0;
    if (lpMem) {
        Size = HeapSize(hHeap, dwFlags, lpMem);
        g_WeolarHeapMem -= Size;

        int it = g_MapHead.find(lpMem); 
        if(-1 == it) {
            //没找到
        }
        else {
            //找到
            g_WeolarHeapMem -= g_MapHead.ValueArr[it];
            g_MapHead.erase(it);
        }
    }   

    __asm
    {
        push [lpMem]
        push [dwFlags];
        push [hHeap];
        call [g_pOldHeapFree];
        mov [bRet], eax;
    }

    return bRet;
}

LPVOID
WINAPI
MyHeapReAlloc(
    __inout HANDLE hHeap,
    __in    DWORD dwFlags,
    __deref LPVOID lpMem,
    __in    SIZE_T dwBytes
    )
{
    LPVOID pRet;
    int Size = 0;
    if (lpMem) {
        Size = HeapSize(hHeap, dwFlags, lpMem);
        g_WeolarHeapMem -= Size;

        int it = g_MapHead.find(lpMem); 
        if(-1 == it) {
            //没找到
        }
        else {
            //找到
            g_WeolarHeapMem -= g_MapHead.ValueArr[it];
            g_MapHead.erase(it);
        }
    }
    __asm
    {
        push [dwBytes]
        push [lpMem];
        push [dwFlags];
        push [hHeap];
        call [g_pOldHeapReAlloc];
        mov [pRet], eax;
    }
    g_MapHead.insert(pRet, dwBytes);

    return pRet;
}

long WINAPI MyZwAllocateVirtualMemory(
    __in     HANDLE ProcessHandle,
    __inout  PVOID *BaseAddress,
    __in     ULONG_PTR ZeroBits,
    __inout  PSIZE_T RegionSize,
    __in     ULONG AllocationType,
    __in     ULONG Protect
    )
{
    long lRet;
    if (RegionSize)
    { g_VirtualMemory += *RegionSize; }
    if (g_VirtualMemory > 12687667)
    {
        __asm int 3;
    }
    __asm
    {
        push [Protect]
        push [AllocationType];
        push [RegionSize];
        push [ZeroBits];
        push [BaseAddress];
        push [ProcessHandle];
        call [g_pOldZwAllocateVirtualMemory];
        mov [lRet], eax;
    }
    return lRet;
}

long WINAPI MyZwFreeVirtualMemory(
    __in     HANDLE ProcessHandle,
    __inout  PVOID *BaseAddress,
    __inout  PSIZE_T RegionSize,
    __in     ULONG FreeType
    )
{
    long lRet;
    if (RegionSize)
    { g_VirtualMemory -= *RegionSize; }
    __asm
    {
        push [FreeType]
        push [RegionSize];
        push [BaseAddress];
        push [ProcessHandle];
        call [g_pOldZwAllocateVirtualMemory];
        mov [lRet], eax;
    }
    return lRet;
}
//---

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
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_KDGUI, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

    KdInitThread();

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow, lpCmdLine))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_KDGUI));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

            //g_pKdGUI->TimerFired();
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KDGUI));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_KDGUI);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
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
            {lpPath[i] = L'/';}
    }
}

static BOOL InitGUI(HINSTANCE hInstance, LPWSTR lpCmdLine, HWND hWnd)
{
    WCHAR szPath[MAX_PATH + 1] = {0};
    WCHAR szModPath[MAX_PATH + 1] = {0};
    LPWSTR pPath = NULL;

    if (3 > wcslen(lpCmdLine))
    { return FALSE; }

    GetModuleFileNameW(hInstance, szModPath, MAX_PATH);

    pPath = lpCmdLine;
    pPath++;
//     if (wcslen(pPath) < wcslen(lpCmdLine) + 11)
//     { return FALSE; }

//     if (0 != strnicmp(pPath, szModPath, strlen(szModPath)))
//     { return FALSE; }

    //pPath += wcslen(szModPath) + 1;
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
    g_pKdGUI->Init(hWnd);
    g_pKdGUI->loadFormUrl(pPath);
    //g_pWebFrame->loadFormUrl("F:/app_prj/qt-win-opensource-src-4.5.3/revecter/test.svg");clipPath4 button2

    return TRUE;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, LPWSTR lpCmdLine)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable
   g_pKdGUI = new WebCore::KWebPage(0, 0);
   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 800, 800, NULL, NULL, hInstance, NULL);

   if (!hWnd)
    {return FALSE;}

   if (!InitGUI(hInstance, lpCmdLine, hWnd))
    {return FALSE;}

   ::SetTimer(hWnd, 123, 10, NULL);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void CallScript() {
    NPIdentifier funcID = NPN_GetStringIdentifier("callback_hello");
    NPVariant result;

    NPVariant vArgs[1];
    STRINGZ_TO_NPVARIANT("Testaaaa", vArgs[0]);
    g_pKdGUI->invokeScript(funcID, vArgs, 1, &result);

    NPN_ReleaseVariantValue(&result);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	//PAINTSTRUCT ps;
	//HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
        CallScript();
// 		wmId    = LOWORD(wParam);
// 		wmEvent = HIWORD(wParam);
// 		// Parse the menu selections:
// 		switch (wmId)
// 		{
// 		case IDM_ABOUT:
// 			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
// 			break;
// 		case IDM_EXIT:
// 			DestroyWindow(hWnd);
// 			break;
// 		default:
// 			return DefWindowProc(hWnd, message, wParam, lParam);
// 		}

		break;
    case WM_TIMER:
        g_pKdGUI->timerFired();
        break;
    case WM_SIZE:
        g_pKdGUI->resizeEvent(hWnd, message, wParam, lParam);
	case WM_PAINT:
        g_pKdGUI->paintEvent(hWnd, message, wParam, lParam);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MOUSEMOVE:
        g_pKdGUI->mouseEvent(hWnd, message, wParam, lParam);
        break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
