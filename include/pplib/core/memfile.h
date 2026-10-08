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

#ifndef PPLIB_CORE_MEMFILE_H_
#define PPLIB_CORE_MEMFILE_H_

#include <pplib/core/fileobject.h>
#include <pplib/types/bytearray.h>

namespace pplib
{

/** @class MemFile
 * @ingroup PPLGroupFileIO
 * @brief Simulation von Dateizugriffen im Hauptspeicher
 *
 * \desc
 * Mit dieser von FileObject abgeleiteten Klasse werden Dateizugriffe im Hauptspeicher simuliert.
 * Sie kann immer dann verwendet werden, wenn sich die zu lesende Datei bereits im Hauptspeicher
 * befindet, oder Daten temporär im Hauptspeicher abgelegt werden sollen.
 *
 * Der zu verwendende Speicherbereich kann entweder über den Konstruktor abgegeben werden (siehe
 * MemFile::MemFile (void * adresse, size_t size) ) oder über die Funktion MemFile::open. Soll
 * der Speicherbereich auch beschrieben werden, muss als dritter Parameter "true" angegeben
 * oder die Funktion MemFile::openReadWrite verwendet werden.
 */
class MemFile : public FileObject
{
private:
    ByteArray writebuffer; // Puffer für Schreiboperationen
    size_t mysize;         // Aktuelle Größe des Speicherbereichs
    size_t pos;            // Aktuelle Position im Speicherbereich
    size_t maxsize;        // Maximale Größe des Speicherbereichs bei Schreiboperationen
    char* MemBase;         // Basisadresse des Speicherbereichs
    bool readonly;         // Gibt an, ob der Speicherbereich nur lesbar ist
    bool is_open;          // Gibt an, ob die Datei geöffnet ist

    /** @brief Passt die Größe des Schreibpuffers an
     *
     * @param size Neue Größe des Schreibpuffers
     * @exception ReadOnlyException Wird geworfen, wenn versucht wird, den Schreibpuffer zu ändern, obwohl der Speicherbereich nur lesbar
     * ist.
     * @exception OverflowException Wird geworfen, wenn die neue Größe des Schreibpuffers die maximal zulässige Größe überschreitet.
     * @exception BufferExceedsLimitException Wird geworfen, wenn die neue Größe des Schreibpuffers die maximal erlaubte Größe
     * überschreitet.
     * @exception OutOfMemoryException Wird geworfen, wenn nicht genügend Speicher für die neue Größe des Schreibpuffers verfügbar ist.
     */
    void resizeBuffer(size_t size);

public:
    /** @brief Konstruktor der Klasse
     *
     * Durch Verwendung dieses Konstruktors wird die Klasse zum Lesen und Schreiben geöffnet, wobei
     * der Speicherbereich initial 0 Byte gross ist. Beim ersten Schreibzugriff wird der notwendige
     * Speicher allokiert.
     */
    MemFile() noexcept;

    /** @brief Konstruktor der Klasse mit Angabe eines Speicherbereichs
     *
     * Mit diesem Konstruktor wird gleichzeitig ein Pointer auf den Speicherbereich \p adresse mit einer
     * Größe von \p size Bytes übergeben. Sämtliche Dateizugriffe werden in diesem Speicherbereich
     * simuliert.
     * @param adresse Pointer auf den zu verwendenden Speicherbereich
     * @param size Größe des Speicherbereichs
     * @param writeable Gibt an, ob der Speicherbereich auch beschreibbar sein soll.
     * @attention Wird der Parameter @p writeable auf "true" gesetzt, geht die Verwaltung des
     * Speichers an die MemFile-Klasse über. Der Speicher darf nicht mehr von der Applikation verändert
     * oder freigegeben werden!
     */
    MemFile(void* adresse, size_t size, bool writeable = false);

    /** @brief Konstruktor der Klasse mit Angabe eines Speicherbereichs
     *
     * Mit diesem Konstruktor wird gleichzeitig ein Pointer auf den Speicherbereich @p adresse mit einer
     * Größe von @p size Bytes übergeben. Sämtliche Dateizugriffe werden in diesem Speicherbereich
     * simuliert. Ein Schreibzugriff auf diesen Speicherbereich ist nicht möglich.
     *
     * @param adresse Pointer auf den zu verwendenden Speicherbereich
     * @param size Größe des Speicherbereichs
     */
    MemFile(const ByteArrayPtr& memory);

    /** @brief Destruktor der Klasse
     *
     * Gibt den von der MemFile-Klasse verwalteten Speicherbereich frei, falls vorhanden.
     */
    ~MemFile();

    /** @brief Kopierkonstruktor ist gelöscht, um Kopien zu verhindern */
    MemFile(const MemFile&) = delete;

    /** @brief Zuweisungsoperator ist gelöscht, um Kopien zu verhindern */
    MemFile& operator=(const MemFile&) = delete;

    /** @brief Move-Konstruktor des MemFile-Objekts
     *
     * @param other Das MemFile-Objekt, das verschoben werden soll.
     */
    MemFile(MemFile&& other) noexcept;

    /** @brief Move-Zuweisungsoperator
     *
     * Mit diesem Operator kann ein MemFile-Objekt effizient verschoben werden, ohne dass der zugrunde liegende
     * Speicherbereich kopiert werden muss.
     *
     * @param other Das MemFile-Objekt, das verschoben werden soll.
     * @return Referenz auf das aktuelle MemFile-Objekt.
     */
    MemFile& operator=(MemFile&& other) noexcept;

    /** @brief Speicherbereich zum Lesen öffnen
     *
     * Mit dieser Funktion wird die simulierte Datei im Hauptspeicher geöffnet. Dazu muss mit
     * @p adresse ein Pointer auf den Beginn des zu verwendenden Hauptspeichers angegeben werden,
     * sowie mit @p size seine Größe. Sämtliche nachfolgenden Dateizugriffe werden dann in diesem
     * Speicherbereich simuliert. Ein Schreibender Zugriff ist nicht möglich.
     *
     * @param adresse Pointer auf den zu verwendenden Speicherbereich
     * @param size Größe des Speicherbereichs
     * @attention Wird der Parameter @p writeable auf "true" gesetzt, geht die Verwaltung des
     * Speichers an die MemFile-Klasse über. Der Speicher darf nicht mehr von der Applikation verändert
     * oder freigegeben werden!
     */
    void open(void* adresse, size_t size, bool writeable = false);

    /** @brief Speicherbereich zum Lesen öffnen
     *
     * Mit dieser Funktion wird die simulierte Datei im Hauptspeicher zum Lesen geöffnet. Dazu muss
     * mit @p adresse ein Pointer auf den Beginn des zu verwendenden Hauptspeichers angegeben werden,
     * sowie mit @p size seine Größe. Sämtliche nachfolgenden Dateizugriffe werden dann in diesem
     * Speicherbereich simuliert. Ein Schreibender Zugriff ist nicht möglich.
     *
     * @param memory Referenz auf eine ByteArrayPtr-Klasse, die den zu verwendenden Speicherbereich enthält
     * @see openReadWrite: Datei zum Lesen und Schreiben öffnen
     */
    void open(const ByteArrayPtr& memory);

    /** @brief Speicherbereich zum Schreiben und Lesen öffnen
     *
     * Mit dieser Funktion wird die simulierte Datei im Hauptspeicher zum Lesen und Schreiben
     * geöffnet. Dazu muss mit @p adresse ein Pointer auf den Beginn des zu verwendenden
     * Hauptspeichers angegeben werden,
     * sowie mit @p size seine initiale Größe. Sämtliche nachfolgenden Dateizugriffe werden
     * dann in diesem Speicherbereich simuliert. Erfolgt ein schreibender Zugriff über dessen
     * Ende hinaus, wird der Speicherbereich automatisch vergrößert.
     *
     * @param adresse Pointer auf den zu verwendenden Speicherbereich
     * @param size Größe des Speicherbereichs
     * @see open: Datei wird nur zum Lesen geöffnet
     * @see setMaxSize: Legt die maximale Größe der Datei im Speicher fest (Default=unlimitiert)
     * @attention Die Verwaltung des Speichers geht an die MemFile-Klasse über. Der Speicher darf nicht mehr
     * von der Applikation verändert oder freigegeben werden!
     */
    void openReadWrite(void* adresse, size_t size);

    /** @brief Interne Adresse des Speicherbereichs
     *
     * Diese Funktion gibt einen Zeiger auf die interne Speicheradresse zurück. Sie kann verwendet werden,
     * um direkt auf den Speicherbereich zuzugreifen, der von der MemFile-Klasse verwaltet wird.
     *
     * @param adresse Offset innerhalb des Speicherbereichs
     * @return Zeiger auf die interne Speicheradresse ab dem angegebenen Offset
     * @exception FileNotOpenException Wird geworfen, wenn die Datei nicht geöffnet ist.
     * @exception OutOfBoundsException Wird geworfen, wenn der angegebene Offset außerhalb des Speicherbereichs liegt.
     * @attention Der zurückgegebene Zeiger ist nur gültig, solange die Datei geöffnet ist und der interne Speicherbereich nicht verändert
     * wird. Die Methode ist mit Vorsicht zu verwenden, da leicht über den gültigen Speicherbereich hinaus zugegriffen werden kann.
     */
    char* adr(size_t adresse = 0);

    /** @brief Maximale Dateigröße festlegen
     *
     * Mit dieser Funktion wird die maximale Größe einer Datei im Hauptspeicher festgelegt.
     * Damit werden alle Schreibenden Zugriffe begrenzt, die Datei kann nicht größer werden
     * als \p size. Standardmäßig gibt es keine Limitierung, die Datei kann somit so groß werden,
     * wie Hauptspeicher zur Verfügung steht.
     * @param[in] size Maximale Größe in Bytes. Der Wert "0" hebt die Limitierung auf.
     * @see Mit der Funktion MemFile::maxSize kann das derzeitige Limit ausgelesen werden.
     */
    void setMaxSize(size_t size);

    /** @brief Maximale Dateigröße abfragen
     *
     * Mit dieser Funktion kann die derzeitige maximale Größe einer Datei im Hauptspeicher abgefragt werden.
     * @return Maximale Größe in Bytes. Der Wert "0" bedeutet, dass keine Limitierung gesetzt ist.
     */
    size_t maxSize() const;

    /** @brief Datei schließen
     *
     * Diese Funktion schließt die aktuell geöffnete Datei. Sie wird automatisch vom Destruktor der
     * Klasse aufgerufen, so dass ihr expliziter Aufruf nicht erforderlich ist.
     *
     * Wenn  der  Stream  zur  Ausgabe  eingerichtet  war,  werden  gepufferte  Daten  zuerst  durch FileObject::Flush
     * geschrieben. Der zugeordnete Datei-Deskriptor wird geschlossen.
     */
    void close() override;

    /** @brief Dateizeiger an den Anfang der Datei bringen
     *
     * Diese Funktion bewegt den internen Dateizeiger an den Anfang der Datei
     */
    void rewind() override;

    /** @brief Dateizeiger auf gewünschte Stelle bringen
     *
     * Diese Funktion bewegt den internen Dateizeiger auf die gewünschte Stelle
     * @param[in] position Gewünschte Position innerhalb der Datei
     * @exception diverse
     */
    void seek(uint64_t position) override;

    /** @brief Dateizeiger auf gewünschte Stelle bringen
     *
     * Die Funktion %seek setzt den Dateipositionszeiger für den Stream. Die neue Position,
     * gemessen in Byte, wird erreicht durch addieren von  \p offset  zu  der  Position,  die  durch  \p origin
     * angegeben  ist. Wenn \p origin auf SEEK_SET, SEEK_CUR, oder SEEK_END, gesetzt ist, ist der Offset relativ
     * zum Dateianfang, der aktuellen Position, oder dem Dateiende.
     * Ein  erfolgreicher  Aufruf  der  Funktion fseek  löscht  den Dateiendezeiger für den Stream.
     * @param offset Anzahl Bytes, die gesprungen werden soll.
     * @param origin Gibt die Richtung an, in die der Dateizeiger bewegt werden soll. Es kann einen
     * der folgenden Werte annehmen:
     * - SEEKSET @p offset wird vom Beginn der Datei berechnet
     * - SEEKCUR @p offset wird von der aktuellen Dateiposition gerechnet
     * - SEEKEND @p offset wird vom Ende der Datei aus nach vorne berechnet
     * @return Liefert die neue Position zurück, wenn der Dateizeiger erfolgreich auf
     * die gewünschte Position bewegt werden konnte.
     * Im Fehlerfall wird eine Exception geworfen. Die Position des Schreib-/Lesezeigers
     * ist in diesem Fall undefiniert und sollte mittels FileObject::ftell verifiziert
     * werden.
     */
    uint64_t seek(int64_t offset, SeekOrigin origin) override;

    /** @brief Aktuelle Dateiposition ermitteln
     *
     * Die Funktion %tell liefert den aktuellen Wert des Dateipositionszeigers für  den  Stream zurück.
     * @return Position des Zeigers innerhalb der Datei. Im Fehlerfall wird eine
     * Exception geworfen
     */
    uint64_t tell() override;

    /** @brief Lesen eines Datenstroms
     *
     * Die  Funktion  %fread  liest \p nmemb Datenelemente vom Dateistrom und speichert
     * es an  der  Speicherposition,  die  durch \p ptr bestimmt ist.  Jedes davon ist
     * \ size Byte lang.
     * @param[out] ptr Pointer auf den Speicherbereich, in den die gelesenen Daten
     * abgelegt werden sollen. Der Aufrufer muss vorher mindestens @p size * @p nmemb
     * Bytes Speicher reserviert haben.
     * @param[in] size Größe der zu lesenden Datenelemente
     * @param[in] nmemb Anzahl zu lesender Datenelemente
     * @return %fread  gibt die Anzahl der erfolgreich gelesenen Elemente zurück
     * (nicht die Anzahl  der  Zeichen).  Wenn  ein Fehler  auftritt  oder  das
     * Dateiende erreicht ist, wird eine Exception geworfen.
     * @exception EndOfFileException: Wird geworfen, wenn das Dateiende erreicht wurde
     */
    size_t fread(void* ptr, size_t size, size_t nmemb) override;

    /** @brief Schreiben eines Datenstroms
     *
     * Die Funktion %fwrite schreibt \p nmemb Datenelemente der Größe \p size Bytes,
     * in  den  Dateistrom. Sie werden von der Speicherstelle, die durch \p ptr angegeben ist, gelesen.
     * @param ptr Pointer auf den Beginn des zu schreibenden Speicherbereiches.
     * @param size Größe der zu schreibenden Datenelemente
     * @param nmemb Anzahl zu schreibender Datenelemente
     * @return %fwrite gibt die Anzahl der erfolgreich geschriebenen Elemente zurück (nicht die
     * Anzahl der Zeichen). Wenn ein Fehler auftritt, wird eine Exception geworfen.
     */
    size_t fwrite(const void* ptr, size_t size, size_t nmemb) override;

    /** @brief String lesen
     *
     * fgets liest höchstens \p num minus ein Zeichen aus der Datei und speichert
     * sie in dem Puffer, auf den \p buffer zeigt. Das Lesen stoppt nach einem
     * EOF oder Zeilenvorschub. Wenn ein Zeilenvorschub gelesen wird, wird
     * er in dem Puffer gespeichert. Am Ende der gelesenen Daten wird ein
     * 0-Byte angehangen.
     * @param buffer Pointer auf den Speicherbereich, in den die gelesenen Daten
     * geschrieben werden sollen. Dieser muss vorher vom Aufrufer allokiert worden
     * sein und mindestens @p num Bytes groß sein.
     * @param num Anzahl zu lesender Zeichen
     * @return Bei Erfolg wird @p buffer zurückgegeben, bei Dateiende wird NULL
     * zurückgegeben. Im Fehlerfall wird eine Exception geworfen.
     */
    char* fgets(char* buffer, size_t num) override;

    /** @brief Wide-Character String lesen
     *
     * %fgetws liest höchstens \p num minus ein Zeichen (nicht Bytes)
     * eines Wide-Character-Strings aus der Datei
     * und speichert sie in dem Puffer, auf den \p buffer zeigt. Das Lesen stoppt
     * nach einem EOF oder Zeilenvorschub. Wenn ein Zeilenvorschub gelesen wird,
     * wird er in dem Puffer gespeichert. Am Ende der gelesenen Daten wird ein
     * 0-Byte angehangen.
     * @param buffer Pointer auf den Speicherbereich, in den die gelesenen Daten
     * geschrieben werden sollen. Dieser muss vorher vom Aufrufer allokiert worden
     * sein und mindestens @p num * @c sizeof(wchar_t) Bytes groß sein.
     * @param num Anzahl zu lesender Zeichen
     * @return Bei Erfolg wird @p buffer zurückgegeben, bei Dateiende wird NULL
     * zurückgegeben. Im Fehlerfall wird eine Exception geworfen.
     *
     * @note Die Funktion ist unter Umständen nicht auf jedem Betriebssystem
     * verfügbar. In diesem Fall wird eine @exception UnimplementedVirtualFunctionException
     * geworfen.
     *
     * @attention Unter Unix konvertiert fgetws aus dem lokalen Format (z.B. UTF-8) nach
     * wchar_t. Unter Windows wird aber UTF-16 erwartet. Das lesen einer Datei, die nicht
     * UTF-16 kodiert ist, wird zu unerwartetem Verhalten führen!
     */
    wchar_t* fgetws(wchar_t* buffer, size_t num = 1024) override;

    /** @brief String schreiben
     *
     * %fputs schreibt die Zeichenkette \p str ohne sein nachfolgendes 0-Byte in
     * den Ausgabestrom.
     * @param str Pointer auf den zu schreibenden String
     */
    void fputc(int c) override;

    /** @brief Zeichen lesen
     *
     * %fgetc liest das  nächste Zeichen aus der Datei und gibt seinen unsigned char Wert gecastet
     * in einem int zurück.
     * @return Bei Erfolg wird der Wert des gelesenen Zeichens zurückgegeben, im
     * Fehlerfall wird eine Exception geworfen.
     */
    int fgetc() override;

    /** @brief Wide-Character Zeichen schreiben
     *
     * %fputwc schreibt das Wide-Character Zeichen \p c in den Ausgabestrom.
     * @param c Zu schreibendes Zeichen
     */
    void fputwc(wchar_t c) override;

    /** @brief Wide-Character Zeichen lesen
     *
     * %fgetwc liest das nächste Wide-Character Zeichen aus der Datei und gibt seinen Wert als Integer
     * zurück.
     * @return Bei Erfolg wird das gelesene Zeichen als Integer Wert zurückgegeben,
     * im Fehlerfall wird eine Exception geworfen.
     */
    wchar_t fgetwc() override;

    /** @brief String schreiben
     *
     * %fputs schreibt die Zeichenkette \p str ohne sein nachfolgendes 0-Byte in
     * den Ausgabestrom.
     * @param str Pointer auf den zu schreibenden String
     */
    void fputs(const char* str) override;

    /** @brief Wide-Character String schreiben
     *
     * %fputws schreibt die Zeichenkette \p str ohne sein nachfolgendes 0-Byte in
     * den Ausgabestrom.
     * @param str Pointer auf den zu schreibenden String
     *
     */
    void fputws(const wchar_t* str) override;

    /** @brief Prüfen, ob Dateiende erreicht ist
     *
     * Die Funktion prüft, ob das Dateiende erreicht wurde
     * @return Liefert @c true zurück, wenn das Dateiende erreicht wurde, sonst @c false
     * Falls die Datei nicht geöffnet war, wird wird eine Exception geworfen.
     */
    bool eof() const override;

    /** @brief Größe der geöffneten Datei
     *
     * Diese Funktion liefert die Größe der geöffneten Datei in Bytes zurück.
     * @return Größe der Datei in Bytes. Falls Fehler auftreten, wird eine Exception geworfen.
     */
    uint64_t size() const override;

    /** @brief Datei in den Speicher mappen
     *
     * Mit dieser Funktion wird ein Teil der Datei in den Speicher gemapped. Dies macht das Lesen und
     * Schreiben von Dateien effizienter, da nur die tatsächlich benötigten Teile in den Speicher geladen
     * werden.
     *
     * Je nach Protection-Modus @p prot kann der gemappte Speicher nur gelesen oder auch beschrieben werden.
     *
     * @param[in] position Die gewünschte Startposition innerhalb der Datei
     * @param[in] size Die Anzahl Bytes, die gemapped werden sollen.
     * @param[in] prot Zugriffsmodus für das Mapping. Standardmäßig wird nur Lesezugriff gewährt.
     * @return Bei Erfolg gibt die Funktion einen Pointer auf den Speicherbereich zurück,
     * in dem sich die Datei befindet, im Fehlerfall wird eine Exception geworfen.
     */
    char* map(uint64_t position, size_t size, MapProtection prot = MapProtection::READ) override;

    using FileObject::map;

    /** @brief Mapping aufheben
     *
     * Ein mit map oder mapRW eingerichtetes Mapping einer Datei in den Hauptspeicher
     * wird wieder aufgehoben.
     */
    void unmap() override;

    /** @brief Minimalgröße des Speicherblocks bei Zugriffen mit FileObject::Map
     *
     * Hat in MemMap keine Funktionalität und wird ignoriert.
     * @param bytes Anzahl Bytes, die im Voraus gemapped werden sollen.
     */
    void setMapReadAhead(size_t bytes) override;

    /** @brief Filenummer der Datei
     *
     * Da MemFile keine echte Datei im Dateisystem repräsentiert, sondern nur einen
     * Speicherbereich, wird hier eine Exception geworfen!
     * @exception OperationUnavailableException Immer, da MemFile keine echte Datei repräsentiert.
     */
    int getFileNo() const override;

    /** @brief Gepufferte Daten schreiben
     *
     * Bei MemFile hat diese Funktion keine Auswirkung, da alle Änderungen direkt im Speicher erfolgen.
     */
    void flush() override;

    /** @brief Gepufferte Daten synchronisieren
     *
     * Bei MemFile hat diese Funktion keine Auswirkung, da alle Änderungen direkt im Speicher erfolgen.
     */
    void sync() override;

    /** @brief Datei abschneiden
     *
     * Die Funktionen Truncate bewirkt, dass die aktuell geöffnete Datei auf eine Größe von
     * exakt \p length Bytes abgeschnitten wird.
     *
     * Wenn die Datei vorher größer war, gehen überschüssige Daten verloren. Wenn die Datei
     * vorher kleiner war, wird sie vergrößert und die zusätzlichen Bytes werden als Nullen geschrieben.
     *
     * Der Dateizeiger wird nicht verändert. Die Datei muss zum Schreiben geöffnet sein.
     *
     * @param length Position, an der die Datei abgeschnitten werden soll.
     */
    void truncate(uint64_t length) override;

    /** @brief Prüfen, ob die Datei geöffnet ist
     *
     * @return Liefert true, wenn die Datei geöffnet ist, sonst false.
     */
    inline bool isOpen() const override
    {
        return is_open;
    }

    /** @brief Datei zum Lesen sperren
     *
     * Diese Funktion steht bei bei dieser Speicherklasse nicht zur Verfügung. Bei
     * Aufruf der Funktion wird eine OperationUnavailableException geworfen.
     * @exception OperationUnavailableException Immer, da MemFile keine echte Datei repräsentiert.
     */
    void lockShared(bool block = true) override;

    /** @brief Datei zum Schreiben sperren
     *
     * Diese Funktion steht bei bei dieser Speicherklasse nicht zur Verfügung. Bei
     * Aufruf der Funktion wird eine OperationUnavailableException geworfen.
     * @exception OperationUnavailableException Immer, da MemFile keine echte Datei repräsentiert.
     */
    void lockExclusive(bool block = true) override;

    /** @brief Dateisperre aufheben
     *
     * Diese Funktion steht bei bei dieser Speicherklasse nicht zur Verfügung. Bei
     * Aufruf der Funktion wird eine OperationUnavailableException geworfen.
     * @exception OperationUnavailableException Immer, da MemFile keine echte Datei repräsentiert.
     */
    void unlock() override;
};

} // namespace pplib

#endif /* PPLIB_CORE_MEMFILE_H_ */
