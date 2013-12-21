/*
    Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)

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

#ifndef KWEBELEMENT_H
#define KWEBELEMENT_H

// #include <QtCore/qstring.h>
// #include <QtCore/qstringlist.h>
// #include <QtCore/qrect.h>
// #include <QtCore/qvariant.h>
// #include <QtCore/qshareddata.h>

// #include "qwebkitglobal.h"
namespace WebCore {
    class Element;
    class Node;
}


#if defined(WTF_USE_V8) && WTF_USE_V8
namespace V8 {
    namespace Bindings {
    class QtWebElementRuntime;
    }
}
#else
namespace JSC {
    namespace Bindings {
    class QtWebElementRuntime;
    }
}
#endif

QT_BEGIN_NAMESPACE
class QPainter;
QT_END_NAMESPACE

class KWebPage;
class KWebElementCollection;
class KWebElementPrivate;

class KWebElement {
public:
    KWebElement();
    KWebElement(const KWebElement&);
    KWebElement &operator=(const KWebElement&);
    ~KWebElement();

    bool operator==(const KWebElement& o) const;
    bool operator!=(const KWebElement& o) const;

    bool isNull() const;

    KWebElementCollection findAll(const QString &selectorQuery) const;
    KWebElement findFirst(const QString &selectorQuery) const;

    void setPlainText(const QString& text);
    QString toPlainText() const;

    void setOuterXml(const QString& markup);
    QString toOuterXml() const;

    void setInnerXml(const QString& markup);
    QString toInnerXml() const;

    void setAttribute(const QString& name, const QString& value);
    void setAttributeNS(const QString& namespaceUri, const QString& name, const QString& value);
    QString attribute(const QString& name, const QString& defaultValue = QString()) const;
    QString attributeNS(const QString& namespaceUri, const QString& name, const QString& defaultValue = QString()) const;
    bool hasAttribute(const QString& name) const;
    bool hasAttributeNS(const QString& namespaceUri, const QString& name) const;
    void removeAttribute(const QString& name);
    void removeAttributeNS(const QString& namespaceUri, const QString& name);
    bool hasAttributes() const;
    QStringList attributeNames(const QString& namespaceUri = QString()) const;

    QStringList classes() const;
    bool hasClass(const QString& name) const;
    void addClass(const QString& name);
    void removeClass(const QString& name);
    void toggleClass(const QString& name);

    bool hasFocus() const;
    void setFocus();

    QRect geometry() const;

    QString tagName() const;
    QString prefix() const;
    QString localName() const;
    QString namespaceUri() const;

    KWebElement parent() const;
    KWebElement firstChild() const;
    KWebElement lastChild() const;
    KWebElement nextSibling() const;
    KWebElement previousSibling() const;
    KWebElement document() const;
    KWebPage *webPage() const;

    // TODO: Add KWebElementCollection overloads
    // docs need example snippet
    void appendInside(const QString& markup);
    void appendInside(const KWebElement& element);

    // docs need example snippet
    void prependInside(const QString& markup);
    void prependInside(const KWebElement& element);

    // docs need example snippet
    void appendOutside(const QString& markup);
    void appendOutside(const KWebElement& element);

    // docs need example snippet
    void prependOutside(const QString& markup);
    void prependOutside(const KWebElement& element);

    // docs need example snippet
    void encloseContentsWith(const KWebElement& element);
    void encloseContentsWith(const QString& markup);
    void encloseWith(const QString& markup);
    void encloseWith(const KWebElement& element);

    void replace(const QString& markup);
    void replace(const KWebElement& element);

    KWebElement clone() const;
    KWebElement& takeFromDocument();
    void removeFromDocument();
    void removeAllChildren();

    QVariant evaluateJavaScript(const QString& scriptSource);

    enum StyleResolveStrategy {
         InlineStyle,
         CascadedStyle,
         ComputedStyle,
    };
    QString styleProperty(const QString& name, StyleResolveStrategy strategy) const;
    void setStyleProperty(const QString& name, const QString& value);

    void render(QPainter* painter);
    void render(QPainter* painter, const QRect& clipRect);

private:
    explicit KWebElement(WebCore::Element*);
    explicit KWebElement(WebCore::Node*);

    static KWebElement enclosingElement(WebCore::Node*);

    friend class DumpRenderTreeSupportQt;
    friend class QWebFrame;
    friend class KWebElementCollection;
    friend class QWebHitTestResult;
    friend class QWebHitTestResultPrivate;
    friend class QWebPage;

#if defined(WTF_USE_V8) && WTF_USE_V8
    friend class V8::Bindings::QtWebElementRuntime;
#else
    friend class JSC::Bindings::QtWebElementRuntime;
#endif

    KWebElementPrivate* d;
    WebCore::Element* m_element;
};

class KWebElementCollectionPrivate;

class /*QWEBKIT_EXPORT*/ KWebElementCollection
{
public:
    KWebElementCollection();
    KWebElementCollection(const KWebElement &contextElement, const QString &query);
    KWebElementCollection(const KWebElementCollection &);
    KWebElementCollection &operator=(const KWebElementCollection &);
    ~KWebElementCollection();

    KWebElementCollection operator+(const KWebElementCollection &other) const;
    inline KWebElementCollection &operator+=(const KWebElementCollection &other)
    {
        append(other); return *this;
    }

    void append(const KWebElementCollection &collection);

    int count() const;
    KWebElement at(int i) const;
    inline KWebElement operator[](int i) const { return at(i); }

    inline KWebElement first() const { return at(0); }
    inline KWebElement last() const { return at(count() - 1); }

    QList<KWebElement> toList() const;

    class const_iterator {
       public:
           inline const_iterator(const KWebElementCollection* collection, int index) : i(index), collection(collection) {}
           inline const_iterator(const const_iterator& o) : i(o.i), collection(o.collection) {}

           inline const KWebElement operator*() const { return collection->at(i); }

           inline bool operator==(const const_iterator& o) const { return i == o.i && collection == o.collection; }
           inline bool operator!=(const const_iterator& o) const { return i != o.i || collection != o.collection; }
           inline bool operator<(const const_iterator& o) const { return i < o.i; }
           inline bool operator<=(const const_iterator& o) const { return i <= o.i; }
           inline bool operator>(const const_iterator& o) const { return i > o.i; }
           inline bool operator>=(const const_iterator& o) const { return i >= o.i; }

           inline const_iterator& operator++() { ++i; return *this; }
           inline const_iterator operator++(int) { const_iterator n(collection, i); ++i; return n; }
           inline const_iterator& operator--() { i--; return *this; }
           inline const_iterator operator--(int) { const_iterator n(collection, i); i--; return n; }
           inline const_iterator& operator+=(int j) { i += j; return *this; }
           inline const_iterator& operator-=(int j) { i -= j; return *this; }
           inline const_iterator operator+(int j) const { return const_iterator(collection, i + j); }
           inline const_iterator operator-(int j) const { return const_iterator(collection, i - j); }
           inline int operator-(const_iterator j) const { return i - j.i; }
       private:
            int i;
            const KWebElementCollection* const collection;
    };
    friend class const_iterator;

    inline const_iterator begin() const { return constBegin(); }
    inline const_iterator end() const { return constEnd(); }
    inline const_iterator constBegin() const { return const_iterator(this, 0); }
    inline const_iterator constEnd() const { return const_iterator(this, count()); };

    class iterator {
    public:
        inline iterator(const KWebElementCollection* collection, int index) : i(index), collection(collection) {}
        inline iterator(const iterator& o) : i(o.i), collection(o.collection) {}

        inline KWebElement operator*() const { return collection->at(i); }

        inline bool operator==(const iterator& o) const { return i == o.i && collection == o.collection; }
        inline bool operator!=(const iterator& o) const { return i != o.i || collection != o.collection; }
        inline bool operator<(const iterator& o) const { return i < o.i; }
        inline bool operator<=(const iterator& o) const { return i <= o.i; }
        inline bool operator>(const iterator& o) const { return i > o.i; }
        inline bool operator>=(const iterator& o) const { return i >= o.i; }

        inline iterator& operator++() { ++i; return *this; }
        inline iterator operator++(int) { iterator n(collection, i); ++i; return n; }
        inline iterator& operator--() { i--; return *this; }
        inline iterator operator--(int) { iterator n(collection, i); i--; return n; }
        inline iterator& operator+=(int j) { i += j; return *this; }
        inline iterator& operator-=(int j) { i -= j; return *this; }
        inline iterator operator+(int j) const { return iterator(collection, i + j); }
        inline iterator operator-(int j) const { return iterator(collection, i - j); }
        inline int operator-(iterator j) const { return i - j.i; }
    private:
        int i;
        const KWebElementCollection* const collection;
    };
    friend class iterator;

    inline iterator begin() { return iterator(this, 0); }
    inline iterator end()  { return iterator(this, count()); }
private:
    QExplicitlySharedDataPointer<KWebElementCollectionPrivate> d;
};

Q_DECLARE_METATYPE(KWebElement)

#endif // KWebElement_H
