/*
* Copyright (c) 2010, Google Inc. All rights reserved.
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

#include "config.h"

#include "GraphicsContextGPU.h"

#include "DrawingBuffer.h"
#include "FloatRect.h"
#include "FloatSize.h"
#include "GraphicsContext3D.h"
//#include "internal_glu.h"
#include "IntRect.h"
#include "LoopBlinnMathUtils.h"
#include "LoopBlinnPathProcessor.h"
#include "LoopBlinnSolidFillShader.h"
#include "Path.h"
#include "PlatformString.h"
#include "SharedGraphicsContext3D.h"
#if USE(SKIA)
#include "SkPath.h"
#endif
#include "Texture.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <wtf/OwnArrayPtr.h>
#include <wtf/text/CString.h>

namespace WebCore {
void GraphicsContextGPU::save()
{
    __asm int 3; // weolar
}

void GraphicsContextGPU::restore()
{
    __asm int 3; // weolar
}
void GraphicsContextGPU::clearRect(const FloatRect& rect)
{
    __asm int 3; // weolar
}

void GraphicsContextGPU::clipPath(const Path& path)
{
    __asm int 3; // weolar
}


void GraphicsContextGPU::clipOut(const Path& path)
{
    ASSERT(!"clipOut is unsupported in GraphicsContextGPU.\n");
}
void GraphicsContextGPU::concatCTM(const AffineTransform& affine)
{
    __asm int 3; // weolar
}

void GraphicsContextGPU::setCTM(const AffineTransform& affine)
{
    __asm int 3; // weolar
}
void GraphicsContextGPU::fillPath(const Path& path)
{
    __asm int 3; // weolar
}
void GraphicsContextGPU::fillRect(const FloatRect& rect, const Color& color, ColorSpace colorSpace)
{
    __asm int 3; // weolar
}

void GraphicsContextGPU::fillRect(const FloatRect& rect)
{
    __asm int 3; // weolar
}
void GraphicsContextGPU::scale(const FloatSize& size)
{
    __asm int 3; // weolar
}
void GraphicsContextGPU::setAlpha(float alpha)
{
    __asm int 3; // weolar
}
void GraphicsContextGPU::setCompositeOperation(CompositeOperator op)
{
    __asm int 3; // weolar
}
void GraphicsContextGPU::setFillColor(const Color& color, ColorSpace colorSpace)
{
    __asm int 3; // weolar
}
void GraphicsContextGPU::rotate(float angleInRadians)
{
    __asm int 3; // weolar
}
void GraphicsContextGPU::translate(float x, float y)
{
    __asm int 3; // weolar
}
void GraphicsContextGPU::drawTexturedRect(unsigned texture, const IntSize& textureSize, const FloatRect& srcRect, const FloatRect& dstRect, ColorSpace colorSpace, CompositeOperator compositeOp)
{
    __asm int 3; // weolar
}

} // namespace WebCore