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

#include <gtest/gtest.h>
#include <pplib.h>
#include <atomic>
#include <chrono>

namespace
{

class SimpleWorkerThread : public pplib::Thread
{
public:
    std::atomic<bool> executed{false};
    std::atomic<int> counter{0};

    void run() override
    {
        executed = true;
        while (!threadShouldStop()) {
            pplib::MSleep(10);
            counter++;
            if (counter > 5) break;
        }
    }
};

TEST(ThreadTest, StartAndJoin)
{
    SimpleWorkerThread t;
    EXPECT_FALSE(t.threadIsRunning());
    EXPECT_EQ(t.threadRunCount(), 0u);

    t.threadStart();
    EXPECT_TRUE(t.threadIsRunning() || t.executed);

    t.threadJoin();
    EXPECT_FALSE(t.threadIsRunning());
    EXPECT_TRUE(t.executed);
    EXPECT_GE(t.counter.load(), 1);
    EXPECT_EQ(t.threadRunCount(), 1u);
}

TEST(ThreadTest, SignalStop)
{
    SimpleWorkerThread t;
    t.threadStart();
    pplib::MSleep(20);
    t.threadSignalStop();
    EXPECT_TRUE(t.threadShouldStop());
    t.threadJoin();
    EXPECT_FALSE(t.threadIsRunning());
}

TEST(ThreadTest, ThreadNameAndId)
{
    SimpleWorkerThread t;
    t.threadSetName("TestWorker");
    EXPECT_EQ(t.threadGetName(), "TestWorker");

    t.threadStart();
    EXPECT_GT(t.threadGetID(), 0u);
    t.threadStop();
}

TEST(ThreadTest, StartThreadFunction)
{
    std::atomic<bool> called{false};
    uint64_t tid = pplib::StartThread([&called]() { called = true; });

    EXPECT_GT(tid, 0u);
    for (int i = 0; i < 50 && !called; ++i) {
        pplib::MSleep(10);
    }
    EXPECT_TRUE(called);
}

TEST(ThreadEventTest, SetAndWaitAutoReset)
{
    pplib::ThreadEvent ev(true, false);
    EXPECT_FALSE(ev.isSignaled());

    ev.set();
    EXPECT_TRUE(ev.wait(100));
    // Auto-reset: nach erfolgreichem wait wieder false
    EXPECT_FALSE(ev.isSignaled());
    EXPECT_FALSE(ev.wait(10));
}

TEST(ThreadEventTest, ManualReset)
{
    pplib::ThreadEvent ev(false, false);
    ev.set();
    EXPECT_TRUE(ev.wait(10));
    EXPECT_TRUE(ev.wait(10)); // bleibt gesetzt
    ev.reset();
    EXPECT_FALSE(ev.wait(10));
}

TEST(MutexTest, LockUnlock)
{
    pplib::Mutex m;
    m.lock();
    EXPECT_FALSE(m.tryLock());
    m.unlock();
    EXPECT_TRUE(m.tryLock());
    m.unlock();

    {
        pplib::MutexLock guard(m);
        EXPECT_FALSE(m.tryLock());
    }
    EXPECT_TRUE(m.tryLock());
    m.unlock();
}

} // namespace
