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
 *       documentation or other materials provided with the distribution.
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

#include <pplib/core/yaml.h>
#include <pplib/core/fileobject.h>
#include <pplib/core/file.h>
#include <pplib/core/memfile.h>
#include <pplib/exceptions.h>
#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/variant.h>
#include <pplib/types/variantarray.h>
#include <pplib/types/assocarray.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/datetime.h>
#include <pplib/types/date.h>
#include <pplib/types/time.h>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace pplib
{

// ============================================================================
// Zeilen-Infrastruktur
// ============================================================================

struct Line
{
    int indent = 0; // Anzahl der führenden Leerzeichen
    String text;    // Inhalt nach der Einrückung, ohne Kommentar und ohne Zeilenumbruch
};

static bool isYamlSpace(int c)
{
    return c == ' ' || c == '\t';
}

// Parser-Zustand: Pushback-Mechanismen
struct ParserState
{
    Line pendingLine;
    bool hasPendingLine = false;
    String pendingRaw; // rohe Zeile (inkl. Einrückung), z. B. aus parseBlockScalar
    bool hasPendingRaw = false;
};

static void pushLine(ParserState& state, const Line& line)
{
    state.pendingLine = line;
    state.hasPendingLine = true;
}

// Liest die nächste nicht-leere Zeile (leere Zeilen und Vollzeilen-Kommentare werden
// übersprungen). Gibt false bei EOF zurück. Tabs in der Einrückung sind ein Fehler.
static bool readLine(ParserState& state, pplib::FileObject& file, Line& out)
{
    if (state.hasPendingLine) {
        state.hasPendingLine = false;
        out = state.pendingLine;
        return true;
    }
    // Pending raw line aus parseBlockScalar verarbeiten
    if (state.hasPendingRaw) {
        state.hasPendingRaw = false;
        String raw = state.pendingRaw;
        state.pendingRaw.clear();
        // Einrückung zählen und Inhalt extrahieren
        size_t i = 0;
        while (i < raw.size() && raw[i] == ' ')
            i++;
        if (i < raw.size() && raw[i] == '\t') {
            throw pplib::UnexpectedCharacterException("Tab in YAML indentation");
        }
        // Kommentar abschneiden
        String content;
        char quote = 0;
        for (size_t p = i; p < raw.size(); ++p) {
            char ch = raw[p];
            if (quote != 0) {
                if (quote == '"' && ch == '\\' && p + 1 < raw.size()) {
                    content.append(ch);
                    ++p;
                    content.append(raw[p]);
                    continue;
                }
                if (ch == quote) quote = 0;
            } else if (ch == '"' || ch == '\'') {
                quote = ch;
            } else if (ch == '#' && (p == i || isYamlSpace(static_cast<unsigned char>(raw[p - 1])))) {
                break;
            }
            content.append(ch);
        }
        while (!content.isEmpty() && isYamlSpace(static_cast<unsigned char>(content[content.size() - 1]))) {
            content = content.substr(0, content.size() - 1);
        }
        if (!content.isEmpty()) {
            out.indent = static_cast<int>(i);
            out.text = std::move(content);
            return true;
        }
        // Leere/Comment-Zeile: weiter mit normalem Lesen fortfahren
    }
    while (!file.eof()) {
        String raw;
        int c;
        while ((c = file.fgetc()) != '\n' && c != EOF) {
            raw.append(static_cast<char>(c));
        }
        if (c == EOF && raw.isEmpty()) return false;

        // trailing \r entfernen (Windows-Zeilenenden)
        if (raw.size() > 0 && raw.endsWith("\r")) {
            raw = raw.substr(0, raw.size() - 1);
        }

        // Einrückung zählen (nur Leerzeichen erlaubt)
        size_t i = 0;
        while (i < raw.size() && raw[i] == ' ')
            i++;
        if (i < raw.size() && raw[i] == '\t') {
            throw pplib::UnexpectedCharacterException("Tab in YAML indentation at position %lld", file.tell());
        }

        // Kommentar abschneiden: '#' am Zeilenanfang oder nach Leerzeichen,
        // außer innerhalb von Anführungszeichen
        String content;
        char quote = 0;
        for (size_t p = i; p < raw.size(); ++p) {
            char ch = raw[p];
            if (quote != 0) {
                if (quote == '"' && ch == '\\' && p + 1 < raw.size()) {
                    content.append(ch);
                    ++p;
                    content.append(raw[p]);
                    continue;
                }
                if (ch == quote) quote = 0;
            } else if (ch == '"' || ch == '\'') {
                quote = ch;
            } else if (ch == '#' && (p == i || isYamlSpace(static_cast<unsigned char>(raw[p - 1])))) {
                break;
            }
            content.append(ch);
        }

        // trailing Whitespace entfernen
        while (!content.isEmpty() && isYamlSpace(static_cast<unsigned char>(content[content.size() - 1]))) {
            content = content.substr(0, content.size() - 1);
        }

        if (content.isEmpty()) continue; // leere Zeile oder reine Kommentarzeile

        out.indent = static_cast<int>(i);
        out.text = std::move(content);
        return true;
    }
    return false;
}

// ============================================================================
// Skalar-Parsing
// ============================================================================

static bool allDigits(const String& s, size_t start, size_t len)
{
    if (start + len > s.size()) return false;
    for (size_t i = 0; i < len; ++i) {
        if (!isdigit(static_cast<unsigned char>(s[start + i]))) return false;
    }
    return true;
}

static bool isIsoDate(const String& s)
{
    // YYYY-MM-DD
    return s.size() == 10 && allDigits(s, 0, 4) && s[4] == '-' && allDigits(s, 5, 2) && s[7] == '-' && allDigits(s, 8, 2);
}

static bool isIsoTime(const String& s)
{
    // HH:MM[:SS[.fff...]]
    if (s.size() < 5 || !allDigits(s, 0, 2) || s[2] != ':' || !allDigits(s, 3, 2)) return false;
    size_t p = 5;
    if (p < s.size() && s[p] == ':') {
        if (!allDigits(s, p + 1, 2)) return false;
        p += 3;
        if (p < s.size() && s[p] == '.') {
            p++;
            size_t d = 0;
            while (p < s.size() && isdigit(static_cast<unsigned char>(s[p]))) {
                ++p;
                ++d;
            }
            if (d == 0) return false;
        }
    }
    // strikt: nichts mehr hinter der Zeit erlaubt
    return p == s.size();
}

static bool isIsoDateTime(const String& s)
{
    // YYYY-MM-DD[T ]HH:MM[:SS[.fff...]][Z|±HH:MM]
    if (s.size() < 16 || !isIsoDate(s.substr(0, 10))) return false;
    char sep = s[10];
    if (sep != 'T' && sep != ' ') return false;
    String rest = s.substr(11);

    // optionales Timezone-Suffix abschneiden: Z oder ±HH:MM / ±HHMM
    size_t tzPos = rest.size();
    if (!rest.isEmpty() && rest[rest.size() - 1] == 'Z') {
        tzPos = rest.size() - 1;
    } else {
        for (size_t p = 5; p < rest.size(); ++p) {
            char ch = rest[p];
            if (ch != '+' && ch != '-') continue;
            size_t len = rest.size() - p;
            bool ok = false;
            if (len == 6 && rest[p + 3] == ':') {
                ok = allDigits(rest, p + 1, 2) && allDigits(rest, p + 4, 2);
            } else if (len == 5) {
                ok = allDigits(rest, p + 1, 4);
            }
            if (ok) tzPos = p;
            break;
        }
    }

    return isIsoTime(rest.substr(0, tzPos));
}

static void appendUtf8(pplib::String& str, uint32_t codePoint)
{
    if (codePoint >= 0xD800 && codePoint <= 0xDFFF) {
        throw pplib::CharacterEncodingException("Lone surrogate 0x%04X in YAML string", codePoint);
    }
    if (codePoint < 0x80) {
        str.append(static_cast<char>(codePoint));
    } else if (codePoint < 0x800) {
        char buf[2];
        buf[0] = static_cast<char>(0xC0 | (codePoint >> 6));
        buf[1] = static_cast<char>(0x80 | (codePoint & 0x3F));
        str.append(buf, 2);
    } else if (codePoint < 0x10000) {
        char buf[3];
        buf[0] = static_cast<char>(0xE0 | (codePoint >> 12));
        buf[1] = static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
        buf[2] = static_cast<char>(0x80 | (codePoint & 0x3F));
        str.append(buf, 3);
    } else {
        char buf[4];
        buf[0] = static_cast<char>(0xF0 | (codePoint >> 18));
        buf[1] = static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F));
        buf[2] = static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
        buf[3] = static_cast<char>(0x80 | (codePoint & 0x3F));
        str.append(buf, 4);
    }
}

// Entschlüsselt den Inhalt eines doppelzierten YAML-Strings (JSON-artige Escapes).
static pplib::String unescapeDoubleQuoted(const String& s)
{
    pplib::String str;
    str.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c != '\\') {
            str.append(c);
            continue;
        }
        ++i;
        if (i >= s.size()) throw pplib::UnexpectedEndOfDataException();
        char e = s[i];
        switch (e) {
        case 'n':
            str.append('\n');
            break;
        case 'r':
            str.append('\r');
            break;
        case 't':
            str.append('\t');
            break;
        case 'b':
            str.append('\b');
            break;
        case 'f':
            str.append('\f');
            break;
        case '"':
            str.append('"');
            break;
        case '\\':
            str.append('\\');
            break;
        case '/':
            str.append('/');
            break;
        case '0':
            str.append('\0');
            break;
        case 'a':
            str.append('\a');
            break;
        case 'v':
            str.append('\v');
            break;
        case 'e':
            str.append('\x1b');
            break;
        case 'x': {
            if (i + 2 >= s.size()) throw pplib::UnexpectedEndOfDataException();
            char hex[3] = {s[i + 1], s[i + 2], 0};
            for (int k = 0; k < 2; ++k) {
                if (!isxdigit(static_cast<unsigned char>(hex[k]))) {
                    throw pplib::InvalidEscapeSequenceException("Invalid hex digit in \\x escape sequence: >>%c<<", hex[k]);
                }
            }
            str.append(static_cast<char>(static_cast<uint8_t>(strtoul(hex, nullptr, 16))));
            i += 2;
            break;
        }
        case 'U': {
            if (i + 8 >= s.size()) throw pplib::UnexpectedEndOfDataException();
            char hex[9] = {s[i + 1], s[i + 2], s[i + 3], s[i + 4], s[i + 5], s[i + 6], s[i + 7], s[i + 8], 0};
            for (int k = 0; k < 8; ++k) {
                if (!isxdigit(static_cast<unsigned char>(hex[k]))) {
                    throw pplib::InvalidEscapeSequenceException("Invalid hex digit in \\U escape sequence: >>%c<<", hex[k]);
                }
            }
            appendUtf8(str, static_cast<uint32_t>(strtoul(hex, nullptr, 16)));
            i += 8;
            break;
        }
        case 'u': {
            if (i + 4 >= s.size()) throw pplib::UnexpectedEndOfDataException();
            char hex[5] = {s[i + 1], s[i + 2], s[i + 3], s[i + 4], 0};
            for (int k = 0; k < 4; ++k) {
                if (!isxdigit(static_cast<unsigned char>(hex[k]))) {
                    throw pplib::InvalidEscapeSequenceException("Invalid hex digit in \\u escape sequence: >>%c<<", hex[k]);
                }
            }
            uint32_t cp = static_cast<uint32_t>(strtoul(hex, nullptr, 16));
            // UTF-16 Surrogate Pair?
            if (cp >= 0xD800 && cp <= 0xDBFF) {
                // High surrogate: expect \uXXXX low surrogate next
                size_t save = i + 5;
                if (save + 5 < s.size() && s[save] == '\\' && s[save + 1] == 'u') {
                    char hex2[5] = {s[save + 2], s[save + 3], s[save + 4], s[save + 5], 0};
                    for (int k = 0; k < 4; ++k) {
                        if (!isxdigit(static_cast<unsigned char>(hex2[k]))) {
                            throw pplib::InvalidEscapeSequenceException("Invalid hex digit in \\u low surrogate: >>%c<<", hex2[k]);
                        }
                    }
                    uint32_t low = static_cast<uint32_t>(strtoul(hex2, nullptr, 16));
                    if (low >= 0xDC00 && low <= 0xDFFF) {
                        // Combine surrogate pair
                        cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
                        i = save + 5;
                    } else {
                        throw pplib::CharacterEncodingException("Invalid low surrogate 0x%04X", low);
                    }
                } else {
                    throw pplib::CharacterEncodingException("Unpaired high surrogate 0x%04X", cp);
                }
            } else if (cp >= 0xDC00 && cp <= 0xDFFF) {
                throw pplib::CharacterEncodingException("Lone low surrogate 0x%04X", cp);
            }
            appendUtf8(str, cp);
            i += 4;
            break;
        }
        default:
            throw pplib::InvalidEscapeSequenceException("Invalid escape sequence >>\\%c<<", e);
        }
    }
    return str;
}

// Parst einen (bereits aus der Zeile extrahierten) Skalar.
// Padet/trimmt den Sekundenbruchteil eines ISO-8601-Timestamps auf 6 Stellen (Mikrosekunden).
static pplib::String padUsec(const String& s)
{
    size_t p = s.find(".");
    if (p == String::npos) return s;
    size_t d = p + 1;
    while (d < s.size() && isdigit(static_cast<unsigned char>(s[d])))
        ++d;
    pplib::String res = s.substr(0, p + 1);
    for (size_t i = 0; i < 6; ++i) {
        if (p + 1 + i < d)
            res.append(s[p + 1 + i]);
        else
            res.append('0');
    }
    return res + s.substr(d);
}

static pplib::Variant parseScalar(const String& s)
{
    // zitierte Strings bleiben immer Strings
    if (!s.isEmpty() && s[0] == '"' && !(s.size() >= 2 && s.endsWith("\""))) {
        throw pplib::UnexpectedEndOfDataException("Unterminated double-quoted string in YAML scalar");
    }
    if (!s.isEmpty() && s[0] == '\'' && !(s.size() >= 2 && s.endsWith("'"))) {
        throw pplib::UnexpectedEndOfDataException("Unterminated single-quoted string in YAML scalar");
    }
    if (s.size() >= 2 && s.startsWith("\"") && s.endsWith("\"")) {
        return pplib::Variant(unescapeDoubleQuoted(s.substr(1, s.size() - 2)));
    }
    if (s.size() >= 2 && s.startsWith("'") && s.endsWith("'")) {
        String inner = s.substr(1, s.size() - 2);
        // in einfach zitierten Strings ist '' das Escape für '
        String out;
        for (size_t i = 0; i < inner.size(); ++i) {
            if (inner[i] == '\'' && i + 1 < inner.size() && inner[i + 1] == '\'') {
                out.append('\'');
                ++i;
            } else {
                out.append(inner[i]);
            }
        }
        return pplib::Variant(out);
    }

    // null
    if (s == "~" || s == "null") return pplib::Variant(nullptr);

    // Booleans (YAML 1.1: true/True/TRUE, false/False/FALSE)
    if (s == "true" || s == "True" || s == "TRUE") return pplib::Variant(true);
    if (s == "false" || s == "False" || s == "FALSE") return pplib::Variant(false);

    // Zahlen
    String t = s;
    bool neg = false;
    size_t start = 0;
    if (!t.isEmpty() && (t[0] == '-' || t[0] == '+')) {
        neg = (t[0] == '-');
        start = 1;
    }
    if (neg) t = t.substr(1);

    // hex / oktal
    if (t.size() > 2 && t.startsWith("0x")) {
        return pplib::Variant(neg ? -static_cast<int64_t>(strtoll(t.c_str(), nullptr, 16))
                                  : static_cast<int64_t>(strtoll(t.c_str(), nullptr, 16)));
    }
    if (t.size() > 2 && t.startsWith("0o")) {
        return pplib::Variant(neg ? -static_cast<int64_t>(strtoll(t.substr(2).c_str(), nullptr, 8))
                                  : static_cast<int64_t>(strtoll(t.substr(2).c_str(), nullptr, 8)));
    }

    // .inf / -.inf / .nan
    if (t == ".inf" || t == "inf") return pplib::Variant(neg ? -INFINITY : INFINITY);
    if (t == ".nan" || t == "nan") return pplib::Variant(NAN);

    // Strenge Float-Validierung: genau ein Punkt, Ziffern davor/nachher, optional Exponent
    bool hasDot = false;
    bool hasExp = false;
    size_t digitCount = 0;
    size_t pos = 0;
    for (pos = 0; pos < t.size(); ++pos) {
        char c = t[pos];
        if (isdigit(static_cast<unsigned char>(c))) {
            ++digitCount;
        } else if (c == '.' && !hasDot && !hasExp) {
            hasDot = true;
        } else if ((c == 'e' || c == 'E') && !hasExp && digitCount > 0) {
            hasExp = true;
            digitCount = 0; // Ziffern nach Exponent separat zählen
            if (pos + 1 < t.size() && (t[pos + 1] == '+' || t[pos + 1] == '-')) ++pos;
        } else {
            break;
        }
    }
    bool validFloat = (hasDot || hasExp) && pos == t.size() && digitCount > 0;
    if (validFloat) {
        return pplib::Variant(s.toDouble());
    }
    if (allDigits(t, 0, t.size())) {
        int64_t v = static_cast<int64_t>(strtoll(s.c_str(), nullptr, 10));
        return pplib::Variant(v);
    }

    // ISO-8601: Datum+Zeit -> DateTime, Datum -> Date, Zeit -> Time
    if (isIsoDateTime(s)) {
        pplib::DateTime dt;
        dt.set(padUsec(s));
        return pplib::Variant(dt);
    }
    if (isIsoDate(s)) {
        pplib::Date d;
        d.set(s);
        return pplib::Variant(d);
    }
    if (isIsoTime(s)) {
        pplib::Time tm;
        tm.set(padUsec(s));
        return pplib::Variant(tm);
    }

    // sonst: String
    return pplib::Variant(s);
}

// ---------------------------------------------------------------------------
// Flow-Style: { ... } und [ ... ] (JSON-artig, Zeilenübergreifend)
// ---------------------------------------------------------------------------

static void skipFlowWs(pplib::FileObject& file)
{
    int c;
    while ((c = file.fgetc()) != EOF) {
        if (isYamlSpace(c)) continue;
        if (c == '#') {
            // Kommentar bis Zeilenende
            do {
                c = file.fgetc();
            } while (c != EOF && c != '\n');
            if (c == EOF) return;
        } else {
            file.seek(-1, pplib::File::SEEKCUR);
            return;
        }
    }
}

static pplib::Variant parseTaggedValue(const String& tag, const String& rest);

static pplib::Variant parseFlow(pplib::FileObject& file, int c)
{
    if (c == '[') {
        pplib::VariantArray arr;
        skipFlowWs(file);
        c = file.fgetc();
        if (c == ']') return pplib::Variant(std::move(arr));
        file.seek(-1, pplib::File::SEEKCUR);
        while (true) {
            skipFlowWs(file);
            c = file.fgetc();
            if (c == EOF) throw pplib::UnexpectedEndOfDataException();
            if (c == ']') return pplib::Variant(std::move(arr)); // trailing comma
            arr.append(parseFlow(file, c));
            skipFlowWs(file);
            c = file.fgetc();
            if (c == ',') continue;
            if (c == ']') return pplib::Variant(std::move(arr));
            throw pplib::UnexpectedCharacterException("Expected ',' or ']' in flow sequence at position %lld, got >>%c<<", file.tell(), c);
        }
    }

    if (c == '{') {
        pplib::AssocArray dict;
        skipFlowWs(file);
        c = file.fgetc();
        if (c == '}') return pplib::Variant(std::move(dict));
        file.seek(-1, pplib::File::SEEKCUR);
        while (true) {
            skipFlowWs(file);
            c = file.fgetc();
            if (c == EOF) throw pplib::UnexpectedEndOfDataException();
            if (c == '}') return pplib::Variant(std::move(dict)); // trailing comma
            pplib::String key;
            int ch = 0;
            if (c == '"' || c == '\'') {
                size_t pos = 0;
                while (true) {
                    ch = file.fgetc();
                    if (ch == EOF) throw pplib::UnexpectedEndOfDataException();
                    if (ch == c) break;
                    key.append(static_cast<char>(ch));
                }
                key = (c == '"') ? unescapeDoubleQuoted(key) : key.replace("''", "'");
                skipFlowWs(file);
                c = file.fgetc();
            } else {
                // plain Key: c ist das erste Zeichen, ':' wird hier konsumiert
                key.append(static_cast<char>(c));
                while (true) {
                    ch = file.fgetc();
                    if (ch == EOF) throw pplib::UnexpectedEndOfDataException();
                    if (ch == ':' || ch == ',' || ch == '}') break;
                    key.append(static_cast<char>(ch));
                }
                key = key.trimmed();
                c = ch;
            }
            if (c != ':') {
                throw pplib::UnexpectedCharacterException("Expected ':' after flow key >>%s<< at position %lld, got >>%c<<", key.c_str(),
                                                          file.tell(), c);
            }
            skipFlowWs(file);
            c = file.fgetc();
            if (c == EOF) throw pplib::UnexpectedEndOfDataException();
            dict.set(key, parseFlow(file, c));
            skipFlowWs(file);
            c = file.fgetc();
            if (c == ',') continue;
            if (c == '}') return pplib::Variant(std::move(dict));
            throw pplib::UnexpectedCharacterException("Expected ',' or '}' in flow mapping at position %lld, got >>%c<<", file.tell(), c);
        }
    }

    // Scalar: c ist bereits das erste Zeichen
    String raw;
    raw.append(static_cast<char>(c));
    if (c == '"' || c == '\'') {
        // zitiert: bis zum schließenden Zitatzeichen (Backslash-Escapes beachten)
        while (true) {
            int ch = file.fgetc();
            if (ch == EOF) throw pplib::UnexpectedEndOfDataException();
            raw.append(static_cast<char>(ch));
            if (c == '"' && ch == '\\') {
                int n = file.fgetc();
                if (n == EOF) throw pplib::UnexpectedEndOfDataException();
                raw.append(static_cast<char>(n));
                continue;
            }
            if (ch == c) break;
        }
    } else {
        // plain: bis zum nächsten Flow-Trennzeichen (wird zurückgestellt)
        while (true) {
            int ch = file.fgetc();
            if (ch == EOF) throw pplib::UnexpectedEndOfDataException();
            if (ch == ',' || ch == ']' || ch == '}') {
                file.seek(-1, pplib::File::SEEKCUR);
                break;
            }
            raw.append(static_cast<char>(ch));
        }
    }
    String trimmed = raw.trimmed();
    if (trimmed.startsWith("!!binary")) return parseTaggedValue("binary", trimmed.substr(8));
    return parseScalar(trimmed);
}

// ---------------------------------------------------------------------------
// !!binary: Base64 -> ByteArray
// ---------------------------------------------------------------------------

static pplib::Variant decodeBinary(const String& content)
{
    String b64;
    for (size_t i = 0; i < content.size(); ++i) {
        char ch = content[i];
        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') continue;
        b64.append(ch);
    }
    pplib::ByteArray ba = pplib::ByteArray::fromBase64(b64);
    return pplib::Variant(std::move(ba));
}

static pplib::Variant parseTaggedValue(const String& tag, const String& rest)
{
    if (tag == "binary") {
        // Rest ist entweder inline-Base64 oder ein Block-Skalar (| / >)
        String r = rest.trimmed();
        // ggf. umgebende Anführungszeichen entfernen (z. B. !!binary "SGVsbG8=")
        if (r.size() >= 2 && ((r[0] == '"' && r[r.size() - 1] == '"') || (r[0] == '\'' && r[r.size() - 1] == '\''))) {
            r = r.substr(1, r.size() - 2);
        }
        if (r.isEmpty()) return pplib::Variant(pplib::ByteArrayPtr());
        char first = r[0];
        if (first == '|' || first == '>') {
            // wird vom Block-Parser übergeben: hier nur als Fallback behandelt
            throw pplib::UnsupportedDataTypeException("!!binary with block scalar must be handled by parseBlock");
        }
        return decodeBinary(r);
    }
    throw pplib::UnsupportedDataTypeException("Unsupported YAML tag >>%s<<", tag.c_str());
}

// ============================================================================
// Block-Skalare (| und >)
// ============================================================================

// Liest eine rohe Zeile inklusive Einrückung. Gibt false bei EOF zurück.
static bool readRawLine(ParserState& state, pplib::FileObject& file, String& out)
{
    if (state.hasPendingRaw) {
        state.hasPendingRaw = false;
        out = std::move(state.pendingRaw);
        state.pendingRaw.clear();
        return true;
    }
    out = "";
    int c;
    while ((c = file.fgetc()) != '\n' && c != EOF) {
        out.append(static_cast<char>(c));
    }
    if (c == EOF && out.isEmpty()) return false;
    if (out.endsWith("\r")) {
        out = out.substr(0, out.size() - 1);
    }
    return true;
}

// Parst einen Block-Skalar. Die Header-Zeile wurde vom Aufrufer bereits konsumiert.
// Liest alle folgenden Zeilen mit Einrückung > parentIndent.
// | behält Zeilenumbrüche bei, > faltet sie zu Leerzeichen (leere Zeilen bleiben Umbrüche).
// Clip-Chomping: genau ein abschließendes Zeichen \n, wenn Inhalt vorhanden ist.
static String parseBlockScalar(ParserState& state, pplib::FileObject& file, char style, int parentIndent)
{
    String result;
    bool first = true;
    while (true) {
        String raw;
        if (!readRawLine(state, file, raw)) break; // EOF
        size_t i = 0;
        while (i < raw.size() && raw[i] == ' ')
            i++;
        // Leere/Whitespace-Zeilen gehören zum Skalar; nur eine nicht-leere
        // Zeile mit geringerer Einrückung beendet ihn.
        if (i < raw.size() && i <= static_cast<size_t>(parentIndent)) {
            // Pushback der beendenden Zeile für den nächsten Parser
            state.pendingRaw = std::move(raw);
            state.hasPendingRaw = true;
            break;
        }
        String content = raw.substr(i);
        if (!first) {
            if (style == '|' || content.isEmpty()) {
                result += "\n";
            } else if (!result.endsWith("\n")) {
                result += " ";
            }
        }
        first = false;
        result += content;
    }
    // Clip-Chomping: genau ein abschließendes \n
    if (!result.isEmpty()) {
        while (result.endsWith("\n")) {
            result = result.substr(0, result.size() - 1);
        }
        result += "\n";
    }
    return result;
}

// ============================================================================
// Block-Parser (Mappings und Sequenzen)
// ============================================================================

static pplib::Variant parseBlock(ParserState& state, pplib::FileObject& file);
static pplib::Variant parseMapping(ParserState& state, pplib::FileObject& file, const Line& first);
static pplib::Variant parseSequence(ParserState& state, pplib::FileObject& file, const Line& first);

// Liest die nächste Zeile und gibt sie sofort wieder zurück (Peek).
static bool peekLine(ParserState& state, pplib::FileObject& file, Line& out)
{
    if (!readLine(state, file, out)) return false;
    pushLine(state, out);
    return true;
}

// Trennt "key: value" in Key und Wert. Der Doppelpunkt muss am Zeilenende
// stehen oder von einem Leerzeichen gefolgt werden (außerhalb von Quotes).
static bool splitKeyValue(const String& text, String& key, String& rest)
{
    char quote = 0;
    for (size_t i = 0; i < text.size(); ++i) {
        char ch = text[i];
        if (quote != 0) {
            if (quote == '"' && ch == '\\' && i + 1 < text.size()) {
                ++i; // escaped char, skip
                continue;
            }
            if (ch == quote) quote = 0;
        } else if (ch == '"' || ch == '\'') {
            quote = ch;
        } else if (ch == ':' && (i + 1 == text.size() || text[i + 1] == ' ')) {
            key = text.substr(0, i).trimmed();
            if (i + 1 < text.size()) rest = text.substr(i + 1).trimmed();
            return true;
        }
    }
    return false;
}

// Entfernt umgebende Anführungszeichen eines Keys.
static String unquoteKey(const String& k)
{
    if (k.size() >= 2 && k[0] == '"' && k[k.size() - 1] == '"') {
        return unescapeDoubleQuoted(k.substr(1, k.size() - 2));
    }
    if (k.size() >= 2 && k[0] == '\'' && k[k.size() - 1] == '\'') {
        return k.substr(1, k.size() - 2).replace("''", "'");
    }
    return k;
}

// Liefert true, wenn alle Flow-Klammern im Text geschlossen sind
// (Quotes und Kommentare werden ignoriert).
static bool flowComplete(const String& s)
{
    int depth = 0;
    char quote = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        char ch = s[i];
        if (quote != 0) {
            if (quote == '"' && ch == '\\') {
                ++i;
                continue;
            }
            if (ch == quote) quote = 0;
            continue;
        }
        if (ch == '"' || ch == '\'') {
            quote = ch;
            continue;
        }
        if (ch == '#') {
            while (i < s.size() && s[i] != '\n')
                ++i;
            continue;
        }
        if (ch == '[' || ch == '{')
            ++depth;
        else if (ch == ']' || ch == '}')
            --depth;
    }
    return depth <= 0;
}

// Parst einen Flow-Wert, der auf der aktuellen Zeile beginnt und ggf. über
// mehrere Zeilen läuft. Der Rest der Zeile liegt in 'rest'; weitere Zeilen
// werden roh nachgelesen. Der vollständige Flow-Text wird per MemFile an
// den zeichenbasierten parseFlow übergeben.
static pplib::Variant parseFlowValue(ParserState& state, pplib::FileObject& file, const String& rest)
{
    String flow = rest;
    while (!flowComplete(flow)) {
        String raw;
        if (!readRawLine(state, file, raw)) throw pplib::UnexpectedEndOfDataException();
        flow += "\n";
        flow += raw;
    }
    // parseFlow erwartet, dass das erste Zeichen bereits konsumiert ist
    pplib::MemFile mf(const_cast<char*>(flow.c_str()) + 1, flow.size() - 1);
    return parseFlow(mf, static_cast<unsigned char>(flow[0]));
}

// Parst einen Block (Mapping, Sequenz oder Flow) ab der nächsten Zeile.
static pplib::Variant parseBlock(ParserState& state, pplib::FileObject& file)
{
    Line line;
    if (!readLine(state, file, line)) throw pplib::UnexpectedEndOfDataException();
    const String& t = line.text;
    if (t == "-" || t.startsWith("- ")) return parseSequence(state, file, line);
    if (t[0] == '{' || t[0] == '[') return parseFlowValue(state, file, t);
    if (t.startsWith("!!binary")) {
        String b64 = t.substr(8).trimmed();
        if (b64.isEmpty()) return pplib::Variant(pplib::ByteArray());
        if (b64[0] == '|' || b64[0] == '>') {
            return decodeBinary(parseBlockScalar(state, file, b64[0], line.indent));
        }
        return parseTaggedValue("binary", b64);
    }
    if (t.startsWith("!!")) {
        ssize_t sp = t.find(" ");
        String tag = t.substr(2, sp == String::npos ? t.size() - 2 : sp - 2);
        String rv = (sp == String::npos) ? String() : t.substr(sp + 1);
        return parseTaggedValue(tag, rv);
    }
    // Prüfe ob es ein Mapping ist oder ein root-Skalar
    String k, r;
    if (splitKeyValue(t, k, r)) {
        return parseMapping(state, file, line);
    }
    // Root-Skalar (z.B. 42, "hello", true)
    return parseScalar(t);
}

static pplib::Variant parseMapping(ParserState& state, pplib::FileObject& file, const Line& first)
{
    pplib::AssocArray dict;
    int indent = first.indent;
    Line line = first;
    while (true) {
        String key, rest;
        if (!splitKeyValue(line.text, key, rest)) {
            throw pplib::UnexpectedCharacterException("Expected 'key: value' in YAML mapping, got >>%s<<", line.text.c_str());
        }
        key = unquoteKey(key);

        pplib::Variant val(nullptr);
        if (rest.isEmpty()) {
            Line next;
            if (peekLine(state, file, next) && next.indent > indent) {
                val = parseBlock(state, file);
            }
        } else if (rest[0] == '|' || rest[0] == '>') {
            val = pplib::Variant(parseBlockScalar(state, file, rest[0], indent));
        } else if (rest.startsWith("!!binary")) {
            String b64 = rest.substr(8).trimmed();
            if (b64.isEmpty()) {
                val = pplib::Variant(pplib::ByteArray());
            } else if (b64[0] == '|' || b64[0] == '>') {
                val = decodeBinary(parseBlockScalar(state, file, b64[0], indent));
            } else {
                val = parseTaggedValue("binary", b64);
            }
        } else if (rest.startsWith("!!")) {
            ssize_t sp = rest.find(" ");
            String tag = rest.substr(2, sp == String::npos ? rest.size() - 2 : sp - 2);
            String rv = (sp == String::npos) ? String() : rest.substr(sp + 1);
            val = parseTaggedValue(tag, rv);
        } else if (rest[0] == '{' || rest[0] == '[') {
            val = parseFlowValue(state, file, rest);
        } else {
            val = parseScalar(rest);
        }
        dict.set(key, std::move(val));

        if (!readLine(state, file, line)) break;
        if (line.indent < indent) {
            pushLine(state, line);
            break;
        }
        if (line.indent > indent) {
            throw pplib::UnexpectedCharacterException("Unexpected indentation in YAML mapping: >>%s<<", line.text.c_str());
        }
    }
    return pplib::Variant(std::move(dict));
}

static pplib::Variant parseSequence(ParserState& state, pplib::FileObject& file, const Line& first)
{
    pplib::VariantArray arr;
    int indent = first.indent;
    Line line = first;
    while (true) {
        if (!(line.text == "-" || line.text.startsWith("- "))) {
            pushLine(state, line);
            break;
        }
        // Inhalt nach dem Dash und dessen Spaltenposition
        String content;
        int itemIndent = indent + 1;
        if (line.text != "-") {
            size_t off = 2;
            while (off < line.text.size() && line.text[off] == ' ')
                ++off;
            itemIndent = indent + static_cast<int>(off);
            content = line.text.substr(off).trimmed();
        }

        pplib::Variant val;
        if (content.isEmpty()) {
            Line next;
            if (peekLine(state, file, next) && next.indent > indent) {
                val = parseBlock(state, file);
            }
            // sonst bleibt das Element null
        } else if (content[0] == '|' || content[0] == '>') {
            val = pplib::Variant(parseBlockScalar(state, file, content[0], indent));
        } else if (content.startsWith("!!binary")) {
            String b64 = content.substr(8).trimmed();
            if (b64.isEmpty()) {
                val = pplib::Variant(pplib::ByteArray());
            } else if (b64[0] == '|' || b64[0] == '>') {
                val = decodeBinary(parseBlockScalar(state, file, b64[0], indent));
            } else {
                val = parseTaggedValue("binary", b64);
            }
        } else if (content.startsWith("!!")) {
            ssize_t sp = content.find(" ");
            String tag = content.substr(2, sp == String::npos ? content.size() - 2 : sp - 2);
            String rv = (sp == String::npos) ? String() : content.substr(sp + 1);
            val = parseTaggedValue(tag, rv);
        } else if (content[0] == '{' || content[0] == '[') {
            val = parseFlowValue(state, file, content);
        } else {
            String k2, r2;
            if (content == "-" || content.startsWith("- ") || splitKeyValue(content, k2, r2)) {
                // verschachtelte Sequenz oder kompaktes Mapping: "- key: value"
                Line synth;
                synth.indent = itemIndent;
                synth.text = content;
                pushLine(state, synth);
                val = parseBlock(state, file);
            } else {
                val = parseScalar(content);
            }
        }
        arr.append(std::move(val));

        if (!readLine(state, file, line)) break;
        if (line.indent < indent) {
            pushLine(state, line);
            break;
        }
        if (line.indent > indent) {
            throw pplib::UnexpectedCharacterException("Unexpected indentation in YAML sequence: >>%s<<", line.text.c_str());
        }
    }
    return pplib::Variant(std::move(arr));
}

// ============================================================================
// Öffentliche API: Laden (Parsen)
// ============================================================================

void Yaml::load(pplib::Variant& data, pplib::FileObject& file)
{
    if (file.eof()) {
        data = pplib::Variant(nullptr);
        return;
    }
    ParserState state;
    data = parseBlock(state, file);
}

void Yaml::loads(pplib::Variant& data, const pplib::String& yaml)
{
    pplib::MemFile mf(const_cast<char*>(yaml.c_str()), yaml.size());
    load(data, mf);
}

pplib::Variant Yaml::load(pplib::FileObject& file)
{
    pplib::Variant v;
    load(v, file);
    return v;
}

pplib::Variant Yaml::loads(const pplib::String& yaml)
{
    pplib::Variant v;
    loads(v, yaml);
    return v;
}

void Yaml::load(pplib::AssocArray& data, pplib::FileObject& file)
{
    pplib::Variant v = load(file);
    if (!v.isAssocArray()) {
        throw pplib::TypeConversionException("YAML root is not an AssocArray (type %d)", v.type());
    }
    data = std::move(v.toAssocArray());
}

void Yaml::loads(pplib::AssocArray& data, const pplib::String& yaml)
{
    pplib::Variant v = loads(yaml);
    if (!v.isAssocArray()) {
        throw pplib::TypeConversionException("YAML root is not an AssocArray (type %d)", v.type());
    }
    data = std::move(v.toAssocArray());
}

void Yaml::load(pplib::VariantArray& data, pplib::FileObject& file)
{
    pplib::Variant v = load(file);
    if (!v.isVariantArray()) {
        throw pplib::TypeConversionException("YAML root is not a VariantArray (type %d)", v.type());
    }
    data = std::move(v.toVariantArray());
}

void Yaml::loads(pplib::VariantArray& data, const pplib::String& yaml)
{
    pplib::Variant v = loads(yaml);
    if (!v.isVariantArray()) {
        throw pplib::TypeConversionException("YAML root is not a VariantArray (type %d)", v.type());
    }
    data = std::move(v.toVariantArray());
}

// ---------------------------------------------------------------------------
// Writer
// ---------------------------------------------------------------------------

static pplib::String escapeString(const pplib::String& s)
{
    pplib::String ret;
    ret.reserve(s.size() + 16);
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        switch (c) {
        case '"':
            ret.append("\\\"");
            break;
        case '\\':
            ret.append("\\\\");
            break;
        case '\b':
            ret.append("\\b");
            break;
        case '\f':
            ret.append("\\f");
            break;
        case '\n':
            ret.append("\\n");
            break;
        case '\r':
            ret.append("\\r");
            break;
        case '\t':
            ret.append("\\t");
            break;
        default:
            if ((unsigned char)c < 0x20) {
                char buf[8];
                std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned int>(static_cast<unsigned char>(c)));
                ret.append(buf);
            } else {
                ret.append(c);
            }
        }
    }
    return ret;
}

// Skalar ohne Zeilenumbruch. Strings werden immer doppelte-quoted, damit sie
// beim Re-Parsen als String erkannt werden (Typ-Treue). Datums-/Zeitwerte und
// Zahlen bleiben unquoted, damit sie als solcher re-parsed werden.
static void writeScalar(pplib::FileObject& file, const pplib::Variant& v)
{
    if (v.isString()) {
        file.puts("\"");
        file.puts(escapeString(v.toString()));
        file.puts("\"");
        return;
    }
    if (v.isWideString()) {
        pplib::ByteArray utf8 = v.toWideString().toUtf8();
        pplib::String s(reinterpret_cast<const char*>(utf8.ptr()), utf8.size());
        file.puts("\"");
        file.puts(escapeString(s));
        file.puts("\"");
        return;
    }
    if (v.isInt64()) {
        file.putsf("%lld", (long long)v.toInt64());
        return;
    }
    if (v.isDouble()) {
        double d = v.toDouble();
        if (std::isnan(d) || std::isinf(d)) {
            file.puts("null");
        } else {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "%.15g", d);
            for (char* p = buf; *p; ++p)
                if (*p == ',') *p = '.';
            // Typ-Identität erhalten: ganzzahlige Doubles als "1.0" schreiben
            bool hasDotOrExp = false;
            for (const char* p = buf; *p; ++p) {
                if (*p == '.' || *p == 'e' || *p == 'E') {
                    hasDotOrExp = true;
                    break;
                }
            }
            if (!hasDotOrExp) {
                file.puts(buf);
                file.puts(".0");
            } else {
                file.puts(buf);
            }
        }
        return;
    }
    if (v.isBool()) {
        file.puts(v.toBool() ? "true" : "false");
        return;
    }
    if (v.isNull()) {
        file.puts("null");
        return;
    }
    if (v.isDateTime()) {
        file.puts(v.toDateTime().getISO8601withUsec());
        return;
    }
    if (v.isDate()) {
        file.puts(v.toDate().toString());
        return;
    }
    if (v.isTime()) {
        file.puts(v.toTime().toString());
        return;
    }
    if (v.isByteArray() || v.isByteArrayPtr()) {
        // Flow-Form: !!binary "Base64"
        file.putsf("!!binary \"%s\"", v.toByteArrayPtr().toBase64().c_str());
        return;
    }
    throw pplib::UnsupportedDataTypeException("Variant Type >>%d<< cannot be serialized to YAML", v.type());
}

static void writeVariantFlow(pplib::FileObject& file, const pplib::Variant& v);

static void writeDictFlow(pplib::FileObject& file, const pplib::AssocArray& dict)
{
    file.puts("{");
    bool first = true;
    for (auto it = dict.begin(); it != dict.end(); ++it) {
        if (!first) file.puts(",");
        first = false;
        file.putsf("\"%s\":", escapeString((*it).first).c_str());
        writeVariantFlow(file, *(*it).second);
    }
    file.puts("}");
}

static void writeArrayFlow(pplib::FileObject& file, const pplib::VariantArray& arr)
{
    file.puts("[");
    bool first = true;
    for (size_t i = 0; i < arr.size(); ++i) {
        if (!first) file.puts(",");
        first = false;
        writeVariantFlow(file, arr[i]);
    }
    file.puts("]");
}

static void writeVariantFlow(pplib::FileObject& file, const pplib::Variant& v)
{
    if (v.isAssocArray()) {
        writeDictFlow(file, v.toAssocArray());
    } else if (v.isVariantArray()) {
        writeArrayFlow(file, v.toVariantArray());
    } else {
        writeScalar(file, v);
    }
}

static void writeDictBlock(pplib::FileObject& file, const pplib::AssocArray& dict, int indent, int col);
static void writeSequenceBlock(pplib::FileObject& file, const pplib::VariantArray& arr, int indent, int col);

static void writeVariant(pplib::FileObject& file, const pplib::Variant& v, int indent)
{
    if (indent < 1) {
        writeVariantFlow(file, v);
        return;
    }
    // Block-Style
    if (v.isAssocArray()) {
        if (v.toAssocArray().size() == 0) {
            file.puts("{}");
            return;
        }
        writeDictBlock(file, v.toAssocArray(), indent, 0);
    } else if (v.isVariantArray()) {
        if (v.toVariantArray().size() == 0) {
            file.puts("[]");
            return;
        }
        writeSequenceBlock(file, v.toVariantArray(), indent, 0);
    } else {
        writeScalar(file, v);
    }
}

static void writeSpaces(pplib::FileObject& file, int n)
{
    for (int i = 0; i < n; ++i)
        file.puts(" ");
}

// Base64 in 76 Zeichen langen Zeilen unterhalb eines "!!binary |" schreiben
static void writeBase64Lines(pplib::FileObject& file, const pplib::String& b64, int col)
{
    for (size_t off = 0; off < b64.size(); off += 76) {
        size_t len = std::min<size_t>(76, b64.size() - off);
        file.puts("\n");
        writeSpaces(file, col);
        file.puts(b64.substr(off, len));
    }
}

static void writeDictBlock(pplib::FileObject& file, const pplib::AssocArray& dict, int indent, int col)
{
    bool first = true;
    for (auto it = dict.begin(); it != dict.end(); ++it) {
        if (!first) {
            file.puts("\n");
            writeSpaces(file, col);
        }
        first = false;
        file.putsf("\"%s\":", escapeString((*it).first).c_str());
        const pplib::Variant& v = *(*it).second;
        if (v.isByteArray() || v.isByteArrayPtr()) {
            file.puts(" !!binary |");
            writeBase64Lines(file, v.toByteArrayPtr().toBase64(), col + indent);
        } else if (v.isVariantArray()) {
            const pplib::VariantArray& a = v.toVariantArray();
            if (a.size() == 0) {
                file.puts(" []");
                continue;
            }
            file.puts("\n");
            writeSpaces(file, col + indent);
            writeSequenceBlock(file, a, indent, col + indent);
        } else if (v.isAssocArray()) {
            const pplib::AssocArray& d = v.toAssocArray();
            if (d.size() == 0) {
                file.puts(" {}");
                continue;
            }
            file.puts("\n");
            writeSpaces(file, col + indent);
            writeDictBlock(file, d, indent, col + indent);
        } else {
            file.puts(" ");
            writeScalar(file, v);
        }
    }
}

static void writeSequenceBlock(pplib::FileObject& file, const pplib::VariantArray& arr, int indent, int col)
{
    bool first = true;
    for (size_t i = 0; i < arr.size(); ++i) {
        if (!first) {
            file.puts("\n");
            writeSpaces(file, col);
        }
        first = false;
        file.puts("-");
        const pplib::Variant& v = arr[i];
        if (v.isByteArray() || v.isByteArrayPtr()) {
            file.puts(" !!binary |");
            writeBase64Lines(file, v.toByteArrayPtr().toBase64(), col + indent);
        } else if (v.isVariantArray()) {
            const pplib::VariantArray& a = v.toVariantArray();
            if (a.size() == 0) {
                file.puts(" []");
                continue;
            }
            file.puts(" ");
            writeSequenceBlock(file, a, indent, col + indent);
        } else if (v.isAssocArray()) {
            const pplib::AssocArray& d = v.toAssocArray();
            if (d.size() == 0) {
                file.puts(" {}");
                continue;
            }
            file.puts(" ");
            writeDictBlock(file, d, indent, col + indent);
        } else {
            file.puts(" ");
            writeScalar(file, v);
        }
    }
}

// ============================================================================

void Yaml::dump(pplib::FileObject& file, const pplib::Variant& data, int indent)
{
    file.rewind();
    file.truncate(0);
    writeVariant(file, data, indent);
    if (indent > 0) {
        file.fputc('\n');
    }
}

void Yaml::dumps(pplib::String& yaml, const pplib::Variant& data, int indent)
{
    pplib::MemFile file(nullptr, 0, true);
    Yaml::dump(file, data, indent);
    size_t size = file.tell();
    yaml.set(file.map(0, size), size);
}

pplib::String Yaml::dumps(const pplib::Variant& data, int indent)
{
    pplib::String result;
    Yaml::dumps(result, data, indent);
    return result;
}

void Yaml::dump(pplib::FileObject& file, const pplib::AssocArray& data, int indent)
{
    file.rewind();
    file.truncate(0);
    writeVariant(file, pplib::Variant(data), indent);
    if (indent > 0) {
        file.fputc('\n');
    }
}

void Yaml::dumps(pplib::String& yaml, const pplib::AssocArray& data, int indent)
{
    pplib::MemFile file(nullptr, 0, true);
    Yaml::dump(file, data, indent);
    size_t size = file.tell();
    yaml.set(file.map(0, size), size);
}

pplib::String Yaml::dumps(const pplib::AssocArray& data, int indent)
{
    pplib::String result;
    Yaml::dumps(result, data, indent);
    return result;
}

void Yaml::dump(pplib::FileObject& file, const pplib::VariantArray& data, int indent)
{
    file.rewind();
    file.truncate(0);
    writeVariant(file, pplib::Variant(data), indent);
    if (indent > 0) {
        file.fputc('\n');
    }
}

void Yaml::dumps(pplib::String& yaml, const pplib::VariantArray& data, int indent)
{
    pplib::MemFile file(nullptr, 0, true);
    Yaml::dump(file, data, indent);
    size_t size = file.tell();
    yaml.set(file.map(0, size), size);
}

pplib::String Yaml::dumps(const pplib::VariantArray& data, int indent)
{
    pplib::String result;
    Yaml::dumps(result, data, indent);
    return result;
}

} // end of namespace pplib
