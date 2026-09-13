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
#include <pplib/core/thread.h>
#include <pplib/core/threadpool.h>
#include <pplib/core/functions.h>
#include <pplib/exceptions.h>
#include <atomic>

namespace
{

class PoolWorker : public pplib::Thread
{
public:
    std::atomic<bool> executed{false};
    std::atomic<bool> was_destroyed{false};

    void run() override
    {
        executed = true;
        while (!threadShouldStop()) {
            threadSleep(10);
        }
    }
};

TEST(ThreadPoolTest, AddAndSize)
{
    pplib::ThreadPool pool;
    EXPECT_EQ(pool.size(), 0u);
    EXPECT_FALSE(pool.running());
    EXPECT_EQ(pool.count_running(), 0u);

    auto* t1 = new PoolWorker();
    auto* t2 = new PoolWorker();

    pool.addThread(t1);
    EXPECT_EQ(pool.size(), 1u);

    pool.addThread(t2);
    EXPECT_EQ(pool.size(), 2u);

    // Duplikat darf Exception werfen
    EXPECT_THROW(pool.addThread(t1), pplib::ThreadAlreadyInPoolException);

    // nullptr prüfen
    EXPECT_THROW(pool.addThread(nullptr), pplib::IllegalArgumentException);
    EXPECT_THROW(pool.removeThread(nullptr), pplib::IllegalArgumentException);
    EXPECT_THROW(pool.destroyThread(nullptr), pplib::IllegalArgumentException);

    pool.destroyAllThreads();
    EXPECT_EQ(pool.size(), 0u);
}

TEST(ThreadPoolTest, StartAndStopThreads)
{
    pplib::ThreadPool pool;
    auto* t1 = new PoolWorker();
    auto* t2 = new PoolWorker();

    pool.addThread(t1);
    pool.addThread(t2);

    pool.startThreads();
    // Nochmal aufrufen (darf bereits laufende Threads ignorieren)
    pool.startThreads();

    EXPECT_TRUE(pool.running());
    for (int i = 0; i < 50 && pool.count_running() < 2; ++i) {
        pplib::MSleep(10);
    }
    EXPECT_EQ(pool.count_running(), 2u);

    // Snapshot der Threads
    auto snapshot = pool.getThreads();
    EXPECT_EQ(snapshot.size(), 2u);

    pool.stopThreads();
    EXPECT_FALSE(pool.running());
    EXPECT_EQ(pool.count_running(), 0u);

    pool.destroyAllThreads();
    EXPECT_EQ(pool.size(), 0u);
}

TEST(ThreadPoolTest, RemoveAndDestroyThread)
{
    pplib::ThreadPool pool;
    auto* t1 = new PoolWorker();
    auto* t2 = new PoolWorker();

    pool.addThread(t1);
    pool.addThread(t2);

    // destroyThread löscht t1 und entfernt ihn
    pool.destroyThread(t1);
    EXPECT_EQ(pool.size(), 1u);

    // removeThread entfernt t2, caller übernimmt Ownership
    pool.removeThread(t2);
    EXPECT_EQ(pool.size(), 0u);
    delete t2;
}

TEST(ThreadPoolTest, DestructorStopsAndCleansUp)
{
    auto* t = new PoolWorker();
    {
        pplib::ThreadPool pool;
        pool.addThread(t);
        pool.startThreads();
        EXPECT_TRUE(pool.running());
    }
    // Nach Verlassen des Scopes hat ~ThreadPool() stopThreads() und destroyAllThreads() gerufen
    // Der Thread wurde sauber beendet und per delete freigegeben.
}

} // namespace
