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

#ifndef PPL7_CORE_DIR_H_
#define PPL7_CORE_DIR_H_

#include <ppl7/types/string.h>
#include <ppl7/types/bytearray.h>
#include <ppl7/types/bytearrayptr.h>
#include <ppl7/types/datetime.h>
#include <ppl7/types/assocarray.h>
#include <ppl7/core/fileobject.h>
#include <ppl7/core/file.h>
#include <vector>

namespace ppl7
{
class DirEntry
{
public:
    DirEntry();
    DirEntry(const DirEntry& other);
    String Filename;
    String Path;
    String File;
    uint64_t Size;
    FileAttr::Attributes Attrib;
    uint32_t Uid;
    uint32_t Gid;
    uint32_t Blocks;
    uint32_t BlockSize;
    uint32_t NumLinks;
    String AttrStr;
    DateTime ATime, CTime, MTime;
    bool isDir();
    bool isFile();
    bool isLink();
    bool isReadable();
    bool isWritable();
    bool isExecutable();
    DirEntry& operator=(const DirEntry& other);
    void toArray(AssocArray& a) const;
    void print(const char* label = NULL);
};

/**@class Dir
 * @ingroup PPLGroupFileIO
 * @brief Klasse zum Durchsuchen von Verzeichnissen
 *
 * Die Klasse Dir wird zum Lesen von Verzeichnissen verwendet. Dazu muss man zunächst mit
 * Dir::open oder Dir::tryOpen ein Verzeichnis öffnen oder das gewünschte Verzeichnis
 * gleich im Konstruktor angeben. Mittels Iteratoren kann man dann die einzelnen Dateien auslesen.
 * Mit Dir::resort kann man jederzeit eine andere Sortierung einstellen.
 *
 *
 */
class Dir
{
public:
    /**@enum class Dir::Sort
     * @brief Sortiermöglichkeiten
     *
     * In dieser Enumeration sind die verschiedenen Sortiermöglichkeiten definiert,
     * die als Parameter der Funktionen Dir::open, Dir::resort und des Konstruktors
     * der Klasse Dir verwendet werden können.
     */
    enum class Sort
    {
        None, /// Keine Sortierung. Die Reihenfolge der Dateien hängt vom Betriebs- und Filesystem ab.

        /** @brief Sortierung nach Dateinamen
         *
         * Es wird eine Sortierung anhand der Dateinamen vorgenommen. Dabei wird Groß- und Kleinschreibung
         * beachtet. Dateien, die mit einem Großbuchstaben beginnen, werden zuerst aufgelistet, danach
         * Dateien mit Kleinbuchstaben.
         */
        Filename,

        /** @brief Sortierung nach Dateinamen, Groß-/Kleinschreibung ignorieren
         *
         * Es wird eine Sortierung anhand der Dateinamen vorgenommen. Dabei wird Groß- und Kleinschreibung
         * ignoriert. Dateien mit Großbuchstaben und Kleinbuchstaben werden vermischt ausgegeben, wobei
         * jedoch die Alphabetische Reihenfolge erhalten bleibt.
         */
        FilenameIgnoreCase,

        /** @brief Sortierung nach Datum des letzten Zugriffs
         *
         * Es wird eine Sortierung nach dem Datum des letzten Zugriffs vorgenommen.
         */
        ATime,

        /** @brief Sortierung nach Datum der letzten Modifikation
         *
         * Es wird eine Sortierung nach dem Datum der letzten Modifikation vorgenommen.
         * Dieser Zeitstempel ändert sich nur bei Neuanlage der Datei oder des Verzeichnisses,
         * oder wenn ein Schreibzugriff stattgefunden hat.
         */
        CTime,

        /** @brief Sortierung nach Datum der letzten Statusänderung
         *
         * Es wird eine Sortierung nach dem Datum der letzten Statusänderung vorgenommen.
         * Eine Statusänderung besteht nicht nur bei Neuanlage und Schreibzugriff, sondern
         * auch bei Änderung der Zugriffsrechte oder Verlinkung.
         */
        MTime,

        /** @brief Sortierung nach Dateigröße
         *
         * Es wird eine Sortierung nach der Größe der Datei vorgenommen.
         */
        Size
    };

    using const_iterator = std::vector<DirEntry>::const_iterator; /// Iterator für die Einträge im Verzeichnis

private:
    std::vector<DirEntry> Files;

    Sort sort;
    String Path;

    void resortMTime();
    void resortCTime();
    void resortATime();
    void resortSize();
    void resortFilename();
    void resortFilenameIgnoreCase();

public:
    PPL7EXCEPTION(PathnameTooLongException, Exception);
    PPL7EXCEPTION(NonexistingPathException, Exception);
    PPL7EXCEPTION(PermissionDeniedException, Exception);
    PPL7EXCEPTION(CreateDirectoryFailedException, Exception);

    /** @brief Default-Konstruktor
     *
     * Mit diesem Konstruktor wird kein Verzeichnis geöffnet. Das Verzeichnis kann später
     * mit Dir::open oder Dir::tryOpen geöffnet werden.
     */
    Dir() = default;

    /** @brief Konstruktor
     *
     * Mit diesem Konstruktor kann ein Verzeichnis geöffnet werden. Dazu muss der Pfad
     * des Verzeichnisses als Parameter angegeben werden. Optional kann auch die gewünschte
     * Sortierreihenfolge angegeben werden. Standardmäßig werden die Dateien unsortiert zurückgegeben.
     * Die Reihenfolge hängt somit im Wesentlichen davon ab, in welcher Reihenfolge
     * die Dateien erstellt wurden, aber auch von Betriebs- und Filesystemabhängigen
     * Vorgängen.
     *
     * \param[in] path Pfad des zu öffnenden Verzeichnisses
     * \param[in] sortOrder Gewünschte Sortierreihenfolge. Siehe dazu Dir::Sort
     * \exception NonexistingPathException Wrd geworfen, wenn das angegebene Verzeichnis nicht existiert.
     * \exception PermissionDeniedException Wird geworfen, wenn das angegebene Verzeichnis nicht geöffnet werden kann.
     */
    explicit Dir(const String& path, Sort sortOrder = Sort::None);

    /*!\brief Verzeichnis einlesen
     *
     * Mit dieser Funktion wird das mit \p path angegebene Verzeichnis geöffnet,
     * eingelesen und mit der Sortiermethode \p sortOrder sortiert.
     *
     * @param[in] path Zu öffnender Pfad
     * @param[in] sortOrder gewünschte Sortierreihenfolge. Defaultmäßig wird keine Sortierung
     * verwendet.
     * @return Die Funktion hat keinen Rückgabewert. Bei Auftreten eines Fehlers wird
     * eine Exception geworfen.
     */
    void open(const String& path, Sort sortOrder = Sort::None);

    /*!\brief Verzeichnis einlesen, ohne Exception
     *
     * Mit dieser Funktion wird das mit \p path angegebene Verzeichnis geöffnet,
     * eingelesen und mit der Sortiermethode \p sortOrder sortiert.
     *
     * @param[in] path Zu öffnender Pfad
     * @param[in] sortOrder gewünschte Sortierreihenfolge. Defaultmäßig wird keine Sortierung
     * verwendet.
     * @return Die Funktion gibt \c true zurück, wenn das Verzeichnis geöffnet werden konnte, sonst \c false.
     */
    bool tryOpen(const String& path, Sort sortOrder = Sort::None);

    /**@brief Sortierung ändern
     *
     * Durch Aufruf dieser Funktion kann die Sortierreihenfolge der bereits eingelesenen
     * Dateien geändert werden. Standardmäßig werden die Dateien unsortiert zurückgegeben.
     * Die Reihenfolge hängt somit im Wesentlichen vom Betriebssystem oder Filesystem ab,
     * oftmals ist dies die Reihenfolge, in der die Dateien erstellt wurden.
     *
     * \param[in] sortOrder Die gewünschte Sortierreihenfolge. Siehe dazu auch die Enumeration Dir::Sort
     */
    void resort(Sort sortOrder) noexcept;

    /**
     * @brief Anzahl Dateien
     *
     * Diese Funktion liefert die Anzahl Einträge im geöffneten Verzeichnis zurück. Sie
     * gibt daher erst nach Aufruf von Dir::open einen korrekten Wert zurück. Einträge können
     * nicht nur Dateien sein, sondern auch Verzeichnisse und Symlinks.
     *
     * @return Anzahl Einträge im geöffneten Verzeichnis
     */
    size_t size() const noexcept
    {
        return Files.size();
    }

    /**
     * @brief Gibt True zurück, wenn das geöffnete Verzeichnis keine Einträge enthält
     */
    bool empty() const noexcept
    {
        return Files.empty();
    }

    /**
     * @brief Iterator auf den ersten Eintrag im Verzeichnis
     */
    const_iterator begin() const noexcept
    {
        return Files.begin();
    }

    /**
     * @brief Iterator auf den letzten Eintrag im Verzeichnis
     */
    const_iterator end() const noexcept
    {
        return Files.end();
    }

    /**
     * @brief Iterator auf den ersten Eintrag im Verzeichnis
     */
    const_iterator cbegin() const noexcept
    {
        return Files.cbegin();
    }

    /**
     * @brief Iterator auf den letzten Eintrag im Verzeichnis
     */
    const_iterator cend() const noexcept
    {
        return Files.cend();
    }

    /**
     * @brief Zugriff auf einen Eintrag im Verzeichnis
     *
     * Mit dieser Funktion kann auf einen Eintrag im Verzeichnis zugegriffen werden.
     * Der Index muss kleiner als Dir::size() sein, sonst wird eine Exception geworfen.
     *
     * @param index Index des gewünschten Eintrags
     * @return Referenz auf den gewünschten Eintrag
     */
    const DirEntry& operator[](size_t index) const
    {
        return Files[index];
    }

    /**
     * @brief Zugriff auf einen Eintrag im Verzeichnis
     *
     * Mit dieser Funktion kann auf einen Eintrag im Verzeichnis zugegriffen werden.
     * Der Index muss kleiner als Dir::size() sein, sonst wird eine Exception geworfen.
     *
     * @param index Index des gewünschten Eintrags
     * @return Referenz auf den gewünschten Eintrag
     */
    const DirEntry& at(size_t index) const
    {
        return Files.at(index);
    }

    /**@brief Verzeichnis auf STDOUT ausgeben
     *
     * \desc
     * Mit dieser Funktion wird das mit Dir::open oder im Konstruktor ausgewählte Verzeichnis
     * auf STDOUT ausgegeben. Die Ausgabe ist ähnlich der des "ls"-Befehls unter Unix, enthält jedoch
     * nicht die Benutzerrechte. Die Funktion wurde hauptsächlich zu Debuggingzwecken
     * eingebaut.
     */
    void print() const;

    /// @name Filterfunktionen
    // @{

    /**
     * @brief Liefert alle Einträge zurück, die zu einem Wildcard-Muster (z.B. "*.txt") passen
     *
     * @param pattern Wildcard-Muster, wie es auch beim Unix-Befehl "ls" oder mit "dir" unter Windows angegeben werden kann. Dabei sind die
     * Wildcards "*" und "?" erlaubt. Das Sternchen "*" steht dabei für beliebig viele Zeichen, das Fragezeichen "?" für ein einzelnes.
     * @param ignorecase Wird diese Variable auf "true" gesetzt, wird Groß- und Kleinschreibung ignoriert. Wird als Pattern beispielsweise
     * "*.TXT" angegeben, würde auch "*.txt" passen. Der Default ist "false".
     * @return std::vector<DirEntry> mit allen Einträgen, die zu dem angegebenen Muster passen
     */
    std::vector<DirEntry> filterPattern(const String& pattern, bool ignorecase = false) const;

    /**
     * @brief Liefert alle Einträge zurück, die zu einer Regular Expression passen
     *
     * @param regexp Regular Expression, wie sie auch in der C++ Standardbibliothek verwendet wird. Siehe dazu auch std::regex.
     * @return std::vector<DirEntry> mit allen Einträgen, die zu der angegebenen Regular Expression passen
     */
    std::vector<DirEntry> filterRegExp(const String& regexp) const;

    /**
     * @brief Liefert alle Einträge zurück, für die das Prädikat true zurückgibt
     *
     * @example
     * @code{.cpp}
     * // Nur lesbare Dateien finden, die größer als 1 MB sind
     * auto largeFiles = dir.filter([](const DirEntry& e) {
     *     return e.isFile() && e.Size > 1024 * 1024;
     *});
     * @endcode
     */
    template <typename Predicate> std::vector<DirEntry> filter(Predicate pred) const
    {
        std::vector<DirEntry> result;
        for (const auto& entry : Files) {
            if (pred(entry)) {
                result.push_back(entry);
            }
        }
        return result;
    }

    /**
     * @brief Sucht den ersten Eintrag, der zum Pattern passt
     * @return std::optional<DirEntry> mit dem Treffer oder std::nullopt
     *
     * @example
     * @code{.cpp}
     * auto entry = dir.findPattern("*.txt");
     * // oder: std::optional<DirEntry> file = dir.findPattern("*.txt");
     * if (entry) {
     *    // Zugriff mit * oder -> (genau wie bei einem Pointer)
     *    std::cout << "Found: " << entry->Filename << std::endl;
     * } else {
     *    std::cout << "No match found" << std::endl;}
     * }
     * @endcode
     */
    std::optional<DirEntry> findPattern(const String& pattern, bool ignorecase = false) const;
    std::optional<DirEntry> findRegExp(const String& regexp) const;

    // @}

    /// @name Statische Funktionen
    // @{

    /**
     * @brief Liefert den aktuellen Arbeitsordner zurück
     *
     * Mit dieser Funktion kann der aktuelle Arbeitsordner ermittelt werden. Der aktuelle Arbeitsordner
     * ist der Ordner, in dem das Programm gestartet wurde. Unter Windows ist dies der Ordner, in dem die
     * EXE-Datei liegt.
     *
     * @return String mit dem aktuellen Arbeitsordner
     */
    static String currentPath();

    /**
     * @brief Liefert den Home-Ordner des aktuellen Benutzers zurück
     *
     * Mit dieser Funktion kann der Home-Ordner des aktuellen Benutzers ermittelt werden. Unter Windows ist dies
     * der Ordner "C:\Users\<Benutzername>", unter Linux und FreeBSD ist dies "/home/<Benutzername>".
     *
     * @return String mit dem Home-Ordner des aktuellen Benutzers
     */
    static String homePath();

    /**
     * @brief Liefert den Pfad zum temporären Ordner zurück
     *
     * Mit dieser Funktion kann der Pfad zum temporären Ordner ermittelt werden. Unter Windows ist dies
     * der Ordner "C:\Users\<Benutzername>\AppData\Local\Temp", unter Linux und FreeBSD ist dies "/tmp".
     *
     * @return String mit dem Pfad zum temporären Ordner
     */
    static String tempPath();

    /**
     * @brief Liefert den Pfad zum Anwendungsdaten-Ordner zurück
     *
     * Mit dieser Funktion kann der Pfad zum Anwendungsdaten-Ordner ermittelt werden. Unter Windows ist dies
     * der Ordner "C:\Users\<Benutzername>\AppData\Roaming", unter Linux und FreeBSD ist dies "/home/<Benutzername>/.local/share".
     *
     * @return String mit dem Pfad zum Anwendungsdaten-Ordner
     */
    static String applicationDataPath();

    /**
     * @brief Liefert den Pfad zum Anwendungsdaten-Ordner zurück
     *
     * Mit dieser Funktion kann der Pfad zum Anwendungsdaten-Ordner ermittelt werden. Unter Windows ist dies
     * der Ordner "C:\Users\<Benutzername>\AppData\Roaming\<company>\<application>", unter Linux und FreeBSD ist dies
     * "/home/<Benutzername>/.local/share/<company>/<application>".
     *
     * @param company Name des Unternehmens oder Entwicklers
     * @param application Name der Anwendung
     * @return String mit dem Pfad zum Anwendungsdaten-Ordner
     */
    static String applicationDataPath(const String& company, const String& application);

    /**
     * @brief Liefert den Pfad zum Dokumente-Ordner des aktuellen Benutzers zurück
     *
     * Mit dieser Funktion kann der Pfad zum Dokumente-Ordner des aktuellen Benutzers ermittelt werden. Unter Windows ist dies
     * der Ordner "C:\Users\<Benutzername>\Documents", unter Linux und FreeBSD ist dies "/home/<Benutzername>/Documents".
     *
     * @return String mit dem Pfad zum Dokumente-Ordner des aktuellen Benutzers
     */
    static String documentsPath();

    /**
     * @brief Liefert den Pfad zum Dokumente-Ordner des aktuellen Benutzers zurück
     *
     * Mit dieser Funktion kann der Pfad zum Dokumente-Ordner des aktuellen Benutzers ermittelt werden. Unter Windows ist dies
     * der Ordner "C:\Users\<Benutzername>\Documents\<company>\<application>", unter Linux und FreeBSD ist dies
     * "/home/<Benutzername>/Documents/<company>/<application>".
     *
     * @param company Name des Unternehmens oder Entwicklers
     * @param application Name der Anwendung
     * @return String mit dem Pfad zum Dokumente-Ordner des aktuellen Benutzers
     */
    static String documentsPath(const String& company, const String& application);

    /**
     * @brief Prüft, ob ein Verzeichnis existiert
     *
     * Mit dieser Funktion kann geprüft werden, ob ein Verzeichnis existiert. Dabei wird nicht geprüft,
     * ob es sich um ein Verzeichnis oder eine Datei handelt. Es wird lediglich geprüft, ob der Pfad
     * existiert.
     *
     * @param dirname Pfad des zu prüfenden Verzeichnisses
     * @return Liefert \c true zurück, wenn das Verzeichnis existiert, sonst \c false.
     */
    static bool exists(const String& dirname);

    /**
     * @brief Prüft, ob ein Verzeichnis geöffnet werden kann
     *
     * Mit dieser Funktion kann geprüft werden, ob ein Verzeichnis geöffnet werden kann. Dabei wird geprüft,
     * ob der Pfad existiert und ob das Programm die notwendigen Rechte hat, um das Verzeichnis zu öffnen.
     *
     * @param path Pfad des zu prüfenden Verzeichnisses
     * @return Liefert \c true zurück, wenn das Verzeichnis geöffnet werden kann, sonst \c false.
     */
    static bool canOpen(const String& path);

    /**
     * @brief Erstellt ein Verzeichnis
     *
     * Mit dieser Funktion kann ein Verzeichnis erstellt werden. Dabei wird geprüft, ob der Pfad existiert
     * und ob das Programm die notwendigen Rechte hat, um das Verzeichnis zu erstellen.
     *
     * @param path Pfad des zu erstellenden Verzeichnisses
     * @param recursive Wenn \c true ist, werden auch alle übergeordneten Verzeichnisse erstellt, falls diese nicht existieren.
     * @return Liefert \c true zurück, wenn das Verzeichnis erstellt wurde, sonst \c false.
     */
    static void mkDir(const String& path, bool recursive = false);

    /**
     * @brief Erstellt ein Verzeichnis mit bestimmten Rechten
     *
     * Mit dieser Funktion kann ein Verzeichnis erstellt werden. Dabei wird geprüft, ob der Pfad existiert
     * und ob das Programm die notwendigen Rechte hat, um das Verzeichnis zu erstellen.
     *
     * @param path Pfad des zu erstellenden Verzeichnisses
     * @param mode Rechte des zu erstellenden Verzeichnisses (z.B. 0755)
     * @param recursive Wenn \c true ist, werden auch alle übergeordneten Verzeichnisse erstellt, falls diese nicht existieren.
     * @return Liefert \c true zurück, wenn das Verzeichnis erstellt wurde, sonst \c false.
     */
    static void mkDir(const String& path, mode_t mode, bool recursive);

    // @}
};

} // namespace ppl7

#endif /* PPL7_CORE_DIR_H_ */