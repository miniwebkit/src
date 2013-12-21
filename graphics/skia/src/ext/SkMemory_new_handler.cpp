// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <new>

#include "include/core/SkTypes.h"
#include "include/core/SkThread.h"

// This implementation of sk_malloc_flags() and friends is identical
// to SkMemory_malloc.c, except that it disables the CRT's new_handler
// during malloc(), when SK_MALLOC_THROW is not set (ie., when
// sk_malloc_flags() would not abort on NULL).

namespace WTF {
    void* fastMalloc(size_t);
    void* fastZeroedMalloc(size_t);
    void* fastCalloc(size_t numElements, size_t elementSize);
    void* fastRealloc(void*, size_t);
    char* fastStrDup(const char*);
    size_t fastMallocSize(const void*);
    void fastFree(void*);
} // WTF

static SkMutex gSkNewHandlerMutex;

void sk_throw() {
    SkASSERT(!"sk_throw");
    abort();
}

void sk_out_of_memory(void) {
    SkASSERT(!"sk_out_of_memory");
    abort();
}

void* sk_malloc_throw(size_t size) {
    return sk_malloc_flags(size, SK_MALLOC_THROW);
}

void* sk_realloc_throw(void* addr, size_t size) {
    //void* p = realloc(addr, size);
    void* p = WTF::fastRealloc(addr, size);
    if (size == 0) {
        return p;
    }
    if (p == NULL) {
        sk_throw();
    }
    return p;
}

void sk_free(void* p) {
    if (p) {
        //free(p);
        WTF::fastFree(p);
    }
}

void* sk_malloc_flags(size_t size, unsigned flags) {
    void* p;
#if defined(ANDROID)
    // Android doesn't have std::set_new_handler.
    p = malloc(size);
#else
    if (!(flags & SK_MALLOC_THROW)) {
      SkAutoMutexAcquire lock(gSkNewHandlerMutex);
      std::new_handler old_handler = std::set_new_handler(NULL);
      //p = malloc(size);
      p = WTF::fastMalloc(size);
      std::set_new_handler(old_handler);
    } else {
      //p = malloc(size);
        p = WTF::fastMalloc(size);
    }
#endif
    if (p == NULL) {
        if (flags & SK_MALLOC_THROW) {
            sk_throw();
        }
    }
    return p;
}
