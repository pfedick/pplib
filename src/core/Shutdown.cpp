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

#include <atomic>
#include <csignal>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#endif

#include <pplib/core/shutdown.h>
#include <pplib/core/threadevent.h>

namespace pplib
{

static std::atomic<bool> g_shutdownRequested{false};
static ThreadEvent g_shutdownEvent(false /* manual_reset */, false /* initial */);

bool IsShutdownRequested() noexcept
{
    return g_shutdownRequested.load(std::memory_order_relaxed);
}

void RequestShutdown() noexcept
{
    g_shutdownRequested.store(true, std::memory_order_relaxed);
    g_shutdownEvent.set();
}

void ResetShutdown() noexcept
{
    g_shutdownRequested.store(false, std::memory_order_relaxed);
    g_shutdownEvent.reset();
}

bool WaitForShutdown(int timeout_ms) noexcept
{
    return g_shutdownEvent.wait(timeout_ms);
}

#ifdef _WIN32
// ============================================================================
// Windows Implementation
// ============================================================================

// LCOV_EXCL_START
static BOOL WINAPI consoleCtrlHandler(DWORD ctrlType)
{
    switch (ctrlType) {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        RequestShutdown();
        return TRUE;
    default:
        return FALSE;
    }
}
// LCOV_EXCL_STOP

void InstallShutdownHandler()
{
    SetConsoleCtrlHandler(consoleCtrlHandler, TRUE);
}

void UninstallShutdownHandler()
{
    SetConsoleCtrlHandler(consoleCtrlHandler, FALSE);
}

#else
// ============================================================================
// POSIX Implementation (Self-Pipe Trick)
// ============================================================================

static int g_pipeFd[2] = {-1, -1};
static std::thread g_watcherThread;
static std::atomic<bool> g_handlerInstalled{false};

// LCOV_EXCL_START
static void posixSignalHandler(int /*sig*/)
{
    if (g_pipeFd[1] != -1) {
        char ch = 'S';
        // write() ist garantiert async-signal-safe
        [[maybe_unused]] auto bytes = write(g_pipeFd[1], &ch, 1);
    }
}
// LCOV_EXCL_STOP

void InstallShutdownHandler()
{
    if (g_handlerInstalled.exchange(true)) {
        return; // bereits installiert
    }

    if (pipe(g_pipeFd) != 0) {
        g_handlerInstalled.store(false);
        return;
    }

    // Watcher-Thread liest aus Pipe
    g_watcherThread = std::thread([]() {
        while (true) {
            char ch = 0;
            ssize_t bytes = read(g_pipeFd[0], &ch, 1);
            if (bytes > 0) {
                if (ch == 'S') {
                    RequestShutdown();
                } else if (ch == 'Q') {
                    break; // Beenden angefordert
                }
            } else if (bytes < 0 && errno == EINTR) {
                continue;
            } else {
                break;
            }
        }
    });

    struct sigaction sa;
    sa.sa_handler = posixSignalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
}

void UninstallShutdownHandler()
{
    if (!g_handlerInstalled.exchange(false)) {
        return; // nicht installiert
    }

    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    // Watcher-Thread zum Beenden auffordern
    if (g_pipeFd[1] != -1) {
        char ch = 'Q';
        [[maybe_unused]] auto bytes = write(g_pipeFd[1], &ch, 1);
    }

    if (g_watcherThread.joinable()) {
        g_watcherThread.join();
    }

    if (g_pipeFd[0] != -1) {
        close(g_pipeFd[0]);
        g_pipeFd[0] = -1;
    }
    if (g_pipeFd[1] != -1) {
        close(g_pipeFd[1]);
        g_pipeFd[1] = -1;
    }
}
#endif

} // namespace pplib