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
#include <algorithm>

#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <knownfolders.h>
#endif

#ifndef _WIN32
#include <unistd.h>
#include <pwd.h>
#endif

#include <ppl7/core/dir.h>
#include <ppl7/exceptions.h>
#include <ppl7/types/string.h>
#include <ppl7/types/widestring.h>
#include <ppl7/core/regex.h>

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

/****************************************************************
 * Statische Funktionen
 ****************************************************************/

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

String Dir::homePath()
{
#ifdef _WIN32
    // 1. Bevorzugt USERPROFILE unter Windows
    if (const char* userProfile = getenv("USERPROFILE"); userProfile && strlen(userProfile) > 0) {
        return String(userProfile);
    }
    // 2. Fallback: HOMEDRIVE + HOMEPATH
    const wchar_t* homeDrive = _wgetenv(L"HOMEDRIVE");
    const wchar_t* homePath = _wgetenv(L"HOMEPATH");
    if (homeDrive && homePath) {
        return String(homeDrive) + String(homePath).trimRight("\\");
    }
#else
    // 1. Umgebungsvariable HOME auslesen
    if (const char* home = getenv("HOME"); home && strlen(home) > 0) {
        return String(home).trimRight("/");
    }
    // 2. Fallback: System-Userdatenbank (/etc/passwd) abfragen
    if (struct passwd* pw = getpwuid(getuid()); pw && pw->pw_dir) {
        return String(pw->pw_dir).trimRight("/");
    }
#endif

    throw UnsupportedFeatureException("Dir::homePath: Could not determine user home directory");
}

String Dir::tempPath()
{
    std::error_code ec;
    auto path = std::filesystem::temp_directory_path(ec);
    if (!ec) {
        return String(path.string()).trimRight("/");
    }

    // Fallback für den unwahrscheinlichen Fall eines Fehlers:
#ifdef _WIN32
    if (const wchar_t* tmp = _wgetenv(L"TEMP")) return String(tmp).trimRight("\\");
    if (const wchar_t* tmp = _wgetenv(L"TMP")) return String(tmp).trimRight("\\");
    return String("C:\\Windows\\Temp");
#else
    if (const char* tmp = getenv("TMPDIR")) return String(tmp).trimRight("/");
    return String("/tmp");
#endif
}

String Dir::applicationDataPath()
{
    String path;
#ifdef _WIN32
    wchar_t* p = _wgetenv(L"LOCALAPPDATA");
    if (!p || wcslen(p) == 0) throw KeyNotFoundException("LOCALAPPDATA");
    return String(p).trimRight("\\");
#else
    return homePath() + "/.config";
#endif
}

String Dir::applicationDataPath(const String& company, const String& application)
{
#ifdef _WIN32
    return applicationDataPath() + "\\" + company + "\\" + application;
#else
    return applicationDataPath() + "/" + company + "/" + application;
#endif
}

String Dir::documentsPath()
{
#ifdef _WIN32
    PWSTR pathPtr = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &pathPtr))) {
        WideString wpath(pathPtr);
        CoTaskMemFree(pathPtr); // Speicher von SHGetKnownFolderPath freigeben
        return String(wpath);
    }
    throw KeyNotFoundException("FOLDERID_Documents");
#else
    return Dir::homePath() + "/Documents";
#endif
}

String Dir::documentsPath(const String& company, const String& application)
{
#ifdef _WIN32
    return documentsPath() + "\\" + company + "\\" + application;
#else
    return documentsPath() + "/" + company + "/" + application;
#endif
}

/***********************************************************************************
 * nicht statische Methoden
 ***********************************************************************************/
Dir::Dir()
{
    sort = Sort::None;
}

Dir::Dir(const String& path, Sort sortOrder)
{
    sort = sortOrder;
    if (path.notEmpty()) open(path, sortOrder);
}

void Dir::print() const
{
    printf("Directory Listing: %s\n", (const char*)Path);
    printf("Total Files: %zu\n", size());

    for (const DirEntry& de : Files) {
        printf("%s %3u ", (const char*)de.AttrStr, de.NumLinks);
        printf("%5u %5u ", de.Uid, de.Gid);
        printf("%10llu ", (unsigned long long)de.Size);
        printf("%s %s\n", (const char*)de.MTime.get(), (const char*)de.Filename);
    }
}

void Dir::resort(Sort s)
{
    switch (s) {
    case Sort::None:
        break;
    case Sort::Filename:
        resortFilename();
        break;
    case Sort::FilenameIgnoreCase:
        resortFilenameIgnoreCase();
        break;
    case Sort::ATime:
        resortATime();
        break;
    case Sort::CTime:
        resortCTime();
        break;
    case Sort::MTime:
        resortMTime();
        break;
    case Sort::Size:
        resortSize();
        break;
    default:
        throw IllegalArgumentException();
    }
    sort = s;
}

void Dir::resortFilename()
{
    std::stable_sort(Files.begin(), Files.end(), [](const DirEntry& a, const DirEntry& b) { return a.Filename < b.Filename; });
}

void Dir::resortFilenameIgnoreCase()
{
    std::stable_sort(Files.begin(), Files.end(),
                     [](const DirEntry& a, const DirEntry& b) { return a.Filename.strCaseCmp(b.Filename) < 0; });
}

void Dir::resortMTime()
{
    std::stable_sort(Files.begin(), Files.end(), [](const DirEntry& a, const DirEntry& b) { return a.MTime < b.MTime; });
}

void Dir::resortCTime()
{
    std::stable_sort(Files.begin(), Files.end(), [](const DirEntry& a, const DirEntry& b) { return a.CTime < b.CTime; });
}

void Dir::resortATime()
{
    std::stable_sort(Files.begin(), Files.end(), [](const DirEntry& a, const DirEntry& b) { return a.ATime < b.ATime; });
}

void Dir::resortSize()
{
    std::stable_sort(Files.begin(), Files.end(), [](const DirEntry& a, const DirEntry& b) { return a.Size < b.Size; });
}

// Filter
std::vector<DirEntry> Dir::filterPattern(const String& pattern, bool ignorecase = false) const
{
    // Wildcard in RegEx umwandeln
    String Pattern = RegEx::escape(pattern);
    Pattern.replace(".", "\\.");
    Pattern.replace("\\*", ".*");
    Pattern.replace("?", ".");
    Pattern = "^" + Pattern + "$";

    int flags = RegEx::Flags::DOTALL;
    if (ignorecase) flags |= RegEx::Flags::CASELESS;

    // RegEx kompilieren
    RegEx::Pattern regex = RegEx::compile(Pattern, flags);

    // Files durchsuchen
    std::vector<DirEntry> result;
    for (const DirEntry& entry : Files) {
        if (RegEx::match(regex, entry.Filename)) {
            result.push_back(entry);
        }
    }
    return result;
}

std::vector<DirEntry> Dir::filterRegExp(const String& regexp) const
{
    RegEx::Pattern regex = RegEx::compile(regexp);
    std::vector<DirEntry> result;
    for (const DirEntry& entry : Files) {
        if (RegEx::match(regex, entry.Filename)) {
            result.push_back(entry);
        }
    }
    return result;
}

std::optional<DirEntry> Dir::findPattern(const String& pattern, bool ignorecase) const
{
    String Pattern = RegEx::escape(pattern);
    Pattern.replace(".", "\\.");
    Pattern.replace("\\*", ".*");
    Pattern.replace("?", ".");
    Pattern = "^" + Pattern + "$";

    int flags = RegEx::Flags::DOTALL;
    if (ignorecase) flags |= RegEx::Flags::CASELESS;

    RegEx::Pattern regex = RegEx::compile(Pattern, flags);

    for (const auto& entry : Files) {
        if (RegEx::match(regex, entry.Filename)) {
            return entry;
        }
    }
    return std::nullopt;
}

std::optional<DirEntry> Dir::findRegExp(const String& regexp) const
{
    RegEx::Pattern regex = RegEx::compile(regexp);

    for (const auto& entry : Files) {
        if (RegEx::match(regex, entry.Filename)) {
            return entry;
        }
    }
    return std::nullopt;
}

void Dir::open(const String& path, Sort sortOrder)
{
    Files.clear();
    sort = sortOrder;
    Path = path.trimmed();
    Path.trimRight("/");
    Path.trimRight("\\");
    if (Path.isEmpty()) {
#ifdef _WIN32
        Path = ".";
#else
        Path = "/";
#endif
    }
    // 1. Verzeichnis über std::filesystem öffnen
#ifdef _WIN32
    std::filesystem::path fsPath(WideString(Path).getPtr());
#else
    std::filesystem::path fsPath(Path.c_str());
#endif

    std::error_code ec;
    auto it = std::filesystem::directory_iterator(fsPath, ec);
    if (ec) {
        if (ec == std::errc::no_such_file_or_directory) {
            throw NonexistingPathException("%s", (const char*)Path);
        } else if (ec == std::errc::permission_denied) {
            throw PermissionDeniedException("%s", (const char*)Path);
        }
        throw CouldNotOpenDirectoryException("%s", (const char*)Path);
    }

    // 2. Einträge einlesen und Meta-Daten via File::statFile ermitteln
    DirEntry de;
    for (const auto& entry : it) {
#ifdef _WIN32
        String currentFile = String(WideString(entry.path().c_str()));
#else
        String currentFile = entry.path().string();
#endif
        try {
            File::statFile(currentFile, de);
            Files.push_back(de);
        }
        catch (...) {
            // Einzelne Dateien ohne Rechte/Zugriff ignorieren
        }
    }

    // 3. Gewünschte Sortierung anwenden
    resort(sort);
}

bool Dir::canOpen(const String& path)
{
    String p = path.trimmed();
    p.trimRight("/");
    p.trimRight("\\");
    if (p.isEmpty()) {
#ifdef _WIN32
        p = ".";
#else
        p = "/";
#endif
    }
    // Verzeichnis über std::filesystem öffnen
#ifdef _WIN32
    std::filesystem::path fsPath(WideString(p).getPtr());
#else
    std::filesystem::path fsPath(p.c_str());
#endif

    std::error_code ec;
    auto it = std::filesystem::directory_iterator(fsPath, ec);
    if (ec) {
        if (ec == std::errc::no_such_file_or_directory) {
            return false;
        } else if (ec == std::errc::permission_denied) {
            return false;
        } else {
            return false;
        }
    }
    return true;
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

void Dir::mkDir(const String& path, bool recursive)
{
#ifdef _WIN32
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
#ifdef _WIN32
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
