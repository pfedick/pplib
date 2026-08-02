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

#ifndef PPL7_CORE_THREADS_H_
#define PPL7_CORE_THREADS_H_

#include <stdint.h>
#include <set>
#include <ppl7/core/mutex.h>

namespace ppl7
{

class Thread
{
private:
    Mutex threadmutex;
    void* threaddata;
    size_t runcount;
    int flags;
    int IsRunning;
    int IsSuspended;
    int deleteMe;
    int myPriority;

public:
    enum Priority
    {
        UNKNOWN = 0,
        LOWEST,
        BELOW_NORMAL,
        NORMAL,
        ABOVE_NORMAL,
        HIGHEST
    };

    Thread();
    virtual ~Thread();
    void threadSetName(const char* name);
    void threadSuspend();
    void threadResume();
    void threadStop();
    void threadSignalStop();
    void threadStart();
    void threadStartUp();
    int threadIsRunning();
    int threadIsSuspended();
    int threadGetFlags();
    size_t threadRunCount();
    uint64_t threadGetID();
    int threadShouldStop();
    void threadWaitSuspended(int msec = 0);
    void threadSleep(int msec = 0);
    void threadDeleteOnExit(int flag = 1);
    int threadShouldDeleteOnExit();
    int threadSetPriority(int priority);
    int threadGetPriority();
    void threadIdle();
    int threadSetStackSize(size_t size = 0);
    size_t threadGetStackSize();
    size_t threadGetMinimumStackSize();
    void threadJoin();

    virtual void run();
};

uint64_t ThreadID();
void* GetTLSData();
void SetTLSData(void* data);
uint64_t StartThread(void (*start_routine)(void*), void* data = NULL);
void ThreadSetPriority(Thread::Priority priority);
Thread::Priority ThreadGetPriority();

} // namespace ppl7

#endif /* PPL7_CORE_THREADS_H_ */