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

#ifndef PPLIB_CORE_ARGS_H_
#define PPLIB_CORE_ARGS_H_

#include <stdint.h>
#include <pplib/types/string.h>

namespace pplib
{
/**
 * @ingroup PPLGroupArgv
 * @brief Gibt den Wert eines bestimmten Arguments zurück
 *
 * Durchsucht die Argumente nach dem angegebenen Argument und gibt den zugehörigen Wert zurück.
 *
 * @param argc Anzahl der Argumente
 * @param argv Array der Argumente
 * @param argument Zu suchendes Argument
 * @return Wert des Arguments oder leerer String, wenn nicht gefunden
 */
String GetArgv(int argc, char* argv[], const String& argument);

/**
 * @ingroup PPLGroupArgv
 * @brief Überprüft, ob ein bestimmtes Argument vorhanden ist
 *
 * Durchsucht die Argumente nach dem angegebenen Argument und gibt zurück, ob es vorhanden ist.
 *
 * @param argc Anzahl der Argumente
 * @param argv Array der Argumente
 * @param argument Zu suchendes Argument
 * @return true, wenn das Argument vorhanden ist, sonst false
 */
bool HaveArgv(int argc, char* argv[], const String& argument);

/**
 * @ingroup PPLGroupArgv
 * @brief Klasse zur Verwaltung von Programmargumenten
 *
 * Diese Klasse kapselt die Argumente argc und argv und bietet Methoden zum Überprüfen und Abrufen von Argumenten.
 */
class Args
{
private:
    int argc_{0};
    char** argv_{nullptr};

public:
    /**
     * @brief Standardkonstruktor
     *
     * Initialisiert die Klasse ohne Argumente.
     */
    Args() = default;

    /**
     * @brief Konstruktor mit Argumenten
     *
     * Initialisiert die Klasse mit den angegebenen Argumenten argc und argv.
     *
     * @param argc Anzahl der Argumente
     * @param argv Array der Argumente
     */
    Args(int argc, char* argv[])
        : argc_(argc),
          argv_(argv)
    {
    }

    /**
     * @brief Destruktor
     *
     * Standarddestruktor der Klasse.
     */
    ~Args() = default;

    /**
     * @brief Kopierkonstruktor
     *
     * Erstellt eine Kopie eines vorhandenen Args-Objekts.
     * @param other Das zu kopierende Args-Objekt
     */
    Args(const Args& other) noexcept = default;

    /**
     * @brief Verschiebekonstruktor
     *
     * Erstellt ein Args-Objekt durch Verschieben eines vorhandenen Args-Objekts.
     * @param other Das zu verschiebende Args-Objekt
     */
    Args(Args&& other) noexcept = default;

    /**
     * @brief Verschiebezuweisungsoperator
     *
     * Weist einem Args-Objekt die Werte eines anderen Args-Objekts durch Verschieben zu.
     * @param other Das zu verschiebende Args-Objekt
     * @return Referenz auf das aktuelle Args-Objekt
     */
    Args& operator=(Args&& other) noexcept = default;

    /**
     * @brief Kopierzuweisungsoperator
     *
     * Weist einem Args-Objekt die Werte eines anderen Args-Objekts durch Kopieren zu.
     * @param other Das zu kopierende Args-Objekt
     * @return Referenz auf das aktuelle Args-Objekt
     */
    Args& operator=(const Args& other) noexcept = default;

    /**
     * @brief Gibt die Anzahl der Argumente zurück
     *
     * @return Anzahl der Argumente
     */
    inline int argc() const
    {
        return argc_;
    }

    /**
     * @brief Gibt das Array der Argumente zurück
     *
     * @return Array der Argumente
     */
    inline char** argv() const
    {
        return argv_;
    }

    /**
     * @brief Überprüft, ob ein bestimmtes Argument vorhanden ist
     *
     * @param argument Das zu überprüfende Argument
     * @return true, wenn das Argument vorhanden ist, sonst false
     */
    inline bool has(const String& argument) const
    {
        return HaveArgv(argc_, argv_, argument);
    }

    /**
     * @brief Gibt den Wert eines bestimmten Arguments zurück
     *
     * @param argument Das zu überprüfende Argument
     * @param default_value Der Standardwert, der zurückgegeben wird, wenn das Argument nicht vorhanden ist
     * @return Wert des Arguments oder der Standardwert, wenn das Argument nicht vorhanden ist
     */
    inline String get(const String& argument, const String& default_value = "") const
    {
        String value = GetArgv(argc_, argv_, argument);
        if (value.isEmpty()) return default_value;
        return value;
    }
};

} // namespace pplib

#endif // PPLIB_CORE_ARGS_H_
