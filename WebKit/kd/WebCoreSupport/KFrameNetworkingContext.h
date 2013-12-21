/*
    Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KFrameNetworkingContext_h
#define KFrameNetworkingContext_h

#include "KWebPage.h"
#include "FrameNetworkingContext.h"
#include <list>

namespace WebCore {

class KNetworkReplyHandler {
public:
    KNetworkReplyHandler(
        ResourceHandleClient* client,
        ResourceHandle* resHandle, 
        KURL m_url
        )
        : m_client(client)
        , m_resHandle(resHandle)
        , m_url(m_url)
        , m_deferred(false)
    {}
//     void setClient(ResourceHandleClient* client) {m_client = client;}
     ResourceHandleClient* client() {return m_client;}
// 
//     void setURL(KURL url) {m_url = url;}
     KURL URL() {return m_url;}
// 
//     void setResHandle(ResourceHandle* resHandle) {m_resHandle = resHandle;}
     ResourceHandle* ResHandle() {return m_resHandle;}

     bool isDeferred() {return m_deferred;}

     void setLoadingDeferred(bool deferred) { m_deferred = deferred; }

protected:
    ResourceHandleClient* m_client;
    ResourceHandle* m_resHandle;
    KURL m_url;
    bool m_deferred;
};

class KFrameNetworkingContext : public FrameNetworkingContext {
public:
    static PassRefPtr<KFrameNetworkingContext> create(Frame*, KWebPage*, bool mimeSniffingEnabled);

    KWebPage* webPage() {return m_webPage;}

    void push_back (KNetworkReplyHandler* replyHandler) {arrReplyHandler.add(replyHandler);}

    ListHashSet<KNetworkReplyHandler*> arrReplyHandler;


    KFrameNetworkingContext(Frame*, KWebPage* webPage, bool mimeSniffingEnabled);
    
    //virtual QObject* originatingObject() const;
    virtual KNetworkAccessManager* networkAccessManager() const;
    virtual bool mimeSniffingEnabled() const;

    virtual String userAgent() const;
    virtual String referrer() const;
    virtual ResourceError blockedError(const ResourceRequest&) const;

private:
    //QObject* m_originatingObject;
    //KNetworkAccessManager* m_networkAccessManager;
    bool m_mimeSniffingEnabled;
    KWebPage* m_webPage;
};

}

#endif // FrameNetworkingContextQt_h
