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

#ifndef PPLIB_CRYPTO_SECURERANDOM_H_
#define PPLIB_CRYPTO_SECURERANDOM_H_

#include <cstddef>
#include <stdint.h>
#include <pplib/types/bytearray.h>

namespace pplib
{


/**
 * @brief Kryptographisch sichere Zufallszahlen erzeugen
 * @ingroup PPLGroupMath
 * @ingroup PPLIB_CRYPT
 *
 * Diese Klasse stellt Funktionen zur Erzeugung kryptographisch sicherer Zufallszahlen bereit.
 * Sie sind geeignet für die Erzeugung von Schlüsseln, Initialisierungsvektoren (IVs) und Nonces.
 * Modul: pplib_crypto
 *
 * @note Diese Klasse verwendet OpenSSL zur Erzeugung kryptographisch sicherer Zufallszahlen.
 * Ist OpenSSL nicht verfügbar, werfen alle Funktionen UnsupportedFeatureException.
 *
 * @note Wenn keine kryptographisch sicheren Zufallszahlen benötigt werden, kann stattdessen die Klasse pplib::Random verwendet werden,
 * die pseudo-Zufallszahlen generiert und kein OpenSSL benötigt.
 * @see Random
 */
class SecureRandom
{
public:
    /** @brief Kryptographisch sichere Zufallsdaten erzeugen
        *
        * Mit dieser Funktion werden \p num_bytes Bytes an kryptographisch sicheren Zufallsdaten (CSPRNG) erzeugt und als
        * ByteArray Objekt zurückgegeben. Im Gegensatz zu pplib::Random::bytes() ist diese Funktion für Schlüssel,
        * IVs und Nonces geeignet. Zur Erzeugung von kryptographisch sicheren Zufallszahlen wird OpenSSL verwendet.
        *
        * @param[in] num_bytes Anzahl Bytes
        * @return Gibt ein ByteArray Objekt mit der gewünschten Anzahl kryptographisch sicherer Zufallsdaten
        * @exception OperationFailedException Wenn die Erzeugung kryptographisch sicherer Zufallsdaten fehlschlägt
        * @exception UnsupportedFeatureException Wenn OpenSSL nicht verfügbar ist
        */
    static ByteArray bytes(size_t num_bytes);

    /** @brief Kryptographisch sichere Zufallsdaten erzeugen
        *
        * Mit dieser Funktion werden \p num_bytes Bytes an kryptographisch sicheren Zufallsdaten (CSPRNG) erzeugt und in das
        * ByteArray \p buffer geschrieben.
        *
        * @param[in] buffer ByteArray Objekt, in dem die Zufallsdaten abgelegt werden
        * @param[in] num_bytes Anzahl Bytes
        * @return Referenz auf das ByteArray Objekt \p buffer
        * @exception OperationFailedException Wenn die Erzeugung kryptographisch sicherer Zufallsdaten fehlschlägt
        * @exception UnsupportedFeatureException Wenn OpenSSL nicht verfügbar ist
        */
    static ByteArray& fill(ByteArray& buffer, size_t num_bytes);

    /** @brief Kryptographisch sichere Zufallsdaten erzeugen
        *
        * Mit dieser Funktion werden kryptographisch sichere Zufallsdaten in das ByteArray \p buffer geschrieben.
        * Es wird die gesamte Größe des ByteArrays \p buffer verwendet.
        *
        * @param[in] buffer ByteArray Objekt, in dem die Zufallsdaten abgelegt werden
        * @return Referenz auf das ByteArray Objekt \p buffer
        * @exception OperationFailedException Wenn die Erzeugung kryptographisch sicherer Zufallsdaten fehlschlägt
        * @exception UnsupportedFeatureException Wenn OpenSSL nicht verfügbar ist
        */
    static ByteArray& fill(ByteArray& buffer);

    /** @brief Kryptographisch sichere Zufallszahlen erzeugen
        *
        * Mit dieser Funktion wird eine kryptographisch sichere Zufallszahl im Bereich von \p min bis \p max erzeugt.
        *
        * @param min Untere Grenze (inklusive)
        * @param max Obere Grenze (inklusive)
        * @return Zufallszahl im Bereich von \p min bis \p max
        * @exception OperationFailedException Wenn die Erzeugung kryptographisch sicherer Zufallszahlen fehlschlägt
        * @exception UnsupportedFeatureException Wenn OpenSSL nicht verfügbar ist
        */
    static size_t get(size_t min, size_t max);
};


} // namespace pplib

#endif // PPLIB_CRYPTO_SECURERANDOM_H_
