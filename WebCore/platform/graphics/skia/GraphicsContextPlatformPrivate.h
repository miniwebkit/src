/*
 * Copyright (c) 2008, Google Inc. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef GraphicsContextPlatformPrivate_h
#define GraphicsContextPlatformPrivate_h


class PlatformContextSkia;

namespace WebCore {

// This class just holds onto a PlatformContextSkia for GraphicsContext.
class GraphicsContextPlatformPrivate {
    WTF_MAKE_NONCOPYABLE(GraphicsContextPlatformPrivate);
public:
    GraphicsContextPlatformPrivate(PlatformContextSkia* platformContext)
        : m_context(platformContext)
        , m_windowsIsTransparencyLayer(false) { }

    PlatformContextSkia* context() { return m_context; }

    //////////////////////////////////////////////////////////////////////////
#if PLATFORM(WIN)
    // On Windows, we need to update the HDC for form controls to draw in the right place.
    void save();
    void restore();
    void flush();
    void clip(const FloatRect&);
    void clip(const Path&);
    void scale(const FloatSize&);
    void rotate(float);
    void translate(float, float);
    void concatCTM(const AffineTransform&);
    void setCTM(const AffineTransform&);

    HDC m_hdc;
    bool m_shouldIncludeChildWindows;
#endif

    void beginTransparencyLayer()
    {
#if PLATFORM(WIN) || !ASSERT_DISABLED
        m_transparencyCount++;
#endif
    }
    void endTransparencyLayer()
    {
#if PLATFORM(WIN) || !ASSERT_DISABLED
        ASSERT(m_transparencyCount > 0);
        m_transparencyCount--;
#endif
    }

#if PLATFORM(WIN) || !ASSERT_DISABLED
    int m_transparencyCount;
    bool m_windowsIsTransparencyLayer;
#endif
    //-----------

private:
    // Non-owning pointer to the PlatformContext.
    PlatformContextSkia* m_context;
};

}  // namespace WebCore

#endif  // GraphicsContextPlatformPrivate_h
