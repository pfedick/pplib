/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
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

#ifndef PPL7_CORE_PFPFILE_H_
#define PPL7_CORE_PFPFILE_H_

#include <ppl7/types/string.h>
#include <ppl7/types/bytearray.h>
#include <ppl7/core/compression.h>
#include <ppl7/core/fileobject.h>
#include <ppl7/core/mutex.h>

#include <list>

namespace ppl7
{

/**
 * @class PFPChunk
 * @brief Daten-Chunk eines PFP-File Version 3
 *
 * In dieser Klasse werden die Daten eines einzelnen Chunks eines PFP-Files Version 3
 * gespeichert. Um einen Chunk zu erstellen, wird mit "new" eine neue Instanz
 * der Klasse generiert und mit der Funktion PFPFile::addChunk einem PFP-File
 * hinzugefügt. Um einen existierenden Chunk zu löschen, muß die Funktion
 * PFPFile::deleteChunk aufgerufen werden.
 *
 * @see PFPFile
 *
 */
class PFPChunk
{
    friend class PFPFile;

private:
    String chunkname;
    ByteArray chunkdata;

public:
    /**
     * @brief Konstruktor des PFPChunk
     *
     * Der Konstruktor initialisiert den Datenbereich mit NULL und setzt als
     * Name "UNKN" ein. Ein derartiger Chunk würde durch die PFPFile-Klasse
     * nicht gespeichert.
     */
    PFPChunk();

    /**
     * @brief Konstruktor des PFPChunk mit Name und Daten
     *
     * Der Konstruktor initialisiert den Datenbereich mit den Daten aus \p data und
     * setzt als Name \p chunkname ein.
     *
     * @param chunkname Name des Chunks, muss exakt 4 Byte lang sein
     * @param data Referenz auf ein ByteArray oder ByteArrayPtr mit den Nutzdaten
     */
    PFPChunk(const String& chunkname, const ByteArrayPtr& data)
    {
        setName(chunkname);
        setData(data);
    }

    /**
     * @brief Copy-Konstruktor des PFPChunk
     *
     * Der Copy-Konstruktor erstellt eine Kopie eines existierenden Chunks.
     */
    PFPChunk(const PFPChunk& other) = default;

    /**
     * @brief Move-Konstruktor des PFPChunk
     *
     * Der Move-Konstruktor erstellt eine Kopie eines existierenden Chunks.
     */
    PFPChunk(PFPChunk&& other) noexcept = default;

    /**
     * @brief Destruktor des PFPChunk
     *
     * Der Destruktor sorgt dafür, dass der Datenbereich des Chunks freigegeben
     * wird, sofern er definiert wurde, und wenn er einem PFPFile zugeordnet
     * wurde, wird er daraus entfernt.
     *
     */
    ~PFPChunk() = default;

    /**
     * @brief Name des Chunks setzen
     *
     * Mit dieser Funktion wird der Name eines Chunks definiert. Der Name muss
     * exakt 4 Byte lang sein und darf nur Großbuchstaben enthalten (es wird
     * eine automatische Konvertierung durchgeführt). Ausserdem sind nur Zeichen
     * aus dem Zeichensatz US-ASCII erlaubt.
     *
     * @param chunkname String mit dem Namen des Strings, muss exakt 4 Byte lang sein
     * @exception IllegalArgumentException Wird geworfen, wenn der Name des Chunks ungültig ist
     */
    void setName(const String& chunkname);

    /**
     * @brief Nutzdaten des Chunks setzen
     *
     * Mit dieser Funktion werden die Nutzdaten des Chunks angegeben. Die
     * Daten werden dabei in einen eigenen Speicherbereich kopiert.
     *
     * @param ptr Ein Pointer auf den Beginn der Daten
     * @param size Größe der Daten in Byte. Ein Chunk darf maximal 2^32 - 9 Bytes groß sein
     * @exception OutOfMemoryException Nicht genug Speicher
     * @exception IllegalArgumentException Der Parameter \p size ist zu groß oder
     * \p ptr ist NULL
     */
    void setData(const void* ptr, size_t size);

    /**
     * @brief Nutzdaten des Chunks setzen
     *
     * Mit dieser Funktion werden die Nutzdaten des Chunks angegeben. Die
     * Daten werden dabei in einen eigenen Speicherbereich kopiert.
     *
     * @param data Eine Referenz auf ein ByteArray oder ByteArrayPtr
     * @exception NullPointerException Wird geworfen, wenn \p ptr auf NULL zeigt
     * @exception OutOfMemoryException Nicht genug Speicher
     */
    void setData(const ByteArrayPtr& data);

    /**
     * @brief Größe der Nutzdaten des Chunks
     *
     * Mit dieser Funktion kann die Größe der Nutzdaten des Chunks ermittelt werden.
     *
     * @return Größe der Nutzdaten in Byte
     */
    inline size_t size() const
    {
        return chunkdata.size();
    }

    /**
     * @brief Pointer auf die Nutzdaten des Chunks
     *
     * Mit dieser Funktion kann ein Pointer auf die Nutzdaten des Chunks ermittelt werden.
     *
     * @return Pointer auf die Nutzdaten
     */
    inline const void* data() const
    {
        return chunkdata.ptr();
    }

    /**
     * @brief Name des Chunks
     *
     * Diese Funktion liefert den Namen des Chunks zurück. Dieser ist
     * immer 4 Byte groß.
     *
     * @return Name des Chunks
     */
    inline const String& name() const
    {
        return chunkname;
    }
};

/**
 * @class PFPFile
 * @brief Klasse zum Lesen und schreiben von PFP-Files Version 3
 *
 * Mit dieser Klasse können Dateien mit "PFP-File"-Header der Version 3 gelesen und geschrieben werden.
 * Mit Version 3 wurde ein mehr generisches Format definiert, als in den beiden Vorgängerversionen.
 * Jedes File, ganz gleich welchen Inhalt es hat, hat bis zum Ende den gleichen Aufbau. Wichtigste
 * Neuerung dabei sind die sogenannten Chunks. Ein File kann aus bliebig vielen Chunks bestehen.
 * Diese werden von der Klasse PFPChunk abgeleitet, bekommen einen Namen und einen beliebigen
 * Inhalt. Diese können dann mit PFPFile::Add in das File hinzugefügt werden.
 *
 * @copydoc PFPFileVersion3
 */
class PFPFile
{
private:
    std::list<PFPChunk> Chunks;  /// Verwaltung aller Chunks in einer Liste
    String id;                   /// ID des Files. Ist immer 4 Byte groß
    uint8_t mainversion;         /// Hauptversion des Files
    uint8_t subversion;          /// Unterversion des Files
    Compression::Algorithm comp; /// Komprimierungsmethode, die für das File verwendet wird

    /**
     * @brief Interne Funktion zum Speichern von vordefinierten Chunks
     *
     * Diese Funktion wird intern verwendet, um die vordefinierten Text-Chunks zu speichern. Sie
     * stellt sicher, dass jeder Chunk nur einmal vorkommt.
     *
     * @param chunkname Pointer auf den Namen des Chunks.
     * @param data Pointer auf den zu setzenden Text-String
     * @see Die Funktion wird intern von folgenden Funktionen aufgerufen:
     * - PFPFile::SetName
     * - PFPFile::SetAuthor
     * - PFPFile::SetDescription
     * - PFPFile::SetCopyright
     */
    void setStringParam(const String& chunkname, const String& data);

public:
    class Iterator
    {
    public:
        std::list<PFPChunk>::const_iterator it;
        String findchunk; /// Name des Chunks, der gesucht wird
        bool started = false;
    };

    /**
     * @brief Konstruktor der Klasse
     *
     * Hier werden einige interne Variablen initialisert, die ID wird auf "UNKN" gesetzt, Version
     * auf 0 und Kompression abgeschaltet
     */
    PFPFile();

    /**
     * @brief Destruktor der Klasse
     *
     * Der Destruktor sorgt dafür, dass sämtlicher von der Klasse allokierter Speicher einschließlich
     * aller geladener Chunks freigegeben wird.
     */
    ~PFPFile();

    /**
     * @brief Inhalt der Klasse löschen
     *
     * Mit dieser Funktion werden alle Chunks im Speicher freigegeben und die Klasse auf den
     * Ursprungszustand zurückgesetzt, das heisst sie ist anschließend leer
     *
     */
    void clear();

    /**
     * @brief Author setzen
     *
     * Mit dieser Funktion wird automatisch ein Author-Chunk ("AUTH") angelegt. Dabei ist sichergestellt,
     * dass der Chunk nur ein einziges mal in der Datei vorkommt.
     *
     * @param author String mit dem Author des Files
     */
    void setAuthor(const String& author);

    /**
     * @brief Copyright setzen
     *
     * Mit dieser Funktion wird automatisch ein Copyright-Chunk ("COPY") angelegt. Dabei ist sichergestellt,
     * dass der Chunk nur ein einziges mal in der Datei vorkommt.
     *
     * @param copy Inhalt des Copyright-Strings
     */
    void setCopyright(const String& copy);

    /**
     * @brief Description setzen
     *
     * Mit dieser Funktion wird automatisch ein Description-Chunk ("DESC") angelegt. Dabei ist sichergestellt,
     * dass der Chunk nur ein einziges mal in der Datei vorkommt.
     *
     * @param descr String mit der Description
     */
    void setDescription(const String& descr);

    /**
     * @brief Name setzen
     *
     * Mit dieser Funktion wird automatisch ein Namens-Chunk ("NAME") angelegt. Dabei ist sichergestellt,
     * dass der Chunk nur ein einziges mal in der Datei vorkommt.
     *
     * @param name String mit dem Namen des Files
     */
    void setName(const String& name);

    /**
     * @brief Version des PFP-Files setzen
     *
     * @todo Sollten wir uint8_t verwenden?
     * @param main Hauptversion, Wert zwischen 0 und 255
     * @param sub Unterversion, Wert zwischen 0 und 255
     * @exception IllegalArgumentException Wird geworfen, wenn \p main oder \p sub ausserhalb des gültigen Bereichs liegen.
     *
     * @remarks
     * Haupt- und Unterversion werden jeweils in einem einzelnen Byte gespeichert. Daher darf die
     * Version nicht größer als 255 werden.
     */
    void setVersion(int main = 0, int sub = 0);

    /**
     * @brief ID des PFP-Files setzen
     *
     * Mit dieser Version wird die ID des PFP-Files festgelegt. Eine ID muss zwingend 4 Byte lang
     * sein und darf nur US-ASCII-Zeichen enthalten.
     *
     * @param id Ein 4-Byte langer String aus US-ASCII-Zeichen (ASCII 32-127).
     * @exception IllegalArgumentException Wird geworfen, wenn die \p id einen ungültigen Wert enthält
     * oder nicht exakt 4 Byte lang ist.
     */
    void setId(const String& id);

    /**
     * @brief Kompression einstellen
     *
     * Mit dieser Funktion wird festgelegt ob und welche Kompression beim Speichern verwendet werden
     * soll.
     *
     * @param type Ein Wert, der die Art der Kompression angibt. Mögliche Werte sind:
     * - Compression::Algo_NONE - Keine Komprimierung
     * - Compression::Algo_ZLIB - Komprimierung mit Zlib
     * - Compression::Algo_BZIP2 - Komprimierung mit Bzip2
     *
     * @exception UnknownCompressionMethodException Wird geworfen, wenn \p type einen ungültigen Wert enthält.
     */
    void setCompression(Compression::Algorithm type);

    /**
     * @brief PFP-File speichern
     *
     * Mit dieser Funktion wird der Inhalt der PFPFile-Klasse in eine Datei geschrieben.
     * Dabei wird der Header und sämtliche Chunks zusammengefasst, gegebenenfalls komprimiert
     * (siehe PFPFile::setCompression) und im Filesystem gespeichert. Der genaue Aufbau der Datei
     * wird weiter unten beschrieben.
     *
     * @param filename String mit dem Dateinamen.
     * @note Eine eventuell vorhandene Datei mit gleichem Namen wird überschrieben.
     *
     * @remarks
     * Die Funktion stellt sicher, dass die Chunks in einer bestimmten Reihenfolge geschrieben
     * werden. Die vordefinierten Chunks mit Name, Author, Copyright und Beschreibung werden in
     * jedem Fall zuerst gespeichert, dann die restlichen Chunks.
     * Die Datei wird auch dann geschrieben, wenn keine Chunks vorhanden sind. In diesem Fall enthält die Datei nur den Header.
     *
     * @par Aufbau der PFP-Datei
     *
     * @copydoc PFPFileVersion3
     */
    void save(const String& filename);

    /**
     * @brief Chunk hinzufügen
     *
     * Mit dieser Funktion wird ein neuer Chunk in die Klasse hinzugefügt. Der Chunk muss von der
     * Anwendung mit "new" erstellt worden sein, einen Namen haben. Ist dies nicht der Fall,
     * gibt die Funktion eine Fehlermeldung zurück.
     *
     * Sobald der Chunk mit AddChunk an die PFPFile-Klasse übergeben wurde, wird er von der Klasse
     * verwaltet und gegebenenfalls auch gelöscht. Die Anwendung braucht kein "delete" darauf zu
     * machen.
     *
     * @param chunk Pointer auf den hinzuzufügenden Chunk
     * @remarks Es ist möglich mehrere Chunks mit gleichem Namen hinzuzufügen. Der Chunk wird nur in der
     * Klasse hinzugefügt, nicht aber in die Datei geschrieben. Zum Speichern muss explizit die Funktion
     * PFPFile::Save aufgerufen werden.
     *
     * @example
     * @code
     * void *ptr=xxxxx;    // Pointer auf die Daten
     * int size=xxxx;      // Größe der Daten in Byte
     * ppl7::PFPFile file;
     * ppl7::PFPChunk *chunk=new ppl7::PFPChunk;
     * chunk->SetName("DATA");
     * chunk->SetData(ptr,size);
     * file.AddChunk(chunk);
     * @endcode
     *
     */
    [[deprecated("Use addChunk(PFPChunk&& chunk) instead")]] void addChunk(PFPChunk* chunk);

    /**
     * @brief Chunk hinzufügen
     *
     * Mit dieser Funktion wird ein neuer Chunk in die Klasse hinzugefügt. Der Chunk muss einen Namen haben.
     * Ist dies nicht der Fall, gibt die Funktion eine Fehlermeldung zurück.
     *
     * @param chunk Referenz auf den hinzuzufügenden Chunk
     * @remarks Es ist möglich mehrere Chunks mit gleichem Namen hinzuzufügen. Der Chunk wird nur in der
     * Klasse hinzugefügt, nicht aber in die Datei geschrieben. Zum Speichern muss explizit die Funktion
     * PFPFile::Save aufgerufen werden.
     *
     * @example
     * @code
     * void *ptr=xxxxx;    // Pointer auf die Daten
     * int size=xxxx;      // Größe der Daten in Byte
     * ppl7::PFPFile file;
     * ppl7::PFPChunk chunk("DATA",ppl7::ByteArrayPtr(ptr,size));
     * file.addChunk(chunk);
     * @endcode
     *
     */
    PFPChunk& addChunk(const PFPChunk& chunk);

    /**
     * @brief Chunk hinzufügen
     *
     * Mit dieser Funktion wird ein neuer Chunk in die Klasse hinzugefügt. Der Chunk muss einen Namen haben.
     * Ist dies nicht der Fall, gibt die Funktion eine Fehlermeldung zurück.
     *
     * @param chunk Rvalue-Referenz auf den hinzuzufügenden Chunk
     * @remarks Es ist möglich mehrere Chunks mit gleichem Namen hinzuzufügen. Der Chunk wird nur in der
     * Klasse hinzugefügt, nicht aber in die Datei geschrieben. Zum Speichern muss explizit die Funktion
     * PFPFile::Save aufgerufen werden.
     *
     * @example
     * @code
     * void *ptr=xxxxx;    // Pointer auf die Daten
     * int size=xxxx;      // Größe der Daten in Byte
     * ppl7::PFPFile file;
     * ppl7::PFPChunk chunk("DATA",ppl7::ByteArrayPtr(ptr,size));
     * file.addChunk(std::move(chunk));
     * @endcode
     *
     */
    PFPChunk& addChunk(PFPChunk&& chunk);

    /**
     * @brief Bestimmten Chunk löschen
     *
     * Mit dieser Funktion wird ein bestimmter Chunk aus der Klasse gelöscht.
     *
     * @param chunk Pointer auf den zu löschenden Chunk
     */
    void deleteChunk(PFPChunk* chunk);

    /**
     * @brief Chunk nach Namen löschen
     *
     * Mit dieser Funktion werden alle Chunks gelöscht, die den angegebenen Namen haben
     *
     * @param chunkname Pointer auf den Namen des Chunks
     */
    void deleteChunk(const String& chunkname);

    /**
     * @brief Chunk nach Namen suchen
     *
     * Mit dieser Funktion wird der erste Chunk mit dem angegebenen Namen gesucht und zurückgegeben.
     * Wird kein Chunk gefunden, wird NULL zurückgegeben.
     *
     * @param it Iterator, der für die Suche verwendet wird.
     * @param chunkname Pointer auf den Namen des Chunks
     * @return Pointer auf den gefundenen Chunk oder NULL, wenn kein Chunk gefunden wurde
     */
    PFPChunk* findFirstChunk(Iterator& it, const String& chunkname) const;

    /**
     * @brief Chunk nach Namen suchen
     *
     * Mit dieser Funktion wird der nächste Chunk mit dem angegebenen Namen gesucht und zurückgegeben.
     * Wird kein Chunk gefunden, wird NULL zurückgegeben.
     * @param it Iterator, der für die Suche verwendet wird.
     * @param chunkname Pointer auf den Namen des Chunks
     * @return Pointer auf den gefundenen Chunk oder NULL, wenn kein Chunk gefunden wurde
     */
    PFPChunk* findNextChunk(Iterator& it, const String& chunkname) const;

    /**
     * @brief Alle Chunks auflisten
     *
     * Mit dieser Funktion werden alle Chunks in der Klasse auf stdout aufgelistet. Dabei werden die Namen und
     * Größen der Chunks ausgegeben. Die Funktion dient nur zu Testzwecken und ist nicht für den produktiven Einsatz gedacht.
     */
    void list() const;

    /**
     * @brief PFP-File laden
     *
     * Mit dieser Funktion wird ein PFP-File geladen. Dabei wird zuerst der Header geladen
     * und überprüft, ob es sich um ein gültiges PFP-File handelt. Danach wird geprüft, ob
     * der Datenbereich komprimiert ist und gegebenenfalls dekomprimiert. Erst danach werden die
     * einzelnen Chunks eingelesen. Kommt es dabei zu Fehlern durch ungültige Chunks, werden diese
     * ignoriert
     *
     * @param ff Referenz auf ein geöffnetes FileObject, aus dem die Daten gelesen werden sollen
     *
     * @remarks
     * Durch das Laden eines PFP-Files werden alle bisher in der Klasse vorhandenen Chunks gelöscht.
     */
    void load(FileObject& ff);

    /**
     * @brief PFP-File laden
     *
     * Mit dieser Funktion wird ein PFP-File geladen. Dabei wird zuerst der Header geladen
     * und überprüft, ob es sich um ein gültiges PFP-File handelt. Danach wird geprüft, ob
     * der Datenbereich komprimiert ist und gegebenenfalls dekomprimiert. Erst danach werden die
     * einzelnen Chunks eingelesen. Kommt es dabei zu Fehlern durch ungültige Chunks, werden diese
     * ignoriert
     *
     * @param file String mit dem Dateinamen des zu ladenden Files
     *
     * @remarks
     * Durch das Laden eines PFP-Files werden alle bisher in der Klasse vorhandenen Chunks gelöscht.
     */
    void load(const String& file);

    /**
     * @brief Prüfen, ob es sich um ein PFP-File handelt
     *
     * Diese Funktion prüft, ob es sich bei der geöffneten Datei \p ff um eine Datei
     * im \ref PFPFileVersion3 PFP-Format Version 3 handelt. Ist dies der Fall, wird deren
     * ID und Version eingelesen.
     *
     * @param ff Referenz auf eine geöffnete Datei
     * @return Gibt \c true zurück, wenn es sich um eine Datei im PFP-Format handelt. Deren
     * ID kann anschließend mit PFPFile::getID ausgelesen werden, Version mit PFPFile::getVersion bzw.
     * PFPFile::getMainVersion und PFPFile::getSubVersion. Handelt es sich nicht um eine Datei
     * im PFP-Format, gibt die Funktion \c false zurück. Es wird keine Exception geworfen.
     */
    bool ident(FileObject& ff);

    /**
     * @brief Prüfen, ob es sich um ein PFP-File handelt
     *
     * Diese Funktion prüft, ob es sich bei der Datei \p file um eine Datei
     * im \ref PFPFileVersion3 PFP-Format Version 3 handelt. Ist dies der Fall, wird deren
     * ID und Version eingelesen.
     *
     * @param file String mit dem Dateinamen
     * @return Gibt \c true zurück, wenn es sich um eine Datei im PFP-Format handelt. Deren
     * ID kann anschließend mit PFPFile::getID ausgelesen werden, Version mit PFPFile::getVersion bzw.
     * PFPFile::getMainVersion und PFPFile::getSubVersion. Handelt es sich nicht um eine Datei
     * im PFP-Format, gibt die Funktion \c false zurück. Es wird keine Exception geworfen.
     */
    bool ident(const String& file);

    /**
     * @brief Name des Files auslesen
     *
     * Mit dieser Funktion wird der Name des Files aus dem Namens-Chunk ("NAME") ausgelesen.
     * \returns String mit dem Namen. Kann auch leer sein, wenn es keinen "NAME"-Chunk in der Datei gibt.
     */
    String getName() const;

    /**
     * @brief Author des Files auslesen
     *
     * Mit dieser Funktion wird der Name des Authors aus dem Author-Chunk ("AUTH") ausgelesen.
     * \returns String mit dem Namen des Authors. Kann auch leer sein, wenn es keinen "AUTH"-Chunk in der Datei gibt.
     */
    String getAuthor() const;

    /**
     * @brief Description des Files auslesen
     *
     * Mit dieser Funktion wird die Description des Files aus dem Description-Chunk ("DESC") ausgelesen.
     * \returns String mit der Description. Kann auch leer sein, wenn es keinen "DESC"-Chunk in der Datei gibt.
     */
    String getDescription() const;

    /**
     * @brief Copyright des Files auslesen
     *
     * Mit dieser Funktion wird das Copyright des Files aus dem Copyright-Chunk ("COPY") ausgelesen.
     * \returns String mit dem Copyright. Kann auch leer sein, wenn es keinen "COPY"-Chunk in der Datei gibt.
     */
    String getCopyright() const;

    /**
     * @brief Version des Files auslesen
     *
     * Mit dieser Funktion wird die Version des Files ausgelesen.
     * \param main Pointer auf eine Variable, in der die Hauptversion gespeichert werden soll
     * \param sub Pointer auf eine Variable, in der die Unterversion gespeichert werden soll
     */
    inline void getVersion(int* main, int* sub) const
    {
        if (main) *main = mainversion;
        if (sub) *sub = subversion;
    }

    /**
     * @brief Version des Files auslesen
     *
     * Mit dieser Funktion wird die Version des Files ausgelesen.
     * \param main Referenz auf eine Variable, in der die Hauptversion gespeichert werden soll
     * \param sub Referenz auf eine Variable, in der die Unterversion gespeichert werden soll
     */
    inline void getVersion(int& main, int& sub) const
    {
        main = mainversion;
        sub = subversion;
    }

    /**
     * @brief ID des Files auslesen
     *
     * Mit dieser Funktion wird die ID des Files ausgelesen.
     * \returns String mit der ID. Die ID ist immer 4 Byte lang.
     */
    inline const String& getID() const
    {
        return id;
    }

    /**
     * @brief Hauptversion auslesen
     *
     * Mit dieser Funktion wird die Hauptversion der Datei ausgelesen.
     * \returns Hauptversion als Integer
     */
    inline uint8_t getMainVersion() const
    {
        return mainversion;
    }

    /**
     * @brief Unterversion auslesen
     *
     * Mit dieser Funktion wird die Unterversion der Datei ausgelesen.
     * \returns Unterversion als Integer
     */
    inline uint8_t getSubVersion() const
    {
        return subversion;
    }

    /**
     * @brief Kompression auslesen
     *
     * Mit dieser Funktion wird die Kompression der Datei ausgelesen.
     * \returns Kompression als Integer
     */
    inline Compression::Algorithm getCompression() const
    {
        return comp;
    }

    /**
     * @brief Zeiger zum Durchwandern der Chunks zurücksetzen
     *
     * Mit dieser Funktion wird der Zeiger, der beim Durchwandern der Chunks mit den Funktionen
     * FindNextChunk und GetNext verwendet wird, wieder auf den Anfang gesetzt.
     *
     * @param it Iterator, der zurückgesetzt werden soll
     *
     * @see
     * - PFPFile::FindFirstChunk
     * - PFPFile::FindNextChunk
     * - PFPFile::Reset
     * - PFPFile::GetFirst
     * - PFPFile::GetNext
     *
     * \since Version 6.1.0
     */
    void reset(Iterator& it) const;

    /**
     * @brief Pointer auf ersten Chunk holen
     *
     * Diese Funktion liefert einen Pointer auf den ersten Chunk in der Datei zurück.
     *
     * @param it Iterator, der zurückgesetzt werden soll
     *
     * @returns Pointer auf den ersten Chunk oder NULL, wenn es keine Chunks gibt.
     * @see
     * - PFPFile::FindFirstChunk
     * - PFPFile::FindNextChunk
     * - PFPFile::Reset
     * - PFPFile::GetFirst
     * - PFPFile::GetNext
     *
     * @since Version 6.1.0
     */
    PFPChunk* getFirst(Iterator& it) const;

    /**
     * @brief Pointer auf nächsten Chunk holen
     *
     * Diese Funktion liefert einen Pointer auf den nächsten Chunk in der Datei zurück.
     *
     * @param it Iterator, der zurückgesetzt werden soll
     *
     * @returns Pointer auf den nächsten Chunk oder NULL, wenn es keine weiteren Chunks gibt.
     * @see
     * - PFPFile::FindFirstChunk
     * - PFPFile::FindNextChunk
     * - PFPFile::Reset
     * - PFPFile::GetFirst
     * - PFPFile::GetNext
     *
     * @since Version 6.1.0
     */
    PFPChunk* getNext(Iterator& it) const;

    auto begin()
    {
        return Chunks.begin();
    }
    auto end()
    {
        return Chunks.end();
    }
    auto begin() const
    {
        return Chunks.cbegin();
    }
    auto end() const
    {
        return Chunks.cend();
    }
};

} // namespace ppl7

#endif /* PPL7_CORE_DIR_H_ */