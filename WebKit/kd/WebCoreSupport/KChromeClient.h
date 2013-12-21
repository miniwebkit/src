/*
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
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

#ifndef KChromeClient_H
#define KChromeClient_H

#include "ChromeClient.h"
#include "FloatRect.h"
#include "RefCounted.h"
#include "KURL.h"
#include "PlatformString.h"
#include "NotImplemented.h"

namespace WebCore {

    class FileChooser;
    class FloatRect;
    class Page;
    class KWebPage;
    struct FrameLoadRequest;

    class KChromeClient : public ChromeClient
    {
    public:
        KChromeClient(KWebPage* webPage);
        ~KChromeClient();
        virtual void chromeDestroyed();

        virtual void* webView() const { return 0; }
        virtual void setWindowRect(const FloatRect&);
        virtual FloatRect windowRect();

        virtual FloatRect pageRect();

        virtual float scaleFactor();

        virtual void focus();
        virtual void unfocus();

        virtual bool canTakeFocus(FocusDirection);
        virtual void takeFocus(FocusDirection);

        // The Frame pointer provides the ChromeClient with context about which
        // Frame wants to create the new Page.  Also, the newly created window
        // should not be shown to the user until the ChromeClient of the newly
        // created Page has its show method called.
        virtual Page* createWindow(Frame*, const FrameLoadRequest&, const WindowFeatures&, const NavigationAction&);
        virtual void show();

        virtual bool canRunModal();
        virtual void runModal();

        virtual void setToolbarsVisible(bool);
        virtual bool toolbarsVisible();

        virtual void setStatusbarVisible(bool);
        virtual bool statusbarVisible();

        virtual void setScrollbarsVisible(bool);
        virtual bool scrollbarsVisible();

        virtual void setMenubarVisible(bool);
        virtual bool menubarVisible();

        virtual void setResizable(bool);

        virtual void addMessageToConsole(MessageSource, MessageType, MessageLevel, const String& message, unsigned int lineNumber, const String& sourceID);

        virtual bool canRunBeforeUnloadConfirmPanel();
        virtual bool runBeforeUnloadConfirmPanel(const String& message, Frame* frame);

        virtual void closeWindowSoon();

        virtual void runJavaScriptAlert(Frame*, const String&);
        virtual bool runJavaScriptConfirm(Frame*, const String&);
        virtual bool runJavaScriptPrompt(Frame*, const String& message, const String& defaultValue, String& result);
        virtual void setStatusbarText(const String&);
        virtual bool shouldInterruptJavaScript();
        virtual bool tabsToLinks() const;

        virtual IntRect windowResizerRect() const;

        // Methods used by HostWindow.
        virtual void repaint(const IntRect&, bool contentChanged, bool immediate = false, bool repaintContentOnly = false);
        virtual void scroll(const IntSize& scrollDelta, const IntRect& rectToScroll, const IntRect& clipRect);
        virtual IntPoint screenToWindow(const IntPoint&) const;
        virtual IntRect windowToScreen(const IntRect&) const;
        virtual PlatformWidget platformWindow() const;
        virtual void contentsSizeChanged(Frame*, const IntSize&) const;
        virtual void scrollRectIntoView(const IntRect&, const ScrollView*) const {} // Platforms other than Mac can implement this if it ever becomes necessary for them to do so.
        // End methods used by HostWindow.

        virtual void mouseDidMoveOverElement(const HitTestResult&, unsigned modifierFlags);

        virtual void setToolTip(const String&, TextDirection);

        virtual void print(Frame*);

        virtual void exceededDatabaseQuota(Frame*, const String& databaseName);

        virtual void runOpenPanel(Frame*, PassRefPtr<FileChooser>);

        virtual void focusedNodeChanged(Node*);
        virtual void focusedFrameChanged(Frame*);

        virtual KeyboardUIMode keyboardUIMode();

        // Notification that the given form element has changed. This function
        // will be called frequently, so handling should be very fast.
        virtual void formStateDidChange(const Node*) {notImplemented();}

        virtual void invalidateWindow(const IntRect& windowRect, bool);
        virtual void invalidateContentsAndWindow(const IntRect& windowRect, bool immediate);
        virtual void invalidateContentsForSlowScroll(const IntRect& windowRect, bool immediate);

        virtual PlatformPageClient platformPageClient() const;

        virtual void scrollbarsModeDidChange() const { }

        virtual void setCursor(const Cursor&);

        virtual void requestGeolocationPermissionForFrame(Frame*, Geolocation*) { }
        virtual void cancelGeolocationPermissionRequestForFrame(Frame*, Geolocation*) { }

        virtual void chooseIconForFiles(const Vector<String>&, FileChooser*);

        virtual void attachRootGraphicsLayer(Frame* frame, GraphicsLayer* graphicsLayer);

        virtual void setNeedsOneShotDrawingSynchronization();
        virtual void scheduleCompositingLayerSync();

        bool selectItemWritingDirectionIsNatural();
        bool selectItemAlignmentFollowsMenuWritingDirection();

        PassRefPtr<PopupMenu> createPopupMenu(PopupMenuClient* client) const;
        PassRefPtr<SearchPopupMenu> createSearchPopupMenu(PopupMenuClient* client) const;

        virtual void showContextMenu() { }

        virtual void setLastSetCursorToCurrentCursor();

        virtual void* getJavascriptCallCppCallback();

        KWebPage* m_webPage;
        WebCore::KURL lastHoverURL;
        WTF::String lastHoverTitle;
        WTF::String lastHoverContent;

        bool toolBarsVisible;
        bool statusBarVisible;
        bool menuBarVisible;
    };
}

#endif
