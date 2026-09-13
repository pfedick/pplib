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

#ifndef PPLIB_CORE_THREADPOOL_H_
#define PPLIB_CORE_THREADPOOL_H_

#include <stdint.h>
#include <set>
#include <pplib/core/mutex.h>
#include <pplib/core/threads.h>

namespace pplib
{

/** @class ThreadPool
 * @ingroup PPLGroupThreads
 * @brief Klasse zum Verwalten mehrerer Threads in einem Pool.
 *
 * Mit dieser Klasse können beliebig viele Threads in einem Pool verwaltet werden.
 * Über den Pool können sie gemeinsam gestartet, überwacht, gestoppt und gelöscht werden.
 * Zur Laufzeit können jederzeit Threads hinzugefügt oder entfernt werden. Mittels
 * getThreads kann die Anwendung eine Liste aller Threads im Pool erhalten, um dann
 * beispielsweise selbst darüber zu itterieren.
 *
 * Ein Thread, der an den Pool übergeben wird, geht in den Besitz des Pools über und wird
 * von diesem verwaltet. Beim beenden des Pools werden alle Threads automatisch gestoppt
 * und gelöscht (aufruf des Thread-Destruktors).
 *
 * Ein Thread muss von der Klasse pplib::Thread abgeleitet sein.
 *
 */
class ThreadPool
{
private:
    std::set<Thread*> threads;
    mutable pplib::Mutex mutex;

public:
    /** @brief Destruktor
     *
     * Falls noch Threads im Pool vorhanden sind, werden diese gestoppt und gelöscht.
     */
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    /** @brief Thread in den Pool hinzufügen
     *
     * Der angegebene Thread \p thread wird dem Pool hinzugefügt.
     * @param thread Pointer auf dem Thread
     * @exception IllegalArgumentException Wird geworfen, wenn der übergebene Thread-Pointer @p thread nullptr ist.
     */
    void addThread(Thread* thread);

    /** @brief Thread aus dem Pool entfernen
     *
     * Der angegebene Thread \p thread wird aus dem Pool entfernt. Die Anwendung ist
     * anschließend dafür verantwortlich den Thread zu stoppen und zu löschen.
     * @param thread Pointer auf dem Thread
     *
     * @exception IllegalArgumentException Wird geworfen, wenn der übergebene Thread-Pointer @p thread nullptr ist.
     */
    void removeThread(Thread* thread);

    /** @brief Thread aus aus dem Pool entfernen und löschen
     *
     * Der angegebene Thread \p thread wird aus dem Pool entfernt, sofern er darin vorhanden
     * war, und anschließend gelöscht. Falls notwendig, wird er vorher noch gestoppt.
     * @param thread Pointer auf dem Thread
     *
     * @exception IllegalArgumentException Wird geworfen, wenn der übergebene Thread-Pointer @p thread nullptr ist.
     */
    void destroyThread(Thread* thread);

    /** @brief Alle Threads aus dem Pool entfernen
     *
     * Alle Threads werden aus dem Pool entfernt. Die Threads selber bleiben unberührt,
     * laufen also ggfs. weiter und belegen Speicher. Falls die Threads auch gestoppt und
     * gelöscht werden sollen, verwenden Sie bitte ThreadPool::destroyAllThreads.
     */
    void clear();

    /** @brief Alle Threads stoppen, aus dem Pool entfernen und löschen
     *
     * Alle Threads werden gestoppt, gelöscht (Aufruf des Destruktors des Threads) und aus
     * dem Pool entfernt.
     */
    void destroyAllThreads();

    /** @brief Threads auffordern zu stoppen
     *
     * Signalisiert allen Threads, dass sie sich beenden sollen. Die Methode wartet jedoch
     * nicht, bis sich die Threads tatsächlich beendet haben. Bitte verwenden Sie ThreadPool::stopThreads,
     * wenn Sie sicherstellen wollen, dass sich die Threads tatsächlich beendet haben.
     */
    void signalStopThreads();

    /** @brief Threads stoppen
     *
     * Stoppt alle Threads im Pool, die aktiv sind. Die Methode kehrt erst dann zurück, wenn
     * alle Threads gestoppt sind.
     * @note Es ist sichergestellt, dass ein runterfahrender Thread sich bei Bedarf selbst aus dem Pool
     * löschen kann, ohne einen Deadlock zu verursachen.
     */
    void stopThreads();

    /** @brief Threads starten
     *
     * Startet alle Threads im Pool, die noch nicht aktiv sind.
     */
    void startThreads();

    /** @brief Anzahl Threads im Pool
     *
     * Liefert die Anzahl Threads im Pool zurück, unabhängig davon, ob sie grade aktiv sind.
     * @return Anzahl Threads
     */
    size_t size() const;

    /** @brief Anzahl aktiver Threads im Pool
     *
     * Liefert die Anzahl Threads im Pool zurück, die grade aktiv sind.
     * @return Anzahl Threads
     */
    size_t count_running() const;

    /** @brief Sind im Pool aktive Threads?
     *
     * Diese Methode prüft, ob im Pool Threads enthalten sind, die grade aktiv sind.
     * @return Gibt @b true oder @b false zurück
     */
    bool running() const;

    /** @brief Liste aller Threads im Pool
     *
     * Liefert eine Liste aller Threads im Pool zurück. Die Liste enthält Zeiger auf die Threads,
     * die vom Pool verwaltet werden. Sie kann genutzt werden, um beispielsweise selbst über die
     * Threads zu iterieren. Die Threads dürfen jedoch nicht direkt gelöscht werden, da sie vom Pool verwaltet werden.
     * @return Vektor mit Zeigern auf alle Threads im Pool
     *
     * @example
     * for (auto* thread : pool.getThreads()) {
     *     auto* worker = dynamic_cast<MyWorkerThread*>(thread);
     *     if (worker) {
     *         worker->setNewTarget(target);
     *     }
     *}
     */
    std::vector<Thread*> getThreads() const
    {
        MutexLock lock(mutex);
        return std::vector<Thread*>(threads.begin(), threads.end());
    }
};

} // namespace pplib

#endif /* PPLIB_CORE_THREADPOOL_H_ */
