
#ifndef _KWEB_FRAME_H_
#define _KWEB_FRAME_H_

//#include <Kernel/KWidget.h>
//#include <QtGui/qicon.h>
//#include <QtCore/qurl.h>
#include <Client/KApp.h>
//#include "qnetworkrequest.h"

#include "KNetworkAccessManager.h"
//#include "qnetworkaccessmanager.h"
#include "IntRect.h"
#include "KChromeClient.h"


#include "KFrameLoaderClient.h"
#include "KContextMenuClient.h"

#include "KDragClient.h"
#include "KEditorClient.h"
#include "page.h"
#include "ResourceHandleClient.h"

namespace WebCore {

class KFrameLoaderClient;
class KChromeClient;
class Page;
class KFrameNetworkingContext;
class KWidget;
class KWebFrameImp;

class KWebFrame
{
public:

    enum RenderLayer {
        ContentsLayer = 0x10,
        ScrollBarLayer = 0x20,
        PanIconLayer = 0x40,

        AllLayers = 0xff
    };

    KWebFrame();

    bool Init(/*WebCore::Page *webcorePage*/);
    bool InitSetting();

    void loadFormUrl(LPCWSTR lpUrl);
    void load(const QNetworkRequest &req,
        KNetworkAccessManager::Operation operation);

    void paintEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void MouseEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void TimerFired();

    void resizeEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
//    void render(QPainter *painter, const QRegion &clip);

    IntSize viewportSize() const {return m_viewportSize;}
    void setViewportSize(const IntSize& size);

    void repaintRequested(const IntRect& windowRect);

    HWND GetHWND() {return m_hWnd;}

    int notifFromResHandle(LPCWSTR lpMsg, void* pContent);

protected:
    //void renderRelativeCoords(GraphicsContext* context, QFlags<KWebFrame::RenderLayer> layers, const QRegion& clip);
    //void renderFrameExtras(GraphicsContext* context, QFlags<KWebFrame::RenderLayer> layers, const QRegion& clip);

    //QWindowSurface *m_windowSurface;
    KFrameLoaderClient* m_frameLoaderClient;
    WebCore::Frame *m_frame;

    KChromeClient* m_chromeClient;
    KContextMenuClient* m_contextMenuClient;
    KEditorClient* m_editorClient;
    Page* m_page;

    KApp* m_QCApp;
    KWidget *m_Widget;

    bool m_bInit;
    IntSize m_viewportSize;
    HWND m_hWnd;

    bool m_LMouseDown;
    bool m_RMouseDown;

    KWebFrameImp* m_Imp;

    //std::list<ResourceHandleClient*> m_arrResHandleClient;
    KFrameNetworkingContext* m_pFrameNetworkingContext;
private:
};

} // namespace WebCore

#endif // _KWEB_FRAME_H_