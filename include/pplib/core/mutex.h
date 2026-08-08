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

#ifndef PPLIB_CORE_MUTEX_H_
#define PPLIB_CORE_MUTEX_H_

#include <mutex>

namespace pplib
{
/**
 * @class Mutex
 * @ingroup PPLGroupThreads
 * @ingroup PPLGroupThreadsMutex
 * @brief Low-Level-Interface für die Synchronisation von Threads
 *
 * Mutexes stellen ein Low-Level-Interface für die Synchronisation von Threads dar. Mit ihnen
 * kann verhindert werden, dass mehrere Threads versuchen gleichzeitig auf die gleichen Daten
 * zuzugreifen.
 *
 * Unter der Haube verwendet die Klasse std::mutex und std::condition_variable aus der C++
 * Standardbibliothek.
 *
 */
class Mutex
{
private:
    void* handle;

public:
    /**
     * @brief Konstruktor
     *
     * Der Konstruktor initialisiert den Mutex. Ist dies nicht möglich, wird eine Exception geworfen.
     *
     * @exception OutOfMemoryException Es konnte kein Speicher für den Mutex reserviert werden
     */
    Mutex();

    /**
     * @brief Destructor
     *
     * Der Destruktor gibt alle Ressourcen des Mutex wieder frei.
     */
    ~Mutex() noexcept;

    /**
     * @brief Mutex sperren
     *
     * Diese Funktion versucht einen Mutex zu sperren. Ist dieser bereits durch einen
     * anderen Thread blockiert, wird gewartet, bis der Mutex wieder frei wird.
     *
     * @exception MutexLockingException Mutex konnte nicht gesperrt werden
     */
    void lock();

    /**
     * @brief Mutex entsperren
     *
     * Mit dieser Funktion wird ein zuvor mit Mutex::lock gesperrter
     * Mutex wieder frei gegeben.
     *
     * @exception MutexLockingException Mutex konnte nicht entsperrt werden
     */
    void unlock();

    /**
     * @brief Mutex versuchen zu sperren
     *
     * Diese Funktion versucht wie CMutex::Lock einen Mutex zu sperren.
     * Ist dieser allerdings bereits durch einen anderen Thread blockiert,
     * wird nicht gewartet, bis der Mutex wieder frei wird, sondern es wird
     * sofort ein Fehler zurückgeliefert.
     *
     * @return Konnte der Mutex erfolgreich gesperrt werden, liefert die Funktion
     * true (1) zurück, im Fehlerfall false (0)
     */
    bool tryLock() noexcept;

    /**
     * @brief Auf Signal warten
     *
     * Der aufrufende Thread wird angehalten, bis ein anderer Thread diesem Mutex
     * ein Signal gibt (siehe CMutex::Signal) oder die optional angegebene Zeit
     * abgelaufen ist.
     *
     * @param milliseconds Optionale Angabe einer Zeit, nach deren Ablauf der Thread
     * automatisch wieder fortgeführt wird. Default = 0 = unendlich.
     *
     * @return Liefert true (1), wenn dem Mutex ein Signal gegeben wurde. Wenn @p microseconds
     * angegeben wurde, liefert die Funktion nach Ablauf des Timeouts 0 zurück, andernfalls wartet
     * die Funktion so lange, bis ein Signal gegeben wurde.
     *
     * @note
     * War der Mutex vor Aufruf dieser Funktion bereits gelocked, ist er es nach dem Aufruf
     * immer noch.
     *
     */
    bool wait(int milliseconds = 0) noexcept;

    /**
     * @brief Signal senden
     *
     * Mit dieser Funktion wird dem Mutex ein Signal gegeben. Hat ein Thread sich selbst
     * durch Aufruf der Wait-Funktion (siehe CMutex::Wait) angehalten, wird er
     * nach Aufruf des Signals fortgesetzt.
     *
     * @return Bei Erfolg liefert die Funktion true (1) zurück, sonst false (0).
     */
    bool signal() noexcept;
};

/// Ein Lock-Guard für Mutexes. Sperrt den Mutex beim Erstellen und gibt ihn beim Zerstören wieder frei.
using MutexLock = std::lock_guard<Mutex>;

} // namespace pplib

#endif /* PPLIB_CORE_MUTEX_H_ */