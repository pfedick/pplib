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
#ifndef PPLIB_TYPES_ASSOCARRAY_H_
#define PPLIB_TYPES_ASSOCARRAY_H_

#include <stdint.h>
#include <map>

#include <pplib/types/variant.h>
#include <pplib/types/string.h>
#include <pplib/core/baseexception.h>

namespace pplib
{

class Variant;

/** @class AssocArray
 * @ingroup PPLGroupDataTypes
 * @brief Komplexes mehrdimensionales %Array mit Strings als Schlüssel
 *
 * Die Klasse AssocArray dient als Container für beliebige Key-Value-Paare. Ein Schlüssel
 * (Key) besteht aus einem String, der aus beliebigen Zeichen bestehen kann. Ein Value kann
 * veschiedene Datentypen enthalten. Gegenwärtig werden folgende Datentypen unterstützt:
 * - String
 * - WideString
 * - Array
 * - ByteArray
 * - ByteArrayPtr
 * - AssocArray
 * - DateTime
 * - Date
 * - Time
 * - TimeDelta
 * - TimeZone
 *
 * Die Schlüssel werden sortiert in einer std::map verwaltet, so dass auch bei
 * sehr großen Arrays eine schnelle Verarbeitung gewährleistet ist. Gross-/Kleinschreibung wird
 * ignoriert, der Schlüssel "TEST" wäre also identisch mit "test" oder "Test".
 *
 * Mehrdimensionale Arrays sind möglich, indem einem Schlüssel als Wert einfach ein anderes AssocArray
 * zugeordnet wird. In einem solchen AssocArray kann jedes Element direkt angesprochen werden, indem man
 * die einzelnen Schlüssel durch Slash (/) getrennt zu einem einzigen Schlüssel zusammenfasst.
 *
 * Mehrdimensionale Arrays werden automatisch generiert. Gibt man bei einem leeren %Array dem Schlüssel
 * <tt>"ebene1/ebene2/key"</tt> einen Wert, werden automatisch folgende Aktionen ausgeführt:
 * - Es wird ein neues AssocArray generiert und mit dem Schlüssel "ebene1" in das %Array eingefügt
 * - In das %Array "ebene1" wird ein weiteres neues %Array mit dem Schlüssel "ebene2" eingefügt
 * - In das %Array "ebene2" wird der eigentliche Wert unter dem Schlüssel "key" eingefügt
 */
class AssocArray
{
private:
    /** @brief Inzterne Vergleichsfunktion für die Schlüssel in der std::map
     *
     * Diese Funktion vergleicht zwei Schlüssel aus dem AssocArray. Sind Beide Strings nummerisch,
     * wird ein nummerischer Vergleich durchgeführt. Ist nur einer der beiden Schlüssel Nummerisch, wird
     * der nummerische Wert vor dem anderen einsortiert. Sind beide Werte Strings, wird ein
     * Case-Insensitiver Vergleich durchgeführt. Die Funktion wird von den Vergleichoperatoren aufgerufen.
     */
    struct ArrayKeyCompare
    {
        // using is_transparent = void; // Ermöglicht find() mit const char*, String etc.

        bool operator()(const String& a, const String& b) const
        {
            // 1. Beide numerisch -> numerischer Vergleich
            bool aNum = a.isNumeric();
            bool bNum = b.isNumeric();
            if (aNum && bNum) {
                return a.toInt64() < b.toInt64();
            }
            // 2. Einer numerisch, einer Text -> Zahlen kommen vor Text (konsistente Ordnung)
            if (aNum != bNum) {
                return aNum; // Numerische Keys zuerst
            }
            // 3. Beide Text -> Case-Insensitiver Vergleich
            return a.strCaseCmp(b) < 0;
        }
    };

    std::map<String, Variant*, ArrayKeyCompare> Tree;
    uint64_t maxint;

    /** @brief Interne Funktion zum Suchen eines Elements
     *
     * Diese Funktion zerlegt den angegebenen Schlüssel (\p key) in seine einzelnen Elemente.
     * Als Trennzeichen wird wie bei einer Unix-Pfadangabe der Slash (/) verwendet. Die Funktion
     * sucht zunächst nach dem erste Element des Schlüssels im eigenen Baum. Ist dies vorhanden
     * und handelt es sich bei dessen Datentyp wieder um ein AssocArray, wird deren
     * findInternal-Funktion mit dem restlichen Schlüssel rekursiv aufgerufen. Dies geschieht
     * solange, bis das letzte Element des Keys gefunden wurde.
     * @param[in] key String mit dem gesuchten Schlüssel
     * @return Konnte der Schlüssel gefunden werden, wir der Pointer auf das Element (Variant)
     * zurückgegeben. Wurde der Schlüssel nicht gefunden, wird NULL zurückgegeben
     * @exception InvalidKeyException: Wird geworfen, wenn der Schlüssel ungültig oder leer ist
     * @note
     * Die Funktion wird von allen Get...- und Concat-Funktionen verwendet.
     */
    Variant* findInternal(const String& key) const;

    /** @brief Interne Funktion, die ein Element im Baum sucht oder anlegt
     *
     * Diese Funktion durchsucht den Baum nach dem gewünschten Element. Ist es vorhanden, wird dessen Pointer
     * zurückgeliefert, wenn nicht, wird es angelegt, jedoch ohne Daten. Bei verschachtelten Schlüsseln wird
     * die Funktion rekursiv aufgerufen, bis das letzte Element erreicht ist. Die Funktion wird intern von allen
     * Funktionen verwendet, die Daten in das Array speichern.
     * @param[in] key Pointer auf den Namen des Schlüssels
     * @param[in] var Pointer auf die Daten, die unter diesem Schlüssel abgelegt werden sollen
     * @return Bei Erfolg liefert die Funktion einen Pointer auf das gewünschte Element zurück.
     * Im Fehlerfall wird eine Exception geworfen.
     *
     * @exception InvalidKeyException: Wird geworfen, wenn der Schlüssel ungültig oder leer ist
     * @exception std::bad_alloc: Kein Speicher mehr frei
     *
     * @remarks Bei der Angabe eines verschachtelten Schlüssels kann es vorkommen, dass bereits vorhandene
     * Elemente überschrieben werden. Beispiel:
     *
     * Das Element <tt>ebene1/schlüssel1</tt> ist im Baum bereits vorhanden und beinhaltet einen String. Nun wird
     * das neue Element <tt>ebene1/schlüssel1/unterschlüssel1</tt> angelegt. Da Schlüssel eindeutig sein müssen,
     * wird der String <tt>ebene1/schlüssel1</tt> gelöscht und in ein Array umgewandelt.
     */
    Variant* createTree(const String& key);

    /** @brief Inhalt des Arrays in einem plattform-unabhängigen Binären-Format exportieren
     *
     * Mit dieser Funktion kann der komplette Inhalt des Arrays in einem plattform-unabhängigem binären Format abgelegt
     * werden, das sich zum Speichern in einer Datei oder zum Übertragen über das Internet eignet.
     * @param[in] buffer Pointer auf einen ausreichend großen Puffer. Die Größe des benötigten Puffers
     * kann zuvor mit der Funktion AssocArray::binarySize ermittelt werden. Wird als Buffer NULL
     * übergeben, wird in der Variable @p realsize ebenfalls die Anzahl Bytes zurückgegeben
     * @param[in] buffersize Die Größe des Puffers in Bytes
     * @param[out] realsize In dieser Variable wird gespeichert, wieviele Bytes tatsächlich für den Export
     * verwendet wurden
     * @exception ExportBufferToSmallException: Wird geworfen, wenn @p buffersize nicht groß genug ist, um
     * das Assoziative Array vollständig exportieren zu können.
     *
     * @attention
     * Es muss daran gedacht werden, dass nicht alle Datentypen exportiert werden können. Gegenwärtig
     * werden folgende Typen unterstützt:
     * - String (Wird als UTF-8 exportiert)
     * - Array
     * - AssocArray
     * - ByteArray
     * - ByteArrayPtr (wird in ein ByteArray umgewandelt!)
     * - DateTime
     * @see
     * - AssocArray::binarySize
     * - AssocArray::importBinary
     *
     * @note
     * Das exportierte Binary ist komptibel mit dem Assoziativen Array der PPL-Version 6
     */
    size_t exportBinary(void* buffer, size_t buffersize) const;

    /** @brief Daten aus einem vorherigen Export wieder importieren
     *
     * Mit dieser Funktion kann ein zuvor mit AssocArray::exportBinary exportiertes Assoziatives %Array wieder
     * importiert werden. Falls im %Array bereits Daten vorhanden sind, werden diese nicht gelöscht, können aber
     * überschrieben werden, wenn es im Export gleichnamige Schlüssel gibt.
     * @param[in] buffer Pointer auf den Puffer, der die zu importierenden Daten enthält
     * @param[in] buffersize Größe des Puffers
     * @exception ImportFailedException
     *
     * @see
     * - AssocArray::exportBinary
     * - AssocArray::binarySize
     */
    size_t importBinary(const void* buffer, size_t buffersize);

public:
    PPLIBEXCEPTION(InvalidKeyException, Exception);
    PPLIBEXCEPTION(ExportBufferToSmallException, Exception);
    PPLIBEXCEPTION(ImportFailedException, Exception);

    typedef std::map<String, Variant*, ArrayKeyCompare>::iterator iterator;
    typedef std::map<String, Variant*, ArrayKeyCompare>::const_iterator const_iterator;
    typedef std::map<String, Variant*, ArrayKeyCompare>::reverse_iterator reverse_iterator;
    typedef std::map<String, Variant*, ArrayKeyCompare>::const_reverse_iterator const_reverse_iterator;

    //!\name Konstruktoren und Destruktoren
    //@{

    /** @brief Konstruktor des Assoziativen Arrays
     *
     * Initialisiert die Instanz mit 0 und initialisiert den AVL-Baum.
     */
    AssocArray();

    /** @brief Copy-Konstruktor des Assoziativen Arrays
     *
     * Macht eine Kopie des Assoziativen Arrays \p other.
     * @param[in] other Referenz auf zu kopierendes Assoziatives Array
     *
     * @exception std::bad_alloc: Kein Speicher mehr frei
     * @exception OutOfMemoryException: Kein Speicher mehr frei
     * @exception InvalidKeyException: Ungültiger Schlüssel
     */
    AssocArray(const AssocArray& other);
    AssocArray(AssocArray&& other);

    /** @brief Destruktor der Klasse
     *
     * Der Destruktor ruft die Funktion AssocArray::clear auf, um alle vorhandenen Elemente zu
     * löschen.
     */
    ~AssocArray();
    //@}

    //!\name Informationen ausgeben/auslesen
    //@{

    /** @brief Anzahl Schlüssel zählen
     *
     * Diese Funktion liefert die Anzahl Schlüssel auf dieser Ebene des Array zurück.
     * @param[in] recursive Falls recursive auf true gesetzt wird die Funktion rekusriv für jeden
     * Schlüssel aufgerufen, dessen Wert ebenfalls ein Array ist.
     *
     * @returns Anzahl Schlüssel
     */
    size_t count(bool recursive = false) const;

    /** @brief Anzahl Schlüssel für ein bestimmtes Element zählen
     *
     * Diese Funktion liefert die Anzahl Schlüssel zurück, die in dem angegebenen Key enthalten sind.
     * @param[in] key Schlüssel-Name eines Assoziativen Arrays innerhalb dieses Arrays
     * @param[in] recursive Falls recursive auf true gesetzt wird die Funktion rekusriv für jeden
     * Schlüssel aufgerufen, dessen Wert ebenfalls ein Array ist.
     *
     * @returns Anzahl Schlüssel
     */
    size_t count(const String& key, bool recursive = false) const;

    /** @brief Anzahl Elemente
     *
     * Diese Funktion liefert die Anzahl Elemente auf dieser Ebene des Arrays zurück.
     * @returns Anzahl Elemente
     */
    size_t size() const;

    /** @brief Inhalt des Arrays ausgeben
     *
     * Diese Funktion dient Debugging-Zwecken. Der Aufruf bewirkt, dass der Inhalt des kompletten Arrays auf
     * STDOUT ausgegeben wird.
     * @param[in] prefix Optionaler Text, der bei der Ausgabe jedem Element vorangestellt wird
     *
     *
     * @code
     * pplib::AssocArray a;
     * pplib::Binary bin;
     * bin.load("main.cpp");
     *
     * a.set("key1","value1");
     * a.set("array1/unterkey1","value2");
     * a.set("array1/unterkey2","value3");
     * a.set("array1/noch ein array/unterkey1","value4");
     * a.set("array1/unterkey2","value5");
     * a.set("key2","value6");
     * a.set("dateien/main.cpp",bin);
     * a.set("array2/unterkey1","value7");
     * a.set("array2/unterkey2","value8");
     * a.set("array2/unterkey1","value9");
     * a.list("prefix");
     * @endcode
     * Ausgabe:
     * @code
     * prefix/array1/noch ein array/unterkey1=value4
     * prefix/array1/unterkey1=value2
     * prefix/array1/unterkey2=value5
     * prefix/array2/unterkey1=value9
     * prefix/array2/unterkey2=value8
     * prefix/dateien/main.cpp=Binary, 806 Bytes
     * prefix/key1=value1
     * prefix/key2=value6
     * @endcode
     *
     * @remarks Die Funktion gibt nur "lesbare" Element aus. Enthält das Array Pointer oder Binaries, wird das Element zwar
     * ausgegeben, jedoch werden als Wert nur Meta-Informationen ausgegeben (Datentyp, Pointer, Größe).
     */
    void list(const String& prefix = "") const;

    //@}

    //!\name Werte setzen
    //@{

    /** @brief %AssocArray kopieren
     *
     * Mit dieser Funktion wird der komplette Inhalt des Assoziativen Arrays \p other
     * in dieses hineinkopiert. Das Array wird vorher nicht gelöscht, so dass vorhandene
     * Schlüssel erhalten bleiben. Gibt es in \p other jedoch gleichnamige Schlüssel,
     * werden die bisherigen Werte überschrieben.
     * @param[in] a Das zu kopierende AssocArray
     *
     * @exception std::bad_alloc: Kein Speicher mehr frei
     * @exception OutOfMemoryException: Kein Speicher mehr frei
     * @exception InvalidKeyException: Ungültiger Schlüssel
     */
    void add(const AssocArray& other);
    inline void set(const String& key, const String& value)
    {
        createTree(key)->set(value);
    }
    inline void set(const String& key, const WideString& value)
    {
        createTree(key)->set(value);
    }
    inline void set(const String& key, const Array& value)
    {
        createTree(key)->set(value);
    }
    inline void set(const String& key, const ByteArray& value)
    {
        createTree(key)->set(value);
    }
    inline void set(const String& key, const ByteArrayPtr& value)
    {
        createTree(key)->set(value);
    }
    inline void set(const String& key, const AssocArray& value)
    {
        createTree(key)->set(value);
    }
    inline void set(const String& key, const DateTime& value)
    {
        createTree(key)->set(value);
    }
    inline void set(const String& key, const Date& value)
    {
        createTree(key)->set(value);
    }
    inline void set(const String& key, const Time& value)
    {
        createTree(key)->set(value);
    }
    inline void set(const String& key, const TimeDelta& value)
    {
        createTree(key)->set(value);
    }
    inline void set(const String& key, const TimeZone& value)
    {
        createTree(key)->set(value);
    }
    inline void set(const String& key, const Variant& value)
    {
        createTree(key)->set(value);
    }

    /** @brief Formatierten String hinzufügen
     *
     * Diese Funktion fügt den Inhalt eines formatierten Strings dem Array hinzu.
     * @param[in] key Name des Schlüssels
     * @param[in] fmt Pointer auf den Format-String des Wertes
     * @param[in] ... Beliebig viele Parameter, die vom Formatstring verwendet werden
     * @exception std::bad_alloc: Kein Speicher mehr frei
     * @exception OutOfMemoryException: Kein Speicher mehr frei
     * @exception InvalidKeyException: Ungültiger Schlüssel
     */
    void setf(const String& key, const char* fmt, ...);

    // Move-Varianten
    inline void set(const String& key, String&& value)
    {
        createTree(key)->set(std::move(value));
    }
    inline void set(const String& key, WideString&& value)
    {
        createTree(key)->set(std::move(value));
    }
    inline void set(const String& key, Array&& value)
    {
        createTree(key)->set(std::move(value));
    }
    inline void set(const String& key, ByteArray&& value)
    {
        createTree(key)->set(std::move(value));
    }
    inline void set(const String& key, AssocArray&& value)
    {
        createTree(key)->set(std::move(value));
    }
    inline void set(const String& key, DateTime&& value)
    {
        createTree(key)->set(std::move(value));
    }
    inline void set(const String& key, Date&& value)
    {
        createTree(key)->set(std::move(value));
    }
    inline void set(const String& key, Time&& value)
    {
        createTree(key)->set(std::move(value));
    }
    inline void set(const String& key, TimeDelta&& value)
    {
        createTree(key)->set(std::move(value));
    }
    inline void set(const String& key, TimeZone&& value)
    {
        createTree(key)->set(std::move(value));
    }
    inline void set(const String& key, Variant&& value)
    {
        createTree(key)->set(std::move(value));
    }

    //@}

    //!\name Werte erweitern (nur Strings)
    //@{

    /** @brief %String verlängern
     *
     * Diese Funktion fügt den Inhalt des Strings \p value an den bereits vorhandenen
     * Wert des Schlüssels \p key an. Falls der optionale Parameter \p concat einen Wert
     * enthält, wird dieser als Trennung zwischen bestehendem und neuem String verwendet.
     * War der Schlüssel bisher nicht vorhanden, wird ein neuer angelegt.
     * @param[in] key Name des Schlüssels
     * @param[in] value Wert
     * @param[in] concat Trennzeichen (Optional, Default=keins)
     * @exception std::bad_alloc: Kein Speicher mehr frei
     * @exception OutOfMemoryException: Kein Speicher mehr frei
     * @exception InvalidKeyException: Ungültiger Schlüssel
     * @exception TypeConversionException: Schlüssel ist bereits vorhanden, enthält aber keinen String
     */
    void append(const String& key, const String& value, const String& concat = "");

    /** @brief %String mit Formatiertem String verlängern
     *
     * Diese Funktion erstellt zuerst einen neuen String anhand des Formatstrings
     * @p fmt und der zusätzlichen optionalen Parameter. Dieser wird an den bereits vorhandenen
     * Wert des Schlüssels @p key angehangen. Falls der optionale Parameter @p concat einen Wert
     * enthält, wird dieser als Trennung zwischen bestehendem und neuem String verwendet.
     * War der Schlüssel bisher nicht vorhanden, wird ein neuer angelegt.
     *
     * @param[in] key Name des Schlüssels
     * @param[in] concat Trennzeichen (Optional, Default=keins)
     * @param[in] fmt Formatstring
     * @param[in] ... Optionale Parameter
     * @exception std::bad_alloc: Kein Speicher mehr frei
     * @exception OutOfMemoryException: Kein Speicher mehr frei
     * @exception InvalidKeyException: Ungültiger Schlüssel
     * @exception TypeConversionException: Schlüssel ist bereits vorhanden, enthält aber keinen String
     */
    void appendf(const String& key, const String& concat, const char* fmt, ...);
    //@}

    //!\name Werte löschen
    //@{

    /** @brief Inhalt des Arrays löschen
     *
     * Mit dieser Funktion wird der komplette Inhalt des Arrays gelöscht. Dabei der Destruktor für jedes
     * vorhandene Element aufgerufen, der wiederum sicherstellt, dass die darin enthaltenen Daten
     * ordnungsgemäß gelöscht werden.
     */
    void clear();

    /** @brief Einzelnen Schlüssel löschen
     *
     * Mit dieser Funktion wird ein einzelner Schlüssel aus dem Array gelöscht.
     * Handelt es sich dabei um den einzigen Schlüssel eines verschachtelten Arrays,
     * wird nur dieser Schlüssel gelöscht, nicht aber das danach leere Array.
     * Ist der Schlüssel ein verschachteltes Array, dann wird dieses und alle darin enthaltenen
     * Schlüssel rekursiv gelöscht.
     *
     * @param[in] key String mit dem Namen des zu löschenden Schlüssels
     *
     * @note
     */
    void erase(const String& key);

    /** @brief Einzelnen Schlüssel löschen
     *
     * Mit dieser Funktion wird ein einzelner Schlüssel aus dem Array gelöscht.
     * @param[in] key String mit dem Namen des zu löschenden Schlüssels
     */
    void remove(const String& key);
    //@}

    //!\name Import und Export von Daten
    //@{

    /** @brief Liefert Anzahl Bytes, die für exportBinary erforderlich sind
     *
     * Diese Funktion liefert die Anzahl Bytes zurück, die für den Buffer der Funktion AssocArray::exportBinary
     * erforderlich sind. Es kann dadurch ein ausreichend großer Puffer vor Aufruf der Funktion exportBinary
     * angelegt werden.
     * @return Anzahl Bytes oder 0 im Fehlerfall
     *
     * @see
     * - AssocArray::exportBinary
     * - AssocArray::importBinary
     */
    size_t binarySize() const;
    ByteArray exportBinary() const;

    /** @brief Inhalt des Arrays in einem plattform-unabhängigen Binären-Format exportieren
     *
     * Mit dieser Funktion kann der komplette Inhalt des Arrays in einem plattform-unabhängigem binären Format abgelegt
     * werden, das sich zum Speichern in einer Datei oder zum Übertragen über das Internet eignet.
     * @param[in,out] buffer %ByteArray, in dem die exportierten Daten gespeichert werden sollen
     *
     * @note
     * Alle Datentypen werden beim Export und Import Unterstützt:
     * - String
     * - WideString (Wird als String exportiert)
     * - Array
     * - AssocArray
     * - ByteArray
     * - ByteArrayPtr (wird in ein ByteArray umgewandelt und beim Import zu einem ByteArray!)
     * - DateTime
     * - Date
     * - Time
     * - TimeDelta
     * - TimeZone
     * @see
     * - AssocArray::binarySize
     * - AssocArray::importBinary
     *
     * @note
     * Das exportierte Binary ist nicht kompatibel mit vorherigen Versionen!
     */
    void exportBinary(ByteArray& buffer) const;

    /** @brief Daten aus einem vorherigen Export wieder importieren
     *
     * Mit dieser Funktion kann ein zuvor mit AssocArray::exportBinary exportiertes Assoziatives %Array wieder
     * importiert werden. Falls im %Array bereits Daten vorhanden sind, werden diese nicht gelöscht, können aber
     * überschrieben werden, wenn es im Export gleichnamige Schlüssel gibt.
     * @param[in] bin Referenz auf ByteArray oder ByteArrayPtr mit den zu importierenden Daten
     *
     * @see
     * - CAssocArray::exportBinary
     * - CAssocArray::binarySize
     */
    void importBinary(const ByteArrayPtr& buffer);
    //@}

    //!\name Werte direkt auslesen
    //@{

    /** @brief Schlüssel auslesen
     *
     * Diese Funktion liefert den Wert des Schlüssels \p key als Variant zurück. Dieser kann
     * von der aufrufenden Anwendung in den jeweiligen Datentyp umgewandelt werden.
     * @param key Name des Schlüssels
     * @return Referenz auf einen Variant mit dem Wert des Schlüssels
     * @exception InvalidKeyException: Ungültiger Schlüssel
     * @exception KeyNotFoundException: Schlüssel wurde nicht gefunden
     * @example
     * Der Variant kann z.B. folgendermaßen in einen String umgewandelt werden:
     * @code
     * pplib::String &str=a.get(L"key1").toString();
     * @endcode
     */
    const Variant& get(const String& key, Variant::DataType type = Variant::DataType::TYPE_UNKNOWN) const;
    Variant& get(const String& key, Variant::DataType type = Variant::DataType::TYPE_UNKNOWN);

    /** @brief String auslesen
     *
     * Diese Funktion liefert den Wert des Schlüssels \p key als String zurück, sofern
     * der Schlüssel auch tatsächlich einen String enthält.
     * @param key Name des Schlüssels
     * @return Referenz auf einen String mit dem Wert des Schlüssels
     * @exception InvalidKeyException: Ungültiger Schlüssel
     * @exception KeyNotFoundException: Schlüssel wurde nicht gefunden
     */
    String& getString(const String& key);
    const String& getString(const String& key) const;

    /** @brief String auslesen mit Standardwert
     *
     * Diese Funktion liefert den Wert des Schlüssels \p key als String zurück, sofern
     * er existiert und einen String enthält. Andernfalls wird der Standardwert \p default_value
     * zurückgegeben. Falls der Schlüssel existiert, aber kein String ist, wird ebenfalls der
     * Standardwert \p default_value zurückgegeben. Falls der Wert ein WideString ist, wird er
     * in einen String konvertiert und zurückgegeben.
     * @param key Name des Schlüssels
     * @param default_value Standardwert, der zurückgegeben wird, wenn der Schlüssel nicht existiert oder keinen String enthält
     * @return Referenz auf einen String mit dem Wert des Schlüssels oder dem Standardwert
     * @exception InvalidKeyException: Ungültiger Schlüssel (wenn @p key leer ist)
     * @exception std::bad_alloc: Speicher konnte nicht zugewiesen werden
     */
    String getString(const String& key, const String& default_value) const;

    /** @brief Integer auslesen mit Standardwert
     *
     * Diese Funktion liefert den Wert des Schlüssels \p key als Integer zurück, sofern
     * er existiert und einen String oder WideString enthält, der in einen Integer konvertiert werden kann.
     * Andernfalls wird der Standardwert \p default_value zurückgegeben.
     * @param key Name des Schlüssels
     * @param default_value Standardwert, der zurückgegeben wird, wenn der Schlüssel nicht existiert oder keinen Integer enthält
     * @return Integer Wert des Schlüssels oder der Standardwert
     * @exception InvalidKeyException: Ungültiger Schlüssel (wenn @p key leer ist)
     */
    int getInt(const String& key, int default_value = 0) const;

    /** @brief 64-Bit Integer auslesen mit Standardwert
     *
     * Diese Funktion liefert den Wert des Schlüssels \p key als 64-Bit Integer zurück, sofern
     * er existiert und einen String oder WideString enthält, der in einen 64-Bit Integer konvertiert werden kann.
     * Andernfalls wird der Standardwert \p default_value zurückgegeben.
     * @param key Name des Schlüssels
     * @param default_value Standardwert, der zurückgegeben wird, wenn der Schlüssel nicht existiert oder keinen 64-Bit Integer enthält
     * @return 64-Bit Integer Wert des Schlüssels oder der Standardwert
     * @exception InvalidKeyException: Ungültiger Schlüssel (wenn @p key leer ist)
     */
    int64_t getInt64t(const String& key, int64_t defaultValue = 0) const;

    /** @brief Boolean auslesen mit Standardwert
     *
     * Diese Funktion liefert den Wert des Schlüssels \p key als Boolean zurück, sofern
     * er existiert und einen String oder WideString enthält. Ist dies nicht der Fall, wird
     * der Standardwert \p default_value zurückgegeben.
     * @param key Name des Schlüssels
     * @param default_value Standardwert, der zurückgegeben wird, wenn der Schlüssel nicht existiert oder keinen Boolean enthält
     * @return Boolean Wert des Schlüssels oder der Standardwert
     * @exception InvalidKeyException: Ungültiger Schlüssel (wenn @p key leer ist)
     */
    bool getBoolean(const String& key, bool default_value = false) const;

    /** @brief AssocArray auslesen
     *
     * @param key Name des Schlüssels
     * @return Referenz auf einen String mit dem Wert des Schlüssels
     * @exception InvalidKeyException: Ungültiger Schlüssel
     * @exception KeyNotFoundException: Schlüssel wurde nicht gefunden
     */
    AssocArray& getAssocArray(const String& key);
    const AssocArray& getAssocArray(const String& key) const;
    Array& getArray(const String& key);
    const Array& getArray(const String& key) const;

    /** @brief Schlüssel vorhanden
     *
     * Diese Funktion prüft, ob der Schlüssels \p key im Assoziativen Array enthalten ist.
     * @param key Name des Schlüssels
     * @return Liefert @c true zurück, wenn der Schlüssel vorhanden ist, sonst @c false
     * @exception InvalidKeyException: Ungültiger Schlüssel
     */
    bool exists(const String& key) const;

    /** @brief Key vorhanden und True
     *
     * Liefert True zurück, wenn der Schlüssel \b key vorhanden ist, und dessen Value einen String oder
     * WideString enthält, dessen Boolean Wert True entspricht. Andernfalls wird False zurückgegeben.
     * @param key Name des Schlüssels
     * @return True oder False
     */
    bool isTrue(const String& key) const;

    //@}

    //!\name Array durchwandern
    //@{
    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
    reverse_iterator rbegin();
    const_reverse_iterator rbegin() const;
    reverse_iterator rend();
    const_reverse_iterator rend() const;

    const_iterator cbegin() const;
    const_iterator cend() const;
    const_reverse_iterator crbegin() const;
    const_reverse_iterator crend() const;
    //@}

    //!\name Operatoren
    //@{
    Variant& operator[](const String& key);

    /** @brief Schlüssel auslesen
     *
     * Dieser Operator liefert den Wert des Schlüssels \p key als Variant zurück. Dieser kann
     * von der aufrufenden Anwendung in den jeweiligen Datentyp umgewandelt werden.
     * @param key Name des Schlüssels
     * @return Referenz auf den einen Variant mit dem Wert des Schlüssels
     * @exception InvalidKeyException: Ungültiger Schlüssel
     * @exception KeyNotFoundException: Schlüssel wurde nicht gefunden
     */
    const Variant& operator[](const String& key) const;

    /** @brief Assoziatives Array kopieren
     *
     * Mit diesem Operator wird der Inhalt das Assoziativen Arrays \p other übernommen.
     * Der bisherige Inhalt dieses Arrays geht verloren.
     * @param other Zu kopierendes assoziatives Array
     * @return Referenz auf dieses Array
     * @exception std::bad_alloc: Kein Speicher mehr frei
     * @exception OutOfMemoryException: Kein Speicher mehr frei
     * @exception InvalidKeyException: Ungültiger Schlüssel
     */
    AssocArray& operator=(const AssocArray& other);
    AssocArray& operator=(AssocArray&& other) noexcept;

    /** @brief Assoziatives Array hinzufügen
     *
     * Mit diesem Operator wird der Inhalt das Assoziativen Arrays \p other dem eigenen
     * Array hinzugefügt. Das Array wird vorher nicht gelöscht, so dass vorhandene
     * Schlüssel erhalten bleiben. Gibt es in \p other jedoch gleichnamige Schlüssel,
     * werden die bisherigen Werte überschrieben.
     * @param other Zu kopierendes assoziatives Array
     * @return Referenz auf dieses Array
     * @exception std::bad_alloc: Kein Speicher mehr frei
     * @exception OutOfMemoryException: Kein Speicher mehr frei
     * @exception InvalidKeyException: Ungültiger Schlüssel
     */
    AssocArray& operator+=(const AssocArray& other);

    bool operator==(const AssocArray& other) const;
    bool operator!=(const AssocArray& other) const;
    //@}
};
AssocArray operator+(const AssocArray& a1, const AssocArray& a2);

} // namespace pplib

#endif /* PPLIB_TYPES_ASSOCARRAY_H_ */
