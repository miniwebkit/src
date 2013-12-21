/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2012 Kingsoft Inc. All rights reserved.
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
 */

#include "KdValArray.h"

KdValArray::KdValArray(int iElementSize, int iPreallocSize /*= 0*/)  
    : m_pVoid(0)
    , m_nCount(0) 
    , m_iElementSize(iElementSize)
    , m_nAllocated(iPreallocSize)
{
    ASSERT(iElementSize>0);
    ASSERT(iPreallocSize>=0);
    if( iPreallocSize > 0 ) m_pVoid = static_cast<LPBYTE>(malloc(iPreallocSize * m_iElementSize));
}

KdValArray::~KdValArray()
{
    if( m_pVoid != NULL ) free(m_pVoid);
}

void KdValArray::Empty()
{   
    m_nCount = 0;  // NOTE: We keep the memory in place
}

bool KdValArray::IsEmpty() const
{
    return m_nCount == 0;
}

bool KdValArray::Add(LPCVOID pData)
{
    if( ++m_nCount >= m_nAllocated) {
        int nAllocated = m_nAllocated * 2;
        if( nAllocated == 0 ) nAllocated = 11;
        LPBYTE pVoid = static_cast<LPBYTE>(realloc(m_pVoid, nAllocated * m_iElementSize));
        if( pVoid != NULL ) {
            m_nAllocated = nAllocated;
            m_pVoid = pVoid;
        }
        else {
            --m_nCount;
            return false;
        }
    }
    ::CopyMemory(m_pVoid + ((m_nCount - 1) * m_iElementSize), pData, m_iElementSize);
    return true;
}

bool KdValArray::Remove(int iIndex)
{
    if( iIndex < 0 || iIndex >= m_nCount ) return false;
    if( iIndex < --m_nCount ) ::CopyMemory(m_pVoid + (iIndex * m_iElementSize), m_pVoid + ((iIndex + 1) * m_iElementSize), (m_nCount - iIndex) * m_iElementSize);
    return true;
}

void KdValArray::SetSize(int nSize)
{
    m_nCount = nSize;
}

int KdValArray::GetSize() const
{
    return m_nCount;
}

LPVOID KdValArray::GetData()
{
    return static_cast<LPVOID>(m_pVoid);
}

LPVOID KdValArray::GetAt(int iIndex) const
{
    if( iIndex < 0 || iIndex >= m_nCount ) return NULL;
    return m_pVoid + (iIndex * m_iElementSize);
}

bool KdValArray::Resize(int iPreallocSize)
{
    if (iPreallocSize <= m_nAllocated)
    { return true; }

    LPBYTE pVoid = static_cast<LPBYTE>(malloc(iPreallocSize * m_iElementSize));
    if (!pVoid)
    { return false; }

    ::CopyMemory(pVoid, m_pVoid, m_iElementSize * m_nCount);
    m_pVoid = pVoid;

    return true;
}

LPVOID KdValArray::operator[] (int iIndex) const
{
    ASSERT(iIndex>=0 && iIndex<m_nCount);
    return m_pVoid + (iIndex * m_iElementSize);
}

UINT __stdcall KdValArray::WriteData( const unsigned char* pData, UINT uSize )
{
	if (m_iElementSize != 1)
		return 0;

	if (m_nAllocated < uSize)
		Resize(uSize);

	memcpy(m_pVoid + m_nCount * m_iElementSize, pData, uSize);;
	m_nCount += uSize;

	return uSize;
}