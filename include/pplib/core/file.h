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

#ifndef PPLIB_CORE_FILE_H_
#define PPLIB_CORE_FILE_H_

#include <pplib/types/string.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/bytearrayptr.h>
#include <pplib/core/fileobject.h>

namespace pplib
{
class DirEntry;

/** @class FileAttr
 * @ingroup PPLGroupFileIO
 * @brief Definitionen der Datei-Attribute
 *
 * Die Klasse FileAttr enthält die Definitionen der Datei-Attribute, die mit
 * File::chmod gesetzt oder mit File::stat ausgelesen werden können.
 *
 */
class FileAttr
{
public:
    /** @enum FileAttr::Attributes
     * \brief Definitionen der Datei-Attribute
     *
     * Diese Enumeration enthält die Definitionen der Datei-Attribute, die mit
     * File::chmod gesetzt oder mit File::stat ausgelesen werden können.
     */
    enum Attributes
    {
        IFFILE = 0x10000,     ///< reguläre Datei
        IFSOCK = 0x20000,     ///< Socket
        IFDIR = 0x40000,      ///< Verzeichnis
        IFLINK = 0x80000,     ///< Symlink
        ISUID = 0x4000,       ///< SUID-Bit
        ISGID = 0x2000,       ///< SGID-Bit
        ISVTX = 0x1000,       ///< Sticky-Bit
        STICKY = 0x1000,      ///< Sticky-Bit
        USR_READ = 0x0400,    ///< Leserechte für Eigentümer
        USR_WRITE = 0x0200,   ///< Schreibrechte für Eigentümer
        USR_EXECUTE = 0x0100, ///< Ausführrechte für Eigentümer
        GRP_READ = 0x0040,    ///< Leserechte für Gruppe
        GRP_WRITE = 0x0020,   ///< Schreibrechte für Gruppe
        GRP_EXECUTE = 0x0010, ///< Ausführrechte für Gruppe
        OTH_READ = 0x0004,    ///< Leserechte für andere
        OTH_WRITE = 0x0002,   ///< Schreibrechte für andere
        OTH_EXECUTE = 0x0001, ///< Ausführrechte für andere
        CHMOD_755 = 0x0755,   ///< Berechtigungen rwxr-xr-x
        CHMOD_644 = 0x0644,   ///< Berechtigungen rw-r--r--
        NONE = 0
    };
};

/** @class File
 * @ingroup PPLGroupFileIO
 * @brief Dateizugriff
 *
 * Mit dieser Klasse können Dateien geladen, verändert und gespeichert werden.
 * Sie dient als Wrapper-Klasse für die Low-Level Funktionen des Betriebssystems.
 *
 */
class File : public FileObject
{
private:
    void* ff;
    char* MapBase;
    uint64_t LastMapStart;
    uint64_t LastMapSize;
    MapProtection LastMapProtection;
    uint64_t ReadAhead;
    uint64_t mysize;
    uint64_t pos;
    bool isPopen;

    void munmap(void* addr, size_t len);
    void* mmap(uint64_t position, size_t size, MapProtection prot);

public:
    /** @enum FileMode
     * @brief Zugriffsmodus beim Öffnen einer Datei
     *
     * Zugriffsmodus beim Öffnen einer Datei mit File::open
     *
     */
    enum class FileMode
    {
        /** @brief Datei zum Lesen öffnen. Zeiger wird an den Anfang der Datei positioniert.
         * @attention Die Datei muss existieren!
         */
        READ = 1,
        /** @brief Datei zum Schreiben öffnen.
         * @attention Falls die Datei schon vorhanden ist, wird sie gelöscht.
         */
        WRITE,
        /** @brief Datei zum Lesen und Schreiben öffnen. Zeiger wird an den Anfang der Datei positioniert.
         * @attention Die Datei muss existieren!
         * @exception FileNotOpenException Wird geworfen, wenn die Datei nicht existiert.
         */
        READWRITE,
        /** @brief Datei zum Anhängen öffnen. Zeiger wird ans Ende der Datei positioniert.
         * @note Wenn die Datei noch nicht existiert, wird sie erstellt.
         */
        APPEND,
        /** @brief Datei zum Lesen und Schreiben öffnen. Falls die Datei noch nicht existiert, wird sie erstellt.
         */
        READWRITE_CREATE,
    };

private:
    /** @brief Exception anhand errno-Variable werfen
     *
     * Diese Funktion wird intern verwendet, um nach Auftreten eines Fehlers, anhand der globalen
     * "errno"-Variablen die passende Exception zu werfen.
     * @param e Errorcode aus der errno-Variablen
     */
    void throwErrno(int e);

public:
    /** @brief Exception anhand errno-Variable werfen
     *
     * Diese Funktion wird intern verwendet, um nach Auftreten eines Fehlers, anhand der globalen
     * "errno"-Variablen die passende Exception zu werfen.
     * @param e Errorcode aus der errno-Variablen
     * @param filename Dateiname, bei der der Fehler aufgetreten ist
     */
    static void throwErrno(int e, const String& filename);

    /** @brief Konstruktor der Klasse
     *
     * Konstruktor der Klasse
     */
    File();

    /** @brief Konstruktor der Klasse mit gleichzeitigem Öffnen einer Datei
     *
     * Konstruktor der Klasse, mit dem gleichzeitig eine Datei geöffnet wird.
     * @param[in] filename Name der zu öffnenden Datei
     * @param[in] mode Zugriffsmodus. Defaultmäßig wird die Datei zum binären Lesen
     * geöffnet (siehe @ref pplib_File_Filemodi)
     */
    File(const String& filename, FileMode mode = FileMode::READ);

    /** @brief Konstruktor mit Übernahme eines C-Filehandles
     *
     * Konstruktor der Klasse mit Übernahme eines C-Filehandles einer bereits mit ::fopen geöffneten Datei.
     * @param[in] handle File-Handle
     */
    File(FILE* handle);
    virtual ~File();

    /** @brief Datei öffnen
     *
     * Mit dieser Funktion wird eine Datei zum Lesen, Schreiben oder beides geöffnet.
     * @param[in] filename Dateiname
     * @param mode Zugriffsmodus
     *
     * @return Kein Rückgabeparameter, im Fehlerfall wirft die Funktion eine Exception
     */
    void open(const String& filename, FileMode mode = FileMode::READ);

    /** @brief Bereits geöffnete Datei übernehmen
     *
     * Mit dieser Funktion kann eine mit der C-Funktion @c fopen bereits geöffnete Datei
     * übernommen werden.
     *
     * @param[in] handle Das Filehandle
     * @return Kein Rückgabeparameter, im Fehlerfall wirft die Funktion eine Exception
     */
    void open(FILE* handle);

    /** @brief Eine temporäre Datei zum Lesen und Schreiben öffnen
     *
     * Diese Funktion erzeugt eine temporäre Datei mit einem eindeutigen Namen.
     * Dieser Name wird aus \p filetemplate erzeugt. Dazu  müssen  die letzten
     * sechs  Buchstaben  des  Parameters template XXXXXX sein, diese werden dann
     * durch eine Zeichenkette ersetzt, die diesen Dateinamen eindeutig  macht.
     * Die  Datei  wird dann mit dem Modus read/write und den Rechten 0666 erzeugt.
     * @param[in] filetemplate Pfad und Vorlage für den zu erstellenden Dateinamen
     *
     * @return Kein Rückgabeparameter, im Fehlerfall wirft die Funktion eine Exception
     */
    void openTemp(const String& filetemplate);

    /** @brief Prozess öffnen
     *
     * Die  Funktion \c Popen öffnet einen Prozess dadurch, dass sie sich nach
     * Erzeugen einer Pipe aufteilt und eine Shell öffnet.  Da eine Pipe  nach
     * Definition  unidirektional  ist,  darf das Argument \p mode nur Lesen oder
     * Schreiben angeben,  nicht  beides;  der  resultierende  Datenkanal  ist
     * demzufolge nur-lesend oder nur-schreibend.
     * @param[in] command Das  Argument @p command  enthält einen String,
     * der ein Shell-Kommandozeile enthält.  Dieses Kommando  wird  an @c /bin/sh
     * unter Verwendung des Flags @c -c übergeben. Interpretation, falls nötig, wird von
     * der Shell durchgeführt.
     * @param[in] mode Dateimodus
     * @return Kein Rückgabeparameter, im Fehlerfall wirft die Funktion eine Exception
     */
    void popen(const String& command, FileMode mode = FileMode::READ);

    /** @brief Geöffnete Datei löschen
     *
     * Mit dieser Funktion wird die aktuelle Datei zunächst geschlossen und dann
     * gelöscht.
     * @return Kein Rückgabewert, im Fehlerfall wird eine Exception geworfen.
     */
    void erase();

    // Virtuelle Funktionen

    /** @brief Datei schließen
     *
     * Diese Funktion schließt die aktuell geöffnete Datei. Sie wird automatisch vom Destruktor der
     * Klasse aufgerufen, so dass ihr expliziter Aufruf nicht erforderlich ist.
     *
     * Wenn  der  Stream  zur  Ausgabe  eingerichtet  war,  werden  gepufferte  Daten  zuerst  durch
     * FileObject::flush
     * geschrieben. Der zugeordnete Datei-Deskriptor wird geschlossen, alle Systemressourcen werden
     * freigegeben.
     *
     * @return Kein Rückgabeparameter, im Fehlerfall wirft die Funktion eine Exception
     */
    virtual void close();

    /** @brief Datei an den Anfang zurücksetzen
     *
     * Diese Funktion setzt den Dateizeiger auf den Anfang der Datei zurück.
     */
    virtual void rewind();

    /** @brief Datei an eine bestimmte Position setzen
     *
     * Diese Funktion setzt den Dateizeiger auf die angegebene Position.
     * @param[in] position Die Position, auf die der Dateizeiger gesetzt werden soll.
     */
    virtual void seek(uint64_t position);

    /** @brief Datei relativ zur aktuellen Position setzen
     *
     * Diese Funktion setzt den Dateizeiger relativ zur aktuellen Position.
     * @param[in] offset Der Offset, um den der Dateizeiger verschoben werden soll.
     * @param[in] origin Die Referenzposition für den Offset.
     * @return Die neue Position des Dateizeigers.
     */
    virtual uint64_t seek(int64_t offset, SeekOrigin origin);

    /** @brief Aktuelle Position des Dateizeigers abfragen
     *
     * Diese Funktion gibt die aktuelle Position des Dateizeigers zurück.
     * @return Die aktuelle Position des Dateizeigers.
     */
    virtual uint64_t tell();

    /** @brief Größe der Datei abfragen
     *
     * Diese Funktion gibt die Größe der Datei zurück.
     * @return Die Größe der Datei in Bytes.
     */
    virtual uint64_t size() const;

    /** @brief Prüfen, ob die Datei geöffnet ist
     *
     * Diese Funktion prüft, ob die Datei aktuell geöffnet ist.
     * @return Liefert @c true zurück, wenn die Datei geöffnet ist, sonst @c false.
     */
    virtual bool isOpen() const;

    /** @brief Lesen eines Datenstroms
     *
     * Die  Funktion  fread  liest \p nmemb Datenelemente vom Dateistrom und speichert
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
    virtual size_t fread(void* ptr, size_t size, size_t nmemb);

    /** @brief Schreiben eines Datenstroms
     *
     * Die Funktion fwrite schreibt \p nmemb Datenelemente aus dem Speicherbereich,
     * der durch \p ptr angegeben ist, in den Dateistrom. Jedes Datenelement ist \p size
     * Byte lang.
     * @param[in] ptr Pointer auf den Speicherbereich, aus dem die zu schreibenden Daten
     * entnommen werden sollen.
     * @param[in] size Größe der zu schreibenden Datenelemente
     * @param[in] nmemb Anzahl zu schreibender Datenelemente
     * @return %fwrite gibt die Anzahl der erfolgreich geschriebenen Elemente zurück
     * (nicht die Anzahl der Zeichen). Wenn ein Fehler auftritt, wird eine Exception geworfen.
     */
    virtual size_t fwrite(const void* ptr, size_t size, size_t nmemb);

    /** @brief String lesen
     *
     * %fgets liest höchstens \p num minus ein Zeichen aus der Datei und speichert
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
    virtual char* fgets(char* buffer, size_t num);

    /** @brief Wide-Character String lesen
     *
     * %fgwets liest höchstens \p num minus ein Zeichen (nicht Bytes)
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
     */
    virtual wchar_t* fgetws(wchar_t* buffer, size_t num = 1024);

    /** @brief String schreiben
     *
     * %fputs schreibt die Zeichenkette \p str ohne sein nachfolgendes 0-Byte in
     * den Ausgabestrom.
     * @param str Pointer auf den zu schreibenden String
     * @return Kein Rückgabewert, im Fehlerfall wird eine Exception geworfen.
     */
    virtual void fputs(const char* str);

    /** @brief Wide-Character String schreiben
     *
     * %fputs schreibt die Zeichenkette \p str ohne sein nachfolgendes 0-Byte in
     * den Ausgabestrom.
     * @param str Pointer auf den zu schreibenden String
     * @return Kein Rückgabewert, im Fehlerfall wird eine Exception geworfen.
     *
     * @note Die Funktion ist unter Umständen nicht auf jedem Betriebssystem
     * verfügbar. In diesem Fall wird Fehlercode 246 zurückgegeben.
     */
    virtual void fputws(const wchar_t* str);

    /** @brief Zeichen schreiben
     *
     * %fputc schreibt das Zeichen \p c, umgesetzt in ein unsigned char,
     * in den Ausgabestrom.
     * @param c Zu schreibendes Zeichen
     * @return Kein Rückgabewert, im Fehlerfall wird eine Exception geworfen.
     */
    virtual void fputc(int c);

    /** @brief Zeichen lesen
     *
     * %fgetc liest das  nächste Zeichen aus der Datei und gibt seinen unsigned char Wert gecastet
     * in einem int zurück.
     * @return Bei Erfolg wird der Wert des gelesenen Zeichens zurückgegeben, im
     * Fehlerfall wird eine Exception geworfen.
     */
    virtual int fgetc();

    /** @brief Wide-Character Zeichen schreiben
     *
     * %fputwc schreibt das Wide-Character Zeichen \p c in den Ausgabestrom.
     * @param c Zu schreibendes Zeichen
     * @return Kein Rückgabewert, im Fehlerfall wird eine Exception geworfen.
     *
     * @note Die Funktion ist unter Umständen nicht auf jedem Betriebssystem
     * verfügbar.
     */
    virtual void fputwc(wchar_t c);

    /** @brief Wide-Character Zeichen lesen
     *
     * %fgetwc liest das nächste Wide-Character Zeichen aus der Datei und gibt seinen Wert als Integer
     * zurück.
     * @return Bei Erfolg wird das gelesene Zeichen als Integer Wert zurückgegeben,
     * im Fehlerfall wird eine Exception geworfen.
     *
     * @note Die Funktion ist unter Umständen nicht auf jedem Betriebssystem
     * verfügbar.
     */
    virtual wchar_t fgetwc();

    /** @brief Prüfen, ob Dateiende erreicht ist
     *
     * Die Funktion prüft, ob das Dateiende erreicht wurde
     * @return Liefert @c true zurück, wenn das Dateiende erreicht wurde, sonst @c false
     * Falls die Datei nicht geöffnet war, wird ebenfalls @c true zurückgegeben.
     */
    virtual bool eof() const;

    /** @brief Filenummer der Datei
     *
     * Die Funktion liefert den Dateideskriptor als Integer zurück, wie er
     * von den Systemfunktionen open , read , write und close genutzt wird.
     * @return Liefert die Filenummer zurück oder wirft eine Exception,
     * wenn die Datei nicht geöffnet war.
     */
    virtual int getFileNo() const;

    /** @brief Gepufferte Daten schreiben
     *
     * Die Funktion Flush bewirkt, dass alle gepufferten Daten des aktuellen Streams
     * mittels der zugrundeliegenden write-Funktion geschrieben werden. Der Status
     * des Streams wird dabei nicht berührt. Die Daten werden nicht zwangsweise auch
     * physikalisch auf die Platte geschrieben, sie können noch immer aus Performancegründen
     * vom Kernel oder Treiber gecached werden. Um 100 Prozent sicher zu gehen, kann man
     * die Funktion FileObject::sync verwenden.
     * @return Kein Rückgabewert, im Fehlerfall wird eine Exception geworfen.
     */
    virtual void flush();

    /** @brief Dateiänderungen sofort auf die Platte schreiben
     *
     * Für gewöhnlich cached das Betriebssysteme Schreibzugriffe auf die Festplatte, um die Performance
     * zu steigern. Je nach Filesystem und Betriebssystem können zwischen Schreibzugriff der Software bis zum
     * tatsächlichen Schreiben auf die Festplatte zwischen einigen wenigen Sekunden bis zu einer Minute vergehen!
     * Tritt in diesem Zeitraum ein System-Crash oder Stromausfall auf, führt dies unweigerlich zu Datenverlust.
     * Ein Aufruf dieser Funktion bewirkt, dass alle Dateiänderungen sofort auf die Festplatte
     * geschrieben werden. Sie sollte daher vor dem Schließen einer kritischen Datei mit CFile::Close aufgerufen
     * werden, unter Umständen sogar nach jedem Schreibzugriff.
     *
     * Die Funktion kehrt erst zurück, wenn alle Daten vollständig geschrieben wurden und liefert dann true (1)
     * zurück. Können die Daten nicht geschrieben werden, wird eine Exception geworfen.
     */
    virtual void sync();

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
    virtual void truncate(uint64_t length);

    /** @brief Datei zum Lesen sperren
     *
     * Mit LockShared wird die Datei zum Lesen gesperrt. Andere Prozesse können weiterhin
     * auf die Datei zugreifen, allerdings ebenfalls nur lesend.
     * @param block Gibt an, ob die Funktion warten soll (blocken), bis die Datei
     * gesperrt werden kann (block=true) oder sofort mit einer Fehlermeldung
     * zurückkehren soll (block=false).
     *
     * @see Siehe auch File::LockExclusive und File::Unlock
     */
    virtual void lockShared(bool block = true);

    /** @brief Datei exklusiv sperren
     *
     * Mit LockExclusive wird die Datei exklusiv zum Schreiben gesperrt. Andere
     * Prozesse können nicht auf die Datei zugreifen, solange die Sperre besteht.
     * @param block Gibt an, ob die Funktion warten soll (blocken), bis die Datei
     * gesperrt werden kann (block=true) oder sofort mit einer Fehlermeldung
     * zurückkehren soll (block=false).
     *
     * @see Siehe auch File::LockShared und File::Unlock
     */
    virtual void lockExclusive(bool block = true);

    /** @brief Dateisperre aufheben
     *
     * Mit Unlock wird eine mit lockShared oder lockExclusive eingerichtete
     * Sperre wieder aufgehoben, so dass auch andere Prozesse wieder uneingeschränkt
     * auf die Datei zugreifen können.
     *
     * @see Siehe auch File::lockShared und File::lockExclusive
     */
    virtual void unlock();

    /** @brief Minimalgröße des Speicherblocks bei Zugriffen mit FileObject::Map
     *
     * Falls mit Map viele aufeinanderfolgende kleine Speicherblöcke gemapped werden,
     * ist es sinnvoll größere Blöcke zu laden, die dann bereits im Cache bzw. Hauptspeicher
     * liegen, wenn sie gebraucht werden. Mit dieser Funktion kann bestimmt werden, wie
     * viele Daten im Voraus gemapped werden sollen.
     * @param bytes Anzahl Bytes, die im Voraus gemapped werden sollen.
     */
    virtual void setMapReadAhead(size_t bytes);

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
    virtual char* map(uint64_t position, size_t size, MapProtection prot = MapProtection::READ);

    /** @brief Mapping aufheben
     *
     * Ein mit map oder mapRW eingerichtetes Mapping einer Datei in den Hauptspeicher
     * wird wieder aufgehoben.
     */
    virtual void unmap();

    // Static Functions

    /** @ingroup PPLGroupFileIO
     * @brief Datei in ein ByteArray laden
     *
     * Mit dieser Funktion wird die Datei mit dem Namen \p filename geöffnet und der
     * kompletten Inhalt in das ByteArray \p object geladen.
     * @param[out] object Das gewünschte Zielobjekt
     * @param[in] filename Der Dateiname
     * @return Kein Returnwert. Im Fehlerfall wird eine Exception geworfen.
     */
    static void load(ByteArray& object, const String& filename);

    /** @ingroup PPLGroupFileIO
     * @brief Datei in einen String laden
     *
     * Mit dieser Funktion wird die Datei mit dem Namen \p filename geöffnet und der
     * kompletten Inhalt in den String \p object geladen.
     * @param[out] object Der String, in den der Dateiinhalt geladen werden soll.
     * @param[in] filename Der Dateiname
     * @return Kein Returnwert. Im Fehlerfall wird eine Exception geworfen.
     */
    static void load(String& object, const String& filename);
    // static void* load(const String& filename, size_t* size = NULL);

    /** @ingroup PPLGroupFileIO
     * @brief Datei in ein ByteArray laden
     *
     * Mit dieser Funktion wird die Datei mit dem Namen \p filename geöffnet und der
     * kompletten Inhalt in ein ByteArray geladen.
     * @param[in] filename Der Dateiname
     * @return Bei Erfolg liefert die Funktion das ByteArray mit dem Dateiinhalt zurück.
     * Im Fehlerfall wird eine Exception geworfen.
     */
    static ByteArray load(const String& filename);

    /** @ingroup PPLGroupFileIO
     * @brief Datei abschneiden
     *
     * Die Funktionen %truncate bewirkt, dass die mit \p filename angegebene Datei
     * an der Position \p bytes abgeschnitten wird.
     *
     * Wenn die Datei vorher größer war, gehen überschüssige Daten verloren. Wenn die Datei
     * vorher kleiner war, wird sie vergrößert und die zusätzlichen Bytes werden als Nullen geschrieben.
     *
     * @param filename Der Name der gewünschten Datei
     * @param bytes Position, an der die Datei abgeschnitten werden soll.
     * @return Kein Returnwert. Im Fehlerfall wird eine Exception geworfen.
     */
    static void truncate(const String& filename, uint64_t bytes);

    /** @ingroup PPLGroupFileIO
     * @brief Prüfen, ob eine Datei existiert
     *
     * Mit %exists kann geprüft werden, ob eine Datei im Filesystem vorhanden ist.
     * @param filename Name der gewünschten Datei
     * @return Ist die Datei forhanden, gibt die Funktion @c true zurück, andernfalls @c false.
     */
    static bool exists(const String& filename);

    /** @ingroup PPLGroupFileIO
     * @brief Datei kopieren
     *
     * Mit CopyFile wird die Datei @p oldfile nach @p newfile kopiert. Dazu wird die Quelldatei Stück
     * für Stück zunächst in den Hauptspeicher geladen und dann in die Zieldatei geschrieben. War die Zieldatei
     * @p newfile schon vorhanden, wird sie überschrieben. Anders als bei File::MoveFile oder File::RenameFile
     * müssen die beiden Dateien nicht im gleichen Filesystem liegen.
     *
     * @param oldfile Name der zu kopierenden Datei
     * @param newfile Name der Zieldatei.
     */
    static void copy(const String& oldfile, const String& newfile);

    /** @ingroup PPLGroupFileIO
     * @brief Datei verschieben oder umbenennen
     *
     * Mit dieser Funktion wird die Datei \p oldfile zu \p newfile umbenannt. Sie ist identisch mit
     * File::rename. Beide Funktionen arbeiten am effizientesten, wenn die Zieldatei im gleichen
     * Filesystem liegt, da in diesem Fall nur die Verzeichniseinträge geändert werden müssen.
     * Ist dies nicht der Fall, wird automatisch die wesentlich langsamere Funktion File::copy
     * gefolgt von File::remove aufgerufen.
     * @param oldfile Name der zu verschiebenden bzw. umzubenennenden Datei
     * @param newfile Neuer Name
     * @return Kein Returnwert. Im Fehlerfall wird eine Exception geworfen.
     */
    static void move(const String& oldfile, const String& newfile);

    /** @ingroup PPLGroupFileIO
     * @brief Datei verschieben oder umbenennen
     *
     * Mit dieser Funktion wird die Datei \p oldfile zu \p newfile umbenannt. Sie ist identisch mit
     * File::move. Beide Funktionen arbeiten am effizientesten, wenn die Zieldatei im gleichen
     * Filesystem liegt, da in diesem Fall nur die Verzeichniseinträge geändert werden müssen.
     * Ist dies nicht der Fall, wird automatisch die wesentlich langsamere Funktion File::CopyFile
     * gefolgt von File::DeleteFile aufgerufen.
     * @param oldfile Name der zu verschiebenden bzw. umzubenennenden Datei
     * @param newfile Neuer Name
     * @return Kein Returnwert. Im Fehlerfall wird eine Exception geworfen.
     */
    static void rename(const String& oldfile, const String& newfile);

    /** @ingroup PPLGroupFileIO
     * @brief Datei löschen
     *
     * Mit dieser Funktion wird die Datei \p filename vom Datenträger gelöscht.
     * @param filename Name der gewünschten Datei
     * @return Kein Returnwert. Im Fehlerfall wird eine Exception geworfen.
     * Ein Fehler kann auftreten, wenn die nötigen Zugriffsrechte fehlen. Ist die Datei
     * nicht vorhanden, wird jedoch kein Fehler geworfen.
     *
     * @note Die Funktionen File::erase, File::unlink und File::remove sind identisch
     *
     * @attention Wird die Funktion auf ein nicht leers Verzeichnis angewendet, wird eine Exception geworfen.
     * Bitte verwende stattdessen Dir::rmDir mit recursive = true.
     */
    static void unlink(const String& filename);

    /** @ingroup PPLGroupFileIO
     * @brief Datei löschen
     *
     * Mit dieser Funktion wird die Datei \p filename vom Datenträger gelöscht.
     * @param filename Name der gewünschten Datei
     * @return Kein Returnwert. Im Fehlerfall wird eine Exception geworfen.
     * Ein Fehler kann auftreten, wenn die nötigen Zugriffsrechte fehlen. Ist die Datei
     * nicht vorhanden, wird jedoch kein Fehler geworfen.
     *
     * @note Die Funktionen File::erase, File::unlink und File::remove sind identisch
     *
     * @attention Wird die Funktion auf ein nicht leers Verzeichnis angewendet, wird eine Exception geworfen.
     * Bitte verwende stattdessen Dir::rmDir mit recursive = true.
     */
    static void remove(const String& filename);

    /** @ingroup PPLGroupFileIO
     * @brief Datei löschen
     *
     * Mit dieser Funktion wird die Datei \p filename vom Datenträger gelöscht.
     * @param filename Name der gewünschten Datei
     * @return Kein Returnwert. Im Fehlerfall wird eine Exception geworfen.
     * Ein Fehler kann auftreten, wenn die nötigen Zugriffsrechte fehlen. Ist die Datei
     * nicht vorhanden, wird jedoch kein Fehler geworfen.
     *
     * @note Die Funktionen File::erase, File::unlink und File::remove sind identisch
     * @attention Wird die Funktion auf ein nicht leers Verzeichnis angewendet, wird eine Exception geworfen.
     * Bitte verwende stattdessen Dir::rmDir mit recursive = true.
     */
    static void erase(const String& filename);

    /** @ingroup PPLGroupFileIO
     * @brief Leere Datei anlegen oder die Zeitstempel des letzten Zugriffs aktualisieren
     *
     * TouchFile arbeitet ähnlich wie das Unix-Lommando \c touch. Ist die angegebene Datei
     * @p filename noch nicht vorhanden, wird sie als leere Datei angelegt. Ist sie bereits vorhanden,
     * wird der Zeitstempel des letzen Zugriffs aktualisiert.
     *
     * @param filename Name der gewünschten Datei
     * @return Kein Returnwert. Im Fehlerfall wird eine Exception geworfen.
     */
    static void touch(const String& filename);

    /** @ingroup PPLGroupFileIO
     * @brief Daten in Datei schreiben
     *
     * Mit dieser Funktion werden \p size Bytes aus dem Speicherbereich beginnend bei
     * @p content in die Datei @p filename geschrieben. War die Datei bereits vorhanden,
     * wird sie überschrieben.
     *
     * @param content Pointer auf den Speicherbereich, der in die Datei geschrieben werdem soll
     * @param size Anzahl zu schreibender Bytes
     * @param filename Name der gewünschten Datei
     * @return Kein Returnwert. Im Fehlerfall wird eine Exception geworfen.
     */
    static void save(const void* content, size_t size, const String& filename);

    /** @ingroup PPLGroupFileIO
     * @brief Daten eines von ByteArrayPtr Objekts in Datei schreiben
     *
     * Mit dieser Funktion wird der Speicher auf den der ByteArrayPtr \p object
     * zeigt in die Datei \p filename geschrieben. War die Datei bereits vorhanden, wird sie überschrieben.
     * @param object Ein ByteArrayPtrm der auf den zu speichernden Speicherbereich zeigt.
     * @param filename Name der gewünschten Datei
     * @return Kein Returnwert. Im Fehlerfall wird eine Exception geworfen.
     */
    static void save(const ByteArrayPtr& object, const String& filename);

    /** @brief Setz die Attribute einer exisitierenden Datei
     * @ingroup PPLGroupFileIO
     *
     * Mit dieser Funktion können die Zugriffsattribute einer existierenden Datei
     * gesetzt werden.
     * @param filename Der Dateinamen
     * @param attr Die Attribute
     * @return Kein Returnwert. Im Fehlerfall wird eine Exception geworfen.
     *
     * @see FileAttr::Attributes
     */
    static void chmod(const String& filename, FileAttr::Attributes attr);

    /** @brief Informationen zu einer Datei oder Verzeichnis
     *
     * Mit dieser statischen Funktion können Informationen zur Datei oder
     * Verzeichnis \p filename ausgelesen werden. Das Ergebnis wird in
     * @p result gespeichert.
     *
     * @param filename Dateiname
     * @param result Objekt, in dem die Daten gespeichert werden
     * @return Kein Returnwert. Im Fehlerfall wird eine Exception geworfen.
     * @throw NullPointerException: Wird geworfen, wenn @p filename auf NULL zeigt
     * @throw FileNotFoundException: Datei oder Verzeichnis nicht vorhanden
     */
    static void statFile(const String& filename, DirEntry& result);

    /** @brief Informationen zu einer Datei oder Verzeichnis
     *
     * Diese Funktion liefert true zurück, wenn die gesuchte Datei existiert und deren
     * Eigenschaften ermittelt werden konnten. In \p result sind dann die Eigenschaften zu
     * finden.
     * @param[in] filename Dateiname
     * @param[out] result Objekt mit dem Ergebnis
     *
     * @return true oder false
     * @throw UnsupportedFeatureException wird geworfen, wenn die stat-Methode vom System
     * nicht unterstützt wird
     */
    static bool tryStatFile(const String& filename, DirEntry& result);

    /** @brief Informationen zu einer Datei oder Verzeichnis
     *
     * Diese Funktion liefert die Eigenschaften der gesuchten Datei zurück.
     * @param filename Dateiname
     * @return Objekt mit den Eigenschaften der Datei
     * @throw NullPointerException: Wird geworfen, wenn @p filename auf NULL zeigt
     * @throw FileNotFoundException: Datei oder Verzeichnis nicht vorhanden
     */
    static DirEntry statFile(const String& filename);

    /** @brief Pfad ohne Dateinamen
     *
     * Diese Funktion liefert den Verzeichnisnamen eines Strings zurück, der Pfad und Dateinamen
     * enthält. Lautet der String beispielsweise "/home/patrick/svn/pplib/README.TXT" liefert die Funktion
     * "/home/patrick/svn/pplib/" zurück.
     * @param path Pfad mit Dateinamen
     * @return String mit dem Pfad
     */
    static String getPath(const String& path);

    /** @brief Dateinamen ohne Pfad
     *
     * Diese Funktion liefert den Dateinamen eines Strings zurück, der Pfad und Dateinamen
     * enthält. Lautet der String beispielsweise "/home/patrick/svn/pplib/README.TXT" liefert die Funktion
     * "README.TXT" zurück.
     * @param path Pfad mit Dateinamen
     * @return String mit dem Dateinamen
     */
    static String getFilename(const String& path);

    /** @brief Dateiendung (Suffix) extrahieren
     *
     * Diese Funktion liefert die Dateiendung eines Strings zurück, der Pfad und Dateinamen
     * enthält. Lautet der String beispielsweise "/home/patrick/svn/pplib/README.TXT" liefert die Funktion
     * "TXT" zurück.
     * @param path Pfad mit Dateinamen
     * @return String mit der Dateiendung
     */
    static String getSuffix(const String& path);

    /** @brief MD5-Hash einer Datei berechnen
     *
     * Diese Funktion berechnet den MD5-Hash der angegebenen Datei und liefert ihn als String zurück.
     * @param filename Dateiname
     * @return String mit dem MD5-Hash
     */
    static String md5Hash(const String& filename);

    /** @brief SHA-256-Hash einer Datei berechnen
     *
     * Diese Funktion berechnet den SHA-256-Hash der angegebenen Datei und liefert ihn als String zurück.
     * @param filename Dateiname
     * @return String mit dem SHA-256-Hash
     */
    static String sha256Hash(const String& filename);

    /** @brief Prüfen, ob der Pfad ein Verzeichnis ist
     *
     * Diese Funktion prüft, ob der angegebene Pfad ein Verzeichnis ist.
     * @param filename Pfad zur Datei oder zum Verzeichnis
     * @return true, falls der Pfad ein Verzeichnis ist, sonst false
     */
    static bool isDir(const String& filename);

    /** @brief Prüfen, ob der Pfad eine reguläre Datei ist
     *
     * Diese Funktion prüft, ob der angegebene Pfad eine reguläre Datei ist.
     * @param filename Pfad zur Datei
     * @return true, falls der Pfad eine reguläre Datei ist, sonst false
     */
    static bool isFile(const String& filename);

    /** @brief Prüfen, ob der Pfad ein symbolischer Link ist
     *
     * Diese Funktion prüft, ob der angegebene Pfad ein symbolischer Link ist.
     * @param filename Pfad zur Datei oder zum Verzeichnis
     * @return true, falls der Pfad ein symbolischer Link ist, sonst false
     */
    static bool isLink(const String& filename);

    /** @brief Prüfen, ob der Pfad lesbar ist
     *
     * Diese Funktion prüft, ob der angegebene Pfad lesbar ist.
     * @param filename Pfad zur Datei oder zum Verzeichnis
     * @return true, falls der Pfad lesbar ist, sonst false
     */
    static bool isReadable(const String& filename);

    /** @brief Prüfen, ob der Pfad beschreibbar ist
     *
     * Diese Funktion prüft, ob der angegebene Pfad beschreibbar ist.
     * @param filename Pfad zur Datei oder zum Verzeichnis
     * @return true, falls der Pfad beschreibbar ist, sonst false
     */
    static bool isWritable(const String& filename);

    /** @brief Prüfen, ob der Pfad ausführbar ist
     *
     * Diese Funktion prüft, ob der angegebene Pfad ausführbar ist.
     * @param filename Pfad zur Datei oder zum Verzeichnis
     * @return true, falls der Pfad ausführbar ist, sonst false
     */
    static bool isExecutable(const String& filename);
};

} // namespace pplib

#endif /* PPLIB_CORE_MEMFILE_H_ */
