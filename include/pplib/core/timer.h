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

#ifndef PPLIB_CORE_TIMER_H_
#define PPLIB_CORE_TIMER_H_

#include <chrono>

namespace pplib
{
/** @class Timer
 * @brief  Ein hochauflösender Timer, der die verstrichene Zeit misst.
 *
 * Diese Klasse ermöglicht es, die verstrichene Zeit zwischen Start und Stopp zu messen.
 * Die Genauigkeit liegt im Bereich von Nanosekunden, abhängig von der Implementierung
 * der Standardbibliothek und der Hardware.
 *
 * Die verstrichene Zeit kann über die Methoden `stop()`, `currentDuration()` und `duration()` abgefragt werden.
 *
 * Mit `stop()` wird der interne Zähler hochgezählt. Mehrere aufeinanderfolgende Aufrufe von `start()` und  `stop()`
 * summieren die verstrichene Zeit. Mit `reset()` kann der Timer zurückgesetzt werden.
 */
class Timer
{
private:
    std::chrono::steady_clock::time_point _start;
    double _duration;

public:
    Timer();

    /**
     * @brief Startet den Timer und setzt den Startzeitpunkt auf die aktuelle Zeit.
     */
    void start();

    /**
     * @brief Stoppt den Timer und gibt die bisher verstrichene Zeit zurück.
     *
     * Diese Methode summiert die verstrichene Zeit seit dem letzten Start zum internen Zähler.
     *
     * @return Die bisher verstrichene Zeit in Sekunden.
     */
    double stop();

    /**
     * @brief Gibt die aktuelle verstrichene Zeit seit dem letzten Start zurück, ohne den internen Zähler zu ändern.
     *
     * @return Die aktuelle verstrichene Zeit in Sekunden.
     */
    double currentDuration() const;

    /**
     * @brief Setzt den Timer zurück, indem der interne Zähler auf 0 gesetzt wird.
     */
    inline void reset()
    {
        _duration = 0.0;
    }

    /**
     * @brief Gibt die verstrichene Zeit bis zum letzten Stopp zurück.
     *
     * @return Die bisher verstrichene Zeit in Sekunden.
     */
    inline double duration() const
    {
        return _duration;
    }

    /**
     * @brief Addiert eine zusätzliche Dauer zur bisherigen verstrichenen Zeit.
     *
     * @param additionalDuration Die zusätzliche Dauer in Sekunden.
     * @return Die aktualisierte verstrichene Zeit in Sekunden.
     */
    inline double addDuration(double additionalDuration)
    {
        _duration += additionalDuration;
        return _duration;
    }

    /**
     * @brief Addiert eine zusätzliche Dauer zur bisherigen verstrichenen Zeit über den `+=` Operator.
     *
     * @param additionalDuration Die zusätzliche Dauer in Sekunden.
     * @return Die aktualisierte verstrichene Zeit in Sekunden.
     */
    inline double operator+=(double additionalDuration)
    {
        _duration += additionalDuration;
        return _duration;
    }

    /**
     * @brief Addiert die verstrichene Zeit eines anderen Timers zur bisherigen verstrichenen Zeit über den `+=` Operator.
     *
     * @param other Der andere Timer.
     * @return Die aktualisierte verstrichene Zeit in Sekunden.
     */
    inline double operator+=(const Timer& other)
    {
        _duration += other.duration();
        return _duration;
    }
};

inline Timer operator+(const Timer& lhs, const Timer& rhs)
{
    Timer result = lhs;
    result += rhs;
    return result;
}

} // namespace pplib

#endif /* PPLIB_CORE_TIMER_H_ */
