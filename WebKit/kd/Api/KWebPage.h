
#ifndef _KWEB_FRAME_H_
#define _KWEB_FRAME_H_

#include <Client/KApp.h>

#include "KNetworkAccessManager.h"
#include "IntRect.h"
#include "KChromeClient.h"

#include "KFrameLoaderClient.h"
#include "page.h"
#include "ResourceHandleClient.h"

#include "KdGuiApi.h"
#include "KdGuiApiImp.h"

#include "KdPageInfo.h"

#include "bridge/npruntime.h"

class KdWidgetMgr;

struct SQDbgServer;
typedef SQDbgServer* HSQREMOTEDBG;

class KQuery;
class KdValArray;

namespace WebCore {

class KFrameLoaderClient;
class KChromeClient;
class KContextMenuClient;
class KEditorClient;
class Page;
class KFrameNetworkingContext;
class KWidget;
class KWebPageImpl;
struct AsynchronousResLoadInfo;

struct KWebApiCallbackSet {
    PFN_KdPageCallback m_xmlHaveFinished;
    PFN_KdPageWinMsgCallback m_msgPreCallBack;
    PFN_KdPageWinMsgCallback m_msgPostCallBack;
    PFN_KdPageCallback m_unintCallBack;
    PFN_KdPageScriptInitCallback m_scriptInitCallBack;
    PFN_KdResCallback m_resHandle;
    PFN_KdResCallback m_resOtherNameQuery;
    PFN_KdPagePaintCallback m_paint;
    NPInvokeFunctionPtr m_javascriptCallCppPtr;
};

class KWebPage
{
    WTF_MAKE_FAST_ALLOCATED;
public:

    enum RenderLayer {
        ContentsLayer = 0x10,
        ScrollBarLayer = 0x20,
        PanIconLayer = 0x40,

        AllLayers = 0xff
    };

    KWebPage(KdGuiObjPtr kdGuiObj, void* foreignPtr);
    ~KWebPage();

    bool Init(HWND hWnd);
    bool InitSetting();

    static KWebPage* KWebPage::createWindow(
        KdGuiObjPtr kdGuiObj,
        KWebPage* parentPage,
        const WindowFeatures* features,
        void* foreignPtr
        );
    static KWebPage* KWebPage::createWindowByRealWnd(
        KdGuiObjPtr kdGuiObj,
        KdPageInfoPtr pageInfo,
        void* foreignPtr
        );

#if 0
    void appendAnimNode(KQuery* node);
    void removeAnimNode(KQuery* node);
    void forceStopAllAnim();
#endif

    void windowCloseRequested();

    void javaScriptAlert(String& message);
    
    void loadFormUrl(LPCWSTR lpUrl);
    void loadFormData(const void* lpData, int nLen);

    void paintEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void mouseEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void captureChangedEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void killFocusEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    
    int  inputEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    bool inputEventToRichEdit(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void timerFired();

    void resizeEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    IntSize viewportSize() const {return m_viewportSize;}
    void setViewportSize(const IntSize& size);

    void repaintRequested(const IntRect& windowRect);

    void setIsDraggableRegionNcHitTest();

    HWND getHWND() {return m_hWnd;}

    int notifFromResHandle(LPCWSTR lpMsg, void* pContent);

    Frame* frame() {return m_frame;}
    Page* page() {return m_page;}

    KdGuiObjPtr getKdGuiObjPtr() {return m_kdGuiObj;}
    void* getForeignPtr() {return m_foreignPtr;}

    KdWidgetMgr* rootWidget() {return m_rootWidget;}

    void setBackgroundColor(COLORREF c);

    void showDebugNodeData();

    void postResToAsynchronousLoad(const WCHAR* pUrl, const void* pResBuf, int nResBufLen, bool bNeedSavaRes);

    bool invokeScript(NPIdentifier methodName, const NPVariant* args, uint32_t argCount, NPVariant* result);
    NPInvokeFunctionPtr getJavascriptCallCppCallback() { return m_callbacks.m_javascriptCallCppPtr; }
    void setJavascriptCallCppCallback(NPInvokeFunctionPtr p) { m_callbacks.m_javascriptCallCppPtr = p; }

    KWebApiCallbackSet m_callbacks;

    bool m_bMouseTrack;

    KdPageDebugInfo m_debugInfo;
    HSQREMOTEDBG m_remoteScriptDbg;

    BOOL* m_messageStackVar; // 给消息处理函数使用，保存地址。例如，WM_TIMER中调用DestroyWindow，
    // 窗口摧毁了，消息函数还在进入WM_TIMER的处理函数，导致野指针崩溃.

protected:
    static KWebPage* _createWindow (
        KdGuiObjPtr kdGuiObj,
        KWebPage* parentPage,
        const WindowFeatures* features,
        KdPageInfoPtr pageInfo, // 优先级是先features，再pageInfo
        void* foreignPtr
        );

    void setCanScheduleResourceLoader();
    bool m_canScheduleResourceLoader;

    void scheduleResourceLoader(KFrameNetworkingContext* pContext);

    KFrameLoaderClient* m_frameLoaderClient;
    Frame* m_frame;

    KChromeClient* m_chromeClient;
    KContextMenuClient* m_contextMenuClient;
    KEditorClient* m_editorClient;
    Page* m_page;

    enum KWebPageState {
        UNINIT,
        INIT,
        DESTROYING
    } m_state;

    IntSize m_viewportSize;
    HWND m_hWnd;

    bool m_LMouseDown;
    bool m_RMouseDown;

    KWebPageImpl* m_pPageImpl;

    KFrameNetworkingContext* m_pFrameNetworkingContext;

    KdWidgetMgr* m_rootWidget;

    KdGuiObjPtr m_kdGuiObj;
    void* m_foreignPtr;

    Vector<KQuery*> m_animatingNodeQueue; // 正在进行动画的节点，方便在关闭时候统一删除动画

    Vector<AsynchronousResLoadInfo*> m_asynResQueue;

    
    bool m_isAlert;
    bool m_isDraggableRegionNcHitTest;
    IntPoint m_lastPosForDrag;
};

} // namespace WebCore

#endif // _KWEB_FRAME_H_