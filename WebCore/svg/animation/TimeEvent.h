

/*
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef TimeEvent_h
#define TimeEvent_h

#include "DOMWindow.h"
#include "Event.h"

namespace WebCore {

typedef DOMWindow AbstractView;

class TimeEvent : public Event {
public:
    static PassRefPtr<TimeEvent> create()
    {
        return adoptRef(new TimeEvent);
    }
    static PassRefPtr<TimeEvent> create(const AtomicString& type, PassRefPtr<AbstractView> view, int detail)
    {
        return adoptRef(new TimeEvent(type, view, detail));
    }
    virtual ~TimeEvent();

    void initTimeEvent(const AtomicString& type, PassRefPtr<AbstractView>, int detail);

    AbstractView* view() const { return m_view.get(); }
    int detail() const { return m_detail; }

protected:
    TimeEvent();
    TimeEvent(const AtomicString& type, PassRefPtr<AbstractView>, int detail);

private:
    RefPtr<AbstractView> m_view;
    int m_detail;
};

} // namespace WebCore

#endif
