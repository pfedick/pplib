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

#ifndef PPLIB_CORE_RANDOM_H_
#define PPLIB_CORE_RANDOM_H_

#include <cstddef>
#include <stdint.h>
#include <pplib/types/bytearray.h>

namespace pplib
{

/** @class Random
 * @brief Zufallszahlengenerator-Klasse
 * @ingroup PPLGroupMath
 *
 * Diese Klasse stellt Funktionen zum Erzeugen von pseudo-Zufallszahlen und Zufallsdaten
 * bereit. Alle Funktionen sind statisch und arbeiten thread-lokal.
 *
 * @attention Die Funktionen sind nicht geeignet um kryptographisch sichere Zufallszahlen zu erzeugen!
 */
class Random
{
public:
    /** @brief Zufallszahlengenerator initialisieren
     *
     * Mit dieser Funktion wird der Zufallszahlengenerator des aktuellen Threads mit dem
     * Wert \p x initialisiert. Kann verwendet werden, um reproduzierbare Zufallszahlen
     * zu erzeugen.
     *
     * @param[in] x Initialisierungswert
     */
    static void seed(uint64_t x); // Thread local

    /** @brief Zufallszahlen erzeugen
     *
     * Mit dieser Funktion werden Zufallszahlen im Bereich von \p min bis \p max erzeugt.
     *
     * @param min Untere Grenze (inklusive)
     * @param max Obere Grenze (inklusive)
     * @return Zufallszahl im Bereich von \p min bis \p max
     * @note \p min und \p max werden automatisch gedreht, falls \p min größer als \p max ist.
     */
    static size_t get(size_t min, size_t max);

    /** @brief Zufallszahlen als float erzeugen
     *
     * Mit dieser Funktion werden Zufallszahlen im Bereich von \p min bis \p max erzeugt.
     *
     * @param min Untere Grenze (inklusive)
     * @param max Obere Grenze (exklusive, wird praktisch nie exakt erreicht)
     * @return Zufallszahl als float im Bereich von \p min bis \p max
     * @note \p min und \p max werden automatisch gedreht, falls \p min größer als \p max ist.
     */
    static float getFloat(float min, float max);
    /** @brief Zufallszahlen als double erzeugen
     *
     * Mit dieser Funktion werden Zufallszahlen im Bereich von \p min bis \p max erzeugt.
     *
     * @param min Untere Grenze (inklusive)
     * @param max Obere Grenze (exklusive, wird praktisch nie exakt erreicht)
     * @return Zufallszahl als double im Bereich von \p min bis \p max
     * @note \p min und \p max werden automatisch gedreht, falls \p min größer als \p max ist.
     */
    static double getDouble(double min, double max);

    /**
     * @brief Zufallsdaten erzeugen
     *
     * Mit dieser Funktion werden \p bytes Bytes an Zufallsdaten erzeugt und als
     * ByteArray Objekt zurückgegeben.
     *
     * @param[in] bytes Anzahl Bytes
     * @return Gibt ein ByteArray Objekt mit der gewünschten Anzahl Zufallsdaten
     * zurück
     */
    static ByteArray bytes(size_t num_bytes);

    /**
     * @brief Zufallsdaten erzeugen
     *
     * Mit dieser Funktion werden \p num_bytes Bytes an Zufallsdaten erzeugt und im
     * ByteArray Objekt \p buffer abgelegt.
     *
     * @param[in] buffer ByteArray Objekt, in dem die Zufallsdaten abgelegt werden
     * @param[in] num_bytes Anzahl Bytes
     * @return Referenz auf das ByteArray Objekt \p buffer
     */
    static ByteArray& fill(ByteArray& buffer, size_t num_bytes);

    /** @brief Zufallsdaten erzeugen
     *
     * Mit dieser Funktion werden so viele Zufallsdaten erzeugt, wie das ByteArray \p buffer groß ist.
     *
     * @param[in] buffer ByteArray Objekt, in dem die Zufallsdaten abgelegt werden
     * @return Referenz auf das ByteArray Objekt \p buffer
     */
    inline static ByteArray& fill(ByteArray& buffer)
    {
        return fill(buffer, buffer.size());
    }
};

/**
 * @ingroup PPLGroupMath
 * @brief Zufallszahlengenerator initialisieren
 *
 * Mit dieser Funktion wird der Zufallszahlengenerator des aktuellen Threads mit dem
 * Wert \p x initialisiert. Kann verwendet werden, um reproduzierbare Zufallszahlen
 * zu erzeugen.
 *
 * @param[in] x Initialisierungswert
 */
inline void srand(uint64_t x)
{
    Random::seed(x);
}

/**
 * @ingroup PPLGroupMath
 * @brief Zufallszahlen erzeugen
 *
 * Mit dieser Funktion werden Zufallszahlen im Bereich von \p min bis \p max erzeugt.
 *
 * @param min Untere Grenze (inklusive)
 * @param max Obere Grenze (inklusive)
 * @return Zufallszahl im Bereich von \p min bis \p max
 */
inline size_t rand(size_t min, size_t max)
{
    return Random::get(min, max);
}

/**
 * @ingroup PPLGroupMath
 * @brief Zufallszahlen erzeugen
 *
 * Mit dieser Funktion werden Zufallszahlen im Bereich von \p min bis \p max erzeugt.
 *
 * @param min Untere Grenze (inklusive)
 * @param max Obere Grenze (exklusive, wird praktisch nie exakt erreicht)
 * @return Zufallszahl als float im Bereich von \p min bis \p max
 */
inline float randf(float min, float max)
{
    return Random::getFloat(min, max);
}

/**
 * @ingroup PPLGroupMath
 * @brief Zufallszahlen erzeugen
 *
 * Mit dieser Funktion werden Zufallszahlen im Bereich von \p min bis \p max erzeugt.
 *
 * @param min Untere Grenze (inklusive)
 * @param max Obere Grenze (exklusive, wird praktisch nie exakt erreicht)
 * @return Zufallszahl als double im Bereich von \p min bis \p max
 */
inline double randd(double min, double max)
{
    return Random::getDouble(min, max);
}

// ByteArray Random(size_t bytes);
// ByteArray& Random(ByteArray& buffer, size_t bytes);

} // namespace pplib

#endif // PPLIB_CORE_RANDOM_H_
