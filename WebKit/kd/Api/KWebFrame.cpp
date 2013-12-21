
#include "windowsx.h"
#include "KWebFrame.h"
#include "FrameTree.h"
//#include "InitializeThreading.h"
#include "FrameView.h"
#include "Settings.h"

#include "PlatformMouseEvent.h"
#include "SystemTime.h"
#include "EventHandler.h"
#include "SecurityOrigin.h"
#include "MIMETypeRegistry.h"
#include "KFrameNetworkingContext.h"
//#include "WebPlatformStrategies.h"

#include <wtf/scoped_ptr.h>
#include <wtf/MainThread.h>
#include <ext/platform_canvas.h>
#include <skia/PlatformContextSkia.h>

namespace WebCore {

static void sendResponseIfNeeded(
    /* in */ KURL kurl,
    /* in */ ResourceHandleClient* client,
    /* in */ long long size,
    /* in */ ResourceHandle* resourceHandle
    );

static bool GetUrlData(
    /* in */ String Url,
    /* out */ Vector<char>& OutData,
    /* out */ long long& fileSize
    );

static bool GetUrlDataFromFile(
    /* in */ String Url,
    /* out */ Vector<char>& OutData,
    /* out */ long long& fileSize
    );

class KWebFrameImp
{
public:
    void Init(Frame* frame) {
        frame_ = frame;
        canScheduleResourceLoader = false;
    }

    void Layout() {
        // layout this frame
        FrameView* view = frame_->view();
        if (view)
            view->layout();
    }

    void paintEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);

        RECT rect = ps.rcPaint;
        //if (GetUpdateRect(hWnd, &rect, FALSE)) {
        //    paint_rect_.unite(IntRect(rect));
        //}
        paint_rect_ = IntRect(rect);

        RECT r;
        GetClientRect(hWnd, &r);
        IntRect client_rect(r);

        // Allocate a canvas if necessary
        if (!canvas_.get()) {
            //ResetScrollRect();
            paint_rect_ = client_rect;
            canvas_.reset(new skia::PlatformCanvas(
                paint_rect_.width(), paint_rect_.height(), true));
        }

        // This may result in more invalidation
        Layout();

        // Paint the canvas if necessary.  Allow painting to generate extra rects the
        // first time we call it.  This is necessary because some WebCore rendering
        // objects update their layout only when painted.
        //for (int i = 0; i < 2; ++i) 
        {
            //client_rect.intersect(paint_rect_);
            //paint_rect_ = client_rect;
            if (!paint_rect_.isEmpty()) {
                PaintRect(paint_rect_);
            }
        }

        // Paint to the screen
        skia::DrawToNativeContext(canvas_.get(), ps.hdc, ps.rcPaint.left,
            ps.rcPaint.top, &ps.rcPaint);
        
        //HGDIOBJ hOldBr = SelectObject(ps.hdc, GetStockObject(NULL_BRUSH));
        //Rectangle(ps.hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);
        //SelectObject(ps.hdc, hOldBr);

        EndPaint(hWnd, &ps);

        // Draw children
        //UpdateWindow(hWnd);
    }

    void PaintRect(const IntRect& rect) {
        set_painting(true);
        //webwidget_->Paint(canvas_.get(), rect);

        if (!rect.isEmpty()) {
            PlatformContextSkia context(canvas_.get());
            // PlatformGraphicsContext is actually a pointer to PlatformContextSkia.
            GraphicsContext gc(reinterpret_cast<PlatformGraphicsContext*>(&context));

            IntRect dirty_rect(rect.x(), rect.y(), rect.width(), rect.height());

            frameview()->updateLayoutAndStyleIfNeededRecursive();
            gc.clip(frameview()->visibleContentRect());
            if (frame_->document() && frameview()) {
                //frameview()->paint(&gc, dirty_rect);
                frameview()->paintContents(&gc, rect);
            } else {
                gc.fillRect(dirty_rect, Color::white, ColorSpaceDeviceRGB);
            }
        }
        set_painting(false);
    }

    void set_painting(bool value) {
        painting_ = value;
    }

    FrameView* frameview() const {
        return frame_ ? frame_->view() : NULL;
    }

    void setCanScheduleResourceLoader() {canScheduleResourceLoader = true;}

    void scheduleResourceLoader(KFrameNetworkingContext* pContext) {
        KNetworkReplyHandler* pReplyHandler;
        ListHashSet<KNetworkReplyHandler*>::iterator iter;

        if (!canScheduleResourceLoader)
        { return; }

        while (pContext && !pContext->arrReplyHandler.isEmpty()) {
            iter = pContext->arrReplyHandler.begin();
            pReplyHandler = *iter;

            if (pReplyHandler->isDeferred())
            { break; }

            Vector<char> qbData;
            long long size;
            GetUrlData(pReplyHandler->URL().string(), qbData, size);

            sendResponseIfNeeded(pReplyHandler->URL(), pReplyHandler->client(), size, pReplyHandler->ResHandle());
            pReplyHandler->client()->didReceiveData(pReplyHandler->ResHandle(), (const char*)(&qbData[0]), size, size);
            pReplyHandler->client()->didFinishLoading(NULL, 0);

            delete pReplyHandler;
            pContext->arrReplyHandler.remove(pReplyHandler);

        }
    }

protected:
    IntRect paint_rect_;
    scoped_ptr<skia::PlatformCanvas> canvas_;
    HWND view_;
    bool painting_;
    WebCore::Frame* frame_;
    bool canScheduleResourceLoader;
};

KWebFrame::KWebFrame()
{
    m_hWnd = NULL;
    m_bInit = false;

//    m_windowSurface = NULL;
    m_frameLoaderClient = NULL;
    m_frame = NULL;

    m_chromeClient = NULL;
    m_contextMenuClient = NULL;
    m_editorClient = NULL;
    m_page = NULL;

    m_pFrameNetworkingContext = NULL;

    m_Imp = NULL;
}

bool KWebFrame::Init(/*WebCore::Page *webcorePage*/)
{
//     int argc = 1;
//     char *argv = "xxx.exe";
//     //m_QCApp = new QApplication(argc, &argv);
// 
//  m_QCApp = KApp::instance();

    m_Imp = new KWebFrameImp();

    //m_Widget = new KWidget();
    //m_Widget->setGeometry(0, 0, 800, 600);
    //m_windowSurface = new QRasterWindowSurface(m_Widget);

    WebCore::InitializeLoggingChannelsIfNecessary();
//    JSC::initializeThreading();

    ScriptController::initializeThreading();
    WTF::initializeMainThread();
    WebCore::SecurityOrigin::setLocalLoadPolicy(WebCore::SecurityOrigin::AllowLocalLoadsForLocalAndSubstituteData);

    //WebPlatformStrategies::initialize();
    //WebCore::FrameLoader::setLocalLoadPolicy(WebCore::FrameLoader::AllowLocalLoadsForLocalAndSubstituteData);

    m_chromeClient = new KChromeClient(this);
    m_contextMenuClient = new KContextMenuClient();
    m_editorClient = new KEditorClient();

    Page::PageClients pageClients;
    pageClients.chromeClient = m_chromeClient;
    pageClients.contextMenuClient = m_contextMenuClient;
    pageClients.editorClient = m_editorClient;
    pageClients.dragClient = new KDragClient(this);
    m_page = new Page(pageClients);

    // ### should be configurable
    m_page->settings()->setDefaultTextEncodingName("iso-8859-1");

    m_frameLoaderClient = new KFrameLoaderClient();
    RefPtr<Frame> newFrame = Frame::create(m_page/*webcorePage*/, NULL /*frameData->ownerElement*/, m_frameLoaderClient);
    m_frame = newFrame.get();
    m_frameLoaderClient->setFrame(this, m_frame);
    m_frame->init();

    // FIXME: All of the below should probably be moved over into WebCore
    const AtomicString sTemp = (const UChar*)L"";
    m_frame->tree()->setName(sTemp /*frameData->name*/);

    InitSetting();

    // balanced by adoptRef in KFrameLoaderClient::createFrame
//     if (frameData->ownerElement)
//         frame->ref();

    m_Imp->Init(m_frame);

    m_bInit = true;

    return true;
}

void KWebFrame::setViewportSize(const IntSize& size)
{
    m_viewportSize = size;
    if (!m_bInit)
        {Init();}

    // QWebFrame *frame = mainFrame();
    if (m_frame && m_frame->view()) {
        WebCore::FrameView* view = m_frame->view();
        //view->setFrameRect(QRect(QPoint(0, 0), size));
        //m_frame->forceLayout();
        view->resize(size) ;
        view->adjustViewSize();
    }
}

class SharedTimerKd /*: public QObject*/ {
    friend void setSharedTimerFiredFunction(void (*f)());
public:
    static SharedTimerKd* inst();

    void start(double);
    void stop();

    //protected:
    void timerEvent(/*QTimerEvent* ev*/);

private:
    SharedTimerKd(/*QObject* parent*/);
    ~SharedTimerKd();
    //KBasicTimer m_timer;
    void (*m_timerFunction)();
};

void KWebFrame::TimerFired()
{
    SharedTimerKd::inst()->timerEvent();
    m_Imp->scheduleResourceLoader(m_pFrameNetworkingContext);
    WTF::dispatchFunctionsFromMainThread();
}

void KWebFrame::resizeEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    UINT cx, cy;
    cx = LOWORD(lParam);
    cy = HIWORD(lParam);

    if (!m_hWnd)
        {m_hWnd = hWnd;}

    IntSize size(cx, cy);
    setViewportSize(size);
}

void KWebFrame::repaintRequested(const IntRect& windowRect)
{
    RECT winRect = {windowRect.x(), windowRect.y(), 
        windowRect.x() + windowRect.width(), windowRect.y() + windowRect.height()};
    ::InvalidateRect(m_hWnd, &winRect, false);
}

void KWebFrame::paintEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    m_Imp->paintEvent(hWnd, message, wParam, lParam);
//     PAINTSTRUCT ps;
//     if (!m_bInit)
//         {return;}
// 
//     HDC hd = BeginPaint(hWnd, &ps);
// 
//     const QRect updateRect(QPoint(ps.rcPaint.left, ps.rcPaint.top),
//         QPoint(ps.rcPaint.right, ps.rcPaint.bottom));
// 
//     m_windowSurface->setGeometry(updateRect);
// 
//     QPaintDevice* paintDevice = m_windowSurface->paintDevice();
//     
//     {
//         //HBRUSH hbrush = CreateSolidBrush(RGB(0xff, 0xff, 0xff));
//         //RECT rcPaint = {0, 0x300, 0, 0x300};
//         //FillRect(widget_dc, &rcPaint, hbrush);
//         //DeleteObject(hbrush);
//     }
// 
//     QRegion rgn(updateRect, QRegion::Rectangle);
//     
//     QPainter p(paintDevice);
//     render(&p, updateRect);
// 
//     HDC widget_dc = paintDevice->paintEngine()->getDC();
//     QRect br = rgn.boundingRect();
//     ::BitBlt(hd, br.x(), br.y(), br.width(), br.height(), widget_dc, br.x(), br.y(), SRCCOPY);
//     //::BitBlt(hd, 0, 0, 0x300, 0x300, widget_dc, 0, 0, SRCCOPY);
// 
//     EndPaint(hWnd, &ps);
}

//void KWebFrame::render(QPainter *painter, const QRegion &clip)
//{
//     GraphicsContext context(painter);
//     if (context.paintingDisabled() && !context.updatingControlTints())
//         return;
// 
//     renderRelativeCoords(&context, AllLayers, clip);
//}

//void KWebFrame::renderRelativeCoords(GraphicsContext* context, QFlags<KWebFrame::RenderLayer> layers, const QRegion& clip)
//{
//     if (!m_frame->view() || !m_frame->contentRenderer())
//         return;
// 
//     QVector<QRect> vector = clip.rects();
//     if (vector.isEmpty())
//         return;
// 
//     QPainter* painter = context->platformContext();
// 
//     WebCore::FrameView* view = m_frame->view();
//     view->updateLayoutAndStyleIfNeededRecursive();
// 
//     if (layers & KWebFrame::ContentsLayer) {
//         for (int i = 0; i < vector.size(); ++i) {
//             const QRect& clipRect = vector.at(i);
// 
//             QRect rect = clipRect.intersected(view->frameRect());
// 
//             context->save();
//             painter->setClipRect(clipRect, Qt::IntersectClip);
// 
//             int x = view->x();
//             int y = view->y();
// 
//             int scrollX = view->scrollX();
//             int scrollY = view->scrollY();
// 
//             context->translate(x, y);
//             rect.translate(-x, -y);
//             context->translate(-scrollX, -scrollY);
//             rect.translate(scrollX, scrollY);
//             context->clip(view->visibleContentRect());
// 
//             view->paintContents(context, rect);
// 
//             context->restore();
//         }
// #if USE(ACCELERATED_COMPOSITING) && USE(TEXTURE_MAPPER)
//         renderCompositedLayers(context, IntRect(clip.boundingRect()));
// #endif
//     }
//     renderFrameExtras(context, layers, clip);
//}

// void KWebFrame::renderFrameExtras(GraphicsContext* context, QFlags<KWebFrame::RenderLayer> layers, const QRegion& clip)
// {
//     if (!(layers & (KWebFrame::PanIconLayer | KWebFrame::ScrollBarLayer)))
//         return;
//     QPainter* painter = context->platformContext();
//     WebCore::FrameView* view = m_frame->view();
//     QVector<QRect> vector = clip.rects();
//     for (int i = 0; i < vector.size(); ++i) {
//         const QRect& clipRect = vector.at(i);
// 
//         QRect intersectedRect = clipRect.intersected(view->frameRect());
// 
//         painter->save();
//         painter->setClipRect(clipRect, Qt::IntersectClip);
// 
//         int x = view->x();
//         int y = view->y();
// 
//         if (layers & KWebFrame::ScrollBarLayer
//             && !view->scrollbarsSuppressed()
//             && (view->horizontalScrollbar() || view->verticalScrollbar())) {
// 
//                 QRect rect = intersectedRect;
//                 context->translate(x, y);
//                 rect.translate(-x, -y);
//                 view->paintScrollbars(context, rect);
//                 context->translate(-x, -y);
//         }
// 
// #if ENABLE(PAN_SCROLLING)
//         if (layers & KWebFrame::PanIconLayer)
//             view->paintPanScrollIcon(context);
// #endif
// 
//         painter->restore();
//     }
//}

void KWebFrame::MouseEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    bool shift = false, ctrl = false, alt = false, meta = false;
    int clickCount = 0;
    MouseButton button = LeftButton;
    IntPoint pos;
    IntPoint globalPos;

    if (!m_frame || !m_frame->view())
        {return;}

    pos.setX(GET_X_LPARAM(lParam));
    pos.setY(GET_Y_LPARAM(lParam));

    POINT widgetpt = {pos.x(), pos.y()};
    ::ClientToScreen(hWnd, &widgetpt);
    globalPos.setX(widgetpt.x);
    globalPos.setY(widgetpt.y);

    MouseEventType eventType;

    switch (message)
    {
    case WM_LBUTTONDOWN:
        button = LeftButton;
        eventType = MouseEventPressed;
        clickCount = 1;
        break;
    case WM_MBUTTONDOWN:
        return;
    case WM_RBUTTONDOWN:
        button = RightButton;
        eventType = MouseEventPressed;
        clickCount = 1;
        break;
    case WM_LBUTTONUP:
        button = LeftButton;
        eventType = MouseEventReleased;
        clickCount = 0;
        break;
    case WM_MBUTTONUP:
        return;
    case WM_RBUTTONUP:
        button = RightButton;
        eventType = MouseEventReleased;
        clickCount = 0;
        break;

    case WM_MOUSEMOVE:
        eventType = MouseEventReleased;
        clickCount = 0;
        break;
    default:
        return;
    }
    double timestamp = WTF::currentTime();
    PlatformMouseEvent ev(pos, globalPos, button, eventType,
        clickCount, shift, ctrl, alt, meta, timestamp);

    bool accepted = false;
    switch (message)
    {
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
        accepted = m_frame->eventHandler()->handleMousePressEvent(ev);
        break;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        accepted = m_frame->eventHandler()->handleMouseReleaseEvent(ev);
        break;
    case WM_MOUSEMOVE:
        accepted = m_frame->eventHandler()->mouseMoved(ev);
        break;
    default:
        return;
    }

    return;
}

static bool fromBase64(const UChar* base64, int size, Vector<char>& outData)
{
    unsigned int buf = 0;
    int nbits = 0;
    Vector<char>& tmp = outData;
    tmp.resize((size * 3) / 4);

    int offset = 0;
    for (int i = 0; i < size; ++i) {
        int ch = base64[i];
        int d;

        if (ch >= L'A' && ch <= L'Z')
            d = ch - L'A';
        else if (ch >= L'a' && ch <= L'z')
            d = ch - L'a' + 26;
        else if (ch >= L'0' && ch <= L'9')
            d = ch - L'0' + 52;
        else if (ch == L'+')
            d = 62;
        else if (ch == L'/')
            d = 63;
        else
            d = -1;

        if (d != -1) {
            buf = (buf << 6) | d;
            nbits += 6;
            if (nbits >= 8) {
                nbits -= 8;
                tmp[offset++] = buf >> nbits;
                buf &= (1 << nbits) - 1;
            }
        }
    }

    //tmp.truncate(offset);
    if (offset < tmp.size())
        tmp.resize(offset);

    return true;
}

static bool GetUrlDataFromBase64Data(
    /* in */ String Url,
    /* out */ Vector<char>& OutData,
    /* out */ long long& fileSize
    )
{
    Url.remove(0, 22);
    fromBase64((const UChar*)Url.characters(), Url.length(), OutData);
    fileSize = OutData.size();

    return true;
}

static int gTest = 0;

static bool GetUrlDataFromFile(
    /* in */ String Url,
    /* out */ Vector<char>& OutData,
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
        MessageBoxW(0, (LPCWSTR)Url.charactersWithNullTermination(), L"打开文件失败", 0);
        //Url += (const UChar*)L"上诉文件打开失败";
        //_CrtDbgReportW(_CRT_WARN, L"KWebFrame.cpp", 667, NULL, L"%ws\n", 
        //    (LPCWSTR)Url.charactersWithNullTermination());
        
        return false; 
    }

    bRet = GetFileSizeEx(hFile, &FileSize);
    if (!bRet || (0 == FileSize.HighPart && 0 == FileSize.LowPart))
    { goto Exit0; }

    OutData.resize(FileSize.LowPart + 10);

    if(!::ReadFile(hFile, (LPVOID)(&OutData[0]), FileSize.LowPart, &bytesReaded, NULL))
    { goto Exit0; }

    fileSize = FileSize.QuadPart;
    bRet = true;

Exit0:
    if (hFile)
    { ::CloseHandle(hFile); }

    return bRet;
}

static bool GetUrlData(
    /* in */ String Url,
    /* out */ Vector<char>& OutData,
    /* out */ long long& fileSize
    )
{
    if (-1 != Url.find(String((const char*)"data:image/png;base64,"), 0, false))
    { return GetUrlDataFromBase64Data(Url, OutData, fileSize); }

    return GetUrlDataFromFile(Url, OutData, fileSize);
}

static void sendResponseIfNeeded (
    KURL kurl,
    ResourceHandleClient* client,
    long long size,
    ResourceHandle* resourceHandle
    )
{
    if (!client)
        return;

    WTF::String contentType;
    WTF::String encoding;
    WTF::String mimeType;

    String extension = kurl.path();
    //int index = extension.lastIndexOf(QLatin1Char('.'));
    int index = extension.reverseFind('.');
    if (index > 0) {
        //extension = extension.mid(index + 1);
        extension = extension.substring(index + 1);
        mimeType = MIMETypeRegistry::getMIMETypeForExtension(extension);
    }

    String suggestedFilename;
    suggestedFilename = kurl.lastPathComponent();

    ResourceResponse response(kurl, mimeType,
        size,
        encoding,
        suggestedFilename);

    response.setHTTPStatusCode(0);
    response.setHTTPStatusText(String());

    response.setHTTPHeaderField(AtomicString("Last-Modified"), String("Fri, 09 Dec 2011 23:26:42 GMT"));
    response.setHTTPHeaderField(AtomicString("Content-Length"), String("416"));

    client->didReceiveResponse(resourceHandle, response);    
}

void KWebFrame_Load(
    //const QNetworkRequest &req,
    LPCWSTR lpUrl,
    WebCore::Frame *pFrame,
    KFrameNetworkingContext** ppFrameNetworkingContext,
    KNetworkAccessManager::Operation operation
    /*const QByteArray &body*/
    )
{
    KURL Url(ParsedURLString, String((const UChar*)lpUrl));
    //QUrl url = ensureAbsoluteUrl(req.url());

    WebCore::ResourceRequest request(Url /*url*/ );

    switch (operation) {
        case KNetworkAccessManager::HeadOperation:
            request.setHTTPMethod("HEAD");
            break;
        case KNetworkAccessManager::GetOperation:
            request.setHTTPMethod("GET");
            break;
        case KNetworkAccessManager::PutOperation:
            request.setHTTPMethod("PUT");
            break;
        case KNetworkAccessManager::PostOperation:
            request.setHTTPMethod("POST");
            break;
        case KNetworkAccessManager::UnknownOperation:
            // eh?
            break;
    }

    //     QList<QByteArray> httpHeaders = req.rawHeaderList();
    //     for (int i = 0; i < httpHeaders.size(); ++i) {
    //         const QByteArray &headerName = httpHeaders.at(i);
    //         request.addHTTPHeaderField(QString::fromLatin1(headerName), QString::fromLatin1(req.rawHeader(headerName)));
    //     }

    //     if (!body.isEmpty())
    //         request.setHTTPBody(WebCore::FormData::create(body.constData(), body.size()));

    pFrame->loader()->load(request, false); // 在ResourceHandle::start(Frame* frame)中接受数据   
}

void KWebFrame::loadFormUrl(LPCWSTR lpUrl)
{
    //QLatin1String sString(lpUrl);
    //QUrl url(sString);
    //load(QNetworkRequest(ensureAbsoluteUrl(url)), KNetworkAccessManager::GetOperation);
    KWebFrame_Load(lpUrl, m_frame, &m_pFrameNetworkingContext, KNetworkAccessManager::GetOperation);
    m_Imp->setCanScheduleResourceLoader();
}

int KWebFrame::notifFromResHandle(LPCWSTR lpMsg, void* pContent)
{
    if (0 == wcscmp(L"ResourceHandle::start", lpMsg)) {
        if (pContent) {
            //m_arrResHandleClient.push_back((ResourceHandleClient*)pContent);
            if (!m_pFrameNetworkingContext) {
                m_pFrameNetworkingContext = (KFrameNetworkingContext*)pContent;
            } else {
                ASSERT(m_pFrameNetworkingContext == pContent);
            }
        }
    }

     return 0;
}

bool KWebFrame::InitSetting()
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
        //         settings->setMinimumLogicalFontSize(size);
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