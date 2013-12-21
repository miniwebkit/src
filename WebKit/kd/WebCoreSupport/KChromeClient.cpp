/*
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "config.h"
#include "KChromeClient.h"

#include "FileChooser.h"
#include "Frame.h"
#include "FrameLoadRequest.h"
#include "FrameLoader.h"
#include "KFrameLoaderClient.h"
#include "FrameView.h"
#include "HitTestResult.h"
#include "NotImplemented.h"
#include "WindowFeatures.h"
//#include "DatabaseTracker.h"
#include "SecurityOrigin.h"

#include "KWebPage.h"

//#include "qwebpage.h"
//#include "qwebpage_p.h"
//#include "qwebframe_p.h"
//#include "qwebsecurityorigin.h"
//#include "qwebsecurityorigin_p.h"

//#include <qtooltip.h>
//#include <qtextdocument.h>

namespace WebCore
{


KChromeClient::KChromeClient(KWebPage* webPage)
    : m_webPage(webPage)
{
    toolBarsVisible = statusBarVisible = menuBarVisible = true;
}

KChromeClient::~KChromeClient()
{

}

void KChromeClient::setWindowRect(const FloatRect& rect)
{
//     if (!m_webPage)
//         return;
//     emit m_webPage->geometryChangeRequested(QRect(qRound(rect.x()), qRound(rect.y()),
//                             qRound(rect.width()), qRound(rect.height())));
}


FloatRect KChromeClient::windowRect()
{
    if (!m_webPage)
        return FloatRect();
    return IntRect(0, 0, m_webPage->viewportSize().width(), m_webPage->viewportSize().height());
}


FloatRect KChromeClient::pageRect()
{
    if (!m_webPage)
        return FloatRect();
    //return FloatRect(QRectF(QPointF(0,0), m_webPage->viewportSize()));
    return FloatRect(0, 0, m_webPage->viewportSize().width(), m_webPage->viewportSize().height());
}


float KChromeClient::scaleFactor()
{
    notImplemented();
    return 1;
}


void KChromeClient::focus()
{
    if (!m_webPage)
        return;
//     QWidget* view = m_webPage->view();
//     if (!view)
//         return;
// 
//     view->setFocus();
    ::SetFocus(m_webPage->getHWND());
}


void KChromeClient::unfocus()
{
//     if (!m_webPage)
//         return;
//     QWidget* view = m_webPage->view();
//     if (!view)
//         return;
//     view->clearFocus();
}

bool KChromeClient::canTakeFocus(FocusDirection)
{
    // This is called when cycling through links/focusable objects and we
    // reach the last focusable object. Then we want to claim that we can
    // take the focus to avoid wrapping.
    return true;
}

void KChromeClient::takeFocus(FocusDirection)
{
    // don't do anything. This is only called when cycling to links/focusable objects,
    // which in turn is called from focusNextPrevChild. We let focusNextPrevChild
    // call QWidget::focusNextPrevChild accordingly, so there is no need to do anything
    // here.
}

Page* KChromeClient::createWindow(Frame*, const FrameLoadRequest&, const WindowFeatures&, const NavigationAction&)
{
//     QWebPage *newPage = m_webPage->createWindow(features.dialog ? QWebPage::WebModalDialog : QWebPage::WebBrowserWindow);
//     if (!newPage)
//         return 0;
//     newPage->mainFrame()->load(request.resourceRequest().url());
//     return newPage->d->page;
    return 0;
}

void KChromeClient::show()
{
    if (!m_webPage)
        return;
//     QWidget* view = m_webPage->view();
//     if (!view)
//         return;
//     view->topLevelWidget()->show();
    
    ::ShowWindow(m_webPage->getHWND(), SW_SHOWNORMAL);
}


bool KChromeClient::canRunModal()
{
    notImplemented();
    return false;
}


void KChromeClient::runModal()
{
    notImplemented();
}


void KChromeClient::setToolbarsVisible(bool visible)
{
    toolBarsVisible = visible;
    //emit m_webPage->toolBarVisibilityChangeRequested(visible);
}


bool KChromeClient::toolbarsVisible()
{
    return toolBarsVisible;
}


void KChromeClient::setStatusbarVisible(bool visible)
{
    //emit m_webPage->statusBarVisibilityChangeRequested(visible);
    statusBarVisible = visible;
}


bool KChromeClient::statusbarVisible()
{
    return statusBarVisible;
    return false;
}


void KChromeClient::setScrollbarsVisible(bool)
{
    notImplemented();
}


bool KChromeClient::scrollbarsVisible()
{
    notImplemented();
    return true;
}


void KChromeClient::setMenubarVisible(bool visible)
{
    menuBarVisible = visible;
    //emit m_webPage->menuBarVisibilityChangeRequested(visible);
}

bool KChromeClient::menubarVisible()
{
    return menuBarVisible;
}

void KChromeClient::setResizable(bool)
{
    notImplemented();
}

void KChromeClient::addMessageToConsole(
    MessageSource, MessageType, MessageLevel, const String& message, unsigned int lineNumber, const String& sourceID)
{
    String x = message;
    String y = sourceID;
    //m_webPage->javaScriptConsoleMessage(x, lineNumber, y);
}

void KChromeClient::chromeDestroyed()
{
    delete this;
}

bool KChromeClient::canRunBeforeUnloadConfirmPanel()
{
    return true;
}

bool KChromeClient::runBeforeUnloadConfirmPanel(const String& message, Frame* frame)
{
    //return runJavaScriptConfirm(frame, message);
    return false;
}

void KChromeClient::closeWindowSoon()
{
    //m_webPage->mainFrame()->d->frame->loader()->stopAllLoaders();
    //emit m_webPage->windowCloseRequested();
}

void KChromeClient::runJavaScriptAlert(Frame* f, const String& msg)
{
    String msgTemp = msg;
    KFrameLoaderClient *fl = static_cast<KFrameLoaderClient*>(f->loader()->client());
    //m_webPage->javaScriptAlert(fl->webFrame(), x);
    MessageBoxW(NULL, (LPCWSTR)msgTemp.charactersWithNullTermination(), L"KdGui", 0);
}

bool KChromeClient::runJavaScriptConfirm(Frame* f, const String& msg)
{
    String msgTemp = msg;
    KFrameLoaderClient *fl = static_cast<KFrameLoaderClient*>(f->loader()->client());
    //return m_webPage->javaScriptConfirm(fl->webFrame(), x);
    return MessageBoxW(NULL, (LPCWSTR)msgTemp.charactersWithNullTermination(), L"KdGui", MB_YESNO);
}

bool KChromeClient::runJavaScriptPrompt(Frame* f, const String& message, const String& defaultValue, String& result)
{
//     QString x = result;
//     FrameLoaderClientQt *fl = static_cast<FrameLoaderClientQt*>(f->loader()->client());
//     bool rc = m_webPage->javaScriptPrompt(fl->webFrame(), (QString)message, (QString)defaultValue, &x);
//     result = x;
//     return rc;
    return false;
}

void KChromeClient::setStatusbarText(const String& msg)
{
    //QString x = msg;
    //emit m_webPage->statusBarMessage(x);
}

bool KChromeClient::shouldInterruptJavaScript()
{
    notImplemented();
    return false;
}

bool KChromeClient::tabsToLinks() const
{
    //return m_webPage->settings()->testAttribute(QWebSettings::LinksIncludedInFocusChain);
    return false;
}

IntRect KChromeClient::windowResizerRect() const
{
    return IntRect();
}

void KChromeClient::repaint(const IntRect& windowRect, bool contentChanged, bool immediate, bool repaintContentOnly)
{
    // No double buffer, so only update the QWidget if content changed.
//     if (contentChanged) {
//         QWidget* view = m_webPage->view();
//         if (view) {
//             QRect rect(windowRect);
//             rect = rect.intersected(QRect(QPoint(0, 0), m_webPage->viewportSize()));
//             if (!rect.isEmpty())
//                 view->update(rect);
//         }
//         emit m_webPage->repaintRequested(windowRect);
//     }
    if (m_webPage)
    {
        /*emit*/ m_webPage->repaintRequested(windowRect);
    }

    // FIXME: There is no "immediate" support for window painting.  This should be done always whenever the flag
    // is set.
}

void KChromeClient::scroll(const IntSize& delta, const IntRect& scrollViewRect, const IntRect&)
{
//     QWidget* view = m_webPage->view();
//     if (view)
//         view->scroll(delta.width(), delta.height(), scrollViewRect);
//     emit m_webPage->scrollRequested(delta.width(), delta.height(), scrollViewRect);
}

IntRect KChromeClient::windowToScreen(const IntRect& rect) const
{
    notImplemented();
    return rect;
}

IntPoint KChromeClient::screenToWindow(const IntPoint& point) const
{
    notImplemented();
    return point;
}

PlatformWidget KChromeClient::platformWindow() const
{
    //return m_webPage->view();
    return NULL;
}

void KChromeClient::contentsSizeChanged(Frame*, const IntSize&) const
{
}

void KChromeClient::mouseDidMoveOverElement(const HitTestResult& result, unsigned modifierFlags)
{
//     if (result.absoluteLinkURL() != lastHoverURL
//         || result.title() != lastHoverTitle
//         || result.textContent() != lastHoverContent) {
//         lastHoverURL = result.absoluteLinkURL();
//         lastHoverTitle = result.title();
//         lastHoverContent = result.textContent();
//         emit m_webPage->linkHovered(lastHoverURL.prettyURL(),
//                 lastHoverTitle, lastHoverContent);
//     }
}

void KChromeClient::setToolTip(const String &tip, TextDirection)
{
// #ifndef QT_NO_TOOLTIP
//     QWidget* view = m_webPage->view();
//     if (!view)
//         return;
// 
//     if (tip.isEmpty()) {
//         view->setToolTip(QString());
//         QToolTip::hideText();
//     } else {
//         QString dtip = QLatin1String("<p>") + Qt::escape(tip) + QLatin1String("</p>");
//         view->setToolTip(dtip);
//     }
// #else
//     Q_UNUSED(tip);
// #endif
}

void KChromeClient::print(Frame *frame)
{
    //emit m_webPage->printRequested(QWebFramePrivate::kit(frame));
}

void KChromeClient::exceededDatabaseQuota(Frame* frame, const String& databaseName)
{
//     quint64 quota = QWebSettings::offlineStorageDefaultQuota();
// #if ENABLE(DATABASE)
//     if (!DatabaseTracker::tracker().hasEntryForOrigin(frame->document()->securityOrigin()))
//         DatabaseTracker::tracker().setQuota(frame->document()->securityOrigin(), quota);
// #endif
//     emit m_webPage->databaseQuotaExceeded(QWebFramePrivate::kit(frame), databaseName);
}

void KChromeClient::runOpenPanel(Frame* frame, PassRefPtr<FileChooser> prpFileChooser)
{
//     RefPtr<FileChooser> fileChooser = prpFileChooser;
//     bool supportMulti = m_webPage->supportsExtension(QWebPage::ChooseMultipleFilesExtension);
// 
//     if (fileChooser->allowsMultipleFiles() && supportMulti) {
//         QWebPage::ChooseMultipleFilesExtensionOption option;
//         option.parentFrame = QWebFramePrivate::kit(frame);
// 
//         if (!fileChooser->filenames().isEmpty())
//             for (int i = 0; i < fileChooser->filenames().size(); ++i)
//                 option.suggestedFileNames += fileChooser->filenames()[i];
// 
//         QWebPage::ChooseMultipleFilesExtensionReturn output;
//         m_webPage->extension(QWebPage::ChooseMultipleFilesExtension, &option, &output);
// 
//         if (!output.fileNames.isEmpty()) {
//             Vector<String> names;
//             for (int i = 0; i < output.fileNames.count(); ++i)
//                 names.append(output.fileNames.at(i));
//             fileChooser->chooseFiles(names);
//         }
//     } else {
//         QString suggestedFile;
//         if (!fileChooser->filenames().isEmpty())
//             suggestedFile = fileChooser->filenames()[0];
//         QString file = m_webPage->chooseFile(QWebFramePrivate::kit(frame), suggestedFile);
//         if (!file.isEmpty())
//             fileChooser->chooseFile(file);
//     }
}

void KChromeClient::focusedNodeChanged(Node*)
{
}

void KChromeClient::focusedFrameChanged(Frame*)
{
}

KeyboardUIMode KChromeClient::keyboardUIMode()
{
    return KeyboardAccessDefault;
}

void KChromeClient::invalidateWindow(const IntRect& windowRect, bool)
{
    __asm int 3;
}

void KChromeClient::invalidateContentsAndWindow(const IntRect& windowRect, bool immediate)
{
    // TODO Weolar
    //__asm int 3;
    m_webPage->repaintRequested(windowRect);
}

void KChromeClient::invalidateContentsForSlowScroll(const IntRect& windowRect, bool immediate)
{
    invalidateContentsAndWindow(windowRect, immediate);
}

PlatformPageClient KChromeClient::platformPageClient() const
{
    return m_webPage->getHWND();
}

void KChromeClient::setCursor(const Cursor& cursor)
{
    //__asm int 3; TODO_weolar
    return ;
}

void KChromeClient::attachRootGraphicsLayer(Frame* frame, GraphicsLayer* graphicsLayer)
{
    __asm int 3;
    return ;
}

void KChromeClient::chooseIconForFiles(const Vector<String>& filenames, FileChooser* chooser)
{
    __asm int 3;
}

void KChromeClient::setNeedsOneShotDrawingSynchronization()
{
    // we want the layers to synchronize next time we update the screen anyway
    __asm int 3;
}

void KChromeClient::scheduleCompositingLayerSync()
{
    // we want the layers to synchronize ASAP
    __asm int 3;
}

bool KChromeClient::selectItemWritingDirectionIsNatural()
{
    return false;
}

bool KChromeClient::selectItemAlignmentFollowsMenuWritingDirection()
{
    return false;
}

PassRefPtr<PopupMenu> KChromeClient::createPopupMenu(PopupMenuClient* client) const
{
    __asm int 3;
    return 0;
}

PassRefPtr<SearchPopupMenu> KChromeClient::createSearchPopupMenu(PopupMenuClient* client) const
{
    __asm int 3;
    return 0;
}

void KChromeClient::setLastSetCursorToCurrentCursor()
{
    __asm int 3;
    return ;
}

void* KChromeClient::getJavascriptCallCppCallback()
{
    return m_webPage->getJavascriptCallCppCallback();
}

}
