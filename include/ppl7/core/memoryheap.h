/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: https://github.com/pfedick/pplib
 *******************************************************************************
 * Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#ifndef PPL7_CORE_MEMORYHEAP_H_
#define PPL7_CORE_MEMORYHEAP_H_

#include <ppl7/types/bytearray.h>
#include <ppl7/types/bytearrayptr.h>
#include <ppl7/exceptions.h>

namespace ppl7
{
class MemoryHeap
{
private:
    void* blocks;
    size_t myElementSize, increaseSize;
    size_t myGrowPercent;
    size_t blocksAllocated, blocksUsed;
    size_t mem_allocated;
    size_t mem_used;
    size_t freeCount;

    void increase(size_t num);

public:
    PPL7EXCEPTION(NotInitializedException, Exception);
    PPL7EXCEPTION(AlreadyInitializedException, Exception);
    PPL7EXCEPTION(HeapCorruptedException, Exception);
    PPL7EXCEPTION(ElementNotInHeapException, Exception);

    MemoryHeap();
    MemoryHeap(size_t elementsize, size_t startnum, size_t increase, size_t growpercent = 30);
    ~MemoryHeap();
    void clear();
    void init(size_t elementsize, size_t startnum, size_t increase, size_t growpercent = 30);
    void* malloc();
    void* calloc();
    void free(void* element);
    size_t memoryUsed() const;
    size_t memoryAllocated() const;
    void dump() const;
    size_t capacity() const;
    size_t count() const;
    size_t elementSize() const;
    void reserve(size_t num);
    void cleanup();
};

} // namespace ppl7

#endif // PPL7_CORE_MEMORYHEAP_H_
