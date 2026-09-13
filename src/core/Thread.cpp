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
#include <pplib/core/functions.h>
#include <pplib/exceptions.h>

#include <atomic>
#include <system_error>

#if defined(_WIN32)
#define _WINSOCKAPI_
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <pthread.h>
#include <sched.h>
#endif

namespace pplib
{

static std::atomic<uint64_t> global_thread_counter{1};
static thread_local uint64_t local_thread_id = 0;

uint64_t ThreadID()
{
    if (local_thread_id == 0) {
        local_thread_id = global_thread_counter.fetch_add(1, std::memory_order_relaxed);
    }
    return local_thread_id;
}

static void applyPriorityToNativeHandle(std::thread::native_handle_type handle, Thread::Priority priority)
{
#if defined(_WIN32)
    int p = THREAD_PRIORITY_NORMAL;
    switch (priority) {
    case Thread::Priority::LOWEST:
        p = THREAD_PRIORITY_LOWEST;
        break;
    case Thread::Priority::BELOW_NORMAL:
        p = THREAD_PRIORITY_BELOW_NORMAL;
        break;
    case Thread::Priority::NORMAL:
        p = THREAD_PRIORITY_NORMAL;
        break;
    case Thread::Priority::ABOVE_NORMAL:
        p = THREAD_PRIORITY_ABOVE_NORMAL;
        break;
    case Thread::Priority::HIGHEST:
        p = THREAD_PRIORITY_HIGHEST;
        break;
    default:
        return;
    }
    SetThreadPriority(reinterpret_cast<HANDLE>(handle), p);
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
    int policy = 0;
    sched_param param{};
    if (pthread_getschedparam(handle, &policy, &param) == 0) {
        int min_prio = sched_get_priority_min(policy);
        int max_prio = sched_get_priority_max(policy);
        int normal = (min_prio + max_prio) / 2;
        switch (priority) {
        case Thread::Priority::LOWEST:
            param.sched_priority = min_prio;
            break;
        case Thread::Priority::BELOW_NORMAL:
            param.sched_priority = (min_prio + normal) / 2;
            break;
        case Thread::Priority::NORMAL:
            param.sched_priority = normal;
            break;
        case Thread::Priority::ABOVE_NORMAL:
            param.sched_priority = (normal + max_prio) / 2;
            break;
        case Thread::Priority::HIGHEST:
            param.sched_priority = max_prio;
            break;
        default:
            return;
        }
        pthread_setschedparam(handle, policy, &param);
    }
#endif
}

static void applyNameToNativeHandle([[maybe_unused]] std::thread::native_handle_type handle, [[maybe_unused]] const String& threadName)
{
#if defined(__linux__)
    if (!threadName.isEmpty()) {
        pthread_setname_np(handle, threadName.toCString());
    }
#elif defined(__APPLE__)
    // Auf macOS kann nur der eigene Thread benannt werden (in run/startup)
#elif defined(_WIN32)
    // Optional SetThreadDescription auf Windows 10+
#endif
}

void ThreadSetPriority(Thread::Priority priority)
{
#if defined(_WIN32)
    applyPriorityToNativeHandle(reinterpret_cast<std::thread::native_handle_type>(GetCurrentThread()), priority);
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
    applyPriorityToNativeHandle(pthread_self(), priority);
#endif
}

Thread::Priority ThreadGetPriority()
{
#if defined(_WIN32)
    int p = GetThreadPriority(GetCurrentThread());
    switch (p) {
    case THREAD_PRIORITY_LOWEST:
        return Thread::Priority::LOWEST;
    case THREAD_PRIORITY_BELOW_NORMAL:
        return Thread::Priority::BELOW_NORMAL;
    case THREAD_PRIORITY_NORMAL:
        return Thread::Priority::NORMAL;
    case THREAD_PRIORITY_ABOVE_NORMAL:
        return Thread::Priority::ABOVE_NORMAL;
    case THREAD_PRIORITY_HIGHEST:
        return Thread::Priority::HIGHEST;
    default:
        return Thread::Priority::UNKNOWN;
    }
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
    int policy = 0;
    sched_param param{};
    if (pthread_getschedparam(pthread_self(), &policy, &param) != 0) {
        return Thread::Priority::UNKNOWN;
    }
    int min_prio = sched_get_priority_min(policy);
    int max_prio = sched_get_priority_max(policy);
    int normal = (min_prio + max_prio) / 2;

    if (param.sched_priority == normal) return Thread::Priority::NORMAL;
    if (param.sched_priority == min_prio) return Thread::Priority::LOWEST;
    if (param.sched_priority == max_prio) return Thread::Priority::HIGHEST;
    if (param.sched_priority < normal) return Thread::Priority::BELOW_NORMAL;
    if (param.sched_priority > normal) return Thread::Priority::ABOVE_NORMAL;
    return Thread::Priority::UNKNOWN;
#else
    return Thread::Priority::UNKNOWN;
#endif
}

uint64_t StartThread(std::function<void()> func)
{
    uint64_t assigned_id = global_thread_counter.fetch_add(1, std::memory_order_relaxed);
    std::thread t([func, assigned_id]() {
        local_thread_id = assigned_id;
        try {
            func();
        }
        catch (...) {
            // Exceptions unterdrücken, um std::terminate zu verhindern
        }
    });
    t.detach();
    return assigned_id;
}

uint64_t StartThread(void (*start_routine)(void*), void* data)
{
    return StartThread([start_routine, data]() {
        if (start_routine) {
            start_routine(data);
        }
    });
}

Thread::Thread()
    : stop_event(true, false) // auto-reset = true, initialState = false
{
}

Thread::~Thread()
{
    try {
        threadStop();
    }
    catch (...) {
    }
}

void Thread::threadSetName(const String& threadName)
{
    std::lock_guard<Mutex> lock(mtx);
    name = threadName;
    if (worker.joinable()) {
        applyNameToNativeHandle(worker.native_handle(), name);
    }
}

const String& Thread::threadGetName() const noexcept
{
    std::lock_guard<Mutex> lock(mtx);
    return name;
}

void Thread::threadStart()
{
    std::lock_guard<Mutex> lock(mtx);
    if (is_running.load(std::memory_order_acquire)) {
        throw ThreadAlreadyRunningException();
    }
    if (worker.joinable()) {
        worker.join();
    }

    should_stop.store(false, std::memory_order_release);
    stop_event.reset();
    is_running.store(true, std::memory_order_release);
    thread_id = global_thread_counter.fetch_add(1, std::memory_order_relaxed);

    try {
        worker = std::thread(&Thread::threadStartUp, this);
    }
    catch (const std::system_error&) {
        is_running.store(false, std::memory_order_release);
        throw ThreadStartException();
    }

    applyPriorityToNativeHandle(worker.native_handle(), my_priority);
    if (!name.isEmpty()) {
        applyNameToNativeHandle(worker.native_handle(), name);
    }
}

void Thread::threadStartUp()
{
    local_thread_id = thread_id;
    runcount.fetch_add(1, std::memory_order_relaxed);

    try {
        run();
    }
    catch (...) {
        // Exception in run() abfangen, damit der Gesamtprozess nicht mit std::terminate abbricht
    }

    is_running.store(false, std::memory_order_release);

    if (delete_on_exit.load(std::memory_order_acquire)) {
        // Bei delete_on_exit muss der Thread vorher detached werden,
        // da delete this den Destruktor ruft, der sonst worker.join() auf sich selbst ausführen würde!
        worker.detach();
        delete this;
    }
}

void Thread::threadStop()
{
    threadSignalStop();
    threadJoin();
}

void Thread::threadSignalStop() noexcept
{
    should_stop.store(true, std::memory_order_release);
    stop_event.set();
}

void Thread::threadJoin()
{
    if (std::this_thread::get_id() == worker.get_id()) {
        // Ein Thread darf sich nicht selbst joinen -> Deadlock
        throw DeadlockException();
    }

    if (worker.joinable()) {
        worker.join();
    }
}

bool Thread::threadIsRunning() const noexcept
{
    return is_running.load(std::memory_order_acquire);
}

bool Thread::threadShouldStop() const noexcept
{
    return should_stop.load(std::memory_order_acquire);
}

size_t Thread::threadRunCount() const noexcept
{
    return runcount.load(std::memory_order_relaxed);
}

uint64_t Thread::threadGetID() const noexcept
{
    return thread_id;
}

void Thread::threadDeleteOnExit(bool flag) noexcept
{
    delete_on_exit.store(flag, std::memory_order_release);
}

bool Thread::threadShouldDeleteOnExit() const noexcept
{
    return delete_on_exit.load(std::memory_order_acquire);
}

bool Thread::threadSetPriority(Priority priority)
{
    std::lock_guard<Mutex> lock(mtx);
    my_priority = priority;
    if (worker.joinable()) {
        applyPriorityToNativeHandle(worker.native_handle(), my_priority);
    }
    return true;
}

Thread::Priority Thread::threadGetPriority() const
{
    std::lock_guard<Mutex> lock(mtx);
    return my_priority;
}

} // namespace pplib
