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

#include <pplib/core/threads.h>
#include <pplib/core/mutex.h>
#include <pplib/core/threadpool.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>

namespace pplib
{

/*!\class ThreadPool
 * \ingroup PPLGroupThreads
 * \brief Klasse zum Verwalten mehrerer Threads in einem Pool.
 *
 * \desc
 * Mit dieser Klasse können beliebig viele Threads in einem Pool verwaltet werden.
 * Über den Pool können sie gemeinsam gestartet, überwacht, gestoppt und gelöscht werden.
 * Zur Laufzeit können jederzeit Threads hinzugefügt oder entfernt werden. Mittels
 * Iteratoren und den Methoden ThreadPool::begin und ThreadPool::end kann die Anwendung auch
 * selbst über die Threads iterieren.
 * @par
 * Ein %Thread muss von der Klasse pplib::Thread abgeleitet sein.
 *
 *
 */

/*!\brief Destruktor
 *
 * \desc
 * Falls noch Threads im Pool vorhanden sind, werden diese gestoppt und gelöscht.
 */
ThreadPool::~ThreadPool()
{
    stopThreads();
    destroyAllThreads();
}

/*!\brief Thread in den Pool hinzufügen
 *
 * \desc
 * Der angegebene Thread \p thread wird dem Pool hinzugefügt.
 *
 * @param thread Pointer auf dem Thread
 */
void ThreadPool::addThread(Thread* thread)
{
    MutexLock lock(mutex);
    std::pair<std::set<Thread*>::iterator, bool> ret;
    ret = threads.insert(thread);
    if (ret.second == false) throw ThreadAlreadyInPoolException();
}

/*!\brief Thread aus dem Pool entfernen
 *
 * \desc
 * Der angegebene Thread \p thread wird aus dem Pool entfernt. Die Anwendung ist
 * anschließend dafür verantwortlich den Thread zu stoppen und zu löschen.
 *
 * @param thread Pointer auf dem Thread
 */
void ThreadPool::removeThread(Thread* thread)
{
    MutexLock lock(mutex);
    threads.erase(thread);
}

/*!\brief Thread aus aus dem Pool entfernen und löschen
 *
 * \desc
 * Der angegebene Thread \p thread wird aus dem Pool entfernt, sofern er darin vorhanden
 * war, und anschließend gelöscht. Falls notwendig, wird er vorher noch gestoppt.
 *
 * @param thread Pointer auf dem Thread
 */
void ThreadPool::destroyThread(Thread* thread)
{
    MutexLock lock(mutex);
    threads.erase(thread);
    delete thread;
}

/*!\brief Alle Threads aus dem Pool entfernen
 *
 * \desc
 * Alle Threads werden aus dem Pool entfernt. Die Threads selber bleiben unberührt,
 * laufen also ggfs. weiter und belegen Speicher. Falls die Threads auch gestoppt und
 * gelöscht werden sollen, verwenden Sie bitte ThreadPool::destroyAllThreads.
 *
 */
void ThreadPool::clear()
{
    MutexLock lock(mutex);
    threads.clear();
}

/*!\brief Alle Threads stoppen, aus dem Pool entfernen und löschen
 *
 * \desc
 * Alle Threads werden gestoppt, gelöscht (Aufruf des Destruktors des Threads) und aus
 * dem Pool entfernt.
 *
 */
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

/*!\brief Iterator auf den ersten Thread im Pool
 *
 * \desc
 * Liefert einen Iterator auf den ersten Thread im Pool zurück.
 *
 * \note Falls Sie mit ThreadPool::begin und ThreadPool::end über alle Threads iterieren
 * wollen und die Gefahr besteht, dass andere Threads den Pool verändern könnten, sollten Sie
 * den Pool vorher mittels ThreadPool::lock sperren und am Ende mit ThreadPool::unlock wieder
 * freigeben.
 *
 * @return Iterator auf den ersten Thread im Pool
 *
 * @example Beispiel zum Iterieren über alle Threads
 * \code
 * ThreadPool pool;
 * ...
 * ThreadPool::iterator it;
 * pool.lock();
 * for (it=pool.begin();it!=pool.end();++it) {
 *    printf ("Thread %llu is %s\n",
 *        (*it)->threadGetID(),
 *        ((*it)->threadIsRunning()?"running":"stopped"));
 * }
 * pool.unlock();
 * \endcode
 */
ThreadPool::iterator ThreadPool::begin()
{
    return threads.begin();
}

/*!@copydoc ThreadPool::begin()
 */
ThreadPool::const_iterator ThreadPool::begin() const
{
    return threads.begin();
}

/*!\brief Iterator auf das Ende des ThreadPools
 *
 * \desc
 * Liefert einen Iterator zurück, der hinter den letzten Thread im Pool zeigt.
 *
 * \see ThreadPool::begin
 *
 * @return Iterator auf das Ende des ThreadPools
 */
ThreadPool::iterator ThreadPool::end()
{
    return threads.end();
}

/*!@copydoc ThreadPool::end()
 */
ThreadPool::const_iterator ThreadPool::end() const
{
    return threads.end();
}

/*!\brief Threads auffordern zu stoppen
 *
 * \desc
 * Signalisiert allen Threads, dass sie sich beenden sollen. Die Methode wartet jedoch
 * nicht, bis sich die Threads tatsächlich beendet haben. Bitte verwenden Sie ThreadPool::stopThreads,
 * wenn Sie sicherstellen wollen, dass sich die Threads tatsächlich beendet haben.
 */
void ThreadPool::signalStopThreads()
{
    std::set<Thread*>::iterator it;
    MutexLock lock(mutex);
    for (it = threads.begin(); it != threads.end(); ++it) {
        (*it)->threadSignalStop();
    }
}

/*!\brief Threads stoppen
 *
 * \desc
 * Stoppt alle Threads im Pool, die aktiv sind. Die Methode kehrt erst dann zurück, wenn
 * alle Threads gestoppt sind.
 *
 * \note Es ist sichergestellt, dass ein runterfahrender Thread sich bei Bedarf selbst aus dem Pool
 * löschen kann, ohne einen Deadlock zu verursachen.
 */
void ThreadPool::stopThreads()
{
    signalStopThreads();
    while (running()) {
        MSleep(1);
    }
}

/*!\brief Threads starten
 *
 * \desc
 * Startet alle Threads im Pool, die noch nicht aktiv sind.
 */
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

/*!\brief Anzahl Threads im Pool
 *
 * \desc
 * Liefert die Anzahl Threads im Pool zurück, unabhängig davon, ob sie grade aktiv sind.
 *
 * @return Anzahl Threads
 */
size_t ThreadPool::size()
{
    MutexLock lock(mutex);
    size_t num = threads.size();
    return num;
}

/*!\brief Anzahl Threads im Pool
 *
 * \desc
 * Liefert die Anzahl Threads im Pool zurück, unabhängig davon, ob sie grade aktiv sind.
 *
 * @return Anzahl Threads
 */
size_t ThreadPool::count()
{
    MutexLock lock(mutex);
    size_t num = threads.size();
    return num;
}

/*!\brief Anzahl aktiver Threads im Pool
 *
 * \desc
 * Liefert die Anzahl Threads im Pool zurück, die grade aktiv sind.
 *
 * @return Anzahl Threads
 */
size_t ThreadPool::count_running()
{
    std::set<Thread*>::const_iterator it;
    size_t count = 0;
    MutexLock lock(mutex);
    for (it = threads.begin(); it != threads.end(); ++it) {
        if ((*it)->threadIsRunning()) count++;
    }
    return count;
}

/*!\brief Sind im Pool aktive Threads?
 *
 * \desc
 * Diese Methode prüft, ob im Pool Threads enthalten sind, die grade aktiv sind.
 *
 * @return Gibt \b true oder \b false zurück
 */
bool ThreadPool::running()
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

/*!\brief %ThreadPool sperren
 *
 * \desc
 * Falls mehrere Operationen durchgeführt werden sollen, zwischen denen sich der Pool nicht
 * ändern darf (zum Beispiel beim Durchiterieren) kann der Pool vorher mit dieser Methode
 * gesperrt werden. Nach Abschluss der Operationen muss der Pool mit  ThreadPool::unlock wieder
 * freigegeben werden.
 *
 * \exception DeadlockException Wird geworfen, wenn durch das Sperren des Mutex ein
 * Deadlock entstehen würde
 * \exception MutexLockingException Mutex konnte nicht gesperrt werden
 */
void ThreadPool::lock()
{
    mutex.lock();
}

/*!\brief %ThreadPool entsperren
 *
 * \desc
 * Entsperrt einen zuvor mit ThreadPool::lock gesperrten Pool.
 *
 * \exception MutexNotLockedException Mutex war nicht gesperrt
 * \exception MutexLockingException Mutex konnte nicht entsperrt werden
 */
void ThreadPool::unlock()
{
    mutex.unlock();
}

} // namespace pplib
