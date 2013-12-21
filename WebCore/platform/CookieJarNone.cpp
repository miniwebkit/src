
#include "CookieJar.h"

namespace WebCore {
    String cookies(const Document*, const KURL&) 
    { return String(); }

    void setCookies(Document*, const KURL&, const String&) 
    { return; }

    bool cookiesEnabled(const Document*)
    { return false; }
}