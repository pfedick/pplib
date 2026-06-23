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

#ifndef PPL7_AUDIO_ID3TAG_H
#define PPL7_AUDIO_ID3TAG_H
#include <ppl7.h>

namespace ppl7
{

/**@ingroup PPLGroupSound
 * @brief Genre-Bezeichnung
 *
 * Mit dieser Funktion kann die Bezeichnung eines Genres anhand der Genre-ID ausgelesen werden.
 * Die Werte 0-79 wurden durch die ID3v1-Spezifikation definiert, die Werte 80-147 sind eine
 * Erweiterung von WinAmp (siehe http://www.mpx.cz/mp3manager/tags.htm).
 *
 * @param[in] id Zahl von 0 bis 147, die den Index des Genres darstellt
 * @return String-Objekt mit der Bezeichnung des Genres. Falls die \p id ungültig ist, wird ein leerer String zurückgegeben.
 */
String GetID3GenreName(int id);

/**@class CID3Frame
 * @ingroup PPLGroupSound
 * @brief Klasse zum Speichern eines einzelnen ID3-Frames
 */
class ID3Frame
{
    friend class ID3Tag;
    friend class ID3TagTranscode;

private:
    String ID;
    int Flags;
    ByteArray data;

public:
    //@brief Konstruktor ohne Parameter
    ID3Frame();

    //@brief Konstruktor mit Frame-Name
    ID3Frame(const String& name);

    //@brief Konstruktor mit Frame-Name, Flags und Daten
    ID3Frame(const String& name, int flags, const ByteArrayPtr& data);

    inline void setData(const ByteArrayPtr& data)
    {
        this->data.copy(data);
    }
    inline void setData(const char* adr, size_t size)
    {
        this->data.copy(adr, size);
    }
    constexpr inline void setFlags(int flags)
    {
        this->Flags = flags;
    }
    void hexDump() const;
    inline const String& name() const
    {
        return ID;
    }
    constexpr inline int flags() const
    {
        return Flags;
    }
    inline size_t size() const
    {
        return data.size();
    }

    inline const char* dataPtr() const
    {
        return (const char*)data.ptr();
    }
    void getData(ByteArray& data) const;

    inline const ByteArrayPtr& getData() const
    {
        return data;
    }
    inline bool hasData() const
    {
        return !data.isEmpty();
    }
    inline bool isEmpty() const
    {
        return data.isEmpty();
    }
};

/**@class CID3Tag
 * @ingroup PPLGroupSound
 * @brief Klasse zum Parsen und Verändern von ID3 v1 und v2 Tags
 *
 * Mit dieser Klasse können ID3v2-Tags aus Audio-Dateien ausgelesen und geschrieben werden.
 * Beim Schreiben von MP3-Dateien kann zusätzlich noch ein ID3v1-Tag erzeugt werden.
 *
 * Dazu muß zunächst die gewünschte Datei mit ID3Tag::load geöffnet werden. Mit den
 * verschiedenen "Get"-Befehlen können verschiedene vorgegebene Frames aus dem ID3v2-Tag
 * ausgelesen werden (z.B. ID3Tag::getArtist um den Namen des Interpreten auszulesen),
 * mit "Set" werden sie verändert (z.B. ID3Tag::setArtist). Mit der Funktionen
 * ID3Tag::findFrame kann auch jedes beliebige andere Frame ausgelesen werden, jedoch
 * gibt es zur Zeit keine generische Funktion zum Speichern von beliebigen Frames.
 *
 * Alle Operationen finden nur im Hauptspeicher statt. Erst durch Aufruf der
 * Funktion ID3Tag::save werden sie zurück in die Datei geschrieben.
 */
class ID3Tag
{
public:
    enum TextEncoding
    {
        ENC_USASCII,
        ENC_ISO88591,
        ENC_UTF16,
        ENC_UTF8
    };

    enum AudioFormat
    {
        AF_UNKNOWN = 0,
        AF_MP3,
        AF_AIFF,
        AF_WAVE
    };

    enum PictureType
    {
        PIC_COVER_FRONT = 3,
        PIC_COVER_BACK = 4,
    };

private:
    String Filename;
    String localCharset;
    int Flags;
    int Size;
    AudioFormat myAudioFormat;
    uint32_t PaddingSize, PaddingSpace, MaxPaddingSpace;

    std::list<ID3Frame> frames;

    void setTextFrameUtf16(const String& framename, const String& text);
    void setTextFrameISO88591(const String& framename, const String& text);
    void setTextFrameUtf8(const String& framename, const String& text);

    /**@brief Text eines Frames dekodieren und in einen String kopieren
     *
     * Mit dieser internen Funktion wird der Text eines Frames ab einem
     * bestimmten \p offset in das lokale Format des Systems konvertiert und dann
     * im String \p s gespeichert.
     *
     * @param[out] s String, in dem der Text gespeichert werden soll
     * @param[in] frame Pointer auf das Frame
     * @param[in] offset Byte-Offset innerhalb des Frames
     */
    void copyAndDecodeText(String& s, const ID3Frame* frame, int offset) const;
    int decode(const ID3Frame* frame, int offset, int encoding, String& target) const;

    AudioFormat identAudioFormat(FileObject& File);
    uint64_t findId3Tag(FileObject& File);
    void saveMP3();
    void saveAiff();
    void saveWave();
    bool trySaveAiffInExistingFile(FileObject& o, ByteArrayPtr& tagV2);
    bool trySaveWaveInExistingFile(FileObject& o, ByteArrayPtr& tagV2);
    void copyAiffToNewFile(FileObject& o, FileObject& n, ByteArrayPtr& tagV2);
    void copyWaveToNewFile(FileObject& o, FileObject& n, ByteArrayPtr& tagV2);
    String getNullPaddedString(const ID3Frame& frame, size_t offset = 0) const;

public:
    ID3Tag();
    ID3Tag(const String& File);
    ~ID3Tag();

    /**@brief ID3-Tags aus einer Audio-Datei laden
     *
     * Mit dieser Funktion werden die ID3-Tags aus der Audio-Datei \p file geladen.
     * @param filename Dateiname
     */
    void load(const String& filename);

    /**@brief ID3-Tags aus einem CFileObject laden
     *
     * Mit dieser Funktion werden die ID3-Tags einer bereits geöffneten Audio-Datei,
     * die durch das FileObject \p file repräsentiert wird, in den Hauptspeicher geladen.
     *
     * @param[in] file Geöffnete Datei
     */
    void load(FileObject& file);

    /**@brief ID3-Tags aus einer Audio-Datei laden
     *
     * Wie bei \p ID3Tag::load werden die ID3-Tags aus der Audio-Datei \p filename geladen.
     * Jedoch wird keine Exception geworfen, wenn dies nicht gelingt. Der Erfolg wird durch
     * den Returnwert angezeigt.
     *
     * @param filename Dateiname
     * @return true, wenn die Tags geladen werden konnten, sonst false
     */
    bool tryLoad(const String& filename);

    /**@brief ID3-Tags aus einem CFileObject laden
     *
     * Mit dieser Funktion werden die ID3-Tags einer bereits geöffneten Audio-Datei,
     * die durch das FileObject \p file repräsentiert wird, geladen.
     * Jedoch wird keine Exception geworfen, wenn dies nicht gelingt. Der Erfolg wird durch
     * den Returnwert angezeigt.
     *
     * @param[in] file Geöffnete Datei
     * @return true, wenn die Tags geladen werden konnten, sonst false
     */
    bool tryLoad(FileObject& file);

    /**@brief Speicher freigeben und Klasse in den Ausgangszustand versetzen
     *
     * Durch Aufruf dieser Funktion wird der komplette durch diese Klasse reservierte
     * Speicher freigegeben und die Klasse in den Ausgangszustand zurückversetzt.
     * Die Funktion wird automatisch vor dem Laden einer Datei mit CID3Tag::load
     * aufgerufen.
     */
    void clear();

    /**@brief Alle Tags löschen
     *
     * Durch Aufruf dieser Funktion werden alle Frames des geladenen Titels im Speicher gelöscht.
     * Die Datei bleibt unverändert, erst durch Aufruf der Funktion CID3Tag::save werden
     * auch die ID3-Tags in der Datei aktualisiert und somit gelöscht.
     */
    void clearTags();

    /**@brief Alle Änderungen in die Datei zurückschreiben
     *
     * Mit dieser Funktion werden alle Änderungen, die an den ID3-Tags im Hauptspeicher vorgenommen wurden, zurück in die Datei geschrieben.
     * Es wird versucht, die Tags direkt in der bestehenden Datei zu aktualisieren. Falls dies nicht möglich ist (z.B. weil die neue
     * Tag-Größe nicht mehr in den vorhandenen Platz passt), wird eine temporäre Datei angelegt, in die die aktualisierten Daten geschrieben
     * werden. Anschließend wird die Originaldatei gelöscht und die temporäre Datei umbenannt.
     */
    void save();

    void setLocalCharset(const String& charset);
    void setPaddingSize(int bytes);
    void setPaddingSpace(int bytes);
    void setMaxPaddingSpace(int bytes);

    /**@brief Frame hinzufügen
     *
     * Mit dieser Funktion wird ein Frame zu den geladenen ID3-Tags hinzugefügt. Inhalt des
     * Frames wird kopiert, so dass der Frame nach dem Aufruf der Funktion wieder gelöscht werden kann.
     * @param Frame Frame, das zu den ID3-Tags hinzugefügt werden soll.
     */
    void addFrame(const ID3Frame& Frame);

    /**@brief Frame hinzufügen mit Move-Semantik
     *
     * Mit dieser Funktion wird das Frame \pFrame in den ID3Tag übernommen. ID3Tag übernimmt
     * dabei die Verwaltung des Speichers, so dass das Frame nach dem Aufruf der Funktion nicht mehr verwendet werden darf.
     * @param frame Frame, das in den ID3Tag übernommen werden soll.
     */
    void addFrame(ID3Frame&& Frame);

    /**@brief Frame löschen
     *
     * Mit dieser Funktion wird ein Frame aus den geladenen ID3-Tags gelöscht. Der Speicher
     * des Frames wird dabei freigegeben.
     * @param frame Pointer auf das zu löschende Frame.
     */
    void removeFrame(ID3Frame* frame);

    /**@brief Bestimmtes Frame finden
     *
     * Mit dieser Funktion wird nach einem Frame mit der ID \p name
     * gesucht und ein Pointer darauf zurückgegeben.
     *
     * @param name String mit der 4-stelligen ID des gesuchten Frames.
     * @return Wurde das gewünschte Frame gefunden, gibt die Funktion einen
     * Pointer auf ein ID3Frame-Objekt zurück, im Fehlerfall \c NULL.
     */
    ID3Frame* findFrame(const String& name) const;

    /**@brief Benutzerdefinierten Text in einem TXXX-Frame finden
     *
     * Mit dieser Funktion wird nach einem benutzerdefinierten Text
     * in einem TXXX-Frame mit dem Namen \p description gesucht
     * und ein Pointer darauf zurückgegeben.
     *
     * @param name String mit der Description des gesuchten Frames.
     * @return Wurde das gewünschte Frame gefunden, gibt die Funktion einen
     * Pointer auf ein ID3Frame-Objekt zurück, im Fehlerfall \c NULL.
     */
    ID3Frame* findUserDefinedText(const String& description) const;

    /**@brief Frames auf STDOUT auflisten
     *
     * Mit dieser Funktion werden alle Frames auf STDOUT ausgegeben. Sie ist zu
     * Debug-Zwecken gedacht.
     */
    void listFrames(bool hexdump = false) const;

    /**@brief Anzahl der Frames
     *
     * Mit dieser Funktion wird die Anzahl der Frames im geladenen ID3-Tag zurückgegeben.
     * @return Anzahl der Frames
     */
    size_t frameCount() const;

    /**@brief Name des Interpreten setzen
     *
     * Mit dieser Funktion wird der Name des Interpreten (Artist) festgelegt.
     * Der Wert wird im Frame "TPE1" gespeichert.
     *
     * @param[in] artist String mit dem Namen des Interpreten
     */
    void setArtist(const String& artist);

    /**@brief Titel des Songs setzen
     *
     * Mit dieser Funktion wird der Titel des Songs festgelegt.
     * Der Wert wird im Frame "TIT2" gespeichert.
     *
     * @param[in] title String mit dem Titel des Songs
     */
    void setTitle(const String& title);

    /**@brief Musikgenre setzen
     *
     * Mit dieser Funktion wird das Musikgenre des Titels festgelegt. Falls
     * das Genre eine offizielle ID hat, wird diese im Frame in Klammern vorangestellt
     * (z.B. <tt>(13)Pop</tt>).
     * Der Wert wird im Frame "TCON" gespeichert.
     *
     * @param[in] genre String mit dem Namen des Musikgenres
     */
    void setGenre(const String& genre);

    /**@brief Name des Remixers oder des Mixes
     *
     * Mit dieser Funktion wird der Name des Remixers oder des Mixes festgelegt.
     * Der Wert wird im Frame "TPE4" gespeichert, sowie in einem benutzerdefinierten
     * "TXXX"-Frame mit dem Namen "TraktorRemixer". Letzterer wird von Traktor
     * von Native Instruments verwendet.
     *
     * @param[in] remixer String mit dem Namen des Remixers oder des Mixes.
     */
    void setRemixer(const String& remixer);

    /**@brief Name des Plattenlabels setzen
     *
     * Mit dieser Funktion wird der Name des Plattenlabels festgelegt,
     * auf dem der Titel erschienen ist.
     * Der Wert wird im Frame "TPUB" gespeichert.
     *
     * @param[in] label String mit dem Namen des Plattenlabels
     */
    void setLabel(const String& label);

    /**@brief Kommentar setzen
     *
     * Mit dieser Funktion wird ein Kommentar zum Titel festgelegt.
     * Der Wert wird im Frame "COMM" gespeichert.
     *
     * @param[in] comment String mit dem Kommentar
     */
    void setComment(const String& comment);

    void setComment(const String& description, const String& comment);

    /**@brief Erscheinungsjahr des Titels setzen
     *
     * Mit dieser Funktion wird das Erscheinungsjahr des Titel festgelegt.
     * Der Wert wird im Frame "TYER" gespeichert.
     *
     * @param[in] year String mit dem Erscheinungsjahr des Titels
     */
    void setYear(const String& year);

    /**@brief Name des Albums setzen
     *
     * Mit dieser Funktion wird der Name des Albums festgelegt, aus dem der Titel stammt.
     * Der Wert wird im Frame "TALB" gespeichert.
     *
     * @param[in] album String mit dem Namen des Albums
     */
    void setAlbum(const String& album);

    /**@brief Nummer des Tracks setzen
     *
     * \desc
     * Mit dieser Funktion wird die Tracknummer des Titels innerhalb des
     * Datenträgers oder Sets gesetzt. Der String muss eine Ziffer enthalten,
     * die die Position des Tracks innerhalb des Datenträgers oder Sets angibt.
     * Optional kann durch Slash getrennt noch die Gesamtzahl der Tracks folgen,
     * also z.B. "4/9".

     * Der Wert wird im Frame "TRCK" gespeichert.
     *
     * @param[in] track String mit der Nummer des Tracks.
     */
    void setTrack(const String& track);
    void setBPM(const String& bpm);
    void setKey(const String& key);
    void setEnergyLevel(const String& energy);

    /**@brief Text in einem Frame setzen
     *
     * Mit dieser Funktion wird das Text-Frame mit dem Namen \p framename
     * auf den Inhalt \p text hinzugefügt. Falls das Frame noch nicht existiert, wird es
     * automatisch angelegt, andernfalls wird der Inhalt des Frames überschrieben.
     *
     * @param framename String mit dem Namen des Frames (z.B. "TIT2" für den Titel)
     * @param text String mit dem Text, der in das Frame geschrieben werden soll
     * @param enc Text-Encoding, das für die Kodierung des Textes verwendet werden soll
     */
    void setTextFrame(const String& framename, const String& text, TextEncoding enc = ENC_UTF16);

    /**@brief Bild eines bestimmten Typs setzen
     *
     * Mit dieser Funktion wird ein Bild eines bestimmten Typs in die Tags eingefügt oder überschrieben.
     *
     * @param type Integer mit dem gewünschten Bild-Typ (z.B. CID3Tag::PIC_COVER_FRONT für das Front-Cover)
     * @param bin ByteArray mit den Binärdaten des Bildes
     * @param MimeType String mit dem MIME-Type des Bildes (z.B. "image/jpeg")
     */
    void setPicture(int type, const ByteArrayPtr& bin, const String& MimeType);

    /**@brief Popularimeter setzen
     *
     * Mit dieser Funktion wird ein Popularimeter-Frame mit der E-Mail-Adresse \p email und der Bewertung \p rating gesetzt.
     *
     * @param email String mit der E-Mail-Adresse des Bewertenden
     * @param rating Bewertung als Zahl von 0 bis 255, wobei 0 die schlechteste und 255 die beste Bewertung darstellt
     */
    void setPopularimeter(const String& email, unsigned char rating);

    /**@brief ID3 Version 2 Tag erstellen
     *
     * Diese Funktion erstellt einen variable langen ID3v2-Tag im Speicher des ByteArray
     * Objekts \p tag. Falls keine Informationen zum Schreiben vorhanden sind, wird ein
     * leeres ByteArray zurückgegeben.
     *
     * @param[out] tag Speicher-Objekt, in dem der Tag gespeichert werden soll
     *
     */
    void generateId3V2Tag(ByteArray& tag) const;

    /**@brief ID3 Version 1 Tag erstellen
     *
     * Diese Funktion erstellt einen 128 Byte langen ID3v1-Tag im Speicher des ByteArray
     * Objekts \p tag. Falls keine Informationen für die Felder des ID3v1-Tags vorhanden sind,
     * wird ein leeres ByteArray zurückgegeben.
     *
     * @param[out] tag Speicher-Objekt, in dem der Tag gespeichert werden soll
     *
     */
    void generateId3V1Tag(ByteArray& tag) const;

    /**@brief Name des Interpreten auslesen
     *
     * Mit dieser Funktion wird der Name des Interpreten aus dem Frame "TPE1"
     * ausgelesen.
     *
     * @return Bei Erfolg wird ein String mit dem Namen des Interpreten zurückgegeben,
     * im Fehlerfall ein leerer String.
     */
    String getArtist() const;

    /**@brief Name des Titels auslesen
     *
     * Mit dieser Funktion wird der Name des Titels aus dem Frame "TIT2"
     * ausgelesen.
     *
     * @return Bei Erfolg wird ein String mit dem Titel des Songs zurückgegeben,
     * im Fehlerfall ein leerer String.
     */
    String getTitle() const;

    /**@brief Genre auslesen
     *
     * Mit dieser Funktion wird der Name des Musikgenres aus dem Frame "TCON"
     * ausgelesen.
     *
     * @note Die Funktion liefert den String zurück, der das Genre spezifiziert,
     * zum Beispiel "Classic Rock". Falls im Tag noch ID's vorangestellt sind,
     * zum Beispiel "(1)Classic Rock", wird die ID übersprungen.
     *
     * @return Bei Erfolg wird ein String mit dem Namen des Genres zurückgegeben,
     * im Fehlerfall ein leerer String.
     */
    String getGenre() const;

    /**@brief Erscheinungsjahr auslesen
     *
     * Mit dieser Funktion wird das Erscheinungsjahr des Titels aus dem Frame "TYER"
     * ausgelesen.
     *
     * @return Bei Erfolg wird ein String mit dem Erscheinungsjahr des Titels zurückgegeben,
     * im Fehlerfall ein leerer String.
     */
    String getYear() const;

    /**@brief Kommentar auslesen
     *
     * Mit dieser Funktion wird der Kommentar des Titels aus dem Frame "COMM"
     * ausgelesen.
     *
     * @return Bei Erfolg wird ein String mit dem Kommentar zurückgegeben,
     * im Fehlerfall ein leerer String.
     */
    String getComment() const;

    /**@brief Kommentar mit einer spezifischen Description auslesen
     *
     * Mit dieser Funktion wird der Kommentar des Titels aus dem Frame "COMM"
     * ausgelesen, deer die Description \p description hat.
     *
     * @return Bei Erfolg wird ein String mit dem Kommentar zurückgegeben,
     * im Fehlerfall ein leerer String.
     */
    String getComment(const String& description) const;

    /**@brief Name des Remixers oder des Mixes auslesen
     *
     * Mit dieser Funktion wird der Names des Remixers oder des Mixes aus dem Frame "TPE4"
     * ausgelesen.
     *
     * @return Bei Erfolg wird ein String mit dem Namen des Remixers oder Mixes zurückgegeben,
     * im Fehlerfall ein leerer String.
     */
    String getRemixer() const;

    /**@brief Name des Plattenlabels auslesen
     *
     * Mit dieser Funktion wird der Name des Plattenlabels aus dem Frame "TPUB"
     * ausgelesen.
     *
     * @return Bei Erfolg wird ein String mit dem Namen des Plattenlabels zurückgegeben,
     * im Fehlerfall ein leerer String.
     */
    String getLabel() const;

    /**@brief Name des Albums auslesen
     *
     * Mit dieser Funktion wird der Names des Albums aus dem Frame "TALB"
     * ausgelesen.
     *
     * @return Bei Erfolg wird ein String mit dem Namen des Albums zurückgegeben,
     * im Fehlerfall ein leerer String.
     */
    String getAlbum() const;

    /**@brief Track-Nummer auslesen
     *
     * Mit dieser Funktion wird die Tracknummer aus dem Frame "TRCK"
     * ausgelesen. Der String enthält eine Ziffer, die die Position des Tracks
     * innerhalb des Datenträgers oder Sets angibt. Optional kann durch Slash getrennt
     * noch die Gesamtzahl der Tracks folgen, also z.B. "4/9".
     *
     * @return Bei Erfolg wird ein String mit der Tracknummer zurückgegeben,
     * im Fehlerfall ein leerer String.
     */
    String getTrack() const;

    /**@brief BPM auslesen
     *
     * Mit dieser Funktion wird die BPM (Beats per Minute) aus dem Frame "TBPM"
     * ausgelesen.
     *
     * @return Bei Erfolg wird ein String mit der BPM zurückgegeben,
     * im Fehlerfall ein leerer String.
     */
    String getBPM() const;

    /**@brief Key auslesen
     *
     * Mit dieser Funktion wird die Key (Tonart) aus dem Frame "TKEY"
     * ausgelesen.
     *
     * @return Bei Erfolg wird ein String mit der Key zurückgegeben,
     * im Fehlerfall ein leerer String.
     */
    String getKey() const;

    /**@brief Energy Level auslesen
     *
     * Mit dieser Funktion wird die Energy Level aus dem Frame "TXXX" mit der Description "EnergyLevel"
     * ausgelesen.
     *
     * @return Bei Erfolg wird ein String mit der Energy Level zurückgegeben,
     * im Fehlerfall ein leerer String.
     */
    String getEnergyLevel() const;

    /**@brief Bild eines bestimmten Typs auslesen
     *
     * Mit dieser Funktion wird das Bild eines bestimmten Typs aus dem Frame "APIC" ausgelesen.
     *
     * @param type Integer mit dem gewünschten Bild-Typ
     * @return Bei Erfolg wird ein ByteArray mit den Binärdaten des Bildes zurückgegeben,
     * im Fehlerfall ein leeres ByteArray.
     */
    ByteArray getPicture(int type) const;

    /**@brief Bild eines bestimmten Typs auslesen
     *
     * Mit dieser Funktion wird das Bild eines bestimmten Typs aus dem Frame "APIC" ausgelesen.
     *
     * @param type Integer mit dem gewünschten Bild-Typ
     * @param bin ByteArray, in dem die Binärdaten des Bildes gespeichert werden sollen
     * @return true, wenn das Bild erfolgreich ausgelesen und im ByteArray gespeichert werden konnte, sonst false
     */
    bool getPicture(int type, ByteArray& bin) const;

    /**@brief Prüft, ob ein Bild eines bestimmten Typs in den Tags enthalten ist
     *
     * Mit dieser Funktion kann geprüft werden, ob ein Bild vom Typ \p type in den Tags
     * vorhanden ist.
     *
     * @param type Integer mit dem gewünschten Bild-Typ
     * @return true oder false
     */
    bool hasPicture(int type) const;

    /**@brief Bild eines bestimmten Typs löschen
     *
     * Mit dieser Funktion wird das Bild eines bestimmten Typs aus den Tags gelöscht.
     *
     * @param type Integer mit dem gewünschten Bild-Typ
     */
    void removePicture(int type);

    /**@brief Benutzerdefinierten Text auslesen
     *
     * Mit dieser Funktion wird der benutzerdefinierte Text mit der Description \p description
     * aus einem TXXX-Frame ausgelesen.
     *
     * @param description String mit der Description des gesuchten benutzerdefinierten Textes
     * @return Bei Erfolg wird ein String mit dem benutzerdefinierten Text zurückgegeben,
     * im Fehlerfall ein leerer String.
     */
    bool getPrivateData(ByteArray& bin, const String& identifier) const;

    /**@brief Benutzerdefinierten Text auslesen
     *
     * Mit dieser Funktion wird der benutzerdefinierte Text mit der Description \p description
     * aus einem TXXX-Frame ausgelesen.
     *
     * @param description String mit der Description des gesuchten benutzerdefinierten Textes
     * @return Bei Erfolg wird ein ByteArrayPtr mit den Binärdaten des benutzerdefinierten Textes zurückgegeben,
     * im Fehlerfall ein leerer ByteArrayPtr.
     */
    ByteArrayPtr getPrivateData(const String& identifier) const;

    /**@brief Popularimeter auslesen
     *
     * Mit dieser Funktion wird die Bewertung eines Popularimeter-Frames mit der E-Mail-Adresse \p email ausgelesen.
     *
     * @param email String mit der E-Mail-Adresse des Bewertenden
     * @return Bei Erfolg wird die Bewertung als Zahl von 0 bis 255 zurückgegeben, im Fehlerfall 0.
     */
    unsigned char getPopularimeter(const String& email) const;

    /**@brief Popularimeter auslesen
     *
     * Mit dieser Funktion wird die Bewertung eines beliebigen Popularimeter-Frames ausgelesen. Falls mehrere Popularimeter-Frames vorhanden
     * sind, wird die Bewertung des ersten gefundenen Frames zurückgegeben.
     *
     * @return Bei Erfolg wird die Bewertung als Zahl von 0 bis 255 zurückgegeben, im Fehlerfall 0.
     */
    unsigned char getPopularimeter() const;

    /**@brief Prüft, ob ein Popularimeter-Frame mit einer bestimmten E-Mail-Adresse in den Tags enthalten ist
     *
     * Mit dieser Funktion kann geprüft werden, ob ein Popularimeter-Frame mit der E-Mail-Adresse \p email in den Tags
     * vorhanden ist.
     *
     * @param email String mit der E-Mail-Adresse des Bewertenden
     * @return true oder false
     */
    bool hasPopularimeter(const String& email) const;

    /**@brief Alle Popularimeter auslesen
     *
     * Mit dieser Funktion werden alle Popularimeter-Frames ausgelesen und die E-Mail-Adressen und Bewertungen in der Map \p data
     * gespeichert.
     *
     * @param data Map, in der die E-Mail-Adressen und Bewertungen der gefundenen Popularimeter-Frames gespeichert werden
     */
    void getAllPopularimeters(std::map<String, unsigned char>& data) const;

    /**@brief Popularimeter löschen
     *
     * Mit dieser Funktion werden alle Popularimeter-Frames gelöscht.
     *
     */
    void removePopularimeter();

    /**@brief Prüft, ob ein oder mehrere Popularimeter-Frames vorhanden sind
     *
     * @return true oder false
     */
    bool hasPopularimeter() const;

    static void copyAndDecodeText(String& s, const ID3Frame* frame, int offset, const ppl7::String& charset);
    static int decode(const ID3Frame* frame, int offset, int encoding, String& target, const ppl7::String& charset);
};

} // end of namespace ppl7

#endif