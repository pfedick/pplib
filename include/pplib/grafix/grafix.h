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

#ifndef PPLIB_INCLUDE_GRAFIX_GRAFIX_H
#define PPLIB_INCLUDE_GRAFIX_GRAFIX_H

#include <stdint.h>
#include <list>
#include <map>
#include <pplib/core/mutex.h>
#include <pplib/core/baseexception.h>
#include <pplib/core/fileobject.h>
#include <pplib/grafix/rgbformat.h>
#include <pplib/grafix/imagelist.h>

namespace pplib::grafix
{

PPLIBEXCEPTION(NoGrafixEngineException, Exception);
PPLIBEXCEPTION(DuplicateGrafixEngineException, Exception);
PPLIBEXCEPTION(UnknownColorFormatException, Exception);
PPLIBEXCEPTION(DuplicateFontEngineException, Exception);
PPLIBEXCEPTION(DuplicateImageFilterException, Exception);
PPLIBEXCEPTION(InvalidFontException, Exception);
PPLIBEXCEPTION(NoSuitableFontEngineException, Exception);

class ImageFilter;
class ImageList;
class FontEngine;
class FontFile;
class Font;

typedef struct DRAWABLE_FUNCTIONS DRAWABLE_FUNCTIONS;
typedef struct IMAGE IMAGE;

/** @class Grafix
 * @ingroup PPLGroupGrafik
 * @brief Hauptklasse für alle Grafik-Operationen
 *
 * Diese Klasse wird für alle grafischen Funktionen benötigt und muß einmalig
 * von der Anwendung instanziiert werden. Beim Versuch eine zweite Instanz anzulegen,
 * wird eine Exception geworfen.
 *
 * Bei Initialisierung werden eventuell eine Reihe von Standard-Image- und Font-Engines geladen,
 * die in der PPLIB enthalten sind. Bei Bedarf können weitere Engines hinzugefügt werden.
 *
 * Beim Laden eines Bildes oder Fonts wird automatisch die passende Engine gesucht und
 * verwendet, sofern eine vorhanden ist.
 *
 * @note Die Klasse ist Thread-Safe. Mehrere Threads können gleichzeitig auf die
 * Funktionen der Klasse zugreifen.
 *
 * @exception ExistingGrafixInstance Wird geworfen, wenn versucht wird eine zweite
 * Instanz der Klasse zu erstellen
 */
class Grafix
{
private:
    Mutex myMutex;
    std::list<ImageFilter*> ImageFilterList;
    std::list<FontEngine*> FontEngineList;
    std::map<String, FontFile*> FontList;

    void initDrawable32(DRAWABLE_FUNCTIONS* fn, const RGBFormat& format) noexcept;

public:
    /**
     * @brief Konstruktor der Klasse Grafix
     *
     * Muss einmalig von der Anwendung aufgerufen werden. Es werden diverse Standard-Image-
     * und Font-Engines geladen, die in der PPLIB enthalten sind.
     *
     * @exception DuplicateGrafixEngineException Wird geworfen, wenn bereits eine Instanz der Klasse existiert.
     */
    Grafix();

    /**
     * @brief Destruktor der Klasse Grafix
     *
     * Gibt alle Ressourcen der Grafix-Engine wieder frei. Alle geladenen Image- und Font-Engines
     * werden ebenfalls freigegeben.
     */
    ~Grafix();

    DRAWABLE_FUNCTIONS* getGrafixFunctions(const RGBFormat& format);

    /// @name Image-Filter und Loader
    //@{

    /** @brief Image-Filter registrieren
     *
     * Mit dieser Funktion wird ein neuer Image-Filter registriert. Ein Image-Filter ist
     * eine von ImageFilter abgeleitete Klasse, die in der Lage ist ein bestimmtes Grafikformat
     * zu lesen und optional auch zu schreiben.
     *
     * Der Filter muss mit "new" angelegt worden sein. Grafix übernimmt dessen
     * Verwaltung, dass heisst die Klasse kümmert sich auch um das Löschen. Mit
     * Grafix::unloadImageFilter kann ein Filter von der Anwendung manuell wieder entfernt
     * werden.
     *
     * @param[in] filter Pointer auf den zu registrierenden Filter
     * @exception DuplicateImageFilterException Wird geworfen, wenn der Filter bereits registriert ist.
     *
     * @see
     * - ImageFilter
     * - Grafix::unloadImageFilter
     * - Grafix::findImageFilter
     */
    void addImageFilter(ImageFilter* filter);

    /** @brief Image-Filter entfernen
     *
     * Mit dieser Funktion wird ein zuvor mit Grafix::addImageFilter registrierter Image-Filter
     * aus der Grafik-Engine entfernt. Die Verantwortung für die Verwaltung des Speichers geht
     * damit an die Anwendung zurück, das heißt die Anwendung muss diesen mit delete löschen.
     *
     * @param[in] filter Pointer auf den zu entfernenden Image-Filter
     */
    void unloadImageFilter(ImageFilter* filter) noexcept;

    /** @brief Filter anhand seines Namens finden
     *
     * Jeder Filter muss einen eindeutigen Namen haben. Mit dieser Funktion kann ein
     * registrierter Filter anhand seines Namens gefunden werden.
     *
     * @param[in] name String mit dem Namen des gesuchten Filters
     * @return Bei Erfolg liefert die Funktion einen Pointer auf den gefundenen Filter
     * zurück, im Fehlerfall einen nullptr.
     *
     * @exception
     * UnknownImageFormatException
     */
    ImageFilter* findImageFilter(const String& name) noexcept;

    /** @brief Filter anhand des Inhalts einer geöffneten Datei finden
     *
     * Mit dieser Funktion kann ein registrierter Filter anhand des Inhalts einer
     * bereits geöffneten Datei gefunden werden. Dazu wird die Funktion ImageFilter::ident
     * von jedem registrierten Filter aufgerufen, bis einer signalisiert, dass er das
     * Format verarbeiten kann.
     *
     * @param[in] ff Referenz auf die geöffnete Datei
     * @param[in] img Referenz auf eine IMAGE-Struktur, die von der Funktion mit den
     * Eigenschaften der Grafikdatei gefüllt wird.
     *
     * @return Bei Erfolg liefert die Funktion einen Pointer auf den gefundenen Filter
     * zurück, im Fehlerfall nullptr
     */
    ImageFilter* findImageFilter(FileObject& ff, IMAGE& img) noexcept;
    //@}

    /// @name Fonts
    //@{

    /** @brief FontEngine hinzufügen
     *
     * Mit dieser Funktion wird eine FontEngine der Grafikengine hinzugefügt. Darunter
     * versteht man eine von FontEngine abgeleitete Klasse, die in der Lage ist Fontdateien
     * in einem bestimmten Format zu verarbeiten und darzustellen.
     *
     * Die FontEngine muss von der Anwendung mit "new" erstellt werden und als Pointer
     * an die Grafix-Engine übergeben werden. Grafix verwaltet die Engine ab diesem
     * Moment und kümmert sich auch um deren Löschung bei Programmende. Die Anwendung darf
     * die FontEngine nicht selbst löschen!
     *
     * @param engine Pointer auf die Klasse mit der FontEngine
     * @exception DuplicateFontEngineException Wird geworfen, wenn die Engine bereits registriert ist.
     */
    void addFontEngine(FontEngine* engine);

    /** @brief Fontdatei laden
     *
     * Mit dieser Funktion wird ein Font aus einer Datei geladen.
     *
     * @param filename Dateiname der Fontdatei
     * @param fontname Name, unter dem der Font in der Grafix-Engine registriert werden soll. Falls dieser
     * nicht angegeben wird, wird entweder der Name aus der Fontdatei verwendet oder ein Name aus dem
     * Dateinamen generiert.
     * @exception InvalidFontException Wird geworfen, wenn die Fontdatei nicht geladen werden konnte.
     * @exception NoSuitableFontEngineException Wird geworfen, wenn keine passende FontEngine für
     */
    void loadFont(const String& filename, const String& fontname = String());

    /**
     * @brief  Fontdatei aus einer geöffneten Datei laden
     *
     * Mit dieser Funktion wird ein Font aus einer bereits geöffneten Datei geladen.
     *
     * @param ff Referenz auf eine bereits geöffnete Datei mit den Fontdaten
     * @param fontname Name, unter dem der Font in der Grafix-Engine registriert werden soll. Falls dieser
     * nicht angegeben wird, wird entweder der Name aus der Fontdatei verwendet oder ein Name aus dem
     * Dateinamen generiert.
     * @exception InvalidFontException Wird geworfen, wenn die Fontdatei nicht geladen werden konnte.
     * @exception NoSuitableFontEngineException Wird geworfen, wenn keine passende FontEngine für
     */
    void loadFont(FileObject& ff, const String& fontname = String());

    /** @brief Fontdatei aus Speicherbereich laden
     *
     * Mit dieser Funktion wird ein Font aus einem Speicherbereich geladen.
     *
     * @param memory Speicherbereich mit den Fontdaten
     * @param fontname Name, unter dem der Font in der Grafix-Engine registriert werden soll. Falls dieser
     * nicht angegeben wird, wird entweder der Name aus der Fontdatei verwendet oder ein Name aus dem
     * Dateinamen generiert.
     * @exception InvalidFontException Wird geworfen, wenn die Fontdatei nicht geladen werden konnte.
     * @exception NoSuitableFontEngineException Wird geworfen, wenn keine passende FontEngine für
     */
    void loadFont(const ByteArrayPtr& memory, const String& fontname = String());

    /** @brief Fontdatei entladen
     *
     * Mit dieser Funktion wird ein zuvor geladener Font wieder aus der Grafix-Engine entfernt.
     * Die Anwendung kann den Font danach nicht mehr verwenden.
     *
     * @param fontname Name des Fonts, der entfernt werden soll
     */
    void unloadFont(const String& fontname) noexcept;

    /** @brief Fontdatei anhand des Namens suchen
     *
     * Mit dieser Funktion kann ein zuvor geladener Font anhand seines Namens gesucht werden.
     *
     * @param fontname Name des gesuchten Fonts
     * @return Bei Erfolg liefert die Funktion einen Pointer auf die Fontdatei zurück, im Fehlerfall nullptr.
     */
    FontFile* findFont(const String& fontname) noexcept;

    /** @brief Fontdatei anhand des Font-Objekts suchen
     *
     * Mit dieser Funktion kann ein zuvor geladener Font anhand eines Font-Objekts gesucht werden.
     *
     * @param font Referenz auf ein Font-Objekt, das den Namen des gesuchten Fonts enthält
     * @return Bei Erfolg liefert die Funktion einen Pointer auf die Fontdatei zurück, im Fehlerfall nullptr.
     */
    FontFile* findFont(const Font& font) noexcept;

    /** @brief Liste aller geladenen Fonts ausgeben
     *
     * Mit dieser Funktion wird eine Liste aller in der Grafix-Engine geladenen Fonts auf die Standardausgabe
     * ausgegeben. Dabei werden der Name des Fonts und die verwendete FontEngine angezeigt.
     */
    void listFonts() noexcept;
    //@}
};

/**
 * @brief Liefert eine Referenz auf die Grafix-Engine zurück.
 *
 * @return Referenz auf die Grafix-Engine
 * @exception NoGrafixEngineException Wird geworfen, wenn noch keine Instanz der Grafix-Klasse existiert.
 */
Grafix& GetGrafix();
} // namespace pplib::grafix
#endif // PPLIB_INCLUDE_GRAFIX_GRAFIX_H
