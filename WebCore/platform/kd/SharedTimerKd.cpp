/*
 * Copyright (C) 2006 George Staikos <staikos@kde.org>
 * Copyright (C) 2006 Dirk Mueller <mueller@kde.org>
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

#include "SharedTimerKd.h"

#include "SystemTime.h"
#include "wtf/CurrentTime.h"

namespace WebCore {

class KBasicTimer
{
    int id;
public:
    inline KBasicTimer() : id(0) {}
    inline ~KBasicTimer() { if (id) stop(); }

    inline bool isActive() const { return id != 0; }
    inline int timerId() const { return id; }

    void start(int msec/*, QObject *obj*/)
    {
//         stop();
//         if (obj)
//             id = obj->startTimer(msec);
    }


    void stop()
    {
//         if (id) {
//             QAbstractEventDispatcher *eventDispatcher = QAbstractEventDispatcher::instance();
//             if (eventDispatcher)
//                 eventDispatcher->unregisterTimer(id);
//         }
//         id = 0;
    }
};

SharedTimerKd::SharedTimerKd(/*QObject* parent*/)
    : m_timerFunction(0)
{}

SharedTimerKd::~SharedTimerKd()
{
//     if (m_timer.isActive())
//         (m_timerFunction)();
}

static WTF::ThreadSpecific<SharedTimerKd> sharedTimerKdTimer;
SharedTimerKd* SharedTimerKd::inst()
{
    if (!sharedTimerKdTimer.hasInit())
        sharedTimerKdTimer = new SharedTimerKd(/*QCoreApplication::instance()*/);

    return sharedTimerKdTimer;
}

void SharedTimerKd::start(double fireTime)
{
    double interval = fireTime - WTF::currentTime();
    unsigned int intervalInMS;
    if (interval < 0)
        intervalInMS = 0;
    else {
        interval *= 1000;
        intervalInMS = (unsigned int)interval;
    }

    //m_timer.start(intervalInMS/*, this*/);
}

void SharedTimerKd::stop()
{
    //m_timer.stop();
}

void SharedTimerKd::timerEvent(/*QTimerEvent* ev*/)
{
    //if (!m_timerFunction || ev->timerId() != m_timer.timerId())
    //    return;

    //m_timer.stop();
    (m_timerFunction)();
}

void setSharedTimerFiredFunction(void (*f)())
{
//     if (!QCoreApplication::instance())
//         return;

    SharedTimerKd::inst()->m_timerFunction = f;
}

void setSharedTimerFireTime(double fireTime)
{
//     if (!QCoreApplication::instance())
//         return;

    SharedTimerKd::inst()->start(fireTime);
}

void stopSharedTimer()
{
//     if (!QCoreApplication::instance())
//         return;

    SharedTimerKd::inst()->stop();
}

}

// vim: ts=4 sw=4 et
