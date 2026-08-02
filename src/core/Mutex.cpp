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

#include <ppl7/core/mutex.h>
#include <ppl7/exceptions.h>

#include <mutex>
#include <condition_variable>
#include <chrono>

namespace ppl7
{

typedef struct tagMutex
{
    std::mutex mtx;
    std::condition_variable cv;
    bool signaled{false};
} PPLMUTEX;

Mutex::Mutex()
{
    handle = new (std::nothrow) PPLMUTEX();
    if (!handle) throw OutOfMemoryException();
}

Mutex::~Mutex() noexcept
{
    if (handle) {
        PPLMUTEX* h = static_cast<PPLMUTEX*>(handle);
        delete h;
        handle = nullptr;
    }
}

void Mutex::lock()
{
    PPLMUTEX* h = static_cast<PPLMUTEX*>(handle);
    try {
        h->mtx.lock();
    }
    catch (...) {
        throw MutexLockingException("Mutex::lock");
    }
}

void Mutex::unlock()
{
    PPLMUTEX* h = static_cast<PPLMUTEX*>(handle);
    try {
        h->mtx.unlock();
    }
    catch (...) {
        throw MutexLockingException("Mutex::unlock");
    }
}

bool Mutex::tryLock() noexcept
{
    PPLMUTEX* h = static_cast<PPLMUTEX*>(handle);
    return h->mtx.try_lock();
}

bool Mutex::wait(int milliseconds) noexcept
{
    if (!handle) return false;
    PPLMUTEX* h = static_cast<PPLMUTEX*>(handle);
    try {
        std::unique_lock<std::mutex> lock(h->mtx);

        auto predicate = [h]() { return h->signaled; };

        bool success = false;
        if (milliseconds > 0) {
            success = h->cv.wait_for(lock, std::chrono::milliseconds(milliseconds), predicate);
        } else {
            h->cv.wait(lock, predicate);
            success = true;
        }

        if (success) {
            h->signaled = false; // Event für die nächste Benutzung zurücksetzen
            return true;
        }
        return false;
    }
    catch (...) {
        return false;
    }
}

bool Mutex::signal() noexcept
{
    if (!handle) return false;
    PPLMUTEX* h = static_cast<PPLMUTEX*>(handle);
    try {
        {
            std::lock_guard<std::mutex> lock(h->mtx);
            h->signaled = true;
        }
        h->cv.notify_one();
        return true;
    }
    catch (...) {
        return false;
    }
}

} // namespace ppl7
