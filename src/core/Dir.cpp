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

#include <filesystem>

#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#include <pwd.h>
#endif

#include <ppl7/core/dir.h>
#include <ppl7/exceptions.h>

namespace ppl7
{

/*!\enum Dir::Sort
 * \brief Sortiermöglichkeiten
 *
 * In dieser Enumeration sind die verschiedenen Sortiermöglichkeiten definiert,
 * die als Parameter der Funktionen Dir::open, Dir::resort und des Konstruktors
 * der Klasse Dir verwendet werden können.
 */

/*!\var Dir::Sort Dir::SORT_NONE
 * Keine Sortierung. Die Reihenfolge der Dateien hängt vom Betriebs- und Filesystem ab.
 */

/*!\var Dir::Sort Dir::SORT_FILENAME
 * Es wird eine Sortierung anhand der Dateinamen vorgenommen. Dabei wird Groß- und Kleinschreibung
 * beachtet. Dateien, die mit einem Großbuchstaben beginnen, werden zuerst aufgelistet, danach
 * Dateien mit Kleinbuchstaben.
 */

/*!\var Dir::Sort Dir::SORT_FILENAME_IGNORCASE
 * Es wird eine Sortierung anhand der Dateinamen vorgenommen. Dabei wird Groß- und Kleinschreibung
 * ignoriert. Dateien mit Großbuchstaben und Kleinbuchstaben werden vermischt ausgegeben, wobei
 * jedoch die Alphabetische Reihenfolge erhalten bleibt.
 */

/*!\var Dir::Sort Dir::SORT_ATIME
 * Es wird eine Sortierung nach dem Datum des letzten Zugriffs vorgenommen.
 */

/*!\var Dir::Sort Dir::SORT_MTIME
 * Es wird eine Sortierung nach dem Datum der letzten Modifikation vorgenommen.
 * Dieser Zeitstempel ändert sich nur bei Neuanlage der Datei oder des Verzeichnisses,
 * oder wenn ein Schreibzugriff stattgefunden hat.
 */

/*!\var Dir::Sort Dir::SORT_CTIME
 * Es wird eine Sortierung nach dem Datum der letzten Statusänderung vorgenommen.
 * Eine Statusänderung besteht nicht nur bei Neuanlage und Schreibzugriff, sondern
 * auch bei Änderung der Zugriffsrechte oder Verlinkung.
 */

/*!\var Dir::Sort Dir::SORT_SIZE
 * Es wird eine Sortierung nach der Größe der Datei vorgenommen.
 */

/*!\typedef ppl7::List<const DirEntry*>::Iterator Dir::Iterator;
 * \brief Iterator zum Durchwandern der Verzeichnisliste
 *
 * \desc
 * Dieser Iterator wird benötigt, wenn man die Verzeichnisliste mit Dir::getNext oder
 * den verwandten Befehlen durchwandern will.
 * \example
\code
    ppl7::String Home=ppl7::Dir::homePath();
    ppl7::Dir d(Home, ppl7::Dir::SORT_FILENAME);
    // Iterator zum Durchwandern des Verzeichnisses anlegen
    ppl7::Dir::Iterator it;
    d.reset(it);
    ppl7::DirEntry e;
    while (d.getNext(e,it)) {
        std::cout << "Datei: " << e.Filename << "\n";
    }
\endcode
 */

/*!\var ppl7::List<DirEntry> ppl7::Dir::Files
 * \brief Interne Liste mit den eingelesenen, unsortierten Verzeichniseinträgen
 *
 * \desc
 * Interne Liste mit den eingelesenen, unsortierten Verzeichniseinträgen
 *
 */

/*!\var ppl7::Dir::SortedFiles
 * \brief Interne sortierte Liste mit Pointern auf die Verzeichniseinträgen
 *
 * \desc
 * Interne sortierte Liste mit Pointern auf die Verzeichniseinträgen
 */

/*!\var ppl7::Dir::sort
 * \brief Aktuelle Sortiermethode
 *
 * \desc
 * Aktuelle Sortiermethode
 */

/*!\var ppl7::Dir::Path
 * \brief Pfad des aktuell geöffneten Verzeichnisses
 *
 * \desc
 * Pfad des aktuell geöffneten Verzeichnisses
 */

/*!\ingroup PPLGroupFileIO
 * \brief Aktuelles Verzeichnis
 *
 * \desc
 * Diese statische Funktion liefert das aktuelle Verzeichnis zurück.
 *
 * \return String mit dem aktuellen Verzeichnis
 */
String Dir::currentPath()
{
    std::error_code ec;
    auto path = std::filesystem::current_path(ec);
    if (!ec) {
        return String(path.string());
    }

    // Gezielte Fehlerbehandlung über std::error_code
    if (ec == std::errc::no_such_file_or_directory) {
        throw NonexistingPathException();
    } else if (ec == std::errc::permission_denied) {
        throw PermissionDeniedException();
    }
    throw UnknownException("Dir::currentPath failed: %s", ec.message().c_str());
}

/*!\ingroup PPLGroupFileIO
 * \brief Homeverzeichnis des aktuellen Users
 *
 * \desc
 * Diese statische Funktion liefert das Homeverzeichnis des aktuellen Benutzers
 * zurück.
 *
 * \return String mit dem Verzeichnis
 * \exception UnsupportedFeatureException Wird geworfen, wenn das Homeverzeichnis
 * nicht ermittelt werden kann.
 */
String Dir::homePath()
{
#ifdef _WIN32
    // 1. Bevorzugt USERPROFILE unter Windows
    if (const char* userProfile = getenv("USERPROFILE"); userProfile && strlen(userProfile) > 0) {
        return String(userProfile);
    }
    // 2. Fallback: HOMEDRIVE + HOMEPATH
    const char* homeDrive = getenv("HOMEDRIVE");
    const char* homePath = getenv("HOMEPATH");
    if (homeDrive && homePath) {
        String ret;
        ret.setf("%s%s", homeDrive, homePath);
        return ret;
    }
#else
    // 1. Umgebungsvariable HOME auslesen
    if (const char* home = getenv("HOME"); home && strlen(home) > 0) {
        return String(home);
    }
    // 2. Fallback: System-Userdatenbank (/etc/passwd) abfragen
    if (struct passwd* pw = getpwuid(getuid()); pw && pw->pw_dir) {
        return String(pw->pw_dir);
    }
#endif

    throw UnsupportedFeatureException("Dir::homePath: Could not determine user home directory");
}

/*!\ingroup PPLGroupFileIO
 * \brief Verzeichnis für temporäre Dateien
 *
 * \desc
 * Diese statische Funktion liefert das Verzeichnis zurück, in dem
 * temporäre Dateien abgelegt werden können.
 *
 * \return String mit dem Verzeichnis
 */
String Dir::tempPath()
{
    std::error_code ec;
    auto path = std::filesystem::temp_directory_path(ec);
    if (!ec) {
        return String(path.string());
    }

    // Fallback für den unwahrscheinlichen Fall eines Fehlers:
#ifdef _WIN32
    if (const char* tmp = getenv("TEMP")) return String(tmp);
    if (const char* tmp = getenv("TMP")) return String(tmp);
    return String("C:\\Windows\\Temp");
#else
    if (const char* tmp = getenv("TMPDIR")) return String(tmp);
    return String("/tmp");
#endif
}
String Dir::applicationDataPath()
{
    String path;
#ifdef WIN32
    wchar_t* p = _wgetenv(L"LOCALAPPDATA");
    if (!p) throw KeyNotFoundException("LOCALAPPDATA");
    WideString wpath(p);
    path = String(wpath);
#else
    path = homePath() + "/.config";
#endif
    path.replace("//", "/");
#ifdef WIN32
    path.replace("/", "\\");
#endif
    return path;
}

String Dir::applicationDataPath(const String& company, const String& application)
{
    String path = Dir::applicationDataPath();
    path += "/" + company + "/" + application;
    path.replace("//", "/");
#ifdef WIN32
    path.replace("/", "\\");
#endif
    return path;
}

String Dir::documentsPath()
{
    String path;
#ifdef WIN32
    wchar_t* buffer = (wchar_t*)malloc(MAX_PATH * sizeof(wchar_t));
    HRESULT result = SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, buffer);
    if (result != S_OK) {
        free(buffer);
        throw KeyNotFoundException("CSIDL_PERSONAL");
    }
    WideString wpath(buffer);
    path = String(wpath);
    free(buffer);
#else
    path = Dir::homePath() + "/Documents";
#endif
    return path;
}

String Dir::documentsPath(const String& company, const String& application)
{
    String path = Dir::documentsPath();
    path += "/" + company + "/" + application;
#ifdef WIN32
    path.replace("/", "\\");
#else
    path.replace("//", "/");
#endif
    return path;
}

Dir::Dir()
{
    sort = Sort::None;
}

Dir::Dir(const String& path, Sort sortOrder)
{
    sort = s;
    if (path.notEmpty()) open(path, sortOrder);
}

/*!\brief Verzeichnis-Eintrag auf STDOUT ausgeben
 *
 * \desc
 * Mit dieser Funktion kann ein Verzeichniseintrag auf STDOUT ausgegeben werden.
 * Die Ausgabe ist ähnlich der des "ls"-Befehls unter Unix, enthält jedoch
 * nicht die Benutzerrechte. Die Funktion wurde hauptsächlich zu Debuggingzwecken
 * eingebaut.
 *
 * \param[in] de Referenz auf einen Verzeichniseintrag
 */
void Dir::print(const DirEntry& de) const
{
    printf("%s %3u ", (const char*)de.AttrStr, de.NumLinks);
    printf("%5u %5u ", de.Uid, de.Gid);
    printf("%10llu ", (unsigned long long)de.Size);
    printf("%s %s\n", (const char*)de.MTime.get(), (const char*)de.Filename);
}

void Dir::print() const
{
    ppl7::List<const DirEntry*>::Iterator it;
    printf("Directory Listing: %s\n", (const char*)Path);
    printf("Total Files: %zu\n", num());
    SortedFiles.reset(it);
    while (SortedFiles.getNext(it)) {
        printf("%s %3u ", (const char*)de.AttrStr, de.NumLinks);
        printf("%5u %5u ", de.Uid, de.Gid);
        printf("%10llu ", (unsigned long long)de.Size);
        printf("%s %s\n", (const char*)de.MTime.get(), (const char*)de.Filename);
    }
}

void Dir::resort(Sort s)
{
    SortedFiles.clear();
    switch (s) {
    case SORT_NONE:
        resortNone();
        break;
    case SORT_FILENAME:
        resortFilename();
        break;
    case SORT_FILENAME_IGNORCASE:
        resortFilenameIgnoreCase();
        break;
    case SORT_ATIME:
        resortATime();
        break;
    case SORT_CTIME:
        resortCTime();
        break;
    case SORT_MTIME:
        resortMTime();
        break;
    case SORT_SIZE:
        resortSize();
        break;
    default:
        throw IllegalArgumentException();
    }
    sort = s;
}

/*!\brief Dateien nach Dateiname sortieren
 *
 * \desc
 * Diese interne Funktion sortiert das durch Dir::open eingescannte Verzeichnis
 * nach Dateiname, unter Beachtung von Gross-/Kleinschreibung.
 * Die Funktion wird
 * von Dir::resort in Abhängigkeit des eingestellten Sortieralgorithmus aufgerufen.
 */
void Dir::resortFilename()
{
    ppl7::List<DirEntry>::Iterator it;
    Files.reset(it);

    std::multimap<String, const DirEntry*> sorter;
    while (Files.getNext(it)) {
        const DirEntry& de = it.value();
        sorter.insert(std::pair<String, const DirEntry*>(de.Filename, &de));
    }
    std::multimap<String, const DirEntry*>::const_iterator sortit;
    for (sortit = sorter.begin(); sortit != sorter.end(); ++sortit) {
        SortedFiles.add((*sortit).second);
    }
}

/*!\brief Dateien nach Dateiname sortieren, Gross-/Kleinschreibung wird ignoriert
 *
 * \desc
 * Diese interne Funktion sortiert das durch Dir::open eingescannte Verzeichnis
 * nach Dateiname, wobei Gross-/Kleinschreibung ignoriert wird.
 * Die Funktion wird
 * von Dir::resort in Abhängigkeit des eingestellten Sortieralgorithmus aufgerufen.
 */
void Dir::resortFilenameIgnoreCase()
{
    ppl7::List<DirEntry>::Iterator it;
    Files.reset(it);

    std::multimap<String, const DirEntry*> sorter;
    String filename;
    while (Files.getNext(it)) {
        const DirEntry& de = it.value();
        filename.set(de.Filename);
        filename.lowerCase();
        sorter.insert(std::pair<String, const DirEntry*>(filename, &de));
    }
    std::multimap<String, const DirEntry*>::const_iterator sortit;
    for (sortit = sorter.begin(); sortit != sorter.end(); ++sortit) {
        SortedFiles.add((*sortit).second);
    }
}

/*!\brief Dateien nach Modifizierungsdatum sortieren
 *
 * \desc
 * Diese interne Funktion sortiert das durch Dir::open eingescannte Verzeichnis
 * nach dem Modifikations-Zeitstempel der Dateien. Dieser Zeitstempel ändert sich
 * nur bei Neuanlage der Datei oder des Verzeichnisses, oder wenn ein Schreibzugriff
 * stattgefunden hat.
 * Falls mehrere Dateien den gleichen Zeitstempel haben, ist deren Reihenfolge unbestimmt.
 * \par
 * Die Funktion wird
 * von Dir::resort in Abhängigkeit des eingestellten Sortieralgorithmus aufgerufen.
 */
void Dir::resortMTime()
{
    ppl7::List<DirEntry>::Iterator it;
    Files.reset(it);

    std::multimap<DateTime, const DirEntry*> sorter;
    while (Files.getNext(it)) {
        const DirEntry& de = it.value();
        sorter.insert(std::pair<DateTime, const DirEntry*>(de.MTime, &de));
    }
    std::multimap<DateTime, const DirEntry*>::const_iterator sortit;
    for (sortit = sorter.begin(); sortit != sorter.end(); ++sortit) {
        SortedFiles.add((*sortit).second);
    }
}

/*!\brief Dateien nach Datum der letzten Statusänderung sortieren
 *
 * \desc
 * Diese interne Funktion sortiert das durch Dir::open eingescannte Verzeichnis
 * nach dem Zeitstempel der letzten Statusänderung der Dateien. Eine Statusänderung
 * besteht nicht nur bei Neuanlage und Schreibzugriff, sondern auch bei Änderung
 * der Zugriffsrechte oder Verlinkung.
 * Falls mehrere Dateien den gleichen Zeitstempel haben, ist deren Reihenfolge unbestimmt.
 * \par
 * Die Funktion wird
 * von Dir::resort in Abhängigkeit des eingestellten Sortieralgorithmus aufgerufen.
 */
void Dir::resortCTime()
{
    ppl7::List<DirEntry>::Iterator it;
    Files.reset(it);

    std::multimap<DateTime, const DirEntry*> sorter;
    while (Files.getNext(it)) {
        const DirEntry& de = it.value();
        sorter.insert(std::pair<DateTime, const DirEntry*>(de.CTime, &de));
    }
    std::multimap<DateTime, const DirEntry*>::const_iterator sortit;
    for (sortit = sorter.begin(); sortit != sorter.end(); ++sortit) {
        SortedFiles.add((*sortit).second);
    }
}

/*!\brief Dateien nach Datum des letzten Zugriffs sortieren
 *
 * \desc
 * Diese interne Funktion sortiert das durch Dir::open eingescannte Verzeichnis
 * nach dem Zeitstempel des letzten Zugriffs auf die Datei.
 * Falls mehrere Dateien den gleichen Zeitstempel haben, ist deren Reihenfolge unbestimmt.
 * \par
 * Die Funktion wird
 * von Dir::resort in Abhängigkeit des eingestellten Sortieralgorithmus aufgerufen.
 */
void Dir::resortATime()
{
    ppl7::List<DirEntry>::Iterator it;
    Files.reset(it);

    std::multimap<DateTime, const DirEntry*> sorter;
    while (Files.getNext(it)) {
        const DirEntry& de = it.value();
        sorter.insert(std::pair<DateTime, const DirEntry*>(de.ATime, &de));
    }
    std::multimap<DateTime, const DirEntry*>::const_iterator sortit;
    for (sortit = sorter.begin(); sortit != sorter.end(); ++sortit) {
        SortedFiles.add((*sortit).second);
    }
}

/*!\brief Dateien nach Dateigröße sortieren
 *
 * \desc
 * Diese interne Funktion sortiert das durch Dir::open eingescannte Verzeichnis
 * nach der Größe der Dateien. Falls mehrere Dateien mit gleicher Größe vorhanden
 * sind, ist deren Reihenfolge unbestimmt.
 * \par
 * Die Funktion wird
 * von Dir::resort in Abhängigkeit des eingestellten Sortieralgorithmus aufgerufen.
 */
void Dir::resortSize()
{
    ppl7::List<DirEntry>::Iterator it;
    Files.reset(it);

    std::multimap<uint64_t, const DirEntry*> sorter;
    while (Files.getNext(it)) {
        const DirEntry& de = it.value();
        sorter.insert(std::pair<uint64_t, const DirEntry*>(de.Size, &de));
    }
    std::multimap<uint64_t, const DirEntry*>::const_iterator sortit;
    for (sortit = sorter.begin(); sortit != sorter.end(); ++sortit) {
        SortedFiles.add((*sortit).second);
    }
}

/*!\brief Zeiger auf den ersten Eintrag des Verzeichnisses
 *
 * \desc
 * Mit dieser Funktion wird der Zeiger des Iterators \p it auf den ersten Eintrag
 * im Verzeichnis gesetzt. Der nächste Aufruf von einer der "getNext..."-Funktionen
 * würde somit den ersten Eintrag zurückliefern.
 * \param it Iterator vom Typ ppl7::Dir::Iterator
 */
void Dir::reset(Iterator& it) const
{
    SortedFiles.reset(it);
}

/*!\brief Erster Verzeichniseintrag
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der Iterator \p it auf den ersten Verzeichniseintrag gesetzt und
 * dessen Daten zurückgegeben. Alle weiteren Einträge können mit Dir::getNext ausgelesen werden.
 * \par
 * Die Reihenfolge der durch Dir::getFirst und Dir::getNext zurückgelieferten Dateien hängt von
 * der eingestellten Sortierung ab. Siehe dazu Dir::resort und Dir::open
 * \param it Iterator vom Typ ppl7::Dir::Iterator
 * @return Referenz auf die erste Datei des Verzeichnisses.
 * \exception EndOfListException Wird geworfen, wenn keine Einträge im geöffneten Verzeichnis vorhanden sind.
 */
const DirEntry& Dir::getFirst(Iterator& it) const
{
    if (SortedFiles.getFirst(it)) return *it.value();
    throw EndOfListException();
}

/*!\brief Nächster Verzeichniseintrag
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der Iterator \p it auf den nächsten Verzeichniseintrag gesetzt und
 * dessen Daten zurückgegeben.
 * \par
 * Die Reihenfolge der durch Dir::getFirst und Dir::getNext zurückgelieferten Dateien hängt von
 * der eingestellten Sortierung ab. Siehe dazu Dir::resort und Dir::open
 *
 * \param it Iterator vom Typ ppl7::Dir::Iterator
 * @return Referenz auf die nächste Datei des Verzeichnisses.
 * \exception EndOfListException Wird geworfen, wenn das Ende der Liste erreicht wurde.
 */
const DirEntry& Dir::getNext(Iterator& it) const
{
    if (SortedFiles.getNext(it)) return *it.value();
    throw EndOfListException();
}

/*!\brief Erster Verzeichniseintrag, der zu einem bestimmten Muster passt
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der Iterator \p it auf die erste Datei gesetzt, die
 * zu dem angegebenen Muster \p pattern passt. Die Daten der Datei werden als Referenz zurückgegeben.
 * Alle weiteren Dateien können mit Dir::getNextPattern ausgelesen werden.
 * \par
 * Die Reihenfolge der durch Dir::getFirstPattern und Dir::getNextPattern zurückgelieferten Dateien hängt von
 * der eingestellten Sortierung ab. Siehe dazu Dir::resort, Dir::open
 *
 * \param[in] it Iterator vom Typ ppl7::Dir::Iterator
 * \param[in] pattern Ein beliebiges Suchpattern, wie es auch beim Unix-Befehl "ls" oder mit
 * "dir" unter Windows angegeben werden kann. Dabei sind die Wildcards "*" und "?" erlaubt.
 * Das Sternchen "*" steht dabei für beliebig viele Zeichen, das Fragezeichen "?" für ein einzelnes.
 * \param[in] ignorecase Wird diese Variable auf "true" gesetzt, wird Groß- und Kleinschreibung
 * ignoriert. Wird als Pattern beispielsweise "*.TXT" angegeben, würde auch "*.txt" passen.
 * Der Default ist "false".
 *
 * @return Referenz auf die erste Datei des Verzeichnisses.
 * \exception EndOfListException Wird geworfen, wenn keine Einträge im geöffneten Verzeichnis vorhanden sind.
 */
const DirEntry& Dir::getFirstPattern(Iterator& it, const String& pattern, bool ignorecase) const
{
    reset(it);
    return getNextPattern(it, pattern, ignorecase);
}

/*!\brief Nächster Verzeichniseintrag, der zu einem bestimmten Muster passt
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der Iterator \p it auf den nächsten Verzeichniseintrag gesetzt,
 * der  zu dem angegebenen Muster \p pattern passt, und dessen Daten zurückgegeben.
 * \par
 * Die Reihenfolge der durch Dir::getFirstPattern und Dir::getNextPattern zurückgelieferten Dateien hängt von
 * der eingestellten Sortierung ab. Siehe dazu Dir::resort, Dir::open
 *
 * \param[in] it Iterator vom Typ ppl7::Dir::Iterator
 * \param[in] pattern Ein beliebiges Suchpattern, wie es auch beim Unix-Befehl "ls" oder mit
 * "dir" unter Windows angegeben werden kann. Dabei sind die Wildcards "*" und "?" erlaubt.
 * Das Sternchen "*" steht dabei für beliebig viele Zeichen, das Fragezeichen "?" für ein einzelnes.
 * \param[in] ignorecase Wird diese Variable auf "true" gesetzt, wird Groß- und Kleinschreibung
 * ignoriert. Wird als Pattern beispielsweise "*.TXT" angegeben, würde auch "*.txt" passen.
 * Der Default ist "false"
 *
 * @return Referenz auf die nächste Datei des Verzeichnisses.
 * \exception EndOfListException Wird geworfen, wenn das Ende der Liste erreicht wurde.
 */
const DirEntry& Dir::getNextPattern(Iterator& it, const String& pattern, bool ignorecase) const
{
    String Pattern;
    Pattern = RegEx::escape(pattern);
    // printf ("Pattern: %ls\n",(const wchar_t*)Pattern);
    // printf ("Pattern: %ls\n",(const wchar_t*)Pattern);
    Pattern.replace(".", "\\.");
    Pattern.replace("\\*", ".*");
    Pattern.replace("\\?", ".");
    Pattern = "^" + Pattern;
    Pattern += "$";
    int flags = RegEx::Flags::DOTALL;
    if (ignorecase) flags |= RegEx::Flags::CASELESS;
    RegEx::Pattern regex = RegEx::compile(Pattern, flags);

    while (SortedFiles.getNext(it)) {
        const DirEntry* de = it.value();
        // Patternmatch
        // printf ("Match gegen: %s\n",(const char*)de->Filename);
        if (RegEx::match(pattern, de->Filename)) return *de;
    }
    throw EndOfListException();
}

/*!\brief Erster Verzeichniseintrag, der zu der angegebenen Regular Expression passt
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der Iterator \p it auf die erste Datei gesetzt, die
 * zu der angegebenen Regular Expression \p regexp passt. Die Daten der Datei werden als Referenz zurückgegeben.
 * Alle weiteren Dateien können mit Dir::getNextRegExp ausgelesen werden.
 * \par
 * Die Reihenfolge der durch Dir::getFirstPattern und Dir::getNextPattern zurückgelieferten Dateien hängt von
 * der eingestellten Sortierung ab. Siehe dazu Dir::resort, Dir::open
 *
 * \param[in] it Iterator vom Typ ppl7::Dir::Iterator
 * \param[in] regexp Eine beliebige Perl kompatible Regular Expression. Beispiel:"/^*.txt$/i"
 *
 * @return Referenz auf die erste Datei des Verzeichnisses.
 * \exception EndOfListException Wird geworfen, wenn keine Einträge im geöffneten Verzeichnis vorhanden sind.
 */
const DirEntry& Dir::getFirstRegExp(Iterator& it, const String& regexp) const
{
    reset(it);
    return getNextRegExp(it, regexp);
}

/*!\brief Nächster Verzeichniseintrag, der zu der angegebenen Regular Expression passt
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der Iterator \p it auf den nächsten Verzeichniseintrag gesetzt,
 * der zu der angegebenen Regular Expression \p regexp passt, und dessen Daten zurückgegeben.
 * \par
 * Die Reihenfolge der durch Dir::getFirstPattern und Dir::getNextPattern zurückgelieferten Dateien hängt von
 * der eingestellten Sortierung ab. Siehe dazu Dir::resort, Dir::open
 *
 * \param[in] it Iterator vom Typ ppl7::Dir::Iterator
 * \param[in] regexp Eine beliebige Perl kompatible Regular Expression. Beispiel:"/^*.txt$/i"
 *
 * @return Referenz auf die nächste Datei des Verzeichnisses.
 * \exception EndOfListException Wird geworfen, wenn das Ende der Liste erreicht wurde.
 */
const DirEntry& Dir::getNextRegExp(Iterator& it, const String& regexp) const
{
    RegEx::Pattern pattern = RegEx::compile(regexp);
    while (SortedFiles.getNext(it)) {
        const DirEntry* de = it.value();
        // Patternmatch
        if (RegEx::match(pattern, de->Filename)) return *de;
    }
    throw EndOfListException();
}

/*!\brief Erster Verzeichniseintrag
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der Iterator \p it auf die erste gefundene Datei gesetzt.
 * Deren Daten werden in das DirEntry Objekt \p e kopiert.
 * Alle weiteren Dateien können mit Dir::getNext ausgelesen werden.
 * \par
 * Die Reihenfolge der durch Dir::getFirst und Dir::getNext zurückgelieferten Dateien hängt von
 * der eingestellten Sortierung ab. Siehe dazu Dir::resort und Dir::open
 * \param[out] e Objekt, in dem die Daten der Datei gespeichert werden sollen
 * \param[in,out] it Iterator vom Typ ppl7::Dir::Iterator
 * @return Gibt \c true zurück, wenn eine Datei im Verzeichnis vorhanden war, sonst \c false.
 * Falls ein Fehler auftritt, kann auch eine Exception geworfen werden.
 */
bool Dir::getFirst(DirEntry& e, Iterator& it) const
{
    if (!SortedFiles.getFirst(it)) return false;
    e = *it.value();
    return true;
}

/*!\brief Nächster Verzeichniseintrag
 *
 * \desc
 * Diese Funktion kopiert die nächste Datei aus dem geöffneten Verzeichnis in das DirEntry Objekt
 * \p e
 * \par
 * Die Reihenfolge der durch Dir::getFirst und Dir::getNext zurückgelieferten Dateien hängt von
 * der eingestellten Sortierung ab. Siehe dazu Dir::resort und Dir::open
 * \param[out] e Objekt, in dem die Daten der Datei gespeichert werden sollen
 * \param[in,out] it Iterator vom Typ ppl7::Dir::Iterator
 * @return Gibt \c true zurück, wenn eine Datei im Verzeichnis vorhanden war, sonst \c false.
 * Falls ein Fehler auftritt, kann auch eine Exception geworfen werden.
 */
bool Dir::getNext(DirEntry& e, Iterator& it) const
{
    if (!SortedFiles.getNext(it)) return false;
    e = *it.value();
    return true;
}

/*!\brief Erster Verzeichniseintrag, der zu einem bestimmten Muster passt
 *
 * \desc
 * Durch Aufruf dieser Funktion wird die erste Datei aus dem Verzeichnis,
 * die zu dem angegebenen Muster \p pattern passt, in das DirEntry Objekt
 * \p e kopiert. Alle weiteren Dateien können mit Dir::getNextPattern
 * ausgelesen werden.
 * \par
 * Die Reihenfolge der durch Dir::getFirstPattern und Dir::getNextPattern zurückgelieferten Dateien hängt von
 * der eingestellten Sortierung ab. Siehe dazu Dir::resort und Dir::open
 *
 * \param[out] e Objekt, in dem die Daten der Datei gespeichert werden sollen
 * \param[in,out] it Iterator vom Typ ppl7::Dir::Iterator
 * \param[in] pattern Ein beliebiges Suchpattern, wie es auch beim Unix-Befehl "ls" oder mit
 * "dir" unter Windows angegeben werden kann. Dabei sind die Wildcards "*" und "?" erlaubt.
 * Das Sternchen "*" steht dabei für beliebig viele Zeichen, das Fragezeichen "?" für ein einzelnes.
 * \param[in] ignorecase Wird diese Variable auf "true" gesetzt, wird Groß- und Kleinschreibung
 * ignoriert. Wird als Pattern beispielsweise "*.TXT" angegeben, würde auch "*.txt" passen.
 * Der Default ist "false"
 *
 * @return Gibt \c true zurück, wenn eine Datei im Verzeichnis vorhanden war, sonst \c false.
 * Falls ein Fehler auftritt, kann auch eine Exception geworfen werden.
 */
bool Dir::getFirstPattern(DirEntry& e, Iterator& it, const String& pattern, bool ignorecase) const
{
    reset(it);
    return getNextPattern(e, it, pattern, ignorecase);
}

/*!\brief Nächster Verzeichniseintrag, der zu einem bestimmten Muster passt
 *
 * \desc
 * Diese Funktion kopiert die nächste Datei aus dem geöffneten Verzeichnis,
 * die zu dem angegebenen Muster \p pattern passt, in das DirEntry Objekt \p e
 * \par
 * Die Reihenfolge der durch Dir::getFirstPattern und Dir::getNextPattern zurückgelieferten Dateien hängt von
 * der eingestellten Sortierung ab. Siehe dazu Dir::resort und Dir::open
 *
 * \param[out] e Objekt, in dem die Daten der Datei gespeichert werden sollen
 * \param[in,out] it Iterator vom Typ ppl7::Dir::Iterator
 * \param[in] pattern Ein beliebiges Suchpattern, wie es auch beim Unix-Befehl "ls" oder mit
 * "dir" unter Windows angegeben werden kann. Dabei sind die Wildcards "*" und "?" erlaubt.
 * Das Sternchen "*" steht dabei für beliebig viele Zeichen, das Fragezeichen "?" für ein einzelnes.
 * \param[in] ignorecase Wird diese Variable auf "true" gesetzt, wird Groß- und Kleinschreibung
 * ignoriert. Wird als Pattern beispielsweise "*.TXT" angegeben, würde auch "*.txt" passen.
 * Der Default ist "false"
 *
 * @return Gibt \c true zurück, wenn eine Datei im Verzeichnis vorhanden war, sonst \c false.
 * Falls ein Fehler auftritt, kann auch eine Exception geworfen werden.
 */
bool Dir::getNextPattern(DirEntry& e, Iterator& it, const String& pattern, bool ignorecase) const
{
    // printf ("Pattern: %s\n",(const char*)pattern);
    String Pattern = RegEx::escape(pattern);
    // printf ("Pattern: %s\n",(const char*)Pattern);
    Pattern.replace(".", "\\.");
    Pattern.replace("\\*", ".*");
    Pattern.replace("\\?", ".");
    Pattern = "^" + Pattern;
    Pattern += "$";
    int flags = RegEx::Flags::DOTALL;
    if (ignorecase) flags |= RegEx::Flags::CASELESS;
    RegEx::Pattern regex = RegEx::compile(Pattern, flags);
    // ppl7::PrintDebug ("final Pattern: %s\n",(const char*)Pattern);
    while (SortedFiles.getNext(it)) {
        const DirEntry* de = it.value();
        // Patternmatch
        // ppl7::PrintDebug ("Match gegen: %s\n",(const char*)de->Filename);
        if (RegEx::match(regex, de->Filename)) {
            e = *de;
            return true;
        }
    }
    return false;
}

/*!\brief Erster Verzeichniseintrag, der zu der angegebenen Regular Expression passt
 *
 * \desc
 * Durch Aufruf dieser Funktion wird die erste Datei aus dem Verzeichnis,
 * die zu der angegebenen Regular Expression \p regexp passt, in das DirEntry Objekt
 * \p e kopiert. Alle weiteren Dateien können mit Dir::getNextPattern
 * ausgelesen werden.
 * \par
 * Die Reihenfolge der durch Dir::getFirstPattern und Dir::getNextPattern zurückgelieferten Dateien hängt von
 * der eingestellten Sortierung ab. Siehe dazu Dir::resort und Dir::open
 *
 * \param[out] e Objekt, in dem die Daten der Datei gespeichert werden sollen
 * \param[in,out] it Iterator vom Typ ppl7::Dir::Iterator
 * \param[in] regexp Eine beliebige Perl kompatible Regular Expression. Beispiel:"/^*.txt$/i"
 *
 * @return Gibt \c true zurück, wenn eine Datei im Verzeichnis vorhanden war, sonst \c false.
 * Falls ein Fehler auftritt, kann auch eine Exception geworfen werden.
 */
bool Dir::getFirstRegExp(DirEntry& e, Iterator& it, const String& regexp) const
{
    reset(it);
    return getNextRegExp(e, it, regexp);
}

/*!\brief Nächster Verzeichniseintrag, der zu der angegebenen Regular Expression passt
 *
 * \desc
 * Diese Funktion kopiert die nächste Datei aus dem geöffneten Verzeichnis, die
 * zu der angegebenen Regular Expression \p regexp passt, in das DirEntry Objekt
 * \p e.
 *
 * \par
 * Die Reihenfolge der durch Dir::getFirstPattern und Dir::getNextPattern zurückgelieferten Dateien hängt von
 * der eingestellten Sortierung ab. Siehe dazu Dir::resort und Dir::open
 *
 * \param[out] e Objekt, in dem die Daten der Datei gespeichert werden sollen
 * \param[in,out] it Iterator vom Typ ppl7::Dir::Iterator
 * \param[in] regexp Eine beliebige Perl kompatible Regular Expression. Beispiel:"/^*.txt$/i"
 *
 * @return Gibt \c true zurück, wenn eine Datei im Verzeichnis vorhanden war, sonst \c false.
 * Falls ein Fehler auftritt, kann auch eine Exception geworfen werden.
 */
bool Dir::getNextRegExp(DirEntry& e, Iterator& it, const String& regexp) const
{
    RegEx::Pattern pattern = RegEx::compile(regexp);
    while (SortedFiles.getNext(it)) {
        const DirEntry* de = it.value();
        // Patternmatch
        if (RegEx::match(pattern, de->Filename)) {
            e = *de;
            return true;
        }
    }
    return false;
}

/*!\brief Verzeichnis einlesen
 *
 * \desc
 * Mit dieser Funktion wird das mit \p path angegebene Verzeichnis geöffnet,
 * eingelesen und mit der Sortiermethode \p s sortiert.
 *
 * @param[in] path Zu öffnender Pfad (siehe auch CDir::Open)
 * @param[in] s gewünschte Sortierreihenfolge. Defaultmäßig wird keine Sortierung
 * verwendet.
 * @return Die Funktion hat keinen Rückgabewert. Bei Auftreten eines Fehlers wird
 * eine Exception geworfen.
 */
void Dir::open(const String& path, Sort s)
{
    open((const char*)path, s);
}

void Dir::open(const char* path, Sort s)
{
    clear();
    sort = s;
    Path = path;
    Path.trim();
    Path.trimRight("/");
    Path.trimRight("\\");
#ifdef WIN32
    {
        HANDLE hFind;
        WIN32_FIND_DATAW FindFileData;
        ppl7::WideString w_path(Path);
        ppl7::WideString path_pattern = w_path + L"/*";
        path_pattern.replace(L"/", L"\\");
        if ((hFind = FindFirstFileW((const wchar_t*)path_pattern, &FindFileData)) == INVALID_HANDLE_VALUE) {
            throw CouldNotOpenDirectoryException("%s", (const char*)Path);
        }
        DirEntry de;
        WideString CurrentFile;
        do {
            CurrentFile = w_path + L"/";
            CurrentFile += FindFileData.cFileName;
            // printf ("found: %ls\n",(const wchar_t*)CurrentFile);
            // CurrentFile.hexDump();
            // printf ("dirwalk dwFileAttributes: %ls: %ld\n",(const wchar_t*)CurrentFile, FindFileData.dwFileAttributes);
            if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) continue;
            if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) continue;

            try {
                File::statFile(CurrentFile, de);
                Files.add(de);
                // printf ("stat ok: %s\n",(const char*)de.Filename);
                // de.Filename.hexDump();
            }
            catch (...) {
            }
        } while (FindNextFileW(hFind, &FindFileData) == true);
        FindClose(hFind);
        resort(sort);
        return;
    }
#else
    if (Path.isEmpty()) Path = "/";
#endif

#ifdef HAVE_OPENDIR
    DIR* dir = opendir((const char*)Path);
    if (!dir) {
        // printf("opendir fehlschlag\n");
        File::throwErrno(errno, path);
    }
    DirEntry de;
    String CurrentFile;
    while (1) {
        struct dirent* result = readdir(dir);
        if (result == NULL) break;
        CurrentFile = Path + "/" + String(result->d_name);
        // ppl7::PrintDebugTime ("DEBUG: CurrentFile=%s\n",(const char*)CurrentFile);
        // CurrentFile.hexDump();
        try {
            File::statFile(CurrentFile, de);
            Files.add(de);
        }
        catch (...) {
        }
    }
    closedir(dir);
    resort(sort);
#else
    throw UnsupportedFeatureException("Dir::open");
#endif
}

bool Dir::canOpen(const String& path)
{
    ppl7::String CheckPath = path;
    CheckPath.trim();
    CheckPath.trimRight("/");
    CheckPath.trimRight("\\");
#ifdef WIN32
    {
        HANDLE hFind;
        WIN32_FIND_DATAW FindFileData;
        ppl7::WideString w_path(CheckPath);
        ppl7::WideString path_pattern = w_path + L"/*";
        path_pattern.replace(L"/", L"\\");
        if ((hFind = FindFirstFileW((const wchar_t*)path_pattern, &FindFileData)) == INVALID_HANDLE_VALUE) {
            return false;
        }
        FindClose(hFind);
        return true;
    }

#endif
#ifdef HAVE_OPENDIR
    DIR* dir = opendir((const char*)CheckPath);
    if (!dir) return false;
    closedir(dir);
    return true;
#endif
    return false;
}

bool Dir::tryOpen(const String& path, Sort s)
{
    try {
        open(path, s);
        return true;
    }
    catch (...) {
    }
    return false;
}

bool Dir::exists(const String& dirname)
{
    try {
        DirEntry f;
        File::statFile(dirname, f);
        if (f.isDir()) return true;
        if (f.isLink()) return true;
        return false;
    }
    catch (...) {
        return false;
    }
    return false;
}

void Dir::mkDir(const String& path)
{
    Dir::mkDir(path, false);
}

void Dir::mkDir(const String& path, bool recursive)
{
#ifdef WIN32
    Dir::mkDir(path, 0, recursive);
#else
    Dir::mkDir(path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH, recursive);
#endif
}

void Dir::mkDir(const String& path, mode_t mode, bool recursive)
{
    String s;
    if (path.isEmpty()) throw IllegalArgumentException("Dir::mkDir got an empty path");
    // Wenn es das Verzeichnis schon gibt, koennen wir sofort aussteigen
    if (Dir::exists(path)) return;

    // printf ("path=%s\n",(const char*)path);
    //  1=erfolgreich, 0=Fehler
    if (!recursive) {
#ifdef WIN32
        s = path;
        s.replace("/", "\\");
        if (_wmkdir((const wchar_t*)WideString(s)) == 0) return;
#else
        if (mkdir((const char*)path, mode) == 0) return;
#endif
        throw CreateDirectoryFailedException("%s", (const char*)path);
    }
    // Wir hangeln uns von unten nach oben
    s = path;
    s.replace("\\", "/");
    Array tok;
    StrTok(tok, s, "/");
    // tok.explode(path,"/");
    // tok.list("tok");
    // throw UnknownException();
    s.clear();
    if (path[0] == '/') s.append("/");
    for (size_t i = 0; i < tok.count(); i++) {
        s.append(tok[i]);
        // Prüfen, ob das Verzeichnis da ist.
        if (!Dir::exists(s)) {
#ifdef _WIN32
            if (s.right(1) != ":") {
                s.replace("/", "\\");
                if (_wmkdir((const wchar_t*)WideString(s)) != 0) throw CreateDirectoryFailedException("%s", (const char*)s);
            }
#else
            if (mkdir((const char*)s, mode) != 0) throw CreateDirectoryFailedException("%s", (const char*)s);
#endif
        }
        s.append("/");
    }
}

} // namespace ppl7
