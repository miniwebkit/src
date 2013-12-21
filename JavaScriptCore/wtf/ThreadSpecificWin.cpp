/*
 * Copyright (C) 2009 Jian Li <jianli@chromium.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"

#include "ThreadSpecific.h"

#pragma init_seg(lib)

#if USE(PTHREADS)
#error This file should not be compiled by ports that do not use Windows native ThreadSpecific implementation.
#endif

namespace WTF {

long& ThreadSpecTlsKeyCount()
{
    static long count = 0;
    return count;
}

static DWORD s_winTlsBufIdx = TlsAlloc();
static DWORD s_winTlsBufUsingIdx = TlsAlloc();

static LPVOID* GetOrInitOurThreadTls(BOOL*& tlsBufUsing)
{
    LPVOID* tlsBuf = (LPVOID*)TlsGetValue(s_winTlsBufIdx);
    tlsBufUsing = (BOOL*)TlsGetValue(s_winTlsBufUsingIdx);
    if (!tlsBuf) {
        tlsBuf = new LPVOID[kMaxTlsKeySize];
        memset(tlsBuf, 0, sizeof(LPVOID)*kMaxTlsKeySize);
        BOOL bSuc = TlsSetValue(s_winTlsBufIdx, tlsBuf);
        ASSERT(bSuc);

        ASSERT(!tlsBufUsing);
        tlsBufUsing = new BOOL[kMaxTlsKeySize];
        memset(tlsBufUsing, 0, sizeof(BOOL)*kMaxTlsKeySize);
        bSuc = TlsSetValue(s_winTlsBufUsingIdx, tlsBufUsing);
        ASSERT(bSuc);
    }

    return tlsBuf;
}

void ThreadSpecificInitTls()
{
    if (s_winTlsBufIdx == TLS_OUT_OF_INDEXES || s_winTlsBufUsingIdx == TLS_OUT_OF_INDEXES)
        CRASH();

    BOOL* tlsBufUsing = NULL;
    GetOrInitOurThreadTls(tlsBufUsing);
}

LPVOID WinTlsTryGetValue( __in DWORD dwTlsIndex )
{
    ASSERT(-1 != s_winTlsBufIdx);
    ASSERT(-1 != s_winTlsBufUsingIdx);
    ASSERT(dwTlsIndex < kMaxTlsKeySize);

    LPVOID* tlsBuf = (LPVOID*)TlsGetValue(s_winTlsBufIdx);
    BOOL* tlsBufUsing = (BOOL*)TlsGetValue(s_winTlsBufUsingIdx);
    if (tlsBuf && tlsBufUsing && tlsBufUsing[dwTlsIndex])
        {return tlsBuf[dwTlsIndex];}

    return NULL;    
}

BOOL WinTlshasInit( __in DWORD dwTlsIndex )
{
    ASSERT(-1 != s_winTlsBufIdx);
    ASSERT(-1 != s_winTlsBufUsingIdx);
    ASSERT(dwTlsIndex < kMaxTlsKeySize);

    BOOL* tlsBufUsing = (BOOL*)TlsGetValue(s_winTlsBufUsingIdx);
    ASSERT(tlsBufUsing);

    return tlsBufUsing[dwTlsIndex];
}

LPVOID WinTlsGetValue( __in DWORD dwTlsIndex )
{
    ASSERT(-1 != s_winTlsBufIdx);
    ASSERT(-1 != s_winTlsBufUsingIdx);
    ASSERT(dwTlsIndex < kMaxTlsKeySize);

    LPVOID* tlsBuf = (LPVOID*)TlsGetValue(s_winTlsBufIdx);
    BOOL* tlsBufUsing = (BOOL*)TlsGetValue(s_winTlsBufUsingIdx);
    ASSERT(tlsBuf && tlsBufUsing);
    ASSERT(TRUE == tlsBufUsing[dwTlsIndex]);

    return tlsBuf[dwTlsIndex];
}

BOOL WinTlsSetValue( __in DWORD dwTlsIndex, __in_opt LPVOID lpTlsValue, BOOL bIsPtr )
{
    ASSERT(-1 != s_winTlsBufIdx);
    ASSERT(-1 != s_winTlsBufUsingIdx);
    ASSERT(dwTlsIndex < kMaxTlsKeySize);

    BOOL* tlsBufUsing = NULL;
    LPVOID* tlsBuf = GetOrInitOurThreadTls(tlsBufUsing);

    ASSERT(tlsBuf);

    if (bIsPtr)
    { ASSERT(FALSE == tlsBufUsing[dwTlsIndex]); }
    
    tlsBuf[dwTlsIndex] = lpTlsValue;
    tlsBufUsing[dwTlsIndex] = TRUE;

    return TRUE;
}

DWORD WinTlsAlloc( VOID )
{
    DWORD tlsIdx = InterlockedIncrement(&ThreadSpecTlsKeyCount()) - 1;
    if (kMaxTlsKeySize < tlsIdx)
    { return TLS_OUT_OF_INDEXES; }
    return tlsIdx;
}

void ThreadSpecificThreadExit()
{
    //notImplemented();
    __asm int 3;
    for (long i = 0; i < ThreadSpecTlsKeyCount(); i++) {
        // The layout of ThreadSpecific<T>::Data does not depend on T. So we are safe to do the static cast to ThreadSpecific<int> in order to access its data member.
        ThreadSpecific<int>::Data* data = static_cast<ThreadSpecific<int>::Data*>(WinTlsGetValue(i));
        if (data)
            data->destructor(data);
    }
}

} // namespace WTF
