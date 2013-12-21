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
        int i = it;
        for (i = it; i < LPMapMax - 1 && (LPVOID)-1 != KeyArr[i]; ++i) {
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
