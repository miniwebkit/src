// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>
#include <psapi.h>

#include "ext/bitmap_platform_device_win.h"

#include "ext/bitmap_platform_device_data.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "include/core/SkUtils.h"

BOOL GdiAlphaBlend(
                   __in  HDC hdcDest,
                   __in  int xoriginDest,
                   __in  int yoriginDest,
                   __in  int wDest,
                   __in  int hDest,
                   __in  HDC hdcSrc,
                   __in  int xoriginSrc,
                   __in  int yoriginSrc,
                   __in  int wSrc,
                   __in  int hSrc,
                   __in  BLENDFUNCTION ftn
                   );

#pragma comment(lib, "gdi32.lib")

namespace skia {

BitmapPlatformDevice::BitmapPlatformDeviceData::BitmapPlatformDeviceData(
    HBITMAP hbitmap)
    : bitmap_context_(hbitmap),
      hdc_(NULL),
      config_dirty_(true),  // Want to load the config next time.
      transform_(SkMatrix::I()) {
  // Initialize the clip region to the entire bitmap.
  BITMAP bitmap_data;
  if (GetObject(bitmap_context_, sizeof(BITMAP), &bitmap_data)) {
    SkIRect rect;
    rect.set(0, 0, bitmap_data.bmWidth, bitmap_data.bmHeight);
    clip_region_ = SkRegion(rect);
  }
}

BitmapPlatformDevice::BitmapPlatformDeviceData::~BitmapPlatformDeviceData() {
  if (hdc_)
    ReleaseBitmapDC();

  // this will free the bitmap data as well as the bitmap handle
  DeleteObject(bitmap_context_);
}

HDC BitmapPlatformDevice::BitmapPlatformDeviceData::GetBitmapDC() {
  if (!hdc_) {
    hdc_ = CreateCompatibleDC(NULL);
    InitializeDC(hdc_);
    HGDIOBJ old_bitmap = SelectObject(hdc_, bitmap_context_);
    // When the memory DC is created, its display surface is exactly one
    // monochrome pixel wide and one monochrome pixel high. Since we select our
    // own bitmap, we must delete the previous one.
    DeleteObject(old_bitmap);
  }

  LoadConfig();
  return hdc_;
}

void BitmapPlatformDevice::BitmapPlatformDeviceData::ReleaseBitmapDC() {
  SkASSERT(hdc_);
  DeleteDC(hdc_);
  hdc_ = NULL;
}

bool BitmapPlatformDevice::BitmapPlatformDeviceData::IsBitmapDCCreated()
    const {
  return hdc_ != NULL;
}


void BitmapPlatformDevice::BitmapPlatformDeviceData::SetMatrixClip(
    const SkMatrix& transform,
    const SkRegion& region) {
  transform_ = transform;
  clip_region_ = region;
  config_dirty_ = true;
}

void BitmapPlatformDevice::BitmapPlatformDeviceData::LoadConfig() {
  if (!config_dirty_ || !hdc_)
    return;  // Nothing to do.
  config_dirty_ = false;

  // Transform.
  LoadTransformToDC(hdc_, transform_);
  LoadClippingRegionToDC(hdc_, clip_region_, transform_);
}

// We use this static factory function instead of the regular constructor so
// that we can create the pixel data before calling the constructor. This is
// required so that we can call the base class' constructor with the pixel
// data.
BitmapPlatformDevice* BitmapPlatformDevice::create(
    HDC screen_dc,
    int width,
    int height,
    bool is_opaque,
    HANDLE shared_section) {
  SkBitmap bitmap;

  // CreateDIBSection appears to get unhappy if we create an empty bitmap, so
  // just create a minimal bitmap
  if ((width == 0) || (height == 0)) {
    width = 1;
    height = 1;
  }

  BITMAPINFOHEADER hdr = {0};
  hdr.biSize = sizeof(BITMAPINFOHEADER);
  hdr.biWidth = width;
  hdr.biHeight = -height;  // minus means top-down bitmap
  hdr.biPlanes = 1;
  hdr.biBitCount = 32;
  hdr.biCompression = BI_RGB;  // no compression
  hdr.biSizeImage = 0;
  hdr.biXPelsPerMeter = 1;
  hdr.biYPelsPerMeter = 1;
  hdr.biClrUsed = 0;
  hdr.biClrImportant = 0;

  void* data = NULL;
  HBITMAP hbitmap = CreateDIBSection(screen_dc,
                                     reinterpret_cast<BITMAPINFO*>(&hdr), 0,
                                     &data,
                                     shared_section, 0);
  if (!hbitmap) {
    return NULL;
  }

  memset(data, 0, width*height);

  bitmap.setConfig(SkBitmap::kARGB_8888_Config, width, height);
  bitmap.setPixels(data);
  bitmap.setIsOpaque(is_opaque);

  // If we were given data, then don't clobber it!
  if (!shared_section) {
    if (is_opaque) {
#ifndef NDEBUG
      // To aid in finding bugs, we set the background color to something
      // obviously wrong so it will be noticable when it is not cleared
      bitmap.eraseARGB(255, 0, 255, 128);  // bright bluish green
#endif
    } else {
      bitmap.eraseARGB(0, 0, 0, 0);
    }
  }

  // The device object will take ownership of the HBITMAP. The initial refcount
  // of the data object will be 1, which is what the constructor expects.
  return new BitmapPlatformDevice(new BitmapPlatformDeviceData(hbitmap),
                                  bitmap);
}

// static
BitmapPlatformDevice* BitmapPlatformDevice::create(int width,
                                                   int height,
                                                   bool is_opaque,
                                                   HANDLE shared_section) {
  HDC screen_dc = GetDC(NULL);
  BitmapPlatformDevice* device = BitmapPlatformDevice::create(
      screen_dc, width, height, is_opaque, shared_section);
  ReleaseDC(NULL, screen_dc);
  return device;
}

// The device will own the HBITMAP, which corresponds to also owning the pixel
// data. Therefore, we do not transfer ownership to the SkDevice's bitmap.
BitmapPlatformDevice::BitmapPlatformDevice(
    BitmapPlatformDeviceData* data,
    const SkBitmap& bitmap)
    : SkDevice(bitmap),
      data_(data) {
  // The data object is already ref'ed for us by create().
  SkDEBUGCODE(begin_paint_count_ = 0);

  SetPlatformDevice(this, this);
}

BitmapPlatformDevice::~BitmapPlatformDevice() {
  SkASSERT(begin_paint_count_ == 0);
  data_->unref();
}

HDC BitmapPlatformDevice::BeginPlatformPaint() {
  SkDEBUGCODE(begin_paint_count_++);
  return data_->GetBitmapDC();
}

void BitmapPlatformDevice::EndPlatformPaint() {
  SkASSERT(begin_paint_count_--);
  PlatformDevice::EndPlatformPaint();
}

void BitmapPlatformDevice::setMatrixClip(const SkMatrix& transform,
                                         const SkRegion& region,
                                         const SkClipStack&) {
  data_->SetMatrixClip(transform, region);
}

void BitmapPlatformDevice::DrawToNativeLayeredContext(HDC dc, int x, int y,
                                               const RECT* src_rect) 
{
    bool created_dc = !data_->IsBitmapDCCreated();
    HDC source_dc = BeginPlatformPaint();

    RECT temp_rect;
    if (!src_rect) {
        temp_rect.left = 0;
        temp_rect.right = width();
        temp_rect.top = 0;
        temp_rect.bottom = height();
        src_rect = &temp_rect;
    }

    int copy_width = src_rect->right - src_rect->left;
    int copy_height = src_rect->bottom - src_rect->top;

    // We need to reset the translation for our bitmap or (0,0) won't be in the
    // upper left anymore
    SkMatrix identity;
    identity.reset();

    //LoadTransformToDC(source_dc, identity);
    {
#define ULW_COLORKEY            0x00000001
#define ULW_ALPHA               0x00000002
#define ULW_OPAQUE              0x00000004
        BLENDFUNCTION blend_function = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        typedef BOOL (WINAPI* PFN_UpdateLayeredWindow) (
            __in HWND hWnd,
            __in_opt HDC hdcDst,
            __in_opt POINT *pptDst,
            __in_opt SIZE *psize,
            __in_opt HDC hdcSrc,
            __in_opt POINT *pptSrc,
            __in COLORREF crKey,
            __in_opt BLENDFUNCTION *pblend,
            __in DWORD dwFlags
            );
        static PFN_UpdateLayeredWindow s_pUpdateLayeredWindow = NULL;
        if (NULL == s_pUpdateLayeredWindow) {
            s_pUpdateLayeredWindow = reinterpret_cast<PFN_UpdateLayeredWindow>(
                GetProcAddress(GetModuleHandleW(L"user32.dll"), "UpdateLayeredWindow"));
        }
        
        SIZE size = { copy_width, copy_height };
        POINT position = {x, y};
        POINT zero = { src_rect->left, src_rect->top };

        typedef struct tagUPDATELAYEREDWINDOWINFO
        {
            DWORD               cbSize;
            HDC                 hdcDst;
            POINT CONST         *pptDst;
            SIZE CONST          *psize;
            HDC                 hdcSrc;
            POINT CONST         *pptSrc;
            COLORREF            crKey;
            BLENDFUNCTION CONST *pblend;
            DWORD               dwFlags;
            RECT CONST          *prcDirty;
        } UPDATELAYEREDWINDOWINFO, *PUPDATELAYEREDWINDOWINFO;

        typedef BOOL (WINAPI* PFN_UpdateLayeredWindowIndirect) (HWND hWnd, UPDATELAYEREDWINDOWINFO const* pULWInfo);
        static PFN_UpdateLayeredWindowIndirect s_pUpdateLayeredWindowIndirect = NULL;
        static BOOL s_bHaveCheckUpdateLayeredWindowIndirect = FALSE;

        if (!s_bHaveCheckUpdateLayeredWindowIndirect) {
            s_bHaveCheckUpdateLayeredWindowIndirect = TRUE;

            if (NULL == s_pUpdateLayeredWindowIndirect) {
                s_pUpdateLayeredWindowIndirect = reinterpret_cast<PFN_UpdateLayeredWindowIndirect>(
                    GetProcAddress(GetModuleHandleW(L"user32.dll"), "UpdateLayeredWindowIndirect"));
            }
        }

        if (s_pUpdateLayeredWindowIndirect) {
            UPDATELAYEREDWINDOWINFO info = {0};
            info.cbSize = sizeof(UPDATELAYEREDWINDOWINFO);
            info.hdcDst = dc;
            info.pptDst = 0;
            info.psize = &size;
            info.hdcSrc = source_dc;
            info.pptSrc = &zero;
            info.crKey = RGB(0xFF, 0xFF, 0xFF);
            info.pblend = &blend_function;
            info.dwFlags = ULW_ALPHA;
            info.prcDirty = src_rect;
            s_pUpdateLayeredWindowIndirect(::WindowFromDC(dc), &info);
        } else {
            s_pUpdateLayeredWindow(::WindowFromDC(dc),
                dc,
                0, // &position,
                &size,
                source_dc,
                &zero,
                RGB(0xFF, 0xFF, 0xFF), &blend_function, ULW_ALPHA);
        }
    }

    //LoadTransformToDC(source_dc, data_->transform());

    EndPlatformPaint();
    if (created_dc)
        data_->ReleaseBitmapDC();
}

void BitmapPlatformDevice::DrawToNativeContext(HDC dc, int x, int y,
                                               const RECT* src_rect) {
  bool created_dc = !data_->IsBitmapDCCreated();
  HDC source_dc = BeginPlatformPaint();

  RECT temp_rect;
  if (!src_rect) {
    temp_rect.left = 0;
    temp_rect.right = width();
    temp_rect.top = 0;
    temp_rect.bottom = height();
    src_rect = &temp_rect;
  }

  int copy_width = src_rect->right - src_rect->left;
  int copy_height = src_rect->bottom - src_rect->top;

  // We need to reset the translation for our bitmap or (0,0) won't be in the
  // upper left anymore
  SkMatrix identity;
  identity.reset();

  LoadTransformToDC(source_dc, identity);
  if (isOpaque()) {
    BitBlt(dc,
           x,
           y,
           copy_width,
           copy_height,
           source_dc,
           src_rect->left,
           src_rect->top,
           SRCCOPY);
  } else {
    SkASSERT(copy_width != 0 && copy_height != 0);
    BLENDFUNCTION blend_function = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    typedef BOOL (WINAPI* PFN_GdiAlphaBlend) (
        __in  HDC hdcDest,
        __in  int xoriginDest,
        __in  int yoriginDest,
        __in  int wDest,
        __in  int hDest,
        __in  HDC hdcSrc,
        __in  int xoriginSrc,
        __in  int yoriginSrc,
        __in  int wSrc,
        __in  int hSrc,
        __in  BLENDFUNCTION ftn
        );
    static PFN_GdiAlphaBlend pGdiAlphaBlend = NULL;
    if (NULL == pGdiAlphaBlend) {
        pGdiAlphaBlend = reinterpret_cast<PFN_GdiAlphaBlend>(
            GetProcAddress(GetModuleHandleW(L"gdi32.dll"), "GdiAlphaBlend"));
    }

    pGdiAlphaBlend(dc,
                  x,
                  y,
                  copy_width,
                  copy_height,
                  source_dc,
                  src_rect->left,
                  src_rect->top,
                  copy_width,
                  copy_height,
                  blend_function);
  }
  LoadTransformToDC(source_dc, data_->transform());

  EndPlatformPaint();
  if (created_dc)
    data_->ReleaseBitmapDC();
}

void BitmapPlatformDevice::onAccessBitmap(SkBitmap* bitmap) {
  // FIXME(brettw) OPTIMIZATION: We should only flush if we know a GDI
  // operation has occurred on our DC.
  if (data_->IsBitmapDCCreated())
    GdiFlush();
}

SkDevice* BitmapPlatformDevice::onCreateCompatibleDevice(
    SkBitmap::Config config, int width, int height, bool isOpaque,
    Usage /*usage*/) {
  SkASSERT(config == SkBitmap::kARGB_8888_Config);
  return BitmapPlatformDevice::create(width, height, isOpaque, NULL);
}

// 约束position和size, 使之限定在[0, available_size]中.
// 如果|size|是-1, 填满available_size. 如果position超出available_size,
// 函数返回false.
static bool Constrain(int available_size, int* position, int* size)
{
    if(*size < -2) {
        return false;
    }

    // 规整到原点.
    if(*position < 0) {
        if(*size != -1) {
            *size += *position;
        }
        *position = 0;
    }
    if(*size==0 || *position>=available_size) {
        return false;
    }

    if(*size > 0) {
        int overflow = (*position + *size) - available_size;
        if(overflow > 0) {
            *size -= overflow;
        }
    } else {
        // 填满available_size.
        *size = available_size - *position;
    }
    return true;
}

static void DoOrNormal(DWORD dwKey, LPVOID pBuff, int nLen)
{
    if(nLen <= 0 || nLen % 4)
    {
        return;
    }
    for(DWORD * pSrc = (DWORD *)pBuff; nLen > 0; nLen -= 4, pSrc++)
    {
        *pSrc |= dwKey;
    }
}

static void OpenAlphaNormal(LPVOID pBuff, int nSize)
{
    if(nSize <= 0 || nSize % 4)
    {
        return;
    }
    DWORD * pSrc = (DWORD *)pBuff;
    for(int i = 0; i < nSize; i += 4)
    {
        if(*pSrc & 0xff000000)
        {
            *pSrc = 0;
        }
        else
        {
            *pSrc |= 0xff000000;
        }
        pSrc++;
    }
}

void BitmapPlatformDevice::makeOpaque(int x, int y, int width, int height, bool needOpenAlpha)
{
    const SkBitmap& bitmap = accessBitmap(true);
    SkASSERT(bitmap.config() == SkBitmap::kARGB_8888_Config);

    // 修改: 这种做法不太好, 不应该在这个层面处理变换. PlatformCanvas应该提供
    // 处理变换的函数(使用变换而不是变通), 传递给我们的是已经变换过的矩形.
    const SkMatrix& matrix = data_->transform();
    int bitmap_start_x = SkScalarRound(matrix.getTranslateX()) + x;
    int bitmap_start_y = SkScalarRound(matrix.getTranslateY()) + y;

    if(Constrain(bitmap.width(), &bitmap_start_x, &width) &&
        Constrain(bitmap.height(), &bitmap_start_y, &height)) {
        SkAutoLockPixels lock(bitmap);
        SkASSERT(bitmap.rowBytes()%sizeof(uint32_t) == 0u);
        size_t row_words = bitmap.rowBytes() / sizeof(uint32_t);
        // 指针指向第一个修改的像素.
        uint32_t* data = bitmap.getAddr32(0, 0) + (bitmap_start_y * row_words) + bitmap_start_x;
#if 1
        if (needOpenAlpha) {
            for(int i=0; i<height; i++) {
                for(int j=0; j<width; j++) {
                    data[j] |= (0xFF << SK_A32_SHIFT);
                }
                data += row_words;
            }
        }
#else
        if (!needOpenAlpha) {
            for(int i=0; i<height; i++) {
                for(int j=0; j<width; j++) {
                    data[j] |= 0x01000000;
                }
                data += row_words;
            }
        } else {
            for(int i=0; i<height; i++) {
                for(int j=0; j<width; j++) {
                    if(data[j] & 0xff000000) {
                        data[j] = 0;
                    } else {
                        data[j] |= 0xff000000;
                    }
                }
                data += row_words;
            }
        }
#endif
    }
}

}  // namespace skia
