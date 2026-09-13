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

#ifndef PPLIB_CORE_THREADS_H_
#define PPLIB_CORE_THREADS_H_

#include <stdint.h>
#include <thread>
#include <atomic>
#include <functional>
#include <pplib/types/string.h>
#include <pplib/core/mutex.h>
#include <pplib/core/threadevent.h>

namespace pplib
{

/**
 * @class Thread
 * @ingroup PPLGroupThreads
 * @brief Klasse zum Verwalten von Threads
 *
 * Klasse zum Starten und Verwalten von Threads. Eine von Thread abgeleitete Klasse muss
 * die run()-Methode implementieren. Falls run() endlos laufen soll, sollte regelmäßig
 * threadShouldStop() überprüft werden, um zu schauen, ob der Thread gestoppt werden soll.
 *
 * Die Methode threadSleep() kann verwendet werden, um den Thread für eine bestimmte Zeit
 * schlafen zu lassen oder auf ein Stop-Signal zu warten.
 *
 * @note Verwendet std::thread intern zur Thread-Verwaltung.
 */
class Thread
{
public:
    /** @enum Priority
     * @brief Priorität des Threads.
     */
    enum class Priority
    {
        UNKNOWN = 0,
        LOWEST,
        BELOW_NORMAL,
        NORMAL,
        ABOVE_NORMAL,
        HIGHEST
    };

private:
    std::thread worker;     ///< Der interne std::thread, der die Ausführung des Threads übernimmt.
    mutable Mutex mtx;      ///< Mutex zum Schutz der internen Daten.
    ThreadEvent stop_event; ///< Ereignis, das signalisiert, dass der Thread stoppen soll.

    std::atomic<bool> is_running{false};     ///< Flag, das angibt, ob der Thread aktuell läuft.
    std::atomic<bool> should_stop{false};    ///< Flag, das angibt, ob der Thread ein Stop-Signal erhalten hat.
    std::atomic<bool> delete_on_exit{false}; ///< Flag, das angibt, ob der Thread nach dem Beenden automatisch gelöscht werden soll.
    std::atomic<size_t> runcount{0};         ///< Zählt, wie oft der Thread gestartet wurde.
    uint64_t thread_id{0};                   ///< Die ID des Threads.

    String name;                            ///< Name des Threads, sofern einer gesetzt wurde.
    Priority my_priority{Priority::NORMAL}; ///< Die Priorität des Threads.

    /** @brief Interne Startfunktion des Threads. */
    void threadStartUp();

public:
    /** @brief Konstruktor der Thread-Klasse. Initialisiert die internen Variablen.
     */
    Thread();
    virtual ~Thread();

    // Nicht kopierbar
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;
    Thread(Thread&&) = delete;
    Thread& operator=(Thread&&) = delete;

    /** @brief Setzt den Namen des Threads.
     * @param threadName Der Name, der dem Thread zugewiesen werden soll.
     */
    void threadSetName(const String& threadName);

    /** @brief Gibt den Namen des Threads zurück.
     * @return Der Name des Threads als String.
     */
    const String& threadGetName() const noexcept;

    /** @brief Startet den Thread. */
    void threadStart();

    /** @brief Signalisiert dem Thread, dass er stoppen soll und wartet
     * auf dessen Beendigung.
     *
     * Der Thread muss selbst regelmäßig threadShouldStop() überprüfen, um auf ein
     * Stop-Signal zu reagieren.
     *
     * @note Diese Methode blockiert, bis der Thread tatsächlich beendet ist.
     * @note Diese Methode sollte nicht von innerhalb des Threads aufgerufen werden, der gestoppt werden soll.
     * Hierdurch würde ein Deadlock entstehen. Die Methode fängt diesen Fall ab und wirft eine DeadlockException.
     * @exception DeadlockException Wenn versucht wird, den Thread von innerhalb desselben Threads zu stoppen.
     */
    void threadStop();

    /** @brief Signalisiert dem Thread, dass er stoppen soll, ohne auf dessen Beendigung zu warten.
     * Der Thread muss selbst regelmäßig threadShouldStop() überprüfen, um auf ein
     * Stop-Signal zu reagieren.
     */
    void threadSignalStop() noexcept;

    /** @brief Wartet auf die Beendigung des Threads. */
    void threadJoin();

    /** @brief Prüft, ob der Thread aktuell läuft.
     * @return true, wenn der Thread läuft, sonst false.
     */
    bool threadIsRunning() const noexcept;

    /** @brief Prüft, ob der Thread ein Stop-Signal erhalten hat.
     * @return true, wenn der Thread stoppen soll, sonst false.
     */
    bool threadShouldStop() const noexcept;

    /** @brief Gibt die Anzahl der bisherigen Ausführungen des Threads zurück.
     * @return Die Anzahl der bisherigen Ausführungen.
     */
    size_t threadRunCount() const noexcept;

    /** @brief Gibt die ID des Threads zurück.
     * @return Die ID des Threads.
     */
    uint64_t threadGetID() const noexcept;

    /** @brief Der Thread schläft für die angegebene Zeit oder bis ein Stop-Signal empfangen wird.
     * @param msec Schlafdauer in Millisekunden.
     * @return true, wenn ein Stop-Signal empfangen wurde oder ansteht, false nach regulärem Timeout.
     */
    bool threadSleep(int msec)
    {
        if (should_stop.load(std::memory_order_acquire)) {
            return true;
        }
        if (stop_event.wait(msec)) {
            return true;
        }
        return should_stop.load(std::memory_order_acquire);
    }

    /** @brief Legt fest, ob der Thread beim Beenden automatisch gelöscht werden soll.
     *
     * Ist dieses Flag auf true gesetzt, wird der Thread beim Beenden automatisch gelöscht (delete).
     * @param flag true, wenn der Thread beim Beenden gelöscht werden soll, sonst false.
     */
    void threadDeleteOnExit(bool flag = true) noexcept;

    /** @brief Prüft, ob der Thread beim Beenden automatisch gelöscht werden soll.
     * @return true, wenn der Thread beim Beenden gelöscht werden soll, sonst false.
     */
    bool threadShouldDeleteOnExit() const noexcept;

    /** @brief Setzt die Priorität des Threads.
     * @param priority Die neue Priorität des Threads.
     * @return true, wenn die Priorität erfolgreich gesetzt wurde, sonst false.
     */
    bool threadSetPriority(Priority priority);

    /** @brief Gibt die aktuelle Priorität des Threads zurück.
     * @return Die aktuelle Priorität des Threads.
     */
    Priority threadGetPriority() const;

    virtual void run() = 0;
};

/** @brief Gibt die ID des aktuellen Threads zurück. */
uint64_t ThreadID();

/** @brief Startet einen neuen Thread mit der angegebenen Funktion.
 * @param func Die Funktion, die im neuen Thread ausgeführt werden soll.
 * @return Die ID des gestarteten Threads.
 */
uint64_t StartThread(std::function<void()> func);

/** @brief Startet einen neuen Thread mit der angegebenen Funktion.
 * @param start_routine Die Funktion, die im neuen Thread ausgeführt werden soll.
 * @param data Optionaler Parameter, der an die Startfunktion übergeben wird.
 * @return Die ID des gestarteten Threads.
 */
uint64_t StartThread(void (*start_routine)(void*), void* data = nullptr);

/** @brief Setzt die Priorität des aktuellen Threads.
 * @param priority Die neue Priorität des Threads.
 */
void ThreadSetPriority(Thread::Priority priority);

/** @brief Gibt die aktuelle Priorität des aktuellen Threads zurück. */
Thread::Priority ThreadGetPriority();

} // namespace pplib

#endif /* PPLIB_CORE_THREADS_H_ */