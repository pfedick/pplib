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
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include <pplib/core/dir.h>
#include <pplib/exceptions.h>
#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/array.h>
#include <pplib/core/regex.h>
#include <pplib/core/functions.h>

namespace pplib
{

/****************************************************************
 * Statische Funktionen
 ****************************************************************/

String Dir::currentPath()
{
    std::error_code ec;
    auto path = std::filesystem::current_path(ec);
    if (!ec) {
        return String(path.c_str());
    }

    // Gezielte Fehlerbehandlung über std::error_code
    if (ec == std::errc::no_such_file_or_directory) {
        throw FileNotFoundException();
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
    const wchar_t* homeDriveEnv = _wgetenv(L"HOMEDRIVE");
    const wchar_t* homePathEnv = _wgetenv(L"HOMEPATH");
    if (homeDriveEnv && homePathEnv) {
        return String(homeDriveEnv) + String(homePathEnv).trimRight("\\");
    }
#else
    // 1. Umgebungsvariable HOME auslesen
    if (const char* home = getenv("HOME"); home && strlen(home) > 0) {
        return String(home).trimRight("/");
    }
    // 2. Fallback: System-Userdatenbank (/etc/passwd) abfragen
    long bufSize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufSize < 0) bufSize = 1024;

    std::vector<char> buffer(bufSize);
    struct passwd pw;
    struct passwd* result = nullptr;

    while (getpwuid_r(getuid(), &pw, buffer.data(), buffer.size(), &result) == ERANGE) {
        buffer.resize(buffer.size() * 2);
    }

    if (result && pw.pw_dir) {
        return String(pw.pw_dir).trimRight("/");
    }
#endif

    throw UnsupportedFeatureException("Dir::homePath: Could not determine user home directory");
}

String Dir::tempPath()
{
    std::error_code ec;
    auto path = std::filesystem::temp_directory_path(ec);
    if (!ec) {
        return String(path.c_str()).trimRight("/\\");
    }

    // Fallback für den unwahrscheinlichen Fall eines Fehlers:
#ifdef _WIN32
    if (const wchar_t* tmp = _wgetenv(L"TEMP")) return String(tmp).trimRight("/\\");
    if (const wchar_t* tmp = _wgetenv(L"TMP")) return String(tmp).trimRight("/\\");
    return String("C:\\Windows\\Temp");
#else
    if (const char* tmp = getenv("TMPDIR")) return String(tmp).trimRight("/");
    return String("/tmp");
#endif
}

String Dir::applicationDataPath()
{
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

bool Dir::exists(const String& dirname)
{
    if (dirname.isEmpty()) return false;

#ifdef _WIN32
    std::filesystem::path fsPath(WideString(dirname).getPtr());
#else
    std::filesystem::path fsPath((const char*)dirname);
#endif

    std::error_code ec;
    return std::filesystem::is_directory(fsPath, ec);
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
    if (path.isEmpty()) throw IllegalArgumentException("Dir::mkDir got an empty path");
    // Wenn es das Verzeichnis schon gibt, koennen wir sofort aussteigen
    if (Dir::exists(path)) return;

    if (!recursive) {
        // Nicht rekursiv, wir versuchen, das Verzeichnis direkt anzulegen
#ifdef _WIN32
        String s = path;
        s.replace("/", "\\");
        if (_wmkdir((const wchar_t*)WideString(s)) == 0) return;
        if (errno == EEXIST && Dir::exists(s)) return;
        throwExceptionFromErrno(errno, s);
#else
        if (mkdir((const char*)path, mode) == 0) return;
        if (errno == EEXIST && Dir::exists(path)) return;
        throwExceptionFromErrno(errno, path);
#endif
    }
    // Rekursiv: Pfad-Komponenten Schritt für Schritt durchlaufen
#ifdef _WIN32
    std::filesystem::path fsPath(WideString(path).getPtr());
#else
    std::filesystem::path fsPath(path.c_str());
#endif

    std::filesystem::path accumulated;
    for (const auto& element : fsPath) {
        accumulated /= element;

#ifdef _WIN32
        String currentPathStr = String(WideString(accumulated.c_str()));
#else
        String currentPathStr = accumulated.string();
#endif
        if (currentPathStr.isEmpty()) continue;

        // Windows-Laufwerks-Roots wie "C:" oder "C:\" überspringen
#ifdef _WIN32
        if (currentPathStr.endsWith(":") || currentPathStr.endsWith(":\\")) continue;
#endif

        // Jedes fehlende Verzeichnis in der Hierarchie direkt mit "mode" anlegen
        if (!Dir::exists(currentPathStr)) {
#ifdef _WIN32
            currentPathStr.replace("/", "\\");
            if (_wmkdir((const wchar_t*)WideString(currentPathStr)) != 0) {
                if (errno == EEXIST && Dir::exists(currentPathStr)) continue;
                throwExceptionFromErrno(errno, currentPathStr);
            }
#else
            if (mkdir((const char*)currentPathStr, mode) != 0) {
                if (errno == EEXIST && Dir::exists(currentPathStr)) continue;
                throwExceptionFromErrno(errno, currentPathStr);
            }
#endif
        }
    }
}

void Dir::rmDir(const String& path, bool recursive)
{
    if (path.isEmpty()) throw IllegalArgumentException("Dir::rmDir: empty path");

#ifdef _WIN32
    std::filesystem::path fsPath(WideString(path).getPtr());
#else
    std::filesystem::path fsPath(path.c_str());
#endif
    std::error_code ec;
    if (recursive) {
        std::filesystem::remove_all(fsPath, ec);
    } else {
        std::filesystem::remove(fsPath, ec);
    }
    if (ec) {
        if (ec == std::errc::no_such_file_or_directory) {
            return; // Verzeichnis existiert nicht, Ziel erreicht :-)
        } else if (ec == std::errc::permission_denied) {
            throw PermissionDeniedException("%s", (const char*)path);
        } else if (ec == std::errc::directory_not_empty) {
            throw DirectoryNotEmptyException("%s", (const char*)path);
        }
        // Fallback für alle weiteren OS-/CRT-Fehlercodes:
        throwExceptionFromErrno(ec.value(), path);
    }
}

/***********************************************************************************
 * nicht statische Methoden
 ***********************************************************************************/

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
        printf("%s %3u ", (const char*)de.getAttrStr(), de.NumLinks);
        printf("%5u %5u ", de.Uid, de.Gid);
        printf("%10llu ", (unsigned long long)de.Size);
        printf("%s %s\n", (const char*)de.MTime.get(), (const char*)de.Filename);
    }
}

void Dir::resort(Sort s) noexcept
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

static String filePatternToRegEx(const String& pattern)
{
    String result;
    result.reserve(pattern.len() * 2 + 2);
    result += '^';

    for (size_t i = 0; i < pattern.len(); ++i) {
        char c = pattern[i];
        switch (c) {
        case '*':
            result += ".*";
            break;
        case '?':
            result += '.';
            break;
        // Alle RegEx-Metazeichen escapen:
        case '.':
        case '+':
        case '^':
        case '$':
        case '[':
        case ']':
        case '(':
        case ')':
        case '{':
        case '}':
        case '|':
        case '\\':
            result += '\\';
            result += c;
            break;
        default:
            result += c;
            break;
        }
    }

    result += '$';
    return result;
}

// Filter
std::vector<DirEntry> Dir::filterPattern(const String& pattern, bool ignorecase) const
{
    // Wildcard in RegEx umwandeln
    String Pattern = filePatternToRegEx(pattern);

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
    String Pattern = filePatternToRegEx(pattern);

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
    skipped_entries_count = 0;
    sort = sortOrder;
    Path = path.trimmed();
    Path.trimRight("/");
#ifdef _WIN32
    Path.trimRight("\\");
#endif
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
            throw FileNotFoundException("%s", (const char*)Path);
        } else if (ec == std::errc::permission_denied) {
            throw PermissionDeniedException("%s", (const char*)Path);
        }
        throw CouldNotOpenDirectoryException("%s", (const char*)Path);
    }

    // 2. Einträge einlesen und Meta-Daten via File::statFile ermitteln
    std::filesystem::directory_iterator end;
    while (it != end) {
        const auto& entry = *it;
#ifdef _WIN32
        String currentFile = String(WideString(entry.path().c_str()));
#else
        String currentFile = entry.path().string();
#endif
        DirEntry de;
        try {
            File::statFile(currentFile, de);
            Files.push_back(de);
        }
        catch (...) {
            // Einzelne Dateien ohne Rechte/Zugriff ignorieren
            ++skipped_entries_count;
        }
        // Nächster Schritt ohne Exception:
        it.increment(ec);
        if (ec) {
            // Ein Fehler beim Vorrücken (z.B. gelöschte Datei oder Permission Denied):
            // Iterator wird bei Fehler im OS oft ungültig/beendet oder man bricht kontrolliert ab.
            ++skipped_entries_count;
            break;
        }
    }

    // 3. Gewünschte Sortierung anwenden
    resort(sort);
}

bool Dir::canOpen(const String& path)
{
    String p = path.trimmed();
    p.trimRight("/");
#ifdef _WIN32
    p.trimRight("\\");
#endif
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
        return false;
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

} // namespace pplib
