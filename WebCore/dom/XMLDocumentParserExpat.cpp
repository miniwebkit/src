/*
 * Copyright (C) 2000 Peter Kelly <pmk@post.com>
 * Copyright (C) 2005, 2006, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Alexey Proskuryakov <ap@webkit.org>
 * Copyright (C) 2007 Samuel Weinig <sam@webkit.org>
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2008 Holger Hans Peter Freyther
 * Copyright (C) 2008, 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 * Copyright (C) 2010 Patrick Gansterer <paroga@paroga.com>
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

#define XML_STATIC 

#include "config.h"
#include "XMLDocumentParser.h"

//#include "CDATASection.h"
#include "CachedScript.h"
#include "Comment.h"
#include "CachedResourceLoader.h"
#include "Document.h"
#include "DocumentFragment.h"
#include "DocumentType.h"
#include "Frame.h"
#include "FrameLoader.h"
#include "FrameView.h"

#include "Element.h"

// #include "HTMLEntityParser.h"
// #include "HTMLHtmlElement.h"
// #include "HTMLLinkElement.h"
#include "HTMLNames.h"
//#include "HTMLStyleElement.h"
//#include "ProcessingInstruction.h"
#include "ResourceError.h"
#include "ResourceHandle.h"
#include "ResourceRequest.h"
#include "ResourceResponse.h"
#include "ScriptElement.h"
#include "ScriptSourceCode.h"
#include "ScriptValue.h"
//#include "SecurityOrigin.h"
#include "TextResourceDecoder.h"
//#include "TransformSource.h"
#include "XMLNSNames.h"
#include "XMLDocumentParserScope.h"
// #include <libxml/parser.h>
// #include <libxml/parserInternals.h>
#include <expat/expat.h>
#include <wtf/text/CString.h>
#include <wtf/StringExtras.h>
#include <wtf/Threading.h>
#include <wtf/UnusedParam.h>
#include <wtf/Vector.h>
#include <NotImplemented.h>
//using namespace std;

// This ensures proper sorting.
#define NSSEP ('\001')

namespace WebCore {

static XML_Char *
xmlStrndup(const XML_Char *cur, int len) {
    XML_Char *ret;

    if ((cur == NULL) || (len < 0)) return(NULL);
    ret = (XML_Char *) malloc((len + 1) * sizeof(XML_Char));
    if (ret == NULL) {
        return(NULL);
    }
    memcpy(ret, cur, len * sizeof(XML_Char));
    ret[len] = 0;
    return(ret);
}

static XML_Char *
xmlStrdup(const XML_Char *cur) {
    const XML_Char *p = cur;

    if (cur == NULL) return(NULL);
    while (*p != 0) p++; /* non input consuming */
    return(xmlStrndup(cur, p - cur));
}

#define xmlMalloc(size) malloc((size))
#define xmlFree(p) free((p))

class PendingCallbacks {
    WTF_MAKE_NONCOPYABLE(PendingCallbacks);
public:
    ~PendingCallbacks() { }
    static PassOwnPtr<PendingCallbacks> create()
    {
        return adoptPtr(new PendingCallbacks);
    }
    
    void appendStartElementNSCallback(
        URIToPrefixMap& uriToPrefixMap, // 从StartNSDecl里收集到的ns 
        const XML_Char *elements,
        const XML_Char **attributes
        )
    {
        //notImplemented();
        OwnPtr<PendingStartElementNSCallback> callback = adoptPtr(new PendingStartElementNSCallback);

        //callback->m_uriToPrefixMap = uriToPrefixMap; // 不需要保存，直接用XMLDocumentParserExpat中XMLParserContext的
        callback->m_elements = xmlStrdup(elements);
        int i = 0;
        for (i = 0; attributes[i]; i += 2) {}
        callback->m_attributeNum = i + 2; // 多出个零结尾的属性
        callback->m_attributes = static_cast<const XML_Char**>(xmlMalloc(sizeof(XML_Char*) * (i + 1) * 2));
        for (i = 0; attributes[i]; i ++) {
            callback->m_attributes[i] = xmlStrdup(attributes[i]);
        }
        callback->m_attributes[i] = 0;
        callback->m_attributes[i + 1] = 0;

        m_callbacks.append(callback.release());
    }

    void appendEndElementNSCallback()
    {
        m_callbacks.append(adoptPtr(new PendingEndElementNSCallback));
    }

    void appendCharactersCallback(const XML_Char* s, int len)
    {
        OwnPtr<PendingCharactersCallback> callback = adoptPtr(new PendingCharactersCallback);

        callback->s = xmlStrndup(s, len);
        callback->len = len;

        m_callbacks.append(callback.release());
    }

    void appendProcessingInstructionCallback(const XML_Char* target, const XML_Char* data)
    {
        OwnPtr<PendingProcessingInstructionCallback> callback = adoptPtr(new PendingProcessingInstructionCallback);

        callback->target = xmlStrdup(target);
        callback->data = xmlStrdup(data);

        m_callbacks.append(callback.release());
    }

    void appendCDATABlockCallback(const XML_Char* s, int len)
    {
        OwnPtr<PendingCDATABlockCallback> callback = adoptPtr(new PendingCDATABlockCallback);

        callback->s = xmlStrndup(s, len);
        callback->len = len;

        m_callbacks.append(callback.release());
    }

    void appendCommentCallback(const XML_Char* s)
    {
        OwnPtr<PendingCommentCallback> callback = adoptPtr(new PendingCommentCallback);

        callback->s = xmlStrdup(s);

        m_callbacks.append(callback.release());
    }

    void appendInternalSubsetCallback(const XML_Char* name, const XML_Char* externalID, const XML_Char* systemID)
    {
        OwnPtr<PendingInternalSubsetCallback> callback = adoptPtr(new PendingInternalSubsetCallback);

        callback->name = xmlStrdup(name);
        callback->externalID = xmlStrdup(externalID);
        callback->systemID = xmlStrdup(systemID);

        m_callbacks.append(callback.release());
    }

    void appendErrorCallback(XMLDocumentParser::ErrorType type, const XML_Char* message, int lineNumber, int columnNumber)
    {
        OwnPtr<PendingErrorCallback> callback = adoptPtr(new PendingErrorCallback);

        callback->message = xmlStrdup(message);
        callback->type = type;
        callback->lineNumber = lineNumber;
        callback->columnNumber = columnNumber;

        m_callbacks.append(callback.release());
    }

    void callAndRemoveFirstCallback(XMLDocumentParser* parser)
    {
        OwnPtr<PendingCallback> callback = m_callbacks.takeFirst();
        callback->call(parser);
    }

    bool isEmpty() const { return m_callbacks.isEmpty(); }

private:
    PendingCallbacks() { }

    struct PendingCallback {
        virtual ~PendingCallback() { }
        virtual void call(XMLDocumentParser* parser) = 0;
    };

    struct PendingStartElementNSCallback : public PendingCallback {
        virtual ~PendingStartElementNSCallback()
        {
            //m_uriToPrefixMap.clear();
            for (int i = 0; i < m_attributeNum; i++)
                xmlFree((void *)m_attributes[i]);
            xmlFree((void *)m_attributes);
        }

        virtual void call(XMLDocumentParser* parser)
        {
            //URIToPrefixMap::iterator it = m_uriToPrefixMap.begin();

//             URIToPrefixMap& uriToPrefixMap = parser->getURIToPrefixMap();
//             URIToPrefixMap::iterator it = uriToPrefixMap.begin();
//             for(; it != uriToPrefixMap.end(); ++it) {
//                 AtomicString& prefix = it->second;
//                 AtomicString& uri = it->first;
//                 parser->StartNSDecl(prefix, uri);
//             }
            
            parser->preStartElementNs(m_elements, m_attributes);
        }

//         XML_Char* xmlLocalName;
//         XML_Char* xmlPrefix;
//         XML_Char* xmlURI;
//         int nb_namespaces;
//         XML_Char** namespaces;
//         int nb_attributes;
//        int nb_defaulted;
//        XML_Char** attributes;

        int m_attributeNum;
        //URIToPrefixMap m_uriToPrefixMap;
        const XML_Char *m_elements;
        const XML_Char **m_attributes;
    };

    struct PendingEndElementNSCallback : public PendingCallback {
        virtual void call(XMLDocumentParser* parser)
        {
            parser->endElementNs();
        }
    };

    struct PendingCharactersCallback : public PendingCallback {
        virtual ~PendingCharactersCallback()
        {
            xmlFree(s);
        }

        virtual void call(XMLDocumentParser* parser)
        {
            parser->characters(s, len);
        }

        XML_Char* s;
        int len;
    };

    struct PendingProcessingInstructionCallback : public PendingCallback {
        virtual ~PendingProcessingInstructionCallback()
        {
            xmlFree(target);
            xmlFree(data);
        }

        virtual void call(XMLDocumentParser* parser)
        {
            parser->processingInstruction(target, data);
        }

        XML_Char* target;
        XML_Char* data;
    };

    struct PendingCDATABlockCallback : public PendingCallback {
        virtual ~PendingCDATABlockCallback()
        {
            xmlFree(s);
        }

        virtual void call(XMLDocumentParser* parser)
        {
            parser->cdataBlock(s, len);
        }

        XML_Char* s;
        int len;
    };

    struct PendingCommentCallback : public PendingCallback {
        virtual ~PendingCommentCallback()
        {
            xmlFree(s);
        }

        virtual void call(XMLDocumentParser* parser)
        {
            parser->comment(s);
        }

        XML_Char* s;
    };

    struct PendingInternalSubsetCallback : public PendingCallback {
        virtual ~PendingInternalSubsetCallback()
        {
            xmlFree(name);
            xmlFree(externalID);
            xmlFree(systemID);
        }

        virtual void call(XMLDocumentParser* parser)
        {
            parser->internalSubset(name, externalID, systemID);
        }

        XML_Char* name;
        XML_Char* externalID;
        XML_Char* systemID;
    };

    struct PendingErrorCallback: public PendingCallback {
        virtual ~PendingErrorCallback()
        {
            xmlFree(message);
        }

        virtual void call(XMLDocumentParser* parser)
        {
            parser->handleError(type, reinterpret_cast<char*>(message), lineNumber, columnNumber);
        }

        XMLDocumentParser::ErrorType type;
        XML_Char* message;
        int lineNumber;
        int columnNumber;
    };

    Deque<OwnPtr<PendingCallback> > m_callbacks;
};
// --------------------------------

static int globalDescriptor = 0;
static ThreadIdentifier libxmlLoaderThread = 0;

static int matchFunc(const char*)
{
    notImplemented();
    // Only match loads initiated due to uses of libxml2 from within XMLDocumentParser to avoid
    // interfering with client applications that also use libxml2.  http://bugs.webkit.org/show_bug.cgi?id=17353
    //return XMLDocumentParserScope::currentCachedResourceLoader && currentThread() == libxmlLoaderThread;
}

// class OffsetBuffer {
// public:
//     OffsetBuffer(const Vector<char>& b) : m_buffer(b), m_currentOffset(0) { }
// 
//     int readOutBytes(char* outputBuffer, unsigned askedToRead)
//     {
//         unsigned bytesLeft = m_buffer.size() - m_currentOffset;
//         unsigned lenToCopy = min(askedToRead, bytesLeft);
//         if (lenToCopy) {
//             memcpy(outputBuffer, m_buffer.data() + m_currentOffset, lenToCopy);
//             m_currentOffset += lenToCopy;
//         }
//         return lenToCopy;
//     }
// 
// private:
//     Vector<char> m_buffer;
//     unsigned m_currentOffset;
// };

// static void switchToUTF16(xmlParserCtxtPtr ctxt)
// {
//     // Hack around libxml2's lack of encoding overide support by manually
//     // resetting the encoding to UTF-16 before every chunk.  Otherwise libxml
//     // will detect <?xml version="1.0" encoding="<encoding name>"?> blocks
//     // and switch encodings, causing the parse to fail.
//     const UChar BOM = 0xFEFF;
//     const unsigned char BOMHighByte = *reinterpret_cast<const unsigned char*>(&BOM);
//     xmlSwitchEncoding(ctxt, BOMHighByte == 0xFF ? XML_CHAR_ENCODING_UTF16LE : XML_CHAR_ENCODING_UTF16BE);
// }

static bool shouldAllowExternalLoad(const KURL& url)
{
    String urlString = url.string();

    // On non-Windows platforms libxml asks for this URL, the
    // "XML_XML_DEFAULT_CATALOG", on initialization.
    if (urlString == "file:///etc/xml/catalog")
        return false;

    // On Windows, libxml computes a URL relative to where its DLL resides.
    if (urlString.startsWith("file:///", false) && urlString.endsWith("/etc/catalog", false))
        return false;

    // The most common DTD.  There isn't much point in hammering www.w3c.org
    // by requesting this URL for every XHTML document.
    if (urlString.startsWith("http://www.w3.org/TR/xhtml", false))
        return false;

    // Similarly, there isn't much point in requesting the SVG DTD.
    if (urlString.startsWith("http://www.w3.org/Graphics/SVG", false))
        return false;

    // The libxml doesn't give us a lot of context for deciding whether to
    // allow this request.  In the worst case, this load could be for an
    // external entity and the resulting document could simply read the
    // retrieved content.  If we had more context, we could potentially allow
    // the parser to load a DTD.  As things stand, we take the conservative
    // route and allow same-origin requests only.
    notImplemented();
//     if (!XMLDocumentParserScope::currentCachedResourceLoader->document()->securityOrigin()->canRequest(url)) {
//         XMLDocumentParserScope::currentCachedResourceLoader->printAccessDeniedMessage(url);
//         return false;
//     }

    return true;
}

// --------------------------------

bool XMLDocumentParser::supportsXMLVersion(const String& version)
{
    return version == "1.0";
}

XMLDocumentParser::XMLDocumentParser(Document* document, FrameView* frameView)
    : ScriptableDocumentParser(document)
    , m_view(frameView)
    , m_context(0)
    , m_pendingCallbacks(PendingCallbacks::create())
    , m_currentNode(document)
    , m_sawError(false)
    , m_sawCSS(false)
    , m_sawXSLTransform(false)
    , m_sawFirstElement(false)
    , m_isXHTMLDocument(false)
    , m_parserPaused(false)
    , m_requestingScript(false)
    , m_finishCalled(false)
    , m_errorCount(0)
    , m_lastErrorPosition(TextPosition1::belowRangePosition())
    , m_pendingScript(0)
    , m_scriptStartPosition(TextPosition1::belowRangePosition())
    , m_parsingFragment(false)
    , m_scriptingPermission(FragmentScriptingAllowed)
{
}

XMLDocumentParser::XMLDocumentParser(DocumentFragment* fragment, Element* parentElement, FragmentScriptingPermission scriptingPermission)
    : ScriptableDocumentParser(fragment->document())
    , m_view(0)
    , m_context(0)
    , m_pendingCallbacks(PendingCallbacks::create())
    , m_currentNode(fragment)
    , m_sawError(false)
    , m_sawCSS(false)
    , m_sawXSLTransform(false)
    , m_sawFirstElement(false)
    , m_isXHTMLDocument(false)
    , m_parserPaused(false)
    , m_requestingScript(false)
    , m_finishCalled(false)
    , m_errorCount(0)
    , m_lastErrorPosition(TextPosition1::belowRangePosition())
    , m_pendingScript(0)
    , m_scriptStartPosition(TextPosition1::belowRangePosition())
    , m_parsingFragment(true)
    , m_scriptingPermission(scriptingPermission)
{
    fragment->ref();

    // Add namespaces based on the parent node
    Vector<Element*> elemStack;
    while (parentElement) {
        elemStack.append(parentElement);

        ContainerNode* n = parentElement->parentNode();
        if (!n || !n->isElementNode())
            break;
        parentElement = static_cast<Element*>(n);
    }

    if (elemStack.isEmpty())
        return;

    for (Element* element = elemStack.last(); !elemStack.isEmpty(); elemStack.removeLast()) {
        if (NamedNodeMap* attrs = element->attributes()) {
            for (unsigned i = 0; i < attrs->length(); i++) {
                Attribute* attr = attrs->attributeItem(i);
                if (attr->localName() == xmlnsAtom)
                    m_defaultNamespaceURI = attr->value();
                else if (attr->prefix() == xmlnsAtom)
                    m_prefixToNamespaceMap.set(attr->localName(), attr->value());
            }
        }
    }

    // If the parent element is not in document tree, there may be no xmlns attribute; just default to the parent's namespace.
    if (m_defaultNamespaceURI.isNull() && !parentElement->inDocument())
        m_defaultNamespaceURI = parentElement->namespaceURI();
}

XMLParserContext::~XMLParserContext()
{
//     if (m_context->myDoc)
//         xmlFreeDoc(m_context->myDoc);
//     xmlFreeParserCtxt(m_context);
    if (m_parser)
    { XML_ParserFree(m_parser); }
}

XMLDocumentParser::~XMLDocumentParser()
{
    // The XMLDocumentParser will always be detached before being destroyed.
    ASSERT(m_currentNodeStack.isEmpty());
    ASSERT(!m_currentNode);

    // FIXME: m_pendingScript handling should be moved into XMLDocumentParser.cpp!
    if (m_pendingScript)
        m_pendingScript->removeClient(this);
}

void XMLDocumentParser::doWrite(const String& parseString)
{
    ASSERT(!isDetached());
    if (!m_context)
        initializeParserContext();

    // Protect the libxml context from deletion during a callback
    RefPtr<XMLParserContext> context = m_context;

    // libXML throws an error if you try to switch the encoding for an empty string.
    if (parseString.length()) {
        // JavaScript may cause the parser to detach during xmlParseChunk
        // keep this alive until this function is done.
        RefPtr<XMLDocumentParser> protect(this);

        //switchToUTF16(context->context());
        //XMLDocumentParserScope scope(document()->cachedResourceLoader());
        //xmlParseChunk(context->context(), reinterpret_cast<const char*>(parseString.characters()), sizeof(UChar) * parseString.length(), 0);
        startDocument("1.0", "UTF-8", 1);
        bool bRet = XML_Parse(context->getParser(), 
            reinterpret_cast<const char*>(parseString.characters()), 
            sizeof(UChar) * parseString.length(), true);
        if (!bRet)
            error(XMLDocumentParser::fatal, XML_ErrorString(XML_GetErrorCode(context->getParser())));
        
        endDocument();
        if (!bRet)
            return;

        // JavaScript (which may be run under the xmlParseChunk callstack) may
        // cause the parser to be stopped or detached.
        if (isStopped())
            return;
    }

    // FIXME: Why is this here?  And why is it after we process the passed source?
    if (document()->decoder() && document()->decoder()->sawError()) {
        // If the decoder saw an error, report it as fatal (stops parsing)
        handleError(fatal, "Encoding error", XML_GetCurrentLineNumber(m_context->getParser()),
            XML_GetCurrentColumnNumber(m_context->getParser()));
    }
}

static inline String toString(const XML_Char* string, size_t size)
{
    return String::fromUTF8(reinterpret_cast<const char*>(string), size);
}

static inline String toString(const XML_Char* string)
{
    return String::fromUTF8(reinterpret_cast<const char*>(string));
}

static inline AtomicString toAtomicString(const XML_Char* string, size_t size)
{
    return AtomicString::fromUTF8(reinterpret_cast<const char*>(string), size);
}

static inline AtomicString toAtomicString(const XML_Char* string)
{
    return AtomicString::fromUTF8(reinterpret_cast<const char*>(string));
}

struct _xmlSAX2Namespace {
    const XML_Char* prefix;
    const XML_Char* uri;
};
typedef struct _xmlSAX2Namespace xmlSAX2Namespace;

static inline void handleElementNamespaces(Element* newElement, URIToPrefixMap& uriToPrefixMap, ExceptionCode& ec, FragmentScriptingPermission scriptingPermission)
{
//     xmlSAX2Namespace* namespaces = reinterpret_cast<xmlSAX2Namespace*>(libxmlNamespaces);
//     for (int i = 0; i < nb_namespaces; i++) {
//         AtomicString namespaceQName = xmlnsAtom;
//         AtomicString namespaceURI = toAtomicString(namespaces[i].uri);
//         if (namespaces[i].prefix)
//             namespaceQName = "xmlns:" + toString(namespaces[i].prefix);
//         newElement->setAttributeNS(XMLNSNames::xmlnsNamespaceURI, namespaceQName, namespaceURI, ec, scriptingPermission);
//         if (ec) // exception setting attributes
//             return;
//     }
    URIToPrefixMap::iterator it = uriToPrefixMap.begin();

    for(; it != uriToPrefixMap.end(); ++it) {
        AtomicString namespaceQName = xmlnsAtom;
        AtomicString namespaceURI = it-> first;
        if (!it->second.isEmpty()) {
            namespaceQName = "xmlns:" + it->second;
        }
        newElement->setAttributeNS(XMLNSNames::xmlnsNamespaceURI, namespaceQName, namespaceURI, ec, scriptingPermission);
    }
}

struct _xmlSAX2Attributes {
    const XML_Char* localname;
    const XML_Char* prefix;
    const XML_Char* uri;
    const XML_Char* value;
    const XML_Char* end;
};
typedef struct _xmlSAX2Attributes xmlSAX2Attributes;

static inline void handleElementAttributes(Element* newElement, URIToPrefixMap& uriToPrefixMap, const XML_Char ** attributes, ExceptionCode& ec, FragmentScriptingPermission scriptingPermission)
{
//     xmlSAX2Attributes* attributes = reinterpret_cast<xmlSAX2Attributes*>(libxmlAttributes);
//     for (int i = 0; i < nb_attributes; i++) {
//         int valueLength = static_cast<int>(attributes[i].end - attributes[i].value);
//         AtomicString attrValue = toAtomicString(attributes[i].value, valueLength);
//         String attrPrefix = toString(attributes[i].prefix);
//         AtomicString attrURI = attrPrefix.isEmpty() ? AtomicString() : toAtomicString(attributes[i].uri);
//         AtomicString attrQName = attrPrefix.isEmpty() ? toAtomicString(attributes[i].localname) : AtomicString(attrPrefix + ":" + toString(attributes[i].localname));
// 
//         newElement->setAttributeNS(attrURI, attrQName, attrValue, ec, scriptingPermission);
//         if (ec) // exception setting attributes
//             return;
//     }
    for (int i = 0; attributes[i]; i += 2) {
        AtomicString localName;
        AtomicString prefix;
        AtomicString attrURI;
        AtomicString attrValue = attributes[i + 1];
        AtomicString attrQName;
        const XML_Char *sep = NULL;
        sep = strrchr(attributes[i], NSSEP); // http://www.w3.org/1999/xlink|xxx='123' xxx='123'
        if (!sep) {
            attrURI = AtomicString();
            attrQName = toAtomicString(attributes[i]);
        } else {
            attrURI = toAtomicString(attributes[i], sep - attributes[i]);
            prefix = uriToPrefixMap.get(attrURI);
            if (prefix.isEmpty())
            { notImplemented(); }

            attrQName = prefix + ":" + toAtomicString(sep + 1);
        }

        newElement->setAttributeNS(attrURI, attrQName, attrValue, ec, scriptingPermission);
        if (ec) // exception setting attributes
            return;
    }
}

void XMLDocumentParser::StartNSDecl(AtomicString prefix, AtomicString uri)
{
    m_context->m_URIToPrefixMap.add((uri), (prefix));
    m_context->m_temURIToPrefixMap.add((uri), (prefix));
}

static inline bool parserLocalNamePrefixURI(
    URIToPrefixMap& uriToPrefixMap,
    const XML_Char *name, // http://www.w3.org/2000/svg|svg形式
    AtomicString& localname, // OUT
    AtomicString& prefix, // OUT
    AtomicString& URI // OUT
    )
{
    const XML_Char *sep = NULL;
    sep = strrchr(name, NSSEP);
    if (!sep) // 每个标签都必须要有ns
        {return false;}

    URI = toAtomicString(name, sep - name);
    prefix = uriToPrefixMap.get(URI);
    localname = toAtomicString(sep + 1);

    return true;
}

void XMLDocumentParser::preStartElementNs(
    const XML_Char *name,
    const XML_Char **atts
    )
{
    startElementNs(m_context->m_temURIToPrefixMap, name, atts);
    m_context->m_temURIToPrefixMap.clear();
}

void XMLDocumentParser::startElementNs(
    URIToPrefixMap& uriToPrefixMap, // 从StartNSDecl里收集到的ns 
    const XML_Char *name,
    const XML_Char **atts
    )
{
    if (isStopped())
        return;

    if (m_parserPaused) {
        m_pendingCallbacks->appendStartElementNSCallback(m_context->m_URIToPrefixMap, name, atts);
        return;
    }

    exitText();

    AtomicString localName;
    AtomicString uri;
    AtomicString prefix;
    if (!parserLocalNamePrefixURI(m_context->m_URIToPrefixMap, name, localName, prefix, uri)) {
        error(XMLDocumentParser::fatal, "parserLocalNamePrefixURI fail!");
        return;
    }

    if (m_parsingFragment && uri.isNull()) {
        if (!prefix.isNull())
            uri = m_prefixToNamespaceMap.get(prefix);
        else
            uri = m_defaultNamespaceURI;
    }

    bool isFirstElement = !m_sawFirstElement;
    m_sawFirstElement = true;

    QualifiedName qName(prefix, localName, uri);
    RefPtr<Element> newElement = document()->createElement(qName, true);
    if (!newElement) {
        stopParsing();
        return;
    }

    ExceptionCode ec = 0;
    handleElementNamespaces(newElement.get(), m_context->m_URIToPrefixMap, ec, m_scriptingPermission);
    if (ec) {
        stopParsing();
        return;
    }

    handleElementAttributes(newElement.get(), m_context->m_URIToPrefixMap, atts, ec, m_scriptingPermission);
    if (ec) {
        stopParsing();
        return;
    }

    newElement->beginParsingChildren();

    ScriptElement* scriptElement = toScriptElement(newElement.get());
    if (scriptElement)
        m_scriptStartPosition = textPositionOneBased();

    m_currentNode->deprecatedParserAddChild(newElement.get());

    pushCurrentNode(newElement.get());
    if (m_view && !newElement->attached())
        newElement->attach();

    if (!m_parsingFragment && isFirstElement && document()->frame())
        document()->frame()->loader()->dispatchDocumentElementAvailable();
}

void XMLDocumentParser::endElementNs()
{
    if (isStopped())
        return;

    if (m_parserPaused) {
        m_pendingCallbacks->appendEndElementNSCallback();
        return;
    }

    // JavaScript can detach the parser.  Make sure this is not released
    // before the end of this method.
    RefPtr<XMLDocumentParser> protect(this);

    exitText();

    RefPtr<Node> n = m_currentNode;
    n->finishParsingChildren();

    if (m_scriptingPermission == FragmentScriptingNotAllowed && n->isElementNode() && toScriptElement(static_cast<Element*>(n.get()))) {
        popCurrentNode();
        ExceptionCode ec;
        n->remove(ec);
        return;
    }

    if (!n->isElementNode() || !m_view) {
        popCurrentNode();
        return;
    }

    Element* element = static_cast<Element*>(n.get());

    // The element's parent may have already been removed from document.
    // Parsing continues in this case, but scripts aren't executed.
    if (!element->inDocument()) {
        popCurrentNode();
        return;
    }

    ScriptElement* scriptElement = toScriptElement(element);
    if (!scriptElement) {
        popCurrentNode();
        return;
    }

    // Don't load external scripts for standalone documents (for now).
    ASSERT(!m_pendingScript);
    m_requestingScript = true;

    bool successfullyPrepared = scriptElement->prepareScript(m_scriptStartPosition, ScriptElement::AllowLegacyTypeInTypeAttribute);
    if (!successfullyPrepared) {
    } else {
        // FIXME: Script execution should be shared between
        // the libxml2 and Qt XMLDocumentParser implementations.

        if (scriptElement->readyToBeParserExecuted())
            scriptElement->executeScript(ScriptSourceCode(scriptElement->scriptContent(), document()->url(), m_scriptStartPosition));
        else if (scriptElement->willBeParserExecuted()) {
            m_pendingScript = scriptElement->cachedScript();
            m_scriptElement = element;
            m_pendingScript->addClient(this);

            // m_pendingScript will be 0 if script was already loaded and addClient() executed it.
            if (m_pendingScript)
                pauseParsing();
        } else
            m_scriptElement = 0;

        // JavaScript may have detached the parser
        if (isDetached())
            return;
    }
    m_requestingScript = false;
    popCurrentNode();
}

void XMLDocumentParser::characters(const XML_Char* s, int len)
{
    if (isStopped())
        return;

    if (m_parserPaused) {
        m_pendingCallbacks->appendCharactersCallback(s, len);
        return;
    }

    if (!m_currentNode->isTextNode())
        enterText();
    m_bufferedText.append(toString(s, len));
}

void XMLDocumentParser::error(ErrorType type, const char* message /*, va_list args*/)
{
    if (isStopped())
        return;

// #if COMPILER(MSVC) || COMPILER(RVCT) || COMPILER(MINGW)
//     char m[1024];
//     vsnprintf(m, sizeof(m) - 1, message, args);
// #else
//     char* m;
//     if (vasprintf(&m, message, args) == -1)
//         return;
// #endif
// 
//     if (m_parserPaused)
//         m_pendingCallbacks->appendErrorCallback(type, reinterpret_cast<const XML_Char*>(m), lineNumber(), columnNumber());
//     else
//         handleError(type, m, lineNumber(), columnNumber());
// 
// #if !COMPILER(MSVC) && !COMPILER(RVCT) && !COMPILER(MINGW)
//     free(m);
// #endif

    if (m_parserPaused)
        m_pendingCallbacks->appendErrorCallback(type, reinterpret_cast<const XML_Char*>(message), lineNumber(), columnNumber());
    else
        handleError(type, message, lineNumber(), columnNumber());
}

void XMLDocumentParser::processingInstruction(const XML_Char* target, const XML_Char* data)
{
    notImplemented();
//     if (isStopped())
//         return;
// 
//     if (m_parserPaused) {
//         m_pendingCallbacks->appendProcessingInstructionCallback(target, data);
//         return;
//     }
// 
//     exitText();
// 
//     // ### handle exceptions
//     ExceptionCode ec = 0;
//     RefPtr<ProcessingInstruction> pi = document()->createProcessingInstruction(
//         toString(target), toString(data), ec);
//     if (ec)
//         return;
// 
//     pi->setCreatedByParser(true);
// 
//     m_currentNode->deprecatedParserAddChild(pi.get());
//     if (m_view && !pi->attached())
//         pi->attach();
// 
//     pi->finishParsingChildren();
// 
//     if (pi->isCSS())
//         m_sawCSS = true;
// #if ENABLE(XSLT)
//     m_sawXSLTransform = !m_sawFirstElement && pi->isXSL();
//     if (m_sawXSLTransform && !document()->transformSourceDocument())
//         stopParsing();
// #endif
}

void XMLDocumentParser::cdataBlock(const XML_Char* s, int len)
{
    notImplemented();
//     if (isStopped())
//         return;
// 
//     if (m_parserPaused) {
//         m_pendingCallbacks->appendCDATABlockCallback(s, len);
//         return;
//     }
// 
//     exitText();
// 
//     RefPtr<Node> newNode = CDATASection::create(document(), toString(s, len));
//     m_currentNode->deprecatedParserAddChild(newNode.get());
//     if (m_view && !newNode->attached())
//         newNode->attach();
}

void XMLDocumentParser::comment(const XML_Char* s)
{
    if (isStopped())
        return;

    if (m_parserPaused) {
        m_pendingCallbacks->appendCommentCallback(s);
        return;
    }

    exitText();

    RefPtr<Node> newNode = Comment::create(document(), toString(s));
    m_currentNode->deprecatedParserAddChild(newNode.get());
    if (m_view && !newNode->attached())
        newNode->attach();
}

void XMLDocumentParser::startDocument(const XML_Char* version, const XML_Char* encoding, int standalone)
{
    ExceptionCode ec = 0;

//     if (version)
//         document()->setXMLVersion(toString(version), ec);
//     document()->setXMLStandalone(standalone == 1, ec); // possible values are 0, 1, and -1
//     if (encoding)
//         document()->setXMLEncoding(toString(encoding));
}

void XMLDocumentParser::endDocument()
{
    exitText();
#if ENABLE(XHTMLMP)
    m_hasDocTypeDeclaration = false;
#endif
}

void XMLDocumentParser::internalSubset(const XML_Char* name, const XML_Char* externalID, const XML_Char* systemID)
{
    if (isStopped())
        return;

    if (m_parserPaused) {
        m_pendingCallbacks->appendInternalSubsetCallback(name, externalID, systemID);
        return;
    }

    notImplemented();
//     if (document()) {
//         document()->parserAddChild(DocumentType::create(document(), toString(name), toString(externalID), toString(systemID)));
//     }
}

static inline XMLDocumentParser* getParser(void* closure)
{
    //xmlParserCtxtPtr ctxt = static_cast<xmlParserCtxtPtr>(closure);
    //return static_cast<XMLDocumentParser*>(ctxt->_private);
    return static_cast<XMLDocumentParser*>(closure);
}

// This is a hack around http://bugzilla.gnome.org/show_bug.cgi?id=159219
// Otherwise libxml seems to call all the SAX callbacks twice for any replaced entity.
static inline bool hackAroundLibXMLEntityBug(void* closure)
{
// #if LIBXML_VERSION >= 20627
//     UNUSED_PARAM(closure);
// 
//     // This bug has been fixed in libxml 2.6.27.
//     return false;
// #else
//     return static_cast<xmlParserCtxtPtr>(closure)->node;
// #endif
    return false;
}


static void XMLCALL StartNSDecl(void *closure, const XML_Char *prefix, const XML_Char *uri)
{
    if (hackAroundLibXMLEntityBug(closure))
        return;

    getParser(closure)->StartNSDecl(AtomicString(prefix), AtomicString(uri));

}

static void XMLCALL EndNSDecl(void *closure, const XML_Char *prefix)
{
}

static void XMLCALL startElementNsHandler(void *closure, const XML_Char *name,
                                  const XML_Char **atts)
{
    if (hackAroundLibXMLEntityBug(closure))
        return;

    getParser(closure)->preStartElementNs(name, atts);
}

static void XMLCALL endElementNsHandler(void *closure, const XML_Char *element)
{
    if (hackAroundLibXMLEntityBug(closure))
        return;

    getParser(closure)->endElementNs();
}

static void XMLCALL charactersHandler(void* closure, const XML_Char* s, int len)
{
    if (hackAroundLibXMLEntityBug(closure))
        return;

    getParser(closure)->characters(s, len);
}

static void XMLCALL processingInstructionHandler(void* closure, const XML_Char* target, const XML_Char* data)
{
    if (hackAroundLibXMLEntityBug(closure))
        return;

    getParser(closure)->processingInstruction(target, data);
}

static void cdataBlockHandler(void* closure, const XML_Char* s, int len)
{
    if (hackAroundLibXMLEntityBug(closure))
        return;

    getParser(closure)->cdataBlock(s, len);
}

static void commentHandler(void* closure, const XML_Char* comment)
{
    if (hackAroundLibXMLEntityBug(closure))
        return;

    getParser(closure)->comment(comment);
}

// WTF_ATTRIBUTE_PRINTF(2, 3)
// static void warningHandler(void* closure, const char* message, ...)
// {
//     va_list args;
//     va_start(args, message);
//     getParser(closure)->error(XMLDocumentParser::warning, message, args);
//     va_end(args);
// }
// 
// WTF_ATTRIBUTE_PRINTF(2, 3)
// static void fatalErrorHandler(void* closure, const char* message, ...)
// {
//     va_list args;
//     va_start(args, message);
//     getParser(closure)->error(XMLDocumentParser::fatal, message, args);
//     va_end(args);
// }
// 
// WTF_ATTRIBUTE_PRINTF(2, 3)
// static void normalErrorHandler(void* closure, const char* message, ...)
// {
//     va_list args;
//     va_start(args, message);
//     getParser(closure)->error(XMLDocumentParser::nonFatal, message, args);
//     va_end(args);
// }

// -----

PassRefPtr<XMLParserContext> XMLParserContext::createMemoryParser(void* userData, const CString& chunk)
{
    return 0;
}

PassRefPtr<XMLParserContext> XMLParserContext::createStringParser(void* userData)
{
    XML_Parser parser = XML_ParserCreateNS(NULL, NSSEP);
    XML_SetElementHandler(parser, startElementNsHandler, endElementNsHandler);
    XML_SetCharacterDataHandler(parser, charactersHandler);
    XML_SetProcessingInstructionHandler(parser, processingInstructionHandler);
    XML_SetNamespaceDeclHandler(parser, StartNSDecl, EndNSDecl);
    XML_SetUserData(parser, userData);

    return adoptRef(new XMLParserContext(parser));
}

XMLParserContext::XMLParserContext(XML_Parser parser)
: m_parser(parser)
{
    m_URIToPrefixMap.add(AtomicString("http://www.w3.org/XML/1998/namespace"), AtomicString("xml"));
}

// -----

// Using a static entity and marking it XML_INTERNAL_PREDEFINED_ENTITY is
// a hack to avoid malloc/free. Using a global variable like this could cause trouble
// if libxml implementation details were to change
// static XML_Char sharedXHTMLEntityResult[5] = {0, 0, 0, 0, 0};
// 
// static xmlEntityPtr sharedXHTMLEntity()
// {
//     static xmlEntity entity;
//     if (!entity.type) {
//         entity.type = XML_ENTITY_DECL;
//         entity.orig = sharedXHTMLEntityResult;
//         entity.content = sharedXHTMLEntityResult;
//         entity.etype = XML_INTERNAL_PREDEFINED_ENTITY;
//     }
//     return &entity;
// }
// 
// static xmlEntityPtr getXHTMLEntity(const XML_Char* name)
// {
//     UChar c = decodeNamedEntity(reinterpret_cast<const char*>(name));
//     if (!c)
//         return 0;
// 
//     CString value = String(&c, 1).utf8();
//     ASSERT(value.length() < 5);
//     xmlEntityPtr entity = sharedXHTMLEntity();
//     entity->length = value.length();
//     entity->name = name;
//     memcpy(sharedXHTMLEntityResult, value.data(), entity->length + 1);
// 
//     return entity;
// }
// 
// static xmlEntityPtr getEntityHandler(void* closure, const XML_Char* name)
// {
//     xmlParserCtxtPtr ctxt = static_cast<xmlParserCtxtPtr>(closure);
//     xmlEntityPtr ent = xmlGetPredefinedEntity(name);
//     if (ent) {
//         ent->etype = XML_INTERNAL_PREDEFINED_ENTITY;
//         return ent;
//     }
// 
//     ent = xmlGetDocEntity(ctxt->myDoc, name);
//     if (!ent && (getParser(closure)->isXHTMLDocument()
// #if ENABLE(XHTMLMP)
//                  || getParser(closure)->isXHTMLMPDocument()
// #endif
//        )) {
//         ent = getXHTMLEntity(name);
//         if (ent)
//             ent->etype = XML_INTERNAL_GENERAL_ENTITY;
//     }
// 
//     return ent;
// }

// static void startDocumentHandler(void* closure)
// {
//     xmlParserCtxt* ctxt = static_cast<xmlParserCtxt*>(closure);
//     //switchToUTF16(ctxt);
//     getParser(closure)->startDocument(ctxt->version, ctxt->encoding, ctxt->standalone);
//     //xmlSAX2StartDocument(closure);
// }
// 
// static void endDocumentHandler(void* closure)
// {
//     getParser(closure)->endDocument();
//     //xmlSAX2EndDocument(closure);
// }
// 
// static void internalSubsetHandler(void* closure, const XML_Char* name, const XML_Char* externalID, const XML_Char* systemID)
// {
//     getParser(closure)->internalSubset(name, externalID, systemID);
//     xmlSAX2InternalSubset(closure, name, externalID, systemID);
// }
// 
// static void externalSubsetHandler(void* closure, const XML_Char*, const XML_Char* externalId, const XML_Char*)
// {
//     String extId = toString(externalId);
//     if ((extId == "-//W3C//DTD XHTML 1.0 Transitional//EN")
//         || (extId == "-//W3C//DTD XHTML 1.1//EN")
//         || (extId == "-//W3C//DTD XHTML 1.0 Strict//EN")
//         || (extId == "-//W3C//DTD XHTML 1.0 Frameset//EN")
//         || (extId == "-//W3C//DTD XHTML Basic 1.0//EN")
//         || (extId == "-//W3C//DTD XHTML 1.1 plus MathML 2.0//EN")
//         || (extId == "-//W3C//DTD XHTML 1.1 plus MathML 2.0 plus SVG 1.1//EN")
//         || (extId == "-//WAPFORUM//DTD XHTML Mobile 1.0//EN")
//        )
//         getParser(closure)->setIsXHTMLDocument(true); // controls if we replace entities or not.
// }
// 
// static void ignorableWhitespaceHandler(void*, const XML_Char*, int)
// {
//     // nothing to do, but we need this to work around a crasher
//     // http://bugzilla.gnome.org/show_bug.cgi?id=172255
//     // http://bugs.webkit.org/show_bug.cgi?id=5792
// }

void XMLDocumentParser::initializeParserContext(const CString& chunk)
{
    DocumentParser::startParsing();
    m_sawError = false;
    m_sawCSS = false;
    m_sawXSLTransform = false;
    m_sawFirstElement = false;

    XMLDocumentParserScope scope(document()->cachedResourceLoader());
    if (m_parsingFragment)
        m_context = XMLParserContext::createMemoryParser(this, chunk);
    else {
        ASSERT(!chunk.data());
        m_context = XMLParserContext::createStringParser(this);
   }
}

void XMLDocumentParser::doEnd()
{
    if (!isStopped()) {
        if (m_context) {
            // Tell libxml we're done.
            {
                //notImplemented();
                //XMLDocumentParserScope scope(document()->cachedResourceLoader());
                //xmlParseChunk(context(), 0, 0, 1);
            }

            m_context = 0;
        }
    }
}

int XMLDocumentParser::lineNumber() const
{
    // FIXME: The implementation probably returns 1-based int, but method should return 0-based.
    //return context() ? context()->input->line : 1;
    return XML_GetCurrentLineNumber(m_context->getParser());
}

int XMLDocumentParser::columnNumber() const
{
    // FIXME: The implementation probably returns 1-based int, but method should return 0-based.
    //return context() ? context()->input->col : 1;
    return XML_GetCurrentColumnNumber(m_context->getParser());
}

TextPosition0 XMLDocumentParser::textPosition() const
{
//     xmlParserCtxtPtr context = this->context();
//     if (!context)
//         return TextPosition0::minimumPosition();
//     // FIXME: The context probably contains 1-based numbers, but we treat them as 0-based,
//     //        to be consistent with fixme's in lineNumber() and columnNumber
//     //        methods.
//     return TextPosition0(WTF::ZeroBasedNumber::fromZeroBasedInt(context->input->line),
//         WTF::ZeroBasedNumber::fromZeroBasedInt(context->input->col));

    if (!this->m_context)
        return TextPosition0::minimumPosition();
    // FIXME: The context probably contains 1-based numbers, but we treat them as 0-based,
    //        to be consistent with fixme's in lineNumber() and columnNumber
    //        methods.
    return TextPosition0(WTF::ZeroBasedNumber::fromZeroBasedInt(this->lineNumber()),
        WTF::ZeroBasedNumber::fromZeroBasedInt(this->columnNumber()));
}

// This method has a correct implementation, in contrast to textPosition() method.
// It should replace textPosition().
TextPosition1 XMLDocumentParser::textPositionOneBased() const
{
//     xmlParserCtxtPtr context = this->context();
//     if (!context)
//         return TextPosition1::minimumPosition();
//     return TextPosition1(WTF::OneBasedNumber::fromOneBasedInt(context->input->line),
//         WTF::OneBasedNumber::fromOneBasedInt(context->input->col));

    if (!this->m_context)
        return TextPosition1::minimumPosition();
    return TextPosition1(WTF::OneBasedNumber::fromOneBasedInt(this->lineNumber()),
        WTF::OneBasedNumber::fromOneBasedInt(this->columnNumber()));
}

void XMLDocumentParser::stopParsing()
{
    DocumentParser::stopParsing();
//     if (m_context)
//         xmlStopParser(m_context);
    if (m_context)
    { XML_StopParser(m_context->getParser(), 0); }
}

void XMLDocumentParser::resumeParsing()
{
    ASSERT(!isDetached());
    ASSERT(m_parserPaused);

    m_parserPaused = false;

    // First, execute any pending callbacks
    while (!m_pendingCallbacks->isEmpty()) {
        m_pendingCallbacks->callAndRemoveFirstCallback(this);

        // A callback paused the parser
        if (m_parserPaused)
            return;
    }

    // Then, write any pending data
    SegmentedString rest = m_pendingSrc;
    m_pendingSrc.clear();
    append(rest);

    // Finally, if finish() has been called and write() didn't result
    // in any further callbacks being queued, call end()
    if (m_finishCalled && m_pendingCallbacks->isEmpty())
        end();
}

bool XMLDocumentParser::appendFragmentSource(const String& chunk)
{
    notImplemented();
    return false;
//     ASSERT(!m_context);
//     ASSERT(m_parsingFragment);
// 
//     CString chunkAsUtf8 = chunk.utf8();
//     
//     // libxml2 takes an int for a length, and therefore can't handle XML chunks larger than 2 GiB.
//     if (chunkAsUtf8.length() > INT_MAX)
//         return false;
// 
//     initializeParserContext(chunkAsUtf8);
//     xmlParseContent(context());
//     endDocument(); // Close any open text nodes.
// 
//     // FIXME: If this code is actually needed, it should probably move to finish()
//     // XMLDocumentParserQt has a similar check (m_stream.error() == QXmlStreamReader::PrematureEndOfDocumentError) in doEnd().
//     // Check if all the chunk has been processed.
//     long bytesProcessed = xmlByteConsumed(context());
//     if (bytesProcessed == -1 || ((unsigned long)bytesProcessed) != chunkAsUtf8.length()) {
//         // FIXME: I don't believe we can hit this case without also having seen an error or a null byte.
//         // If we hit this ASSERT, we've found a test case which demonstrates the need for this code.
//         ASSERT(m_sawError || (bytesProcessed >= 0 && !chunkAsUtf8.data()[bytesProcessed]));
//         return false;
//     }
// 
//     // No error if the chunk is well formed or it is not but we have no error.
//     return context()->wellFormed || !xmlCtxtGetLastError(context());
}

// --------------------------------

struct AttributeParseState {
    HashMap<String, String> attributes;
    bool gotAttributes;
};

static void attributesStartElementNsHandler(void* closure, const XML_Char* xmlLocalName, const XML_Char* /*xmlPrefix*/,
                                            const XML_Char* /*xmlURI*/, int /*nb_namespaces*/, const XML_Char** /*namespaces*/,
                                            int nb_attributes, int /*nb_defaulted*/, const XML_Char** libxmlAttributes)
{
//     if (strcmp(reinterpret_cast<const char*>(xmlLocalName), "attrs") != 0)
//         return;
// 
//     xmlParserCtxtPtr ctxt = static_cast<xmlParserCtxtPtr>(closure);
//     AttributeParseState* state = static_cast<AttributeParseState*>(ctxt->_private);
// 
//     state->gotAttributes = true;
// 
//     xmlSAX2Attributes* attributes = reinterpret_cast<xmlSAX2Attributes*>(libxmlAttributes);
//     for (int i = 0; i < nb_attributes; i++) {
//         String attrLocalName = toString(attributes[i].localname);
//         int valueLength = (int) (attributes[i].end - attributes[i].value);
//         String attrValue = toString(attributes[i].value, valueLength);
//         String attrPrefix = toString(attributes[i].prefix);
//         String attrQName = attrPrefix.isEmpty() ? attrLocalName : attrPrefix + ":" + attrLocalName;
// 
//         state->attributes.set(attrQName, attrValue);
//     }
    notImplemented();
}

HashMap<String, String> parseAttributes(const String& string, bool& attrsOK)
{
    AttributeParseState state;
    notImplemented();
//     state.gotAttributes = false;
// 
//     xmlSAXHandler sax;
//     memset(&sax, 0, sizeof(sax));
//     sax.startElementNs = attributesStartElementNsHandler;
//     sax.initialized = XML_SAX2_MAGIC;
//     RefPtr<XMLParserContext> parser = XMLParserContext::createStringParser(&sax, &state);
//     String parseString = "<?xml version=\"1.0\"?><attrs " + string + " />";
//     xmlParseChunk(parser->context(), reinterpret_cast<const char*>(parseString.characters()), parseString.length() * sizeof(UChar), 1);
//     attrsOK = state.gotAttributes;
    return state.attributes;
}

}
