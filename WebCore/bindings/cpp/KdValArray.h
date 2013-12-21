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

#ifndef KdValArray_h
#define KdValArray_h

#include "WebCommon.h"

// class IKdGuiBuffer
// {
// public:
// 	virtual UINT __stdcall WriteData(const unsigned char* pData, UINT uSize) = 0;
// };

class /*WEBKIT_EXPORT*/ KdValArray //: public IKdGuiBuffer
{
public:
    KdValArray(int iElementSize, int iPreallocSize);
    ~KdValArray();

    void Empty();
    bool IsEmpty() const;
    bool Add(LPCVOID pData);
    bool Remove(int iIndex);
    bool Resize(int iPreallocSize);
    int GetSize() const;
    void SetSize(int nSize);
    LPVOID GetData();
    
    LPVOID GetAt(int iIndex) const;
    LPVOID operator[] (int nIndex) const;

	UINT __stdcall WriteData(const unsigned char* pData, UINT uSize);

protected:
    LPBYTE m_pVoid;
    int m_iElementSize;
    int m_nCount;
    int m_nAllocated;
};

#endif // KdValArray_h