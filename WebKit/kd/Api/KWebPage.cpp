
#include "windowsx.h"
#include "commctrl.h"

#include "Settings.h"

#include "KWebPage.h"
#include "KdGuiApiImp.h"
#include "Document.h"
#include "FrameTree.h"
#include "FrameView.h"

#include "PlatformMouseEvent.h"
#include "SystemTime.h"
#include "EventHandler.h"
#include "FocusController.h"
#include "PlatformKeyboardEvent.h"
#include "MIMETypeRegistry.h"
#include "KFrameNetworkingContext.h"
#include "KContextMenuClient.h"
#include "KDragClient.h"
#include "KEditorClient.h"

#include "WindowFeatures.h"
#include "MemoryCache.h"

#include "kd/SharedTimerKd.h"

#include "bridge/npruntime_impl.h"

#include "IntRect.h"
#include <WTF/scoped_ptr.h>
#include <WTF/RandomNumber.h>
#include <Ext/platform_canvas.h>
#include <Skia/PlatformContextSkia.h>
#include <cpp/KdValArray.h>
#include <text/TextEncoding.h>

extern WCHAR szTitle[];
extern WCHAR szWindowClass[];

namespace WebCore {

static void sendResponseIfNeeded(
    /* in */ KURL kurl,
    /* in */ ResourceHandleClient* client,
    /* in */ long long size,
    /* in */ ResourceHandle* resourceHandle,
    /* in */ bool needIgnoringCacheData
    );

static bool GetUrlDataFromFile(
    /* in */ String Url,
    /* out */ KdValArray& OutData,
    /* out */ long long& fileSize
    );

#define UseKdMsgSystem 1
#define QueryPerformance 1

class PaintRequestFilter { // 过滤极短时间内大量的绘制请求
public:
    PaintRequestFilter () {

    }

    bool canFilter(IntRect* rect)
    {
        DWORD nowTime = GetTickCount();
        if (m_recordRectQueue.size() != 0) {
            IntRect* lastRect = m_recordRectQueue.last();
            int unionAreaOfAll = WebCore::unionArea(lastRect, rect);
            int delta = std::min(unionAreaOfAll- rect->rectArea(), unionAreaOfAll- lastRect->rectArea());
            if (delta < 5) { // 如果两个面积相差不大，而且时间间隔也不远，就可以不用绘制            
                if (nowTime - m_recordTimeQueue.last() < m_recordMinTime &&
                    nowTime - m_recordTimeQueue[0] < m_recordMaxTime)
                { return true; }
            }
        }

        m_recordTimeQueue.append(nowTime);
        IntRect* rectDummy = new (fastMalloc(sizeof(RECT))) IntRect(*rect);
        m_recordRectQueue.append(rectDummy);
        if (m_recordTimeQueue.size() > m_recordQueueSize) {
            m_recordTimeQueue.remove(0, 1);
            fastFree(m_recordRectQueue[0]);
            m_recordRectQueue.remove(0, 1);
        }
        
        return false;
    }

private:
    static const int m_recordMinTime = 30;
    static const int m_recordMaxTime = 70;
    static const int m_recordQueueSize = 3;
    Vector<DWORD> m_recordTimeQueue;
    Vector<IntRect*> m_recordRectQueue;
};

struct AsynchronousResLoadInfo
{
    AsynchronousResLoadInfo(const UChar* pUrl, const void* pResBuf, int nResBufLen, bool bNeedSavaRes) 
        : m_url(pUrl)
    {
        m_bNeedSavaRes = bNeedSavaRes;
        if (bNeedSavaRes) {
            m_pAlloc = new KdValArray(1, nResBufLen);
            m_pAlloc->SetSize(nResBufLen);
            memcpy(m_pAlloc->GetData(), pResBuf, nResBufLen);
            m_pResBuf = 0;
            m_nResBufLen = 0;
        } else {
            m_pResBuf = pResBuf;
            m_nResBufLen = nResBufLen;
        }
    }

    ~AsynchronousResLoadInfo()
    {
        if (m_bNeedSavaRes) {
            delete m_pAlloc;
            m_pAlloc = 0;
        }
    }

    String m_url;
    const void* m_pResBuf;
    int m_nResBufLen;
    KdValArray* m_pAlloc;
    bool m_bNeedSavaRes;
};

class KWebPageImpl {
public:
    KWebPageImpl()
    {
        m_pagePtr = 0;
        m_bdColor = RGB(199, 237, 204)|0xff000000;
    }

    ~KWebPageImpl()
    {
        m_dirtyCanvas.reset(0);
        m_memoryCanvas.reset(0);
        
        m_pagePtr = 0;
    }

    void Init(KWebPage* pagePtr) {
        LONG windowStyle = GetWindowLong(pagePtr->getHWND(), GWL_EXSTYLE);
        m_useLayeredBuffer = !!((windowStyle) & WS_EX_LAYERED);

        m_canScheduleResourceLoader = false;

        m_scheduleMessageCount = 0;
        m_postpaintMessageCount = 0;

        m_hasResize = false;
        m_postMouseLeave = false;

        m_pagePtr = pagePtr;

        m_bNeedCallXmlHaveFinished = false;
    }

    void Layout() {
        // layout this frame
        FrameView* view = m_pagePtr->frame()->view();
        if (view)
            view->layout();
    }

    void clearPaintWhenLayeredWindow(skia::PlatformCanvas* canvas, const IntRect& rect)
    {
        if (!m_useLayeredBuffer)
            return;

        // When using transparency mode clear the rectangle before painting.
        SkPaint clearPaint;
        clearPaint.setARGB(0,0,0,0);
        clearPaint.setXfermodeMode(SkXfermode::kClear_Mode);

        SkRect skrc;
        skrc.set(rect.x(), rect.y(), rect.x() + rect.width(), rect.y() + rect.height());
        canvas->drawRect(skrc, clearPaint);
    }

    // Merge any areas that would reduce the total area
    void mergeDirtyList()
    { while ( doMergeDirtyList(true) ) {}; }
    
    bool doMergeDirtyList(bool forceMerge)
    {
        int nDirty = (int)m_paintMessageQueue.size();
        if (nDirty < 1) {
            return false;}
        
        int bestDelta = forceMerge ? 0x7FFFFFFF : 0;
        int mergeA = 0;
        int mergeB = 0;
        for ( int i = 0; i < nDirty-1; i++ ) {
            for ( int j = i+1; j < nDirty; j++ ) {
                int delta = unionArea(m_paintMessageQueue[i], m_paintMessageQueue[j]) -
                    m_paintMessageQueue[i]->rectArea() - m_paintMessageQueue[j]->rectArea();
                if ( bestDelta > delta ) {
                    mergeA = i;
                    mergeB = j;
                    bestDelta = delta;
                }
            }
        }

        if ( mergeA != mergeB ) {
            m_paintMessageQueue[mergeA]->unite(*(m_paintMessageQueue[mergeB]));
            for ( int i = mergeB+1; i < nDirty; i++ )
            { *(m_paintMessageQueue[i-1]) = *(m_paintMessageQueue[i]); }
           
            fastFree(m_paintMessageQueue[nDirty - 1]);
            m_paintMessageQueue.removeLast();
            return true;
        }

        return false;
    }

    void postPaintMessage(const IntRect* paintRect)
    {
        if (!paintRect || paintRect->isEmpty()) {
            return;}

        m_postpaintMessageCount++;

        if (m_paintMessageQueue.size() > m_paintMessageQueueSize && 0 != m_scheduleMessageCount) {
            return;} // 从SchedulePaintEvent发送过来的

        if (m_paintMessageQueue.size() > m_paintMessageQueueSize) {
            IntRect* destroyRect = m_paintMessageQueue[0];
            fastFree(destroyRect);
            m_paintMessageQueue.remove(0);
        }

        // TODO 脏矩形合并
        for (int i = 0; i < (int)m_paintMessageQueue.size(); ++i) {
            IntRect* paintRectFromQueue = m_paintMessageQueue[i];
            if (paintRectFromQueue ==  paintRect)
            { paintRectFromQueue->setWidth(0); }
        }

        IntRect* paintRectDummy = new (fastMalloc(sizeof(RECT))) IntRect(*paintRect);
        m_paintMessageQueue.append(paintRectDummy);

        m_postpaintMessageCount--;
    }

    void testPaint() {
        for (size_t index = 0; index < m_paintMessageQueue.size(); ++index) {
            IntRect* paintRect = m_paintMessageQueue[index];
            WCHAR msg[100] = {0};
            swprintf(msg, L"testPaint: %d %d %x\n", paintRect->y(), paintRect->height(), index);
            OutputDebugStringW(msg);
        }
    }

    void SchedulePaintEvent()
    {
        if (0 != m_scheduleMessageCount)
        { notImplemented(); }

        if (!m_pagePtr->getHWND()) 
        { return; }

        m_scheduleMessageCount++;

        HDC psHdc = ::GetDC(m_pagePtr->getHWND());

        IntRect lastTimeRect;

BeginPaint:
        lastTimeRect.setSize(IntSize());
        mergeDirtyList();
        int queueSize = (int)m_paintMessageQueue.size();
        for (int i = 0; i < queueSize; ++i) {
            IntRect* paintRect = m_paintMessageQueue[i];
            if (lastTimeRect == *paintRect || lastTimeRect.contains(*paintRect)) {
                fastFree(paintRect);
                continue;
            }
            lastTimeRect = *paintRect;
            fastFree(paintRect);
            if (!lastTimeRect.isEmpty()) { // 这里可能会重入postPaintMessage，所以需要重新合并脏矩形，并且小心一些bug
                //if (!m_paintRequestFilter.canFilter(&lastTimeRect))
                { doPaint(psHdc, &lastTimeRect); }
                // 这里还有优化空间，可以把很短间隔的差距不大的矩形忽略掉
                if (queueSize != (int)m_paintMessageQueue.size()) { // 如果在绘制的时候被请求了脏矩形，则重新合并
                    m_paintMessageQueue.remove(0, i + 1);
                    goto BeginPaint;
                }
            }
        }
        ::ReleaseDC(m_pagePtr->getHWND(), psHdc);

        m_paintMessageQueue.clear();
        m_scheduleMessageCount--;
    }

    void doPaintUseLayeredBuffer(HDC psHdc, const IntRect* paintRect)
    {
        RECT rcPaint = {0, 0, 500, 500};
        m_paintRect = *paintRect;

        if (!m_dirtyCanvas.get() || m_hasResize) {
            m_paintRect = m_clientRect;
            m_dirtyCanvas.reset(new skia::PlatformCanvas(m_clientRect.width(), m_clientRect.height(), !m_useLayeredBuffer));
            m_memoryCanvas.reset(new skia::PlatformCanvas(m_clientRect.width(), m_clientRect.height(), !m_useLayeredBuffer));
            m_hasResize = false;
        }

        if (m_paintRect.isEmpty())
        { return; }

#if QueryPerformance
        LARGE_INTEGER now0;
        LARGE_INTEGER now1 = {0};
        QueryPerformanceCounter(&now0);
#endif

        clearPaintWhenLayeredWindow(m_dirtyCanvas.get(), m_paintRect);
        clearPaintWhenLayeredWindow(m_memoryCanvas.get(), m_paintRect);

        Layout();
        PaintRect(m_paintRect, m_dirtyCanvas.get());

        HDC hMemoryDC = skia::BeginPlatformPaint(m_memoryCanvas.get());

        bool bNeedContinue = true;
        void* pCallBackContext = 0;
        void* pKdGuiForeignPtr = m_pagePtr->getKdGuiObjPtr() ? m_pagePtr->getKdGuiObjPtr()->pForeignPtr : 0;
        if (m_pagePtr->m_callbacks.m_paint) {
            m_pagePtr->m_callbacks.m_paint(m_pagePtr,
                pKdGuiForeignPtr, m_pagePtr->getForeignPtr(), m_pagePtr->getHWND(),
                KDPPaintStepPrePaintToMemoryDC, &pCallBackContext, &bNeedContinue, &(RECT)m_paintRect, hMemoryDC, psHdc);
        }
        if (!bNeedContinue) {
            goto Exit0;}
        
        // 先把脏矩形绘制到内存dc上
        skia::DrawToNativeContext(m_dirtyCanvas.get(), hMemoryDC, m_paintRect.x(), m_paintRect.y(), &((RECT)m_paintRect));
        
        if (m_pagePtr->m_callbacks.m_paint) {
            m_pagePtr->m_callbacks.m_paint(m_pagePtr,
                pKdGuiForeignPtr, m_pagePtr->getForeignPtr(), m_pagePtr->getHWND(),
                KDPPaintStepPostPaintToMemoryDC, &pCallBackContext, &bNeedContinue, &(RECT)m_paintRect, hMemoryDC, psHdc);
        }
        if (!bNeedContinue) {
            goto Exit0;}

        if (m_useLayeredBuffer) { // 再把内存dc画到psHdc上
            RECT rtWnd;
            ::GetWindowRect(m_pagePtr->getHWND(), &rtWnd);
            m_winodwRect = rtWnd;

            //skia::DrawToNativeLayeredContext(m_memoryCanvas.get(), psHdc, m_winodwRect.x(), m_winodwRect.y(), &((RECT)m_clientRect));
        } else {
            skia::DrawToNativeContext(m_memoryCanvas.get(), psHdc, 0, 0, &((RECT)m_clientRect));
        }

        if (m_pagePtr->m_callbacks.m_paint) {
            m_pagePtr->m_callbacks.m_paint(m_pagePtr,
                pKdGuiForeignPtr, m_pagePtr->getForeignPtr(), m_pagePtr->getHWND(),
                KDPPaintStepPostPaintToScreenDC, &pCallBackContext, &bNeedContinue, &(RECT)m_paintRect, hMemoryDC, psHdc);
        }

        if (m_bNeedCallXmlHaveFinished && m_pagePtr->m_callbacks.m_xmlHaveFinished) {
            m_bNeedCallXmlHaveFinished = false;
            m_pagePtr->m_callbacks.m_xmlHaveFinished(m_pagePtr, pKdGuiForeignPtr, m_pagePtr->getForeignPtr(), m_pagePtr->getHWND());}

//         QueryPerformanceCounter(&now1);
        WCHAR msg[200] = {0};
        //wsprintfW(msg, L"Rectangle : %d \n", now1.LowPart - now0.LowPart);
        wsprintfW(msg, L"Rectangle : %d %d %d %d\n", m_paintRect.x(), m_paintRect.y(), m_paintRect.width(), m_paintRect.height());
        OutputDebugStringW(msg);

#if 1
//         HGDIOBJ hOldBr = SelectObject(psHdc, GetStockObject(NULL_BRUSH));
//         Rectangle(psHdc, m_paintRect.x(), m_paintRect.y(), m_paintRect.maxX(), m_paintRect.maxY());
//         SelectObject(psHdc, hOldBr);

//         HBRUSH hbrush = CreateSolidBrush(RGB(19, 237, 04));
//         FillRect(psHdc, &(RECT)m_paintRect, hbrush);
//         DeleteObject(hbrush);
#endif
Exit0:
        skia::EndPlatformPaint(m_memoryCanvas.get()); 
    }

    void doPaint(HDC psHdc, const IntRect* paintRect)
    {
        RECT rcPaint = {0, 0, 500, 500};
        m_paintRect = *paintRect;

        if (!m_memoryCanvas.get() || m_hasResize) {
            m_paintRect = m_clientRect;
            m_memoryCanvas.reset(new skia::PlatformCanvas(m_clientRect.width(), m_clientRect.height(), !m_useLayeredBuffer));
            //m_dirtyCanvas.reset(new skia::PlatformCanvas(m_clientRect.width(), m_clientRect.height(), !m_useLayeredBuffer));
            //m_dirtyCanvas.reset(m_memoryCanvas.get());
            m_hasResize = false;
        }

        m_paintRect.intersect(m_clientRect);
        
        if (m_paintRect.isEmpty())
            return;

#if QueryPerformance
        LARGE_INTEGER now0;
        LARGE_INTEGER now1 = {0};
        QueryPerformanceCounter(&now0);
#endif

        //clearPaintWhenLayeredWindow(m_dirtyCanvas.get(), m_paintRect);
        clearPaintWhenLayeredWindow(m_memoryCanvas.get(), m_paintRect);

        HDC hMemoryDC = skia::BeginPlatformPaint(m_memoryCanvas.get());

        bool bNeedContinue = true;
        void* pCallBackContext = 0;
        void* pKdGuiForeignPtr = m_pagePtr->getKdGuiObjPtr() ? m_pagePtr->getKdGuiObjPtr()->pForeignPtr : 0;
        if (m_pagePtr->m_callbacks.m_paint) {
            m_pagePtr->m_callbacks.m_paint(m_pagePtr,
                pKdGuiForeignPtr, m_pagePtr->getForeignPtr(), m_pagePtr->getHWND(),
                KDPPaintStepPrePaintToMemoryDC, &pCallBackContext, &bNeedContinue, &(RECT)m_paintRect, hMemoryDC, psHdc);
        }
        if (!bNeedContinue)
            goto Exit0;

        Layout();
        PaintRect(m_paintRect, m_memoryCanvas.get()); // 绘制脏矩形

        // 先把脏矩形绘制到内存dc上
        //skia::DrawToNativeContext(m_dirtyCanvas.get(), hMemoryDC, m_paintRect.x(), m_paintRect.y(), &((RECT)m_paintRect));

        if (m_pagePtr->m_callbacks.m_paint) {
            m_pagePtr->m_callbacks.m_paint(m_pagePtr,
                pKdGuiForeignPtr, m_pagePtr->getForeignPtr(), m_pagePtr->getHWND(),
                KDPPaintStepPostPaintToMemoryDC, &pCallBackContext, &bNeedContinue, &(RECT)m_paintRect, hMemoryDC, psHdc);
        }
        if (!bNeedContinue)
            goto Exit0;

        if (m_useLayeredBuffer) { // 再把内存dc画到psHdc上
            RECT rtWnd;
            ::GetWindowRect(m_pagePtr->getHWND(), &rtWnd);
            m_winodwRect = rtWnd;

            //skia::DrawToNativeLayeredContext(m_memoryCanvas.get(), psHdc, m_winodwRect.x(), m_winodwRect.y(), &((RECT)m_clientRect));
        } else {
//             WCHAR msg[200] = {0};
//             wsprintfW(msg, L"Rectangle : %d %d %d %d\n", m_paintRect.x(), m_paintRect.y(), m_paintRect.width(), m_paintRect.height());
//             OutputDebugStringW(msg);

            skia::DrawToNativeContext(m_memoryCanvas.get(), psHdc, m_paintRect.x(), m_paintRect.y(), &((RECT)m_paintRect));
        }

        if (m_pagePtr->m_callbacks.m_paint) {
            m_pagePtr->m_callbacks.m_paint(m_pagePtr,
                pKdGuiForeignPtr, m_pagePtr->getForeignPtr(), m_pagePtr->getHWND(),
                KDPPaintStepPostPaintToScreenDC, &pCallBackContext, &bNeedContinue, &(RECT)m_paintRect, hMemoryDC, psHdc);
        }

        if (m_bNeedCallXmlHaveFinished && m_pagePtr->m_callbacks.m_xmlHaveFinished) {
            m_bNeedCallXmlHaveFinished = false;
            m_pagePtr->m_callbacks.m_xmlHaveFinished(m_pagePtr, pKdGuiForeignPtr, m_pagePtr->getForeignPtr(), m_pagePtr->getHWND());}

Exit0:
        //::Rectangle(psHdc, m_paintRect.x(), m_paintRect.y(), m_paintRect.maxX(), m_paintRect.maxY());
        skia::EndPlatformPaint(m_memoryCanvas.get());
    }

    void PaintRect(const IntRect& dirtyRect, skia::PlatformCanvas* pCanvas) {

        if (dirtyRect.isEmpty()) 
            return;

        setPainting(true);

        PlatformContextSkia context(pCanvas);
        // PlatformGraphicsContext is actually a pointer to PlatformContextSkia.
        GraphicsContext gc(reinterpret_cast<PlatformGraphicsContext*>(&context));
        gc.save();
        gc.setWindowsIsTransparencyLayer(m_useLayeredBuffer);
        
        frameView()->updateLayoutAndStyleIfNeededRecursive();
        //gc.clip(frameView()->visibleContentRect());
        gc.clip(dirtyRect);
        if (m_pagePtr->frame()->document() && frameView()) {
            if (!gc.windowsIsTransparencyLayer())
                gc.fillRect(dirtyRect, Color(m_bdColor), ColorSpaceDeviceRGB);
            frameView()->paintContents(&gc, dirtyRect);
        } else
            gc.fillRect(dirtyRect, Color(m_bdColor), ColorSpaceDeviceRGB);

        gc.restore();

        setPainting(false);
    }

    void OldPaintRect(const IntRect& rect, skia::PlatformCanvas* pCanvas) {

        if (rect.isEmpty()) 
        { return; }

        setPainting(true);

        PlatformContextSkia context(pCanvas);
        // PlatformGraphicsContext is actually a pointer to PlatformContextSkia.
        GraphicsContext gc(reinterpret_cast<PlatformGraphicsContext*>(&context));
        gc.setWindowsIsTransparencyLayer(m_useLayeredBuffer);

        frameView()->updateLayoutAndStyleIfNeededRecursive();
        //gc.clip(frameView()->visibleContentRect());
        gc.clip(rect);
        if (m_pagePtr->frame()->document() && frameView()) {
            if (!gc.windowsIsTransparencyLayer()) {
                gc.fillRect(rect, Color(m_bdColor), ColorSpaceDeviceRGB); }
            frameView()->paintContents(&gc, rect);
        } else {
            gc.fillRect(rect, Color(m_bdColor), ColorSpaceDeviceRGB);
        }

        setPainting(false);
    }

#if UseKdMsgSystem

    void paintEvent(HDC psHdc, const RECT* paint_rect)
    {
        if (!m_memoryCanvas.get() || m_clientRect.isEmpty())
        { return; }

        skia::DrawToNativeContext(m_memoryCanvas.get(), psHdc, 0, 0, &((RECT)m_clientRect));
    }

#else

    void paintEvent(HDC psHdc, const RECT* paint_rect)
    {
        PAINTSTRUCT ps;
        //BeginPaint(hWnd, &ps);
        ps.hdc = psHdc;
        ps.rcPaint = *paint_rect;

        RECT rect = ps.rcPaint;
        m_paintRect = IntRect(rect);

        RECT r;
        GetClientRect(m_hwnd, &r);
        IntRect client_rect(r);

        // Allocate a canvas if necessary
        if (!m_dirtyCanvas.get()) {
            //ResetScrollRect();
            m_paintRect = client_rect;
            m_dirtyCanvas.reset(new skia::PlatformCanvas(m_paintRect.width(), m_paintRect.height(), true));

            m_memoryCanvas.reset(new skia::PlatformCanvas(m_paintRect.width(), m_paintRect.height(), true));
        }

#if QueryPerformance
        LARGE_INTEGER now0;
        LARGE_INTEGER now1;
        QueryPerformanceCounter(&now0);
#endif  
        // This may result in more invalidation
        Layout();

        if (!m_paintRect.isEmpty())
        { PaintRect(m_paintRect); }
        
        // Paint to the screen
        skia::DrawToNativeContext(m_dirtyCanvas.get(), ps.hdc, ps.rcPaint.left, ps.rcPaint.top, &ps.rcPaint);

#if QueryPerformance
        QueryPerformanceCounter(&now1);
        WCHAR msg[200] = {0};
        wsprintfW(msg, L"Rectangle2 : %d %d %d %d\n", 
            ps.rcPaint.left, ps.rcPaint.top, (ps.rcPaint.right-ps.rcPaint.left), (ps.rcPaint.bottom-ps.rcPaint.top));
        OutputDebugStringW(msg);
#endif   

    }
#endif

    void setPainting(bool value)
    { m_painting = value; }

    FrameView* frameView() const {
        return m_pagePtr->frame() ? m_pagePtr->frame()->view() : NULL;
    }

    bool m_useLayeredBuffer;

    //HWND m_hwnd;

    IntRect m_winodwRect;
    IntRect m_clientRect; // 方便KWebPage修改

    bool m_hasResize;

    bool m_postMouseLeave; // 系统的MouseLeave获取到的鼠标位置不太准确，自己在定时器里再抛一次

    bool m_bNeedCallXmlHaveFinished; // 如果xml已经解析完毕，则需要在渲染完成一张整图后调用，而不是立马调用这个回调

    RGBA32 m_bdColor;

protected:
    IntRect m_paintRect;
    scoped_ptr<skia::PlatformCanvas> m_dirtyCanvas;
    scoped_ptr<skia::PlatformCanvas> m_memoryCanvas;

    bool m_painting;
    //WebCore::Frame* m_frame;
    bool m_canScheduleResourceLoader;

    Vector<IntRect*> m_paintMessageQueue;
    static const int m_paintMessageQueueSize = 200;

    int m_postpaintMessageCount;
    int m_scheduleMessageCount;

    PaintRequestFilter m_paintRequestFilter;

    KWebPage* m_pagePtr;
};

KWebPage::KWebPage(KdGuiObjPtr kdGuiObj, void* foreignPtr)
{
    m_kdGuiObj = kdGuiObj;
    m_foreignPtr = foreignPtr;
    if (0xcccccccc == (DWORD)kdGuiObj || 0xcccccccc == (DWORD)foreignPtr) {
        __asm int 3;
    }

    m_hWnd = NULL;
    m_state = UNINIT;

    m_frameLoaderClient = NULL;
    m_frame = NULL;

    m_chromeClient = NULL;
    m_page = NULL;

    m_pFrameNetworkingContext = NULL;

    m_pPageImpl = NULL;

    m_isAlert = false;
    m_isDraggableRegionNcHitTest = false;

    m_canScheduleResourceLoader = false;

    m_callbacks.m_xmlHaveFinished = 0;

    m_callbacks.m_javascriptCallCppPtr = 0;

    //m_rootWidget = 0;

//     m_debugInfo.addr = htonl (INADDR_ANY);
//     m_debugInfo.port = htons(0);
    m_remoteScriptDbg = 0;
}

KWebPage::~KWebPage()
{
    m_state = DESTROYING;
    FrameLoader *loader = frame()->loader();
    if (loader)
        loader->detachFromParent();

//     if (m_rootWidget) {
//         delete m_rootWidget;
//         m_rootWidget = 0;
//     }

    if (m_pPageImpl) {
        delete m_pPageImpl;
        m_pPageImpl = 0;
    }

    // 在Page::~Page()中销毁
//     if (m_contextMenuClient)
//         delete m_contextMenuClient;
//     m_contextMenuClient = 0;
    
    
//     if (m_editorClient)
//         delete m_editorClient;
//     m_editorClient = 0;
    
    if (m_page) {
        delete m_page;
        m_page = 0;
    }
}

void KWebPage::setCanScheduleResourceLoader() 
{
    m_canScheduleResourceLoader = true;
}

bool KWebPage::Init(HWND hWnd)
{
    if (UNINIT != m_state)
    { return true; }

    m_bMouseTrack = true;

    memset(&m_callbacks, 0, sizeof(KWebApiCallbackSet));

    m_hWnd = hWnd;

    m_chromeClient = new KChromeClient(this);
    m_contextMenuClient = new KContextMenuClient();
    m_editorClient = new KEditorClient(this);

    Page::PageClients pageClients;
    pageClients.chromeClient = m_chromeClient;
    pageClients.contextMenuClient = m_contextMenuClient;
    pageClients.editorClient = m_editorClient;
    pageClients.dragClient = new KDragClient(this);
    m_page = new Page(pageClients);

    InitSetting();

    m_page->settings()->setDefaultTextEncodingName("iso-8859-1");

    m_frameLoaderClient = new KFrameLoaderClient();

    RefPtr<Frame> newFrame = Frame::create(m_page, 0, m_frameLoaderClient);
    m_frame = newFrame.get();
    m_frameLoaderClient->setPage(this, m_frame);
    m_frame->init();

    // FIXME: All of the below should probably be moved over into WebCore
    const AtomicString sTemp = (const UChar*)L"";
    m_frame->tree()->setName(sTemp);

    m_pPageImpl = new KWebPageImpl();
    m_pPageImpl->Init(this);

    m_frame->view()->setTransparent(m_pPageImpl->m_useLayeredBuffer);

    // m_rootWidget = new KdWidgetMgr; // test_weolar
    // KdCommonControlsRegisteredWidgetClass(m_rootWidget);

    m_state = INIT;

    return true;
}

void KWebPage::setViewportSize(const IntSize& size)
{
    m_viewportSize = size;
    if (UNINIT == m_state) {
        Init(m_hWnd);
    } else if (DESTROYING == m_state) {
        return;
    }

    if (size.isEmpty() || !m_frame) {
        return;}

    WebCore::FrameView* view = m_frame->view();
    if (!view || view->size() == size) {
        return;}

    view->resize(size) ;
    view->adjustViewSize();

    if (!m_pPageImpl) {
        return;}

    m_pPageImpl->m_clientRect = IntRect(0, 0, size.width(), size.height());

    RECT rtWnd;
    ::GetWindowRect(m_hWnd, &rtWnd);
    m_pPageImpl->m_winodwRect = rtWnd;
    m_pPageImpl->m_hasResize = true;
}

static bool ResourceLoaderFromBuf(Vector<AsynchronousResLoadInfo*>& asynVec, const String url, KdValArray* resData)
{
    for (size_t i = 0; i < asynVec.size(); ++i) {
        AsynchronousResLoadInfo* pInfo = asynVec[i];
        if (url == pInfo->m_url) {
            if (pInfo->m_bNeedSavaRes) {
                resData->Resize(pInfo->m_pAlloc->GetSize());
                resData->SetSize(pInfo->m_pAlloc->GetSize());
                memcpy(resData->GetData(), pInfo->m_pAlloc->GetData(), resData->GetSize());
            } else {
                resData->Resize(pInfo->m_nResBufLen);
                resData->SetSize(pInfo->m_nResBufLen);
                memcpy(resData->GetData(), pInfo->m_pResBuf, resData->GetSize());
            }
            asynVec.remove(i, 1);
            delete pInfo;
            return true;
        }
    }

    return false;
}

void KWebPage::scheduleResourceLoader(KFrameNetworkingContext* pContext) {
    KNetworkReplyHandler* pReplyHandler;
    ListHashSet<KNetworkReplyHandler*>::iterator iter;

    if (!m_canScheduleResourceLoader)
    { return; }

    while (pContext && !pContext->arrReplyHandler.isEmpty()) {
        iter = pContext->arrReplyHandler.begin();
        pReplyHandler = *iter;

        if (pReplyHandler->isDeferred())
        { break; }

        String url = pReplyHandler->URL().string();
        url = decodeURLEscapeSequences(url);

        size_t needIgnoringCacheData = url.find("?content=no-store");
        if (-1 != needIgnoringCacheData) {
            url = url.left(needIgnoringCacheData);}
        
        KdValArray resData(sizeof(char), 0x1024);
        long long size = 0;
        bool needLoadResMyself = true;

        if (ResourceLoaderFromBuf(m_asynResQueue, url, &resData)) {
            size = resData.GetSize();
            needLoadResMyself = false;
        } else if (m_callbacks.m_resHandle) {
            HRESULT hr = m_callbacks.m_resHandle(this, getKdGuiObjPtr() ? getKdGuiObjPtr()->pForeignPtr : 0, getForeignPtr(), getHWND(),
                (const WCHAR *)url.charactersWithNullTermination(), &resData);
            if (0 == hr) {
                size = resData.GetSize();
                needLoadResMyself = false;
            }
        }
        if (needLoadResMyself) {
            GetUrlDataFromFile(url, resData, size);}

        sendResponseIfNeeded(pReplyHandler->URL(), pReplyHandler->client(), size, pReplyHandler->ResHandle(), -1 != needIgnoringCacheData);
        pReplyHandler->client()->didReceiveData(pReplyHandler->ResHandle(), (const char*)(resData.GetData()), size, size);
        pReplyHandler->client()->didFinishLoading(NULL, 0);

        delete pReplyHandler;
        pContext->arrReplyHandler.remove(pReplyHandler);
    }
}

// 本次点击是一次模拟标题栏
void KWebPage::setIsDraggableRegionNcHitTest()
{
    m_isDraggableRegionNcHitTest = true;
}

// 这个函数一般被脚本用
KWebPage* KWebPage::createWindow(
    KdGuiObjPtr kdGuiObj,
    KWebPage* parentPage,
    const WindowFeatures* features,
    void* foreignPtr
    )
{
    return _createWindow(kdGuiObj, parentPage, features, 0, foreignPtr);
}

// 这个函数一般被外部导出函数用
KWebPage* KWebPage::createWindowByRealWnd(
    KdGuiObjPtr kdGuiObj,
    KdPageInfoPtr pageInfo,
    void* foreignPtr
    )
{
    return _createWindow(kdGuiObj, 0, 0, pageInfo, foreignPtr);
}

KWebPage* KWebPage::_createWindow(
    KdGuiObjPtr kdGuiObj,
    KWebPage* parentPage,
    const WindowFeatures* features,
    KdPageInfoPtr pageInfo, // 优先级是先features，再pageInfo
    void* foreignPtr
    )
{
    KWebPage* page = 0;
    page = new KWebPage(kdGuiObj, foreignPtr);
    if (!page)
    { goto Exit0; }

    HWND hWnd = 0;
    if (features) { // 脚本窗口的窗口，特性不是很多，不如c++创建的那么多style
        hWnd = CreateWindowExW(0, szWindowClass, L"",  WS_POPUP|WS_VISIBLE,
            features->x, features->y, features->width, features->height, 
            NULL, NULL, NULL, (LPVOID)page);
    } else if (pageInfo) {
        hWnd = CreateWindowExW(pageInfo->dwExStyle, szWindowClass, L"", pageInfo->dwStyle,
            pageInfo->X, pageInfo->Y, pageInfo->nWidth, pageInfo->nHeight, 
            pageInfo->hWndParent, NULL, NULL, (LPVOID)page);
    } else
        { ASSERT_NOT_REACHED(); }
    if (!hWnd)
    { ASSERT_NOT_REACHED(); }

    page->m_debugInfo = pageInfo->DebugInfo;
    page->m_hWnd = hWnd;
    page->Init(hWnd);
    
    if (kdGuiObj)
    { kdGuiObj->pages.append(page); }
    
    //if (false == kdGuiObj->bHaveTimer) 
    {
        ::SetTimer(hWnd, (UINT_PTR)page, 10, NULL);
        //kdGuiObj->bHaveTimer = true;
    }

Exit0:
    if (!page) {
        ASSERT_NOT_REACHED();
    }
    return page;
}

void KWebPage::javaScriptAlert(String& message)
{
    if (true == m_isAlert)
        {return;}
    
    m_isAlert = true;
    MessageBoxW(NULL, (LPCWSTR)message.charactersWithNullTermination(), L"KdGui", 0);
    m_isAlert = false;
}

#if 0

void KWebPage::appendAnimNode(KQuery* node)
{
    size_t size = m_animatingNodeQueue.size();
    for (int i = 0; i < size; ++i) {
        if (node == m_animatingNodeQueue[i])
        { return; }
    }

    node->ref();
    m_animatingNodeQueue.append(node);
}

void KWebPage::removeAnimNode(KQuery* node)
{
    size_t size = m_animatingNodeQueue.size();
    for (int i = 0; i < size; ++i) {
        if (node == m_animatingNodeQueue[i]) {

            m_animatingNodeQueue.remove(i, 1);
            node->deref();
            return;
        }
    }
}

void KWebPage::forceStopAllAnim()
{
    // 因为在强制停止的时候会进入removeAnimNode操作队列，所以需要复制一份
    while (0 != m_animatingNodeQueue.size()) {
        Vector<KQuery*> animatingNodeQueueDummy = m_animatingNodeQueue;
        size_t size = animatingNodeQueueDummy.size();
        for (int i = 0; i < size; ++i) { // 在停止的时候，有可能用户会继续向里面加动画，所以外层需要一个循环
            animatingNodeQueueDummy[i]->forceStopAllAnimAndDestroy();
        }
    }

    ASSERT(0 == m_animatingNodeQueue.size());
}

static void clearAllBindAndAdditionalData(Node* root)
{
    if (root->kquery()) {
        root->kquery()->setUserdata(NULL);
        root->kquery()->setWidget(NULL);
    }

    root->EventTarget::removeAllEventListeners();
    Node* n = root->firstChild();
    while (n) {
        clearAllBindAndAdditionalData(n);
        n = n->nextSibling();
    }
}

#endif

void KWebPage::windowCloseRequested()
{
    if (INIT != m_state)
    { return; }
    m_state = DESTROYING;

    void* pKdGuiForeignPtr = getKdGuiObjPtr() ? getKdGuiObjPtr()->pForeignPtr : 0;
    if (m_callbacks.m_unintCallBack) {
        m_callbacks.m_unintCallBack(this, 
            pKdGuiForeignPtr, 
            getForeignPtr(),
            m_hWnd);
    }

//     // 在这里给脚本发消息，而不是在Frame::pageDestroyed()
//     frame()->script()->willCloseScript();
// 
//     if (m_remoteScriptDbg) {
//         sq_rdbg_shutdown(m_remoteScriptDbg);}
    
    ::SetWindowLongPtr(m_hWnd, GWLP_USERDATA, 0);

    // 在KFrameLoaderClient::frameLoaderDestroyed()也会调用到此，所以在给脚本发消息的时候注意一下
    if (m_pFrameNetworkingContext)
    { m_pFrameNetworkingContext->arrReplyHandler.clear(); }

//     forceStopAllAnim();
//     clearAllBindAndAdditionalData(m_frame->document());

    // WebCore::KWebPage::windowCloseRequested+0x1a           
    // WebCore::KFrameLoaderClient::frameLoaderDestroyed+0xc  
    // WebCore::FrameLoader::~FrameLoader+0x7e                
    // WebCore::Frame::~Frame+0xd0                            
    // WTF::RefCounted<WebCore::Frame>::deref+0x16            
    // WebCore::Frame::lifeSupportTimerFired+0x5              
    // WebCore::Timer<WebCore::MainResourceLoader>::fired+0xc 
    // WebCore::ThreadTimers::sharedTimerFiredInternal+0x8b   
    // WebCore::ThreadTimers::sharedTimerFired+0xe            
    // WebCore::KWebPage::windowCloseRequested+0x99
    // 所以这里可能有重入
    SharedTimerKd::inst()->timerEvent(); // 很多异步清理资源的工作会放在定时器里，所以最后再执行一遍

//     ASSERT(1 == m_rootWidget->refCount());
//     m_rootWidget->deref();
//     m_rootWidget = 0;

    delete m_pPageImpl;
    m_pPageImpl = 0;

    // TODO_Weolar
    // 发生unload消息给脚本

    // 清空webkit的资源缓存
    memoryCache()->evictResources();

    ::KillTimer(m_hWnd, (UINT_PTR)this);
}

#ifndef NDEBUG
BOOL gd_bShowTree = FALSE;
#endif

void KWebPage::timerFired()
{
    SharedTimerKd::inst()->timerEvent();
    WTF::dispatchFunctionsFromMainThread();

    scheduleResourceLoader(m_pFrameNetworkingContext);

    if (m_pPageImpl)
        m_pPageImpl->SchedulePaintEvent();
    
#ifndef NDEBUG
    if (gd_bShowTree)
        showDebugNodeData();
#endif
}

void KWebPage::showDebugNodeData()
{
#ifndef NDEBUG
    frame()->document()->showTreeForThis();
#endif
}

void KWebPage::resizeEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (INIT != m_state || m_isAlert)
    { return; }

    UINT cx, cy;
    cx = LOWORD(lParam);
    cy = HIWORD(lParam);

    if (!m_hWnd)
        {m_hWnd = hWnd;}

    IntSize size(cx, cy);
    setViewportSize(size);
}

void KWebPage::repaintRequested(const IntRect& windowRect)
{
    if (INIT != m_state || windowRect.isEmpty() || windowRect.maxY() < 0 || windowRect.maxX() < 0)
    { return; }

    RECT winRect = {windowRect.x(), windowRect.y(), 
        windowRect.x() + windowRect.width(), windowRect.y() + windowRect.height()};

//     WCHAR msg[200] = {0};
//     wsprintfW(msg, L"repaintRequested : %d %d %d %d\n", windowRect.x(), windowRect.y(), windowRect.width(), windowRect.maxY());
//     OutputDebugStringW(msg);

 
#if UseKdMsgSystem
    if (!windowRect.isEmpty()) {
        m_pPageImpl->postPaintMessage(&windowRect); }
#else
    ::InvalidateRect(m_hWnd, &winRect, false);
#endif
}

void KWebPage::paintEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (true == m_isAlert)
    { return; }

    PAINTSTRUCT ps;
    BeginPaint(hWnd, &ps);
    m_pPageImpl->paintEvent(ps.hdc, &ps.rcPaint);
    EndPaint(hWnd, &ps);
}

static void MakeDraggableRegionNcHitTest(HWND hWnd, LPARAM lParam, bool* isDraggableRegionNcHitTest, IntPoint& lastPosForDrag)
{
    int xPos = GET_X_LPARAM(lParam); 
    int yPos = GET_Y_LPARAM(lParam); 
    if (true == *isDraggableRegionNcHitTest) {
        //::PostMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(xPos, yPos));
        ::PostMessage(hWnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
        lastPosForDrag = IntPoint(xPos, yPos);
    } else {
        ::SetCapture(hWnd);
    }
    //*isDraggableRegionNcHitTest = false;
}

void KWebPage::captureChangedEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
//     const INT vkey = GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON;
//     LONG bLButtonIsDown = GetAsyncKeyState(vkey) & 0x8000;
//     
//     WCHAR msg[100] = {0};
//     swprintf(msg, L"KWebPage::captureChangedEvent 1: %x %x\n", m_isDraggableRegionNcHitTest, bLButtonIsDown);
//     OutputDebugStringW(msg);

    if (m_isDraggableRegionNcHitTest /*&& 0 == bLButtonIsDown*/) {
        ::ReleaseCapture();
        m_isDraggableRegionNcHitTest = false;

//         POINT ptCursor;
//         ::GetCursorPos(&ptCursor);
//         ::ScreenToClient(hWnd, &ptCursor);
//         lParam = MAKELONG(ptCursor.x, ptCursor.y);
        lParam = MAKELONG(m_lastPosForDrag.x(), m_lastPosForDrag.y());
        mouseEvent(hWnd, WM_LBUTTONUP, wParam, lParam);
    }
}

void KWebPage::killFocusEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // only set the focused frame inactive so that we stop painting the caret
    // and the focus frame. But don't tell the focus controller so that upon
    // focusInEvent() we can re-activate the frame.
    FocusController *focusController = page()->focusController();
    // Call setFocused first so that window.onblur doesn't get called twice
    focusController->setFocused(false);
    focusController->setActive(false);
    focusController->setFocusedNode(0, frame());
}

void KWebPage::mouseEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    m_isDraggableRegionNcHitTest = false;
    if (true == m_isAlert)
        return;

    if (m_bMouseTrack && m_pPageImpl && !m_pPageImpl->m_postMouseLeave) { // 若允许追踪，则
        TRACKMOUSEEVENT csTME;
        csTME.cbSize = sizeof(csTME);
        csTME.dwFlags = TME_LEAVE|TME_HOVER;
        csTME.hwndTrack = m_hWnd;  // 指定要追踪的窗口
        csTME.dwHoverTime = 10;    // 鼠标在按钮上停留超过10ms，才认为状态为HOVER
        ::TrackMouseEvent(&csTME); // 开启Windows的WM_MOUSELEAVE，WM_MOUSEHOVER事件支持
        m_bMouseTrack = false;     // 若已经追踪，则停止追踪
    }

    bool shift = false, ctrl = false, alt = false, meta = false;
    int clickCount = 0;
    MouseButton button = NoButton; // LeftButton;
    IntPoint pos;
    IntPoint globalPos;

    if (!m_frame || !m_frame->view())
        {return;}

    if (WM_MOUSELEAVE == message) {
//         if (m_pPageImpl)
//         { m_pPageImpl->m_postMouseLeave = true; }

        POINT ptCursor;
        ::GetCursorPos(&ptCursor);
        globalPos = ptCursor;
        ::ScreenToClient(hWnd, &ptCursor);
        if (ptCursor.x < 2)
        { ptCursor.x = -1;}
        else if (ptCursor.x > 10)
        {ptCursor.x += 2;}

        if (ptCursor.y < 2)
        { ptCursor.y = -1;}
        else if (ptCursor.y > 10)
        { ptCursor.y += 2; }

        pos = ptCursor;

        lParam = MAKELPARAM(ptCursor.x, ptCursor.y);
    } else {
        pos.setX(GET_X_LPARAM(lParam));
        pos.setY(GET_Y_LPARAM(lParam));

        POINT widgetpt = {pos.x(), pos.y()};
        ::ClientToScreen(hWnd, &widgetpt);
        globalPos.setX(widgetpt.x);
        globalPos.setY(widgetpt.y);
    }

    if (WM_MOUSELEAVE == message)
        {m_bMouseTrack = true;}

    PlatformMouseEvent ev(hWnd, message, wParam, lParam, FALSE);

    bool accepted = false;
    switch (message)
    {
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
        m_isDraggableRegionNcHitTest = false;
        accepted = m_frame->eventHandler()->handleMousePressEvent(ev);
        MakeDraggableRegionNcHitTest(hWnd, lParam, &m_isDraggableRegionNcHitTest, m_lastPosForDrag);

        break;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        ::ReleaseCapture();
        accepted = m_frame->eventHandler()->handleMouseReleaseEvent(ev);
        break;
    case WM_MOUSEMOVE:
    case WM_MOUSELEAVE:
        accepted = m_frame->eventHandler()->mouseMoved(ev);
        break;
    default:
        return;
    }

    return;
}

#if 1

// 返回1表示调用def函数。这是因为1是S_FALSE
int KWebPage::inputEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    bool handled = false;
    WebCore::Frame* frame = m_page->focusController()->focusedOrMainFrame();
    
    // we forward the key event to WebCore first to handle potential DOM
    // defined event handlers and later on end up in EditorClientQt::handleKeyboardEvent
    // to trigger editor commands via triggerAction().

    PlatformKeyboardEvent::Type type = (PlatformKeyboardEvent::Type)-1;
    if (WM_KEYUP == message) {
        type = PlatformKeyboardEvent::KeyUp;
    } else if (WM_KEYDOWN == message) {
        type = PlatformKeyboardEvent::RawKeyDown;
    } else if (WM_CHAR == message) {
        type = PlatformKeyboardEvent::Char;
    }

    if ((PlatformKeyboardEvent::Type)-1 != type) {
        PlatformKeyboardEvent evt(hWnd, wParam, lParam, type, true);
        handled = frame->eventHandler()->keyEvent(evt);
        if (handled) {
            return 1;}
    }

    return inputEventToRichEdit(hWnd, message, wParam, lParam);
}

// 一般都是richedit的事件
bool KWebPage::inputEventToRichEdit(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    bool handled = false;

//     Node* node = m_frame->document()->focusedNode();
//     if (node && node->localName() == *SVGNames::svgricheditbaseTag) {
//         SVGRichEditBaseElement* edit = (SVGRichEditBaseElement*)node;
//         PassRefPtr<WinNativeEvent> evt = WinNativeEvent::create(hWnd, message, wParam, lParam);
//         edit->defaultEventHandler((Event*)evt.get());
//         return evt->m_hRes;
//     } else {
//         node = node;
//     }
    
    return 1;
}

#endif

static int gTest = 0;
static bool GetUrlDataFromFile(
    /* in */ String Url,
    /* out */ KdValArray& OutData,
    /* out */ long long& fileSize
    )
{
    HANDLE        hFile        = NULL;
    DWORD         bytesReaded  = 0;
    UINT          DataSize     = 8;
    bool          bRet         = false;
    LARGE_INTEGER FileSize     = {0};

    if (-1 != Url.find(String((const UChar*)L"file:"), 0, false))
    { Url.remove(0, 8); }
    
    hFile = CreateFileW((LPCWSTR)Url.charactersWithNullTermination(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    if(!hFile || INVALID_HANDLE_VALUE == hFile) {
        if (gTest++ > 2)
        { return false; }

        __asm int 3;
        OutputDebugStringW((LPCWSTR)Url.charactersWithNullTermination());
        MessageBoxW(0, (LPCWSTR)Url.charactersWithNullTermination(), L"打开文件失败", 0);
        
        return false; 
    }

    bRet = GetFileSizeEx(hFile, &FileSize);
    if (!bRet || (0 == FileSize.HighPart && 0 == FileSize.LowPart))
    { goto Exit0; }

    OutData.Resize(FileSize.LowPart + 10);

    if(!::ReadFile(hFile, (LPVOID)(OutData.GetData()), FileSize.LowPart, &bytesReaded, NULL))
    { goto Exit0; }

    fileSize = FileSize.QuadPart;
    bRet = true;

Exit0:
    if (hFile && INVALID_HANDLE_VALUE != hFile) { 
        ::CloseHandle(hFile);
        hFile = NULL;
    }

    return bRet;
}

static void sendResponseIfNeeded (
    KURL kurl,
    ResourceHandleClient* client,
    long long size,
    ResourceHandle* resourceHandle,
    bool needIgnoringCacheData
    )
{
    if (!client)
        return;

    WTF::String contentType;
    WTF::String encoding;
    WTF::String mimeType;

    String extension = kurl.path();
    int index = extension.reverseFind('.');
    if (index > 0) {
        //extension = extension.mid(index + 1);
        extension = extension.substring(index + 1);
        mimeType = MIMETypeRegistry::getMIMETypeForExtension(extension);
    }

    String suggestedFilename;
    suggestedFilename = kurl.lastPathComponent();

    ResourceResponse response(kurl, mimeType, size, encoding, suggestedFilename);

//     response.setHTTPStatusCode(0);
//     response.setHTTPStatusText(String());
//
//     response.setHTTPHeaderField(AtomicString("Last-Modified"), String("Fri, 09 Dec 2011 23:26:42 GMT"));
//     response.setHTTPHeaderField(AtomicString("Content-Length"), String("416"));
    if (needIgnoringCacheData) {
        //response.setCachePolicy(ReloadIgnoringCacheData);
        response.setHTTPHeaderField(AtomicString("no-store"), String("yes"));}
    
    client->didReceiveResponse(resourceHandle, response);    
}

void KWebPage_Load(
    LPCWSTR lpUrl,
    WebCore::Frame *pFrame,
    KFrameNetworkingContext** ppFrameNetworkingContext,
    KNetworkAccessManager::Operation operation
    )
{
    KURL url(ParsedURLString, String((const UChar*)lpUrl));

    WebCore::ResourceRequest request(url);
    pFrame->loader()->load(request, false); // 在ResourceHandle::start(Frame* frame)中接受数据   
}

void KWebPage::loadFormUrl(LPCWSTR lpUrl)
{
    KWebPage_Load(lpUrl, m_frame, &m_pFrameNetworkingContext, KNetworkAccessManager::GetOperation);
    setCanScheduleResourceLoader();
}

void KWebPage::loadFormData(const void* lpData, int nLen)
{
    String sUrl;
    sUrl = sUrl.format("res:///kdData_%f.svg", randomNumber());

    postResToAsynchronousLoad((const WCHAR *)sUrl.charactersWithNullTermination(), lpData, nLen, true);
    loadFormUrl((const WCHAR *)sUrl.charactersWithNullTermination());
}

void KWebPage::postResToAsynchronousLoad(const WCHAR* pUrl, const void* pResBuf, int nResBufLen, bool bNeedSavaRes)
{
    AsynchronousResLoadInfo* pInfo = new AsynchronousResLoadInfo((const UChar *)pUrl, pResBuf, nResBufLen, bNeedSavaRes);
    m_asynResQueue.append(pInfo);
}

int KWebPage::notifFromResHandle(LPCWSTR lpMsg, void* pContent)
{
    void* pForeignPtr = getKdGuiObjPtr() ? getKdGuiObjPtr()->pForeignPtr : 0;
    if (0 == wcscmp(L"ResourceHandle::start", lpMsg)) {
        if (pContent) {
            if (!m_pFrameNetworkingContext) {
                m_pFrameNetworkingContext = (KFrameNetworkingContext*)pContent;
            } else {
                ASSERT(m_pFrameNetworkingContext == pContent);
            }
        }
    } else if (0 == wcscmp(L"KdCallbackWhenSvgInit", lpMsg) && m_pPageImpl) {
        m_pPageImpl->m_bNeedCallXmlHaveFinished = true;
//         if (m_callbacks.m_xmlHaveFinished)
//         { m_callbacks.m_xmlHaveFinished(this, pForeignPtr, getForeignPtr(), getHWND()); }
    } else if (0 == wcscmp(L"initThread", lpMsg)) {
        if (m_callbacks.m_scriptInitCallBack)
        { m_callbacks.m_scriptInitCallBack(this, pForeignPtr, getForeignPtr(), getHWND(), (HSQUIRRELVM)pContent); }
    }

    return 0;
}

void KWebPage::setBackgroundColor(COLORREF c) {
    m_pPageImpl->m_bdColor = c;
}

bool KWebPage::invokeScript(NPIdentifier methodName, const NPVariant* args, uint32_t argCount, NPVariant* result)
{
    NPObject* o = frame()->script()->windowScriptNPObject();
    return _NPN_Invoke(0, o, methodName, args, argCount, result);
}

bool KWebPage::InitSetting()
{
    Settings* settings = m_page->settings();
    if (settings) {
        settings->setTextAreasAreResizable(true);

        //         QWebSettingsPrivate *global = QWebSettings::globalSettings()->d;
        // 
        //         QString family = fontFamilies.value(QWebSettings::StandardFont,
        //             global->fontFamilies.value(QWebSettings::StandardFont));
        //         settings->setStandardFontFamily(family);
        // 
        //         family = fontFamilies.value(QWebSettings::FixedFont,
        //             global->fontFamilies.value(QWebSettings::FixedFont));
        //         settings->setFixedFontFamily(family);
        // 
        //         family = fontFamilies.value(QWebSettings::SerifFont,
        //             global->fontFamilies.value(QWebSettings::SerifFont));
        //         settings->setSerifFontFamily(family);
        // 
        //         family = fontFamilies.value(QWebSettings::SansSerifFont,
        //             global->fontFamilies.value(QWebSettings::SansSerifFont));
        //         settings->setSansSerifFontFamily(family);
        // 
        //         family = fontFamilies.value(QWebSettings::CursiveFont,
        //             global->fontFamilies.value(QWebSettings::CursiveFont));
        //         settings->setCursiveFontFamily(family);
        // 
        //         family = fontFamilies.value(QWebSettings::FantasyFont,
        //             global->fontFamilies.value(QWebSettings::FantasyFont));
        //         settings->setFantasyFontFamily(family);
        // 
        //         int size = fontSizes.value(QWebSettings::MinimumFontSize,
        //             global->fontSizes.value(QWebSettings::MinimumFontSize));
        settings->setMinimumFontSize(10);
        // 
        //         size = fontSizes.value(QWebSettings::MinimumLogicalFontSize,
        //             global->fontSizes.value(QWebSettings::MinimumLogicalFontSize));
        settings->setMinimumLogicalFontSize(10);
        // 
        //         size = fontSizes.value(QWebSettings::DefaultFontSize,
        //             global->fontSizes.value(QWebSettings::DefaultFontSize));
        settings->setDefaultFontSize(16);
        // 
        //         size = fontSizes.value(QWebSettings::DefaultFixedFontSize,
        //             global->fontSizes.value(QWebSettings::DefaultFixedFontSize));
        settings->setDefaultFixedFontSize(16);
        // 
        //         bool value = attributes.value(QWebSettings::AutoLoadImages,
        //             global->attributes.value(QWebSettings::AutoLoadImages));
        settings->setLoadsImagesAutomatically(true);
        // 
        //         value = attributes.value(QWebSettings::JavascriptEnabled,
        //             global->attributes.value(QWebSettings::JavascriptEnabled));
        settings->setJavaScriptEnabled(true);
        // 
        //         value = attributes.value(QWebSettings::JavascriptCanOpenWindows,
        //             global->attributes.value(QWebSettings::JavascriptCanOpenWindows));
        //         settings->setJavaScriptCanOpenWindowsAutomatically(value);
        // 
        //         value = attributes.value(QWebSettings::JavaEnabled,
        //             global->attributes.value(QWebSettings::JavaEnabled));
        //         settings->setJavaEnabled(value);
        // 
        //         value = attributes.value(QWebSettings::PluginsEnabled,
        //             global->attributes.value(QWebSettings::PluginsEnabled));
        //         settings->setPluginsEnabled(value);
        // 
        //         value = attributes.value(QWebSettings::PrivateBrowsingEnabled,
        //             global->attributes.value(QWebSettings::PrivateBrowsingEnabled));
        //         settings->setPrivateBrowsingEnabled(value);
        // 
        //         value = attributes.value(QWebSettings::JavascriptCanAccessClipboard,
        //             global->attributes.value(QWebSettings::JavascriptCanAccessClipboard));
        //         settings->setDOMPasteAllowed(value);
        // 
        //         value = attributes.value(QWebSettings::DeveloperExtrasEnabled,
        //             global->attributes.value(QWebSettings::DeveloperExtrasEnabled));
        //         settings->setDeveloperExtrasEnabled(value);
        // 
        //         QUrl location = !userStyleSheetLocation.isEmpty() ? userStyleSheetLocation : global->userStyleSheetLocation;
        //         settings->setUserStyleSheetLocation(WebCore::KURL(location));
        // 
        //         QString localStoragePath = !localStorageDatabasePath.isEmpty() ? localStorageDatabasePath : global->localStorageDatabasePath;
        //         settings->setLocalStorageDatabasePath(localStoragePath);
        // 
        //         value = attributes.value(QWebSettings::ZoomTextOnly,
        //             global->attributes.value(QWebSettings::ZoomTextOnly));
        //         settings->setZoomsTextOnly(value);
        // 
        //         value = attributes.value(QWebSettings::PrintElementBackgrounds,
        //             global->attributes.value(QWebSettings::PrintElementBackgrounds));
        //         settings->setShouldPrintBackgrounds(value);
        // 
        //         value = attributes.value(QWebSettings::OfflineStorageDatabaseEnabled,
        //             global->attributes.value(QWebSettings::OfflineStorageDatabaseEnabled));
        //         settings->setDatabasesEnabled(value);
        // 
        //         value = attributes.value(QWebSettings::OfflineWebApplicationCacheEnabled,
        //             global->attributes.value(QWebSettings::OfflineWebApplicationCacheEnabled));
        //         settings->setOfflineWebApplicationCacheEnabled(value);
        // 
        //         value = attributes.value(QWebSettings::LocalStorageDatabaseEnabled,
        //             global->attributes.value(QWebSettings::LocalStorageDatabaseEnabled));
        //         settings->setLocalStorageEnabled(value);

        settings->setFontRenderingMode(AlternateRenderingMode);
    }

    return true;
}

} // namespace WebCore