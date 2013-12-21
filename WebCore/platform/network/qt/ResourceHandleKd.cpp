/*
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2008 Holger Hans Peter Freyther
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
#include <windows.h>
#include "Frame.h"
#include "DocLoader.h"
#include "ResourceHandle.h"
#include "ResourceHandleClient.h"
#include "ResourceHandleInternal.h"
//#include "qwebpage_p.h"
#include "ChromeClientQt.h"
//#include "FrameLoaderClientQt.h"
#include "Page.h"
//#include "QNetworkReplyHandler.h"

#include "NotImplemented.h"

//#include <QCoreApplication>
//#include <QUrl>
// #if QT_VERSION >= 0x040400
// #include <KNetworkAccessManager.h>
// // #include <QNetworkRequest>
// // #include <QNetworkReply>
// #else
// #include "qwebnetworkinterface_p.h"
// #endif

#include "MIMETypeRegistry.h"
#include "KWebFrame.h"
#include "KFrameNetworkingContext.h"

namespace WebCore {

// class WebCoreSynchronousLoader : public ResourceHandleClient {
// public:
//     WebCoreSynchronousLoader();
// 
//     void waitForCompletion();
// 
//     virtual void didReceiveResponse(ResourceHandle*, const ResourceResponse&);
//     virtual void didReceiveData(ResourceHandle*, const char*, int, int lengthReceived);
//     virtual void didFinishLoading(ResourceHandle*);
//     virtual void didFail(ResourceHandle*, const ResourceError&);
// 
//     ResourceResponse resourceResponse() const { return m_response; }
//     ResourceError resourceError() const { return m_error; }
//     Vector<char> data() const { return m_data; }
// 
// private:
//     ResourceResponse m_response;
//     ResourceError m_error;
//     Vector<char> m_data;
//     bool m_finished;
// };
// 
// WebCoreSynchronousLoader::WebCoreSynchronousLoader()
//     : m_finished(false)
// {
// }
// 
// void WebCoreSynchronousLoader::didReceiveResponse(ResourceHandle*, const ResourceResponse& response)
// {
//     m_response = response;
// }
// 
// void WebCoreSynchronousLoader::didReceiveData(ResourceHandle*, const char* data, int length, int)
// {
//     m_data.append(data, length);
// }
// 
// void WebCoreSynchronousLoader::didFinishLoading(ResourceHandle*)
// {
//     m_finished = true;
// }
// 
// void WebCoreSynchronousLoader::didFail(ResourceHandle*, const ResourceError& error)
// {
//     m_error = error;
//     m_finished = true;
// }
// 
// void WebCoreSynchronousLoader::waitForCompletion()
// {
//     while (!m_finished)
//         QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
// }
// 
ResourceHandleInternal::~ResourceHandleInternal()
{
}

ResourceHandle::~ResourceHandle()
{
    if (d->m_job)
        cancel();
}

bool ResourceHandle::start(NetworkingContext* context) /*Frame* frame*/
{
//     if (!frame)
//         return false;
// 
//     Page *page = frame->page();
//     // If we are no longer attached to a Page, this must be an attempted load from an
//     // onUnload handler, so let's just block it.
//     if (!page)
//         return false;
// 
//     ResourceHandleInternal *d = getInternal();
//     KURL url = d->m_request.url();
//     QByteArray qbData;
//     long long size;
//     GetUrlDataFromFile(url.string(), qbData, size);
// 
//     sendResponseIfNeeded(url, client(), size, this);
//     client()->didReceiveData(this, qbData.constData(), qbData.length(), qbData.length() /*FixMe*/);
// 
//     KWebFrame* webFrame = static_cast<KFrameLoaderClient*>(frame->loader()->client())->webFrame();
//     webFrame->notifFromResHandle(L"didFinishLoading", client());
//     // client()->didFinishLoading(this); // weolar!

    // If NetworkingContext is invalid then we are no longer attached to a Page,
    // this must be an attempted load from an unload event handler, so let's just block it.
    if (context && !context->isValid())
        return false;

//    KURL url(firstRequest().url());
//
//     QByteArray qbData;
//     long long size;
//     GetUrlDataFromFile(url.string(), qbData, size);
// 
//     sendResponseIfNeeded(url, client(), size, this);
//     client()->didReceiveData(this, qbData.constData(), qbData.length(), qbData.length());
// 
//     KFrameNetworkingContext* frameNetworkingContext = (KFrameNetworkingContext*)context;
//     KWebFrame* webFrame = frameNetworkingContext->webFrame();
//     webFrame->notifFromResHandle(L"didFinishLoading", client());

    KFrameNetworkingContext* frameNetworkingContext = (KFrameNetworkingContext*)context;
    d->m_context = frameNetworkingContext;
    KWebFrame* webFrame = frameNetworkingContext->webFrame();
    
    d->m_job = new KNetworkReplyHandler(client(), this, firstRequest().url());
    frameNetworkingContext->push_back(d->m_job);

    webFrame->notifFromResHandle(L"ResourceHandle::start", frameNetworkingContext);

    return true;
}

void ResourceHandle::cancel()
{
    //__asm int 3; // TODO_weolar
#if QT_VERSION < 0x040400
    //QWebNetworkManager::self()->cancel(this);
#else
//     if (d->m_job)
//         d->m_job->abort();
#endif
}

bool ResourceHandle::loadsBlocked()
{
    return false;
}

bool ResourceHandle::willLoadFromCache(ResourceRequest&, Frame*)
{
    notImplemented();
    return false;
}

bool ResourceHandle::supportsBufferedData()
{
    return false;
}

PassRefPtr<SharedBuffer> ResourceHandle::bufferedData()
{
    ASSERT_NOT_REACHED();
    return 0;
}

void ResourceHandle::loadResourceSynchronously(NetworkingContext*, const ResourceRequest&, StoredCredentials, ResourceError&, ResourceResponse&, Vector<char>& data)
{
    __asm int 3;
//     WebCoreSynchronousLoader syncLoader;
//     ResourceHandle handle(request, &syncLoader, true, false, true);
// 
// #if QT_VERSION < 0x040400
//     if (!QWebNetworkManager::self()->add(&handle, QWebNetworkInterface::defaultInterface(), QWebNetworkManager::SynchronousJob)) {
//         // FIXME Create a sane ResourceError
//         error = ResourceError(String(), -1, String(), String());
//         return;
//     }
// #else
//     ResourceHandleInternal *d = handle.getInternal();
//     d->m_frame = static_cast<FrameLoaderClientQt*>(frame->loader()->client())->webFrame();
//     d->m_job = new QNetworkReplyHandler(&handle, QNetworkReplyHandler::LoadNormal);
// #endif
// 
//     syncLoader.waitForCompletion();
//     error = syncLoader.resourceError();
//     data = syncLoader.data();
//     response = syncLoader.resourceResponse();
}

 
// void ResourceHandle::setDefersLoading(bool defers)
// {
//     d->m_defersLoading = defers;
// 
// }

void ResourceHandle::platformSetDefersLoading(bool defers)
{
    if (!d->m_job)
        return;
     d->m_job->setLoadingDeferred(defers);
     d->m_defersLoading = defers;
    // TODO_weolar __asm int 3;
}

// this is called on mouse over a href and on page loading
void prefetchDNS(const String& hostname)
{
//     if (QWebSettings::globalSettings()->testAttribute(QWebSettings::DnsPrefetchEnabled)) {
//         static DnsPrefetchHelper dnsPrefetchHelper;
//         dnsPrefetchHelper.lookup(QString(hostname));
//     }
}


} // namespace WebCore
