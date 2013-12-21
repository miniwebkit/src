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

#include "config.h"

#include "KFrameNetworkingContext.h"

//#include "qwebframe.h"
//#include "qwebpage.h"
#include <KNetworkAccessManager.h>
//#include <QObject>

namespace WebCore {

KFrameNetworkingContext::KFrameNetworkingContext(
    Frame* frame, 
    KWebPage * webPage,
    bool mimeSniffingEnabled
    )
    : FrameNetworkingContext(frame)
    , m_webPage(webPage)
    //, m_originatingObject(originatingObject)
    //, m_networkAccessManager(networkAccessManager)
    , m_mimeSniffingEnabled(mimeSniffingEnabled)
{
}

PassRefPtr<KFrameNetworkingContext> KFrameNetworkingContext::create(
    Frame* frame, 
    KWebPage* webPage,
    bool mimeSniffingEnabled
    )
{
    return adoptRef(new KFrameNetworkingContext(frame, webPage, mimeSniffingEnabled));
}

// QObject* KFrameNetworkingContext::originatingObject() const
// {
//     //return m_originatingObject;
//     return 0;
// }

KNetworkAccessManager* KFrameNetworkingContext::networkAccessManager() const
{
    // weolar
    __asm int 3;
    return 0;
    //return (qobject_cast<QWebFrame*>(m_originatingObject))->page()->networkAccessManager();
}

bool KFrameNetworkingContext::mimeSniffingEnabled() const
{
    __asm int 3;
    return m_mimeSniffingEnabled;
}

String KFrameNetworkingContext::userAgent() const
{
    return "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.1 (KHTML, like Gecko) Chrome/21.0.1180.60 Safari/537.1 CoolNovo/2.0.4.12";
}

String KFrameNetworkingContext::referrer() const
{
    return frame()->loader()->referrer();
}

WebCore::ResourceError KFrameNetworkingContext::blockedError(const WebCore::ResourceRequest& request) const
{
    return frame()->loader()->client()->blockedError(request);
}

}
