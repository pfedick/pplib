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

#ifndef PPLIB_CORE_THREADEVENT_H_
#define PPLIB_CORE_THREADEVENT_H_

#include <mutex>
#include <condition_variable>

namespace pplib
{

/**
 * @class ThreadEvent
 * @ingroup PPLGroupThreads
 * @brief Ein Thread-Synchronisationsereignis (Event), das von Threads gesetzt und abgewartet werden kann.
 *
 * Diese Klasse stellt einen Mechanismus bereit, mit dem Threads auf ein Signalereignis warten können.
 * Sie unterstützt sowohl den Auto-Reset- als auch den manuellen Reset-Modus.
 *
 * Im Auto-Reset-Modus wird das Ereignis nach dem erfolgreichen Warten automatisch zurückgesetzt.
 * Im manuellen Reset-Modus muss das Ereignis explizit zurückgesetzt werden.
 *
 * @note Diese Klasse ist nicht kopierbar oder verschiebbar.
 */
class ThreadEvent
{
private:
    mutable std::mutex mtx;     ///< Mutex zum Schutz des internen Zustands.
    std::condition_variable cv; ///< Bedingungsvariable zum Warten auf das Ereignis.
    bool signaled{false};       ///< Gibt an, ob das Ereignis gesetzt ist.
    bool auto_reset{true};      ///< Gibt an, ob das Ereignis im Auto-Reset-Modus arbeitet.

public:
    /** @brief Erstellt ein Thread-Ereignis.
     *
     * @param autoReset Gibt an, ob das Ereignis im Auto-Reset-Modus arbeiten soll (Default: true)
     * @param initialState Der anfängliche Zustand des Ereignisses (gesetzt oder nicht gesetzt) (Default: false)
     */
    explicit ThreadEvent(bool autoReset = true, bool initialState = false)
        : auto_reset(autoReset),
          signaled(initialState)
    {
    }

    ~ThreadEvent() = default;

    // Nicht kopierbar
    ThreadEvent(const ThreadEvent&) = delete;
    ThreadEvent& operator=(const ThreadEvent&) = delete;
    ThreadEvent(ThreadEvent&&) = delete;
    ThreadEvent& operator=(ThreadEvent&&) = delete;

    /** @brief Setzt das Ereignis (signalisiert).
     *
     * Weckt wartende Threads auf. Im Auto-Reset-Modus wird genau ein wartender Thread
     * geweckt und das Ereignis beim Aufwachen automatisch wieder zurückgesetzt.
     * Wartet aktuell kein Thread, bleibt das Signal für den nächsten wait()-Aufruf erhalten.
     * Im manuellen Reset-Modus werden alle wartenden Threads geweckt und das Ereignis
     * bleibt gesetzt, bis reset() aufgerufen wird.
     */
    void set() noexcept
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            signaled = true;
        }
        if (auto_reset) {
            cv.notify_one();
        } else {
            cv.notify_all();
        }
    }

    /** @brief Setzt das Ereignis zurück.
     *
     * Diese Methode setzt das Ereignis auf den nicht gesetzten Zustand zurück.
     */
    void reset() noexcept
    {
        std::lock_guard<std::mutex> lock(mtx);
        signaled = false;
    }

    /** @brief Prüft, ob das Ereignis gesetzt ist.
     *
     * @return true, wenn das Ereignis gesetzt ist, sonst false.
     */
    bool isSignaled() const noexcept
    {
        std::lock_guard<std::mutex> lock(mtx);
        return signaled;
    }

    /** @brief Wartet auf das Ereignis.
     *
     * Diese Methode blockiert den aufrufenden Thread, bis das Ereignis gesetzt ist oder das optionale Timeout abläuft.
     *
     * @param timeout_ms Die maximale Wartezeit in Millisekunden (0 oder negative Werte bedeuten unbegrenzt).
     * @return Gibt true zurück, wenn das Ereignis gesetzt war oder innerhalb des Timeouts gesetzt wurde;
     * false bei einem Timeout.
     */
    bool wait(int timeout_ms = 0) noexcept
    {
        std::unique_lock<std::mutex> lock(mtx);
        auto predicate = [this]() { return signaled; };

        bool success = false;
        if (timeout_ms > 0) {
            success = cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), predicate);
        } else {
            cv.wait(lock, predicate);
            success = true;
        }

        if (success && auto_reset) {
            signaled = false;
        }
        return success;
    }
};

} // namespace pplib

#endif /* PPLIB_CORE_THREADEVENT_H_ */