
#ifndef __skia_ext_platform_device_win_h__
#define __skia_ext_platform_device_win_h__

#pragma once

#include <windows.h>

#include <vector>

#include "SkDevice.h"

class SkMatrix;
class SkPath;
class SkRegion;

namespace skia
{

    // PlatformDevice是SkBitmap的基本封装, 为SkCanvas提供绘图表面. 设备为Windows提供
    // 了一个可写的表面, 且提供了与GDI绘图函数配合的功能. PlatformDevice是抽象类必须
    // 被派生实现, 要么使用后台位图, 要么不用.
    class PlatformDevice : public SkDevice
    {
    public:
        typedef HDC PlatformSurface;

        // 位图对应的DC, GDI操作通过它在位图中绘制. 这是一个重量级的接口, 所以只有
        // 在渲染传参的时候才现取.
        virtual HDC getBitmapDC() = 0;

        // 在给定的屏幕DC绘图, 如果位图DC不存在, 会临时创建. 如果你已经创建过位图DC,
        // 且函数调用前没有被释放, 会高效一些, 因为不会再创建一次位图DC. 如果src_rect
        // 为空, 整个源设备都会被拷贝.
        virtual void drawToHDC(HDC dc, int x, int y, const RECT* src_rect) = 0;

        // 设置给定区域的所有像素都是不透明的.
        virtual void makeOpaque(int x, int y, int width, int height) {}

        // 返回首选的渲染引擎是基于矢量的还是基于位图的.
        virtual bool IsVectorial() = 0;

        // 设备是否允许GDI渲染文本.
        virtual bool IsNativeFontRenderingAllowed() { return true; }

        // 初始化DC的缺省设置和颜色.
        static void InitializeDC(HDC context);

        // 加载一个SkPath到DC. 路径加载后可用于裁剪或用于描边.
        static void LoadPathToDC(HDC context, const SkPath& path);

        // 加载一个SkRegion到DC.
        static void LoadClippingRegionToDC(HDC context, const SkRegion& region,
            const SkMatrix& transformation);

    protected:
        struct CubicPoints
        {
            SkPoint p[4];
        };
        typedef std::vector<CubicPoints> CubicPath;
        typedef std::vector<CubicPath> CubicPaths;

        // 传递|bitmap|到SkDevice的构造函数.
        PlatformDevice(const SkBitmap& bitmap);

        // 加载Skia的变换到DC的世界变换, 不包括视角(GDI不支持).
        static void LoadTransformToDC(HDC dc, const SkMatrix& matrix);

        // SkPath的路径转换成一系列的三次贝塞尔曲线路径.
        static bool SkPathToCubicPaths(CubicPaths* paths, const SkPath& skpath);
    };

} //namespace skia

#endif //__skia_ext_platform_device_win_h__