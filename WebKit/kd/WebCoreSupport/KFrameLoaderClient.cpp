
#include "KFrameLoaderClient.h"
#include "DocumentLoader.h"
#include "MIMETypeRegistry.h"
#include "FrameView.h"
#include "HTTPStatusCodes.h"
#include "HTTPParsers.h"
#include "KFrameNetworkingContext.h"

namespace WebCore
{

    KFrameLoaderClient::KFrameLoaderClient()
        : m_frame(0)
        , m_webPage(0)
        , m_hasSentResponseToPlugin(false)
        , m_firstData(false)
        , m_loadSucceeded(false)
        , m_policyFunction(NULL)
    {
        //connect(this, SIGNAL(sigCallPolicyFunction(int)), this, SLOT(slotCallPolicyFunction(int)), Qt::QueuedConnection);
    }


    KFrameLoaderClient::~KFrameLoaderClient()
    {
    }

    void KFrameLoaderClient::setPage(KWebPage* webPage, Frame* frame)
    {
        m_webPage = webPage;
        m_frame = frame;
//         if (!m_webPage || !m_webPage->page()) {
//             qWarning("KFrameLoaderClient::setFrame frame without Page!");
//             return;
//         }
// 
//         connect(this, SIGNAL(loadStarted()),
//             m_webPage->page(), SIGNAL(loadStarted()));
//         connect(this, SIGNAL(loadProgress(int)),
//             m_webPage->page(), SIGNAL(loadProgress(int)));
//         connect(this, SIGNAL(loadFinished(bool)),
//             m_webPage->page(), SIGNAL(loadFinished(bool)));
//         connect(this, SIGNAL(titleChanged(const QString&)),
//             m_webPage, SIGNAL(titleChanged(const QString&)));
    }

    KWebPage* KFrameLoaderClient::webPage() const
    {
        return m_webPage;
    }

    bool KFrameLoaderClient::hasWebView() const
    {
        //notImplemented();
        return true;
    }

    void KFrameLoaderClient::savePlatformDataToCachedPage(CachedPage*) 
    {
        notImplemented();
    }

    void KFrameLoaderClient::transitionToCommittedFromCachedPage(CachedPage*)
    {
    }

    void KFrameLoaderClient::transitionToCommittedForNewPage()
    {
        ASSERT(m_frame);
        ASSERT(m_webPage);

        //QBrush brush = m_webPage->page()->palette().brush(QPalette::Base);
        //QColor backgroundColor = brush.style() == Qt::SolidPattern ? brush.color() : QColor();
        Color backgroundColor(199, 237, 204, 255);
//         WebCore::FrameLoaderClient::transitionToCommittedForNewPage(m_frame, m_webPage->viewportSize(),
//             color, false, IntSize(), false, ScrollbarAlwaysOff, ScrollbarAlwaysOff);

        //const QSize preferredLayoutSize = page->preferredContentsSize();

        ScrollbarMode hScrollbar = ScrollbarAlwaysOff;
        ScrollbarMode vScrollbar = ScrollbarAlwaysOff;
        bool hLock = hScrollbar != ScrollbarAuto;
        bool vLock = vScrollbar != ScrollbarAuto;

        IntSize currentVisibleContentSize = m_frame->view() ? m_frame->view()->actualVisibleContentRect().size() : IntSize();

        m_frame->createView(m_webPage->viewportSize(),
            backgroundColor, !backgroundColor.alpha(),
            IntSize(),
            false,
            hScrollbar, hLock,
            vScrollbar, vLock);

//         bool isMainFrame = m_frame == m_frame->page()->mainFrame();
//         if (isMainFrame && page->d->client) {
//             m_frame->view()->setPaintsEntireContents(page->d->client->viewResizesToContentsEnabled());
//             m_frame->view()->setDelegatesScrolling(page->d->client->viewResizesToContentsEnabled());
//         }

        // The HistoryController will update the scroll position later if needed.
        m_frame->view()->setActualVisibleContentRect(IntRect(IntPoint::zero(), currentVisibleContentSize));

    }


    void KFrameLoaderClient::makeRepresentation(DocumentLoader*)
    {
        // don't need this for now I think.
    }


    void KFrameLoaderClient::forceLayout()
    {
        m_frame->view()->forceLayout(true);
    }


    void KFrameLoaderClient::forceLayoutForNonHTML()
    {
    }


    void KFrameLoaderClient::setCopiesOnScroll()
    {
        // apparently mac specific
    }


    void KFrameLoaderClient::detachedFromParent2()
    {
    }


    void KFrameLoaderClient::detachedFromParent3()
    {
    }

    void KFrameLoaderClient::dispatchDidHandleOnloadEvents()
    {
        // don't need this one
        //if (dumpFrameLoaderCallbacks)
        //    printf("%s - didHandleOnloadEventsForFrame\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));

    }


    void KFrameLoaderClient::dispatchDidReceiveServerRedirectForProvisionalLoad()
    {
        //if (dumpFrameLoaderCallbacks)
        //    printf("%s - didReceiveServerRedirectForProvisionalLoadForFrame\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));

        notImplemented();
    }


    void KFrameLoaderClient::dispatchDidCancelClientRedirect()
    {
        //if (dumpFrameLoaderCallbacks)
        //   printf("%s - didCancelClientRedirectForFrame\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));

        notImplemented();
    }


    void KFrameLoaderClient::dispatchWillPerformClientRedirect(const KURL& url,
        double interval,
        double fireDate)
    {
       // if (dumpFrameLoaderCallbacks)
        //    printf("%s - willPerformClientRedirectToURL: %s \n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)), qPrintable(drtDescriptionSuitableForTestResult(url)));

        notImplemented();
    }


    void KFrameLoaderClient::dispatchDidChangeLocationWithinPage()
    {
//         if (dumpFrameLoaderCallbacks)
//             printf("%s - didChangeLocationWithinPageForFrame\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));
// 
//         if (!m_webPage)
//             return;
// 
//         emit m_webPage->urlChanged(m_webPage->url());
//         m_webPage->page()->d->updateNavigationActions();
    }


    void KFrameLoaderClient::dispatchWillClose()
    {
//         if (dumpFrameLoaderCallbacks)
//             printf("%s - willCloseFrame\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));
    }


    void KFrameLoaderClient::dispatchDidStartProvisionalLoad()
    {
//         if (dumpFrameLoaderCallbacks)
//             printf("%s - didStartProvisionalLoadForFrame\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));
// 
//         if (m_webPage)
//             emit m_webPage->provisionalLoad();
    }


    void KFrameLoaderClient::dispatchDidReceiveTitle(const StringWithDirection&)
    {
//         if (dumpFrameLoaderCallbacks)
//             printf("%s - didReceiveTitle: %s\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)), qPrintable(QString(title)));
// 
//         if (!m_webPage)
//             return;
// 
// 
// 
//         // ### hack
//         emit m_webPage->urlChanged(m_webPage->url());
//         emit titleChanged(title);
    }


    void KFrameLoaderClient::dispatchDidCommitLoad()
    {
//         if (dumpFrameLoaderCallbacks)
//             printf("%s - didCommitLoadForFrame\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));
// 
//         if (m_frame->tree()->parent() || !m_webPage)
//             return;
// 
//         m_webPage->page()->d->updateNavigationActions();
// 
//         // We should assume first the frame has no title. If it has, then the above dispatchDidReceiveTitle()
//         // will be called very soon with the correct title.
//         // This properly resets the title when we navigate to a URI without a title.
//         emit titleChanged(String());
    }


    void KFrameLoaderClient::dispatchDidFinishDocumentLoad()
    {
//         if (dumpFrameLoaderCallbacks)
//             printf("%s - didFinishDocumentLoadForFrame\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));
// 
//         if (QWebPagePrivate::drtRun) {
//             int unloadEventCount = m_frame->eventHandler()->pendingFrameUnloadEventCount();
//             if (unloadEventCount)
//                 printf("%s - has %u onunload handler(s)\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)), unloadEventCount);
//         }
// 
//         if (m_frame->tree()->parent() || !m_webPage)
//             return;
// 
//         m_webPage->page()->d->updateNavigationActions();
    }


    void KFrameLoaderClient::dispatchDidFinishLoad()
    {
//         if (dumpFrameLoaderCallbacks)
//             printf("%s - didFinishLoadForFrame\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));
// 
//         m_loadSucceeded = true;
// 
//         if (m_frame->tree()->parent() || !m_webPage)
//             return;
//         m_webPage->page()->d->updateNavigationActions();
    }


    void KFrameLoaderClient::dispatchDidFirstLayout()
    {
//         if (m_webPage)
//             emit m_webPage->initialLayoutCompleted();
    }

    void KFrameLoaderClient::dispatchDidFirstVisuallyNonEmptyLayout()
    {
        notImplemented();
    }

    void KFrameLoaderClient::dispatchShow()
    {
        notImplemented();
    }


    void KFrameLoaderClient::cancelPolicyCheck()
    {
    }


    void KFrameLoaderClient::dispatchWillSubmitForm(FramePolicyFunction function,
        PassRefPtr<FormState>)
    {
        notImplemented();
        //Q_ASSERT(!m_policyFunction);
        // FIXME: This is surely too simple
        //callPolicyFunction(function, PolicyUse);
    }


    void KFrameLoaderClient::dispatchDidLoadMainResource(DocumentLoader*)
    {
    }


    void KFrameLoaderClient::revertToProvisionalState(DocumentLoader*)
    {
        notImplemented();
    }


    void KFrameLoaderClient::postProgressStartedNotification()
    {
//         if (m_webPage && m_frame->page()) {
//             emit loadStarted();
//             postProgressEstimateChangedNotification();
//         }
//         if (m_frame->tree()->parent() || !m_webPage)
//             return;
//         m_webPage->page()->d->updateNavigationActions();
    }

    void KFrameLoaderClient::postProgressEstimateChangedNotification()
    {
//         if (m_webPage && m_frame->page())
//             emit loadProgress(qRound(m_frame->page()->progress()->estimatedProgress() * 100));
    }

    void KFrameLoaderClient::postProgressFinishedNotification()
    {
        // send a mousemove event to
        // (1) update the cursor to change according to whatever is underneath the mouse cursor right now
        // (2) display the tool tip if the mouse hovers a node which has a tool tip
//         if (m_frame && m_frame->eventHandler() && m_webPage->page()) {
//             QWidget* view = m_webPage->page()->view();
//             if (view && view->hasFocus()) {
//                 QPoint localPos = view->mapFromGlobal(QCursor::pos());
//                 if (view->rect().contains(localPos)) {
//                     QMouseEvent event(QEvent::MouseMove, localPos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
//                     m_frame->eventHandler()->mouseMoved(PlatformMouseEvent(&event, 0));
//                 }
//             }
//         }
// 
//         if (m_webPage && m_frame->page())
//             emit loadFinished(m_loadSucceeded);
    }

    void KFrameLoaderClient::setMainFrameDocumentReady(bool b)
    {
        // this is only interesting once we provide an external API for the DOM
    }


    void KFrameLoaderClient::willChangeTitle(DocumentLoader*)
    {
        // no need for, dispatchDidReceiveTitle is the right callback
    }


    void KFrameLoaderClient::didChangeTitle(DocumentLoader *)
    {
        // no need for, dispatchDidReceiveTitle is the right callback
    }


    void KFrameLoaderClient::finishedLoading(DocumentLoader* loader)
    {
//         if (!m_pluginView) {
            if(m_firstData) {
                loader->writer()->setEncoding(m_response.textEncodingName(), false);
                m_firstData = false; 
            }
//         }
//         else {
//             m_pluginView->didFinishLoading();
//             m_pluginView = 0;
//             m_hasSentResponseToPlugin = false;
//         }
    }


    bool KFrameLoaderClient::canShowMIMEType(const String& MIMEType) const
    {
        if (MIMETypeRegistry::isSupportedImageMIMEType(MIMEType))
            return true;

        if (MIMETypeRegistry::isSupportedNonImageMIMEType(MIMEType))
            return true;

//         if (m_frame && m_frame->settings()  && m_frame->settings()->arePluginsEnabled()
//             && PluginDatabase::installedPlugins()->isMIMETypeRegistered(MIMEType))
//             return true;

        return false;
    }

    bool KFrameLoaderClient::representationExistsForURLScheme(const String& URLScheme) const
    {
        return false;
    }


    String KFrameLoaderClient::generatedMIMETypeForURLScheme(const String& URLScheme) const
    {
        notImplemented();
        return String();
    }


    void KFrameLoaderClient::frameLoadCompleted()
    {
        // Note: Can be called multiple times.
        // Even if already complete, we might have set a previous item on a frame that
        // didn't do any data loading on the past transaction. Make sure to clear these out.
/*        m_frame->loader()->setPreviousHistoryItem(0);*/
    }


    void KFrameLoaderClient::restoreViewState()
    {
//         if (!m_webPage)
//             return;
//         emit m_webPage->page()->restoreFrameStateRequested(m_webPage);
    }


    void KFrameLoaderClient::provisionalLoadStarted()
    {
        // don't need to do anything here
    }


    void KFrameLoaderClient::didFinishLoad()
    {
        //     notImplemented();
    }


    void KFrameLoaderClient::prepareForDataSourceReplacement()
    {
        
    }

    void KFrameLoaderClient::setTitle(const StringWithDirection&, const KURL&)
    {
        // no need for, dispatchDidReceiveTitle is the right callback
    }


    String KFrameLoaderClient::userAgent(const KURL& url)
    {
//         if (m_webPage) {
//             return m_webPage->page()->userAgentForUrl(url);
//         }
        return String("Mozilla/5.0 (Windows; N; ; zh-CN) AppleWebKit/527+ (KHTML, like Gecko, Safari/419.3)  demobrowser/0.1");
    }

    void KFrameLoaderClient::dispatchDidReceiveIcon()
    {
//         if (dumpFrameLoaderCallbacks)
//             printf("%s - didReceiveIconForFrame\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));
// 
//         if (m_webPage) {
//             emit m_webPage->iconChanged();
//         }
    }

    void KFrameLoaderClient::frameLoaderDestroyed()
    {
        //delete m_webPage;
        m_webPage->windowCloseRequested();
        m_frame = 0;
        m_webPage = 0;

        delete this;
    }

    bool KFrameLoaderClient::canHandleRequest(const WebCore::ResourceRequest&) const
    {
        return true;
    }

    void KFrameLoaderClient::windowObjectCleared()
    {
//         if (dumpFrameLoaderCallbacks)
//             printf("%s - didClearWindowObjectForFrame\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));
// 
//         if (m_webPage)
//             emit m_webPage->javaScriptWindowObjectCleared();
    }

    void KFrameLoaderClient::didPerformFirstNavigation() const
    {
//         if (m_frame->tree()->parent() || !m_webPage)
//             return;
//         m_webPage->page()->d->updateNavigationActions();
    }

    void KFrameLoaderClient::registerForIconNotification(bool)
    {
        notImplemented();
    }

    void KFrameLoaderClient::updateGlobalHistory()
    {
//         QWebHistoryInterface *history = QWebHistoryInterface::defaultInterface();
//         if (history)
//             history->addHistoryEntry(m_frame->loader()->documentLoader()->urlForHistory().prettyURL());
    }

    bool KFrameLoaderClient::shouldGoToHistoryItem(WebCore::HistoryItem *item) const
    {
        return true;
    }

    void KFrameLoaderClient::saveViewStateToItem(WebCore::HistoryItem* item)
    {
//         QWebHistoryItem historyItem(new QWebHistoryItemPrivate(item));
//         emit m_webPage->page()->saveFrameStateRequested(m_webPage, &historyItem);
    }

    bool KFrameLoaderClient::canCachePage() const
    {
        return true;
    }

    void KFrameLoaderClient::setMainDocumentError(WebCore::DocumentLoader* loader, const WebCore::ResourceError& error)
    {
//         if (!m_pluginView) {
            if (m_firstData) {
                loader->writer()->setEncoding(m_response.textEncodingName(), false);
                m_firstData = false;
            }
//         } else {
//             m_pluginView->didFail(error);
//             m_pluginView = 0;
//             m_hasSentResponseToPlugin = false;
//         }
    }

    void KFrameLoaderClient::committedLoad(WebCore::DocumentLoader* loader, const char* data, int length)
    {
//         if (!m_pluginView) {
            if (!m_frame)
                return;
            loader->commitData(data, length);
//         }
// 
//         // We re-check here as the plugin can have been created
//         if (m_pluginView) {
//             if (!m_hasSentResponseToPlugin) {
//                 m_pluginView->didReceiveResponse(loader->response());
//                 // didReceiveResponse sets up a new stream to the plug-in. on a full-page plug-in, a failure in
//                 // setting up this stream can cause the main document load to be cancelled, setting m_pluginView
//                 // to null
//                 if (!m_pluginView)
//                     return;
//                 m_hasSentResponseToPlugin = true;
//             }
//             m_pluginView->didReceiveData(data, length);
//        }
    }

    WebCore::ResourceError KFrameLoaderClient::cancelledError(const WebCore::ResourceRequest& request)
    {
        return ResourceError("Error", -999, request.url().prettyURL(), 
            "Request blocked");
    }

    // copied from WebKit/Misc/WebKitErrors[Private].h
    enum {
        WebKitErrorCannotShowMIMEType =                             100,
        WebKitErrorCannotShowURL =                                  101,
        WebKitErrorFrameLoadInterruptedByPolicyChange =             102,
        WebKitErrorCannotUseRestrictedPort = 103,
        WebKitErrorCannotFindPlugIn =                               200,
        WebKitErrorCannotLoadPlugIn =                               201,
        WebKitErrorJavaUnavailable =                                202,
    };

    WebCore::ResourceError KFrameLoaderClient::blockedError(const WebCore::ResourceRequest& request)
    {
        return ResourceError("Error", WebKitErrorCannotUseRestrictedPort, request.url().prettyURL(),
            "Request blocked");
    }


    WebCore::ResourceError KFrameLoaderClient::cannotShowURLError(const WebCore::ResourceRequest& request)
    {
        return ResourceError("Error", WebKitErrorCannotShowURL, request.url().string(),
            "Cannot show URL");
    }

    WebCore::ResourceError KFrameLoaderClient::interruptForPolicyChangeError(const WebCore::ResourceRequest& request)
    {
        return ResourceError("Error", WebKitErrorFrameLoadInterruptedByPolicyChange, request.url().string(),
            "Frame load interruped by policy change");
    }

    WebCore::ResourceError KFrameLoaderClient::cannotShowMIMETypeError(const WebCore::ResourceResponse& response)
    {
        return ResourceError("Error", WebKitErrorCannotShowMIMEType, response.url().string(),
            "Cannot show mimetype");
    }

    WebCore::ResourceError KFrameLoaderClient::fileDoesNotExistError(const WebCore::ResourceResponse& response)
    {
        return ResourceError("Error", -998 /* ### */, response.url().string(),
            "File does not exist");
    }

    WebCore::ResourceError KFrameLoaderClient::pluginWillHandleLoadError(const WebCore::ResourceResponse& response)
    {
        notImplemented();
        return ResourceError();
    }

    bool KFrameLoaderClient::shouldFallBack(const WebCore::ResourceError&)
    {
        notImplemented();
        return false;
    }

    WTF::PassRefPtr<WebCore::DocumentLoader> KFrameLoaderClient::createDocumentLoader(const WebCore::ResourceRequest& request, const SubstituteData& substituteData)
    {
        RefPtr<DocumentLoader> loader = DocumentLoader::create(request, substituteData);
        if (substituteData.isValid())
            loader->setDeferMainResourceDataLoad(false);
        return loader.release();
    }

    void KFrameLoaderClient::download(WebCore::ResourceHandle* handle, const WebCore::ResourceRequest&, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&)
    {
#if QT_VERSION >= 0x040400
//         if (!m_webPage)
//             return;
// 
//         QNetworkReplyHandler* handler = handle->getInternal()->m_job;
//         QNetworkReply* reply = handler->release();
//         if (reply) {
//             QWebPage *page = m_webPage->page();
//             if (page->forwardUnsupportedContent())
//                 emit m_webPage->page()->unsupportedContent(reply);
//             else
//                 reply->abort();
//         }
#endif
    }

    void KFrameLoaderClient::assignIdentifierToInitialRequest(unsigned long identifier, WebCore::DocumentLoader* loader, const WebCore::ResourceRequest& request)
    {
//         if (dumpResourceLoadCallbacks)
//             dumpAssignedUrls[identifier] = drtDescriptionSuitableForTestResult(request.url());
    }

    void KFrameLoaderClient::dispatchWillSendRequest(WebCore::DocumentLoader*, unsigned long identifier, WebCore::ResourceRequest& newRequest, const WebCore::ResourceResponse& redirectResponse)
    {
//         if (dumpResourceLoadCallbacks)
//             printf("%s - willSendRequest %s redirectResponse %s\n",
//             qPrintable(dumpAssignedUrls[identifier]),
//             qPrintable(drtDescriptionSuitableForTestResult(newRequest)),
//             qPrintable(drtDescriptionSuitableForTestResult(redirectResponse)));

        // seems like the Mac code doesn't do anything here by default neither
        //qDebug() << "KFrameLoaderClient::dispatchWillSendRequest" << request.isNull() << request.url().string`();
    }

    bool
        KFrameLoaderClient::shouldUseCredentialStorage(DocumentLoader*, unsigned long)
    {
        notImplemented();
        return false;
    }

    void KFrameLoaderClient::dispatchDidReceiveAuthenticationChallenge(DocumentLoader*, unsigned long, const AuthenticationChallenge&)
    {
        notImplemented();
    }

    void KFrameLoaderClient::dispatchDidCancelAuthenticationChallenge(DocumentLoader*, unsigned long, const AuthenticationChallenge&)
    {
        notImplemented();
    }

    void KFrameLoaderClient::dispatchDidReceiveResponse(WebCore::DocumentLoader*, unsigned long, const WebCore::ResourceResponse& response)
    {

        m_response = response;
        m_firstData = true;
        //qDebug() << "    got response from" << response.url().string();
    }

    void KFrameLoaderClient::dispatchDidReceiveContentLength(WebCore::DocumentLoader*, unsigned long, int)
    {
    }

    void KFrameLoaderClient::dispatchDidFinishLoading(WebCore::DocumentLoader* loader, unsigned long)
    {
    }

    void KFrameLoaderClient::dispatchDidFailLoading(WebCore::DocumentLoader* loader, unsigned long identifier, const WebCore::ResourceError& error)
    {
//         if (dumpResourceLoadCallbacks)
//             printf("%s - didFailLoadingWithError: %s\n", qPrintable(dumpAssignedUrls[identifier]), qPrintable(drtDescriptionSuitableForTestResult(error)));
// 
        if (m_firstData) {
            FrameLoader *fl = loader->frameLoader();
            loader->writer()->setEncoding(m_response.textEncodingName(), false);
            m_firstData = false;
        }
    }

    bool KFrameLoaderClient::dispatchDidLoadResourceFromMemoryCache(WebCore::DocumentLoader*, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&, int)
    {
        notImplemented();
        return false;
    }

    void KFrameLoaderClient::dispatchDidFailProvisionalLoad(const WebCore::ResourceError&)
    {
//         if (dumpFrameLoaderCallbacks)
//             printf("%s - didFailProvisionalLoadWithError\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));

        m_loadSucceeded = false;
    }

    void KFrameLoaderClient::dispatchDidFailLoad(const WebCore::ResourceError&)
    {
//         if (dumpFrameLoaderCallbacks)
//             printf("%s - didFailLoadWithError\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));

        m_loadSucceeded = false;
    }

    WebCore::Frame* KFrameLoaderClient::dispatchCreatePage(const WebCore::NavigationAction&)
    {
//         if (!m_webPage)
//             return 0;
//         QWebPage *newPage = m_webPage->page()->createWindow(QWebPage::WebBrowserWindow);
//         if (!newPage)
//             return 0;
//         return newPage->mainFrame()->d->frame;
        return 0;
    }

    void KFrameLoaderClient::callPolicyFunction(FramePolicyFunction function, PolicyAction action)
    {
        //ASSERT(!m_policyFunction);
        ASSERT(function);

        m_policyFunction = function;
        //emit sigCallPolicyFunction(action);
        (m_frame->loader()->policyChecker()->*function)(action);
    }

    void KFrameLoaderClient::slotCallPolicyFunction(int action)
    {
        if (!m_frame || !m_policyFunction)
            return;
        FramePolicyFunction function = m_policyFunction;
        m_policyFunction = 0;
        __asm int 3;
        //(m_frame->loader()->*function)(WebCore::PolicyAction(action));
    }

    void KFrameLoaderClient::dispatchDecidePolicyForMIMEType(FramePolicyFunction function, const String& MIMEType, const WebCore::ResourceRequest&)
    {
        // we need to call directly here
        //ASSERT(!m_policyFunction);
        m_policyFunction = function;
        if (canShowMIMEType(MIMEType))
            callPolicyFunction(function, PolicyUse);
        else
            callPolicyFunction(function, PolicyDownload);
    }

    void KFrameLoaderClient::dispatchDecidePolicyForNewWindowAction(FramePolicyFunction function, const WebCore::NavigationAction& action, const WebCore::ResourceRequest& request, PassRefPtr<WebCore::FormState>, const String&)
    {
         //ASSERT(!m_policyFunction);
         ASSERT(m_webPage);
         m_policyFunction = function;
// #if QT_VERSION < 0x040400
//         QWebNetworkRequest r(request);
// #else
//         QNetworkRequest r(request.toNetworkRequest());
// #endif
//         QWebPage* page = m_webPage->page();
// 
//         if (!page->d->acceptNavigationRequest(0, r, QWebPage::NavigationType(action.type()))) {
//             if (action.type() == NavigationTypeFormSubmitted || action.type() == NavigationTypeFormResubmitted)
//                 m_frame->loader()->resetMultipleFormSubmissionProtection();
// 
//             if (action.type() == NavigationTypeLinkClicked && r.url().hasFragment()) {
//                 ResourceRequest emptyRequest;
//                 m_frame->loader()->activeDocumentLoader()->setLastCheckedRequest(emptyRequest);
//             }
// 
//             callPolicyFunction(PolicyIgnore);
//             return;
//         }
         callPolicyFunction(function, PolicyUse);
    }

    void KFrameLoaderClient::dispatchDecidePolicyForNavigationAction(FramePolicyFunction function, const WebCore::NavigationAction& action, const WebCore::ResourceRequest& request, PassRefPtr<WebCore::FormState>)
    {
         //ASSERT(!m_policyFunction);
         ASSERT(m_webPage);
         m_policyFunction = function;
// #if QT_VERSION < 0x040400
//         QWebNetworkRequest r(request);
// #else
//         QNetworkRequest r(request.toNetworkRequest());
// #endif
//         QWebPage*page = m_webPage->page();
// 
//         if (!page->d->acceptNavigationRequest(m_webPage, r, QWebPage::NavigationType(action.type()))) {
//             if (action.type() == NavigationTypeFormSubmitted || action.type() == NavigationTypeFormResubmitted)
//                 m_frame->loader()->resetMultipleFormSubmissionProtection();
// 
//             if (action.type() == NavigationTypeLinkClicked && r.url().hasFragment()) {
//                 ResourceRequest emptyRequest;
//                 m_frame->loader()->activeDocumentLoader()->setLastCheckedRequest(emptyRequest);
//             }
// 
//             callPolicyFunction(PolicyIgnore);
//             return;
//         }
        callPolicyFunction(function, PolicyUse);

        //(m_frame->loader()->*function)(WebCore::PolicyAction(PolicyUse));
        //m_frame->loader()->continueAfterNavigationPolicy(WebCore::PolicyAction(PolicyUse));
    }

    void KFrameLoaderClient::dispatchUnableToImplementPolicy(const WebCore::ResourceError&)
    {
        notImplemented();
    }

    void KFrameLoaderClient::startDownload(const WebCore::ResourceRequest& request)
    {
// #if QT_VERSION >= 0x040400
//         if (!m_webPage)
//             return;
// 
//         QWebPage *page = m_webPage->page();
//         emit m_webPage->page()->downloadRequested(request.toNetworkRequest());
// #endif
    }

    PassRefPtr<Frame> KFrameLoaderClient::createFrame(const KURL& url, const String& name, HTMLFrameOwnerElement* ownerElement,
        const String& referrer, bool allowsScrolling, int marginWidth, int marginHeight)
    {
//         if (!m_webPage)
//             return 0;
// 
//         QWebFrameData frameData;
//         frameData.url = url;
//         frameData.name = name;
//         frameData.ownerElement = ownerElement;
//         frameData.referrer = referrer;
//         frameData.allowsScrolling = allowsScrolling;
//         frameData.marginWidth = marginWidth;
//         frameData.marginHeight = marginHeight;
// 
//         QWebFrame* webFrame = new QWebFrame(m_webPage, &frameData);
//         emit m_webPage->page()->frameCreated(webFrame);
// 
//         RefPtr<Frame> childFrame = adoptRef(webFrame->d->frame);
// 
//         // ### set override encoding if we have one
// 
//         FrameLoadType loadType = m_frame->loader()->loadType();
//         FrameLoadType childLoadType = FrameLoadTypeRedirectWithLockedHistory;
// 
//         childFrame->loader()->loadURL(frameData.url, frameData.referrer, String(), childLoadType, 0, 0);
// 
//         // The frame's onload handler may have removed it from the document.
//         if (!childFrame->tree()->parent())
//             return 0;
// 
//         return childFrame.release();
        return 0;
    }

    PassRefPtr<Widget> KFrameLoaderClient::createPlugin(const IntSize&, HTMLPlugInElement*, const KURL&, const Vector<String>&, const Vector<String>&, const String&, bool)
    {
        return 0;
    }

    void KFrameLoaderClient::redirectDataToPlugin(Widget* pluginWidget)
    {

    }

    PassRefPtr<Widget> KFrameLoaderClient::createJavaAppletWidget(
        const IntSize&, HTMLAppletElement*, const KURL& baseURL, const Vector<String>& paramNames, 
        const Vector<String>& paramValues)
    {
        notImplemented();
        return 0;
    }

    ObjectContentType KFrameLoaderClient::objectContentType(const KURL&, const String& mimeTypeIn, bool shouldPreferPlugInsForImages)
    {
        //    qDebug()<<" ++++++++++++++++ url is "<<url.prettyURL()<<", mime = "<<_mimeType;
//         if (_mimeType == "application/x-qt-plugin" || _mimeType == "application/x-qt-styled-widget")
//             return ObjectContentOtherPlugin;
// 
//         if (url.isEmpty() && !_mimeType.length())
//             return ObjectContentNone;
// 
//         String mimeType = _mimeType;
//         if (!mimeType.length()) {
//             QFileInfo fi(url.path());
//             mimeType = MIMETypeRegistry::getMIMETypeForExtension(fi.suffix());
//         }
// 
//         if (!mimeType.length())
//             return ObjectContentFrame;
// 
//         if (MIMETypeRegistry::isSupportedImageMIMEType(mimeType))
//             return ObjectContentImage;
// 
//         if (PluginDatabase::installedPlugins()->isMIMETypeRegistered(mimeType))
//             return ObjectContentNetscapePlugin;
// 
//         if (m_frame->page() && m_frame->page()->pluginData() && m_frame->page()->pluginData()->supportsMimeType(mimeType))
//             return ObjectContentOtherPlugin;
// 
//         if (MIMETypeRegistry::isSupportedNonImageMIMEType(mimeType))
//             return ObjectContentFrame;
// 
//         if (url.protocol() == "about")
//             return ObjectContentFrame;

        return ObjectContentNone;
    }

//     static const CSSPropertyID qstyleSheetProperties[] = {
//         CSSPropertyColor,
//         CSSPropertyFontFamily,
//         CSSPropertyFontSize,
//         CSSPropertyFontStyle,
//         CSSPropertyFontWeight
//     };

//    const unsigned numqStyleSheetProperties = sizeof(qstyleSheetProperties) / sizeof(qstyleSheetProperties[0]);

//     class QtPluginWidget: public Widget
//     {
//     public:
//         QtPluginWidget(QWidget* w = 0): Widget(w) {}
//         ~QtPluginWidget()
//         {
//             if (platformWidget())
//                 platformWidget()->deleteLater();
//         }
//         virtual void invalidateRect(const IntRect& r)
//         { 
//             if (platformWidget())
//                 platformWidget()->update(r);
//         }
//         virtual void frameRectsChanged()
//         {
//             if (!platformWidget())
//                 return;
// 
//             IntRect windowRect = convertToContainingWindow(IntRect(0, 0, frameRect().width(), frameRect().height()));
//             platformWidget()->setGeometry(windowRect);
// 
//             ScrollView* parentScrollView = parent();
//             if (!parentScrollView)
//                 return;
// 
//             ASSERT(parentScrollView->isFrameView());
//             IntRect clipRect(static_cast<FrameView*>(parentScrollView)->windowClipRect());
//             clipRect.move(-windowRect.x(), -windowRect.y());
//             clipRect.intersect(platformWidget()->rect());
// 
//             QRegion clipRegion = QRegion(clipRect);
//             platformWidget()->setMask(clipRegion);
// 
//             // if setMask is set with an empty QRegion, no clipping will
//             // be performed, so in that case we hide the platformWidget
//             platformWidget()->setVisible(!clipRegion.isEmpty());
//         }
//     };

//     Widget* KFrameLoaderClient::createPlugin(const IntSize& pluginSize, Element* element, const KURL& url, const Vector<String>& paramNames,
//         const Vector<String>& paramValues, const String& mimeType, bool loadManually)
//     {
//         //     qDebug()<<"------ Creating plugin in KFrameLoaderClient::createPlugin for "<<url.prettyURL() << mimeType;
//         //     qDebug()<<"------\t url = "<<url.prettyURL();
// 
//         if (!m_webPage)
//             return 0;
// 
//         QStringList params;
//         QStringList values;
//         QString classid(element->getAttribute("classid"));
// 
//         for (int i = 0; i < paramNames.size(); ++i) {
//             params.append(paramNames[i]);
//             if (paramNames[i] == "classid")
//                 classid = paramValues[i];
//         }
//         for (int i = 0; i < paramValues.size(); ++i)
//             values.append(paramValues[i]);
// 
//         QString urlStr(url.string());
//         QUrl qurl = urlStr;
// 
//         QObject* object = 0;
// 
//         if (mimeType == "application/x-qt-plugin" || mimeType == "application/x-qt-styled-widget") {
//             object = m_webPage->page()->createPlugin(classid, qurl, params, values);
// #ifndef QT_NO_STYLE_STYLESHEET
//             QWidget* widget = qobject_cast<QWidget*>(object);
//             if (widget && mimeType == "application/x-qt-styled-widget") {
// 
//                 QString styleSheet = element->getAttribute("style");
//                 if (!styleSheet.isEmpty())
//                     styleSheet += QLatin1Char(';');
// 
//                 for (int i = 0; i < numqStyleSheetProperties; ++i) {
//                     CSSPropertyID property = qstyleSheetProperties[i];
// 
//                     styleSheet += QString::fromLatin1(::getPropertyName(property));
//                     styleSheet += QLatin1Char(':');
//                     styleSheet += computedStyle(element)->getPropertyValue(property);
//                     styleSheet += QLatin1Char(';');
//                 }
// 
//                 widget->setStyleSheet(styleSheet);
//             }
// #endif // QT_NO_STYLE_STYLESHEET
//         }
// 
// #if QT_VERSION >= 0x040400
//         if (!object) {
//             QWebPluginFactory* factory = m_webPage->page()->pluginFactory();
//             if (factory)
//                 object = factory->create(mimeType, qurl, params, values);
//         }
// #endif
// 
//         if (object) {
//             QWidget* widget = qobject_cast<QWidget*>(object);
//             if (widget) {
//                 QWidget* view = m_webPage->page()->view();
//                 if (view)
//                     widget->setParent(view);
//                 QtPluginWidget* w = new QtPluginWidget();
//                 w->setPlatformWidget(widget);
//                 // Make sure it's invisible until properly placed into the layout
//                 w->setFrameRect(IntRect(0, 0, 0, 0));
//                 return w;
//             }
//             // FIXME: make things work for widgetless plugins as well
//             delete object;
//         } else { // NPAPI Plugins
//             PluginView* pluginView = PluginView::create(m_frame, pluginSize, element, url,
//                 paramNames, paramValues, mimeType, loadManually);
//             return pluginView;
//         }
// 
//         return 0;
//     }

//     void KFrameLoaderClient::redirectDataToPlugin(Widget* pluginWidget)
//     {
//         ASSERT(!m_pluginView);
//         m_pluginView = static_cast<PluginView*>(pluginWidget);
//         m_hasSentResponseToPlugin = false;
//     }

//     Widget* KFrameLoaderClient::createJavaAppletWidget(const IntSize&, Element*, const KURL& baseURL,
//         const Vector<String>& paramNames, const Vector<String>& paramValues)
//     {
//         notImplemented();
//         return 0;
//     }

    String KFrameLoaderClient::overrideMediaType() const
    {
        return String();
    }

//     QString KFrameLoaderClient::chooseFile(const QString& oldFile)
//     {
//         return webFrame()->page()->chooseFile(webFrame(), oldFile);
//     }

    void KFrameLoaderClient::dispatchDidPushStateWithinPage()
    {
        notImplemented();
    }

    void KFrameLoaderClient::dispatchDidReplaceStateWithinPage()
    {
        notImplemented();
    }

    void KFrameLoaderClient::dispatchDidPopStateWithinPage()
    {
        notImplemented();
    }

    void KFrameLoaderClient::dispatchDidChangeIcons(WebCore::IconType)
    {
        if (!m_webPage)
            return;

        // FIXME: In order to get notified of icon URLS' changes, add a notification.
        // emit iconsChanged();
    }

    void KFrameLoaderClient::dispatchDecidePolicyForResponse(FramePolicyFunction function, const WebCore::ResourceResponse& response, const WebCore::ResourceRequest&)
    {
        // We need to call directly here.
        switch (response.httpStatusCode()) {
            case HTTPResetContent:
        // FIXME: a 205 response requires that the requester reset the document view.
        // Fallthrough
            case HTTPNoContent:
                callPolicyFunction(function, PolicyIgnore);
                return;
        }

        if (WebCore::contentDispositionType(response.httpHeaderField("Content-Disposition")) == WebCore::ContentDispositionAttachment)
            callPolicyFunction(function, PolicyDownload);
        else if (canShowMIMEType(response.mimeType()))
            callPolicyFunction(function, PolicyUse);
        else
            callPolicyFunction(function, PolicyDownload);
    }

    void KFrameLoaderClient::updateGlobalHistoryRedirectLinks()
    {
        ;
    }

    bool KFrameLoaderClient::shouldStopLoadingForHistoryItem(WebCore::HistoryItem*) const
    {
        return true;
    }

    void KFrameLoaderClient::dispatchDidAddBackForwardItem(WebCore::HistoryItem*) const
    {
    }

    void KFrameLoaderClient::dispatchDidRemoveBackForwardItem(WebCore::HistoryItem*) const
    {
    }

    void KFrameLoaderClient::dispatchDidChangeBackForwardIndex() const
    {
    }

    void KFrameLoaderClient::didDisplayInsecureContent()
    {
        notImplemented();
    }

    void KFrameLoaderClient::didRunInsecureContent(WebCore::SecurityOrigin*, const KURL&)
    {
        notImplemented();
    }

    bool KFrameLoaderClient::canShowMIMETypeAsHTML(const String& MIMEType) const
    {
        notImplemented();
        return false;
    }

    void KFrameLoaderClient::savePlatformDataToCachedFrame(CachedFrame*) 
    {
        notImplemented();
    }

    void KFrameLoaderClient::transitionToCommittedFromCachedFrame(CachedFrame*)
    {
    }

    void KFrameLoaderClient::didSaveToPageCache()
    {
    }

    void KFrameLoaderClient::didRestoreFromPageCache()
    {
    }

    void KFrameLoaderClient::dispatchDidBecomeFrameset(bool)
    {
    }

    void KFrameLoaderClient::didTransferChildFrameToNewDocument(Page*)
    {
        __asm int 3;
    }

    void KFrameLoaderClient::transferLoadingResourceFromPage(unsigned long, DocumentLoader*, const ResourceRequest&, Page*)
    {
    }

    void KFrameLoaderClient::dispatchDidClearWindowObjectInWorld(DOMWrapperWorld* world)
    {
        if (world != mainThreadNormalWorld())
            return;

        // TODO woelar
//        __asm int 3;
//         if (m_webPage)
//             emit m_webPage->javaScriptWindowObjectCleared();
    }

    void KFrameLoaderClient::documentElementAvailable()
    {
        return;
    }

    PassRefPtr<FrameNetworkingContext> KFrameLoaderClient::createNetworkingContext()
    {      
//         QVariant value = m_webPage->page()->property("_q_MIMESniffingDisabled");
//         bool MIMESniffingDisabled = value.isValid() && value.toBool();
// 
         return KFrameNetworkingContext::create(m_frame, m_webPage, false);
     }
}