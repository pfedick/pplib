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
 * Unter der Haube verwendet die Klasse std::mutex aus der C++ Standardbibliothek.
 *
 */
class Mutex
{
private:
    std::mutex mtx; ///< Der eigentliche Mutex aus der C++ Standardbibliothek

public:
    /**
     * @brief Konstruktor
     *
     * Der Konstruktor initialisiert den Mutex. Ist dies nicht möglich, wird eine Exception geworfen.
     *
     * @exception OutOfMemoryException Es konnte kein Speicher für den Mutex reserviert werden
     */
    Mutex() = default;

    ~Mutex() = default;

    // Nicht kopierbar
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;
    Mutex(Mutex&&) = delete;
    Mutex& operator=(Mutex&&) = delete;

    /**
     * @brief Mutex sperren
     *
     * Diese Funktion versucht einen Mutex zu sperren. Ist dieser bereits durch einen
     * anderen Thread blockiert, wird gewartet, bis der Mutex wieder frei wird.
     *
     * @exception std::system_error Mutex konnte nicht gesperrt werden
     */
    inline void lock()
    {
        mtx.lock();
    }

    /**
     * @brief Mutex entsperren
     *
     * Mit dieser Funktion wird ein zuvor mit Mutex::lock gesperrter
     * Mutex wieder frei gegeben.
     *
     * @exception std::system_error Mutex konnte nicht entsperrt werden
     */
    inline void unlock()
    {
        mtx.unlock();
    }

    /**
     * @brief Mutex versuchen zu sperren
     *
     * Diese Funktion versucht wie Mutex::lock einen Mutex zu sperren.
     * Ist dieser allerdings bereits durch einen anderen Thread blockiert,
     * wird nicht gewartet, bis er wieder frei wird, sondern es wird
     * sofort false zurückgeliefert.
     *
     * @return Konnte der Mutex erfolgreich gesperrt werden, liefert die Funktion
     * true zurück, sonst false
     */
    inline bool tryLock() noexcept
    {
        return mtx.try_lock();
    }
};

/// Ein Lock-Guard für Mutexes. Sperrt den Mutex beim Erstellen und gibt ihn beim Zerstören wieder frei.
using MutexLock = std::lock_guard<Mutex>;

} // namespace pplib

#endif /* PPLIB_CORE_MUTEX_H_ */