/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLIB).
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

#include <pplib/core/thread.h>
#include <pplib/core/mutex.h>
#include <pplib/core/threadpool.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>

namespace pplib
{

ThreadPool::~ThreadPool()
{
    stopThreads();
    destroyAllThreads();
}

void ThreadPool::addThread(Thread* thread)
{
    if (!thread) throw IllegalArgumentException("ThreadPool::addThread");
    thread->threadDeleteOnExit(false);
    MutexLock lock(mutex);
    std::pair<std::set<Thread*>::iterator, bool> ret;
    ret = threads.insert(thread);
    if (ret.second == false) throw ThreadAlreadyInPoolException();
}

void ThreadPool::removeThread(Thread* thread)
{
    if (!thread) throw IllegalArgumentException("ThreadPool::removeThread");
    MutexLock lock(mutex);
    threads.erase(thread);
}

void ThreadPool::destroyThread(Thread* thread)
{
    if (!thread) throw IllegalArgumentException("ThreadPool::destroyThread");
    MutexLock lock(mutex);
    threads.erase(thread);
    delete thread;
}

void ThreadPool::destroyAllThreads()
{
    stopThreads();
    MutexLock lock(mutex);
    std::set<Thread*>::iterator it;
    for (it = threads.begin(); it != threads.end(); ++it) {
        delete (*it);
    }
    threads.clear();
}

void ThreadPool::signalStopThreads()
{
    std::set<Thread*>::iterator it;
    MutexLock lock(mutex);
    for (it = threads.begin(); it != threads.end(); ++it) {
        (*it)->threadSignalStop();
    }
}

void ThreadPool::stopThreads()
{
    signalStopThreads();
    std::vector<Thread*> copy;
    {
        MutexLock lock(mutex);
        copy.assign(threads.begin(), threads.end());
    }
    for (auto* t : copy) {
        t->threadJoin();
    }
}

void ThreadPool::startThreads()
{
    std::set<Thread*>::iterator it;
    MutexLock lock(mutex);
    for (it = threads.begin(); it != threads.end(); ++it) {
        if ((*it)->threadIsRunning() == false) {
            (*it)->threadStart();
        }
    }
}

size_t ThreadPool::size() const
{
    MutexLock lock(mutex);
    size_t num = threads.size();
    return num;
}

size_t ThreadPool::count_running() const
{
    std::set<Thread*>::const_iterator it;
    size_t count = 0;
    MutexLock lock(mutex);
    for (it = threads.begin(); it != threads.end(); ++it) {
        if ((*it)->threadIsRunning()) count++;
    }
    return count;
}

bool ThreadPool::running() const
{
    std::set<pplib::Thread*>::const_iterator it;
    MutexLock lock(mutex);
    for (it = threads.begin(); it != threads.end(); ++it) {
        if ((*it)->threadIsRunning()) {
            return true;
        }
    }
    return false;
}

} // namespace pplib
