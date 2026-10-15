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

#include <pplib/core/yaml.h>
#include <pplib/core/fileobject.h>
#include <pplib/core/memfile.h>
#include <pplib/exceptions.h>
#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/variant.h>
#include <pplib/types/variantarray.h>
#include <pplib/types/assocarray.h>
#include <pplib/types/array.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/bytearrayptr.h>
#include <pplib/types/date.h>
#include <pplib/types/datetime.h>
#include <pplib/types/time.h>
#include <pplib/types/timedelta.h>
#include <pplib/types/timezone.h>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <limits>
#include <map>
#include <string>
#include <vector>

namespace pplib
{

static constexpr int MAX_YAML_DEPTH = 128;

// ============================================================================
// Zeilenbasierter Reader mit 1-Zeilen-Lookahead (Stream- und Pipe-tauglich)
// ============================================================================

struct YamlLine
{
    pplib::String text; // Rohe Zeile ohne \n / \r
    int number = 0;     // 1-basierte Zeilennummer für Fehlermeldungen
};

class YamlLineReader
{
private:
    static constexpr size_t BUFFER_SIZE = 4096;
    pplib::FileObject& file;
    ByteArray ba;
    char* buffer = nullptr;
    size_t bufPos = 0;
    size_t bufLen = 0;
    int lineNo = 0;
    bool hasPeeked = false;
    YamlLine peeked;

    int getChar()
    {
        if (bufPos >= bufLen) {
            bufPos = 0;
            bufLen = 0;
            try {
                size_t n = file.fread(buffer, 1, BUFFER_SIZE);
                bufLen = n;
            }
            catch (const pplib::EndOfFileException&) {
                return EOF;
            }
            if (bufLen == 0) return EOF;
        }
        return static_cast<unsigned char>(buffer[bufPos++]);
    }

    bool readLineRaw(YamlLine& out)
    {
        pplib::String s;
        s.reserve(128);
        int c;
        bool any = false;
        while ((c = getChar()) != EOF) {
            any = true;
            if (c == '\n') break;
            if (c == '\r') {
                int c2 = getChar();
                if (c2 != '\n' && c2 != EOF) {
                    // einsames \r: Zeilenende, c2 gehört zur nächsten Zeile
                    if (bufPos > 0) bufPos--;
                }
                break;
            }
            s.append(static_cast<char>(c));
        }
        if (c == EOF && !any) return false;
        out.text = std::move(s);
        out.number = ++lineNo;
        return true;
    }

public:
    explicit YamlLineReader(pplib::FileObject& f)
        : file(f)
    {
        buffer = static_cast<char*>(ba.malloc(BUFFER_SIZE));
    }

    bool peekLine(YamlLine& out)
    {
        if (!hasPeeked) {
            if (!readLineRaw(peeked)) return false;
            hasPeeked = true;
        }
        out = peeked;
        return true;
    }

    bool readLine(YamlLine& out)
    {
        if (hasPeeked) {
            out = peeked;
            hasPeeked = false;
            return true;
        }
        return readLineRaw(out);
    }
};

// ============================================================================
// Parser-Kontext (explizit durchgereicht, keine globalen Zustände)
// ============================================================================

struct YamlCtx
{
    std::map<pplib::String, pplib::Variant> anchors;
};

// ============================================================================
// Hilfsfunktionen
// ============================================================================

static bool isBlankOrComment(const YamlLine& line)
{
    std::string_view text = line.text;
    for (char c : text) {
        if (c == '#') return true;
        if (c != ' ' && c != '\t') return false;
    }
    return true;
}

// Position des ersten inhaltlichen Zeichens (Whitespace übersprungen),
// std::string_view::npos wenn die Zeile nur aus Whitespace besteht.
static size_t firstContentPos(std::string_view text)
{
    size_t p = 0;
    while (p < text.size() && (text[p] == ' ' || text[p] == '\t')) p++;
    return (p == text.size()) ? std::string_view::npos : p;
}

static bool isWhitespaceOnly(std::string_view text)
{
    return firstContentPos(text) == std::string_view::npos;
}

static int indentOf(const YamlLine& line)
{
    int n = 0;
    std::string_view text = line.text;
    while (n < (int)text.size() && text[n] == ' ')
        n++;
    return n;
}

// YAML erlaubt Tabs nicht als Einrückung (DESIGN.md).
static void checkIndentTabs(const YamlLine& line)
{
    std::string_view text = line.text;
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == ' ') continue;
        if (text[i] == '\t')
            throw pplib::UnexpectedCharacterException("Tabs are not allowed in YAML indentation at line %d", line.number);
        break;
    }
}

static bool isDocMarker(const YamlLine& line, const char* marker)
{
    std::string_view text = line.text;
    if (text.compare(0, strlen(marker), marker) != 0) return false;
    size_t p = strlen(marker);
    while (p < line.text.size() && (line.text[p] == ' ' || line.text[p] == '\t'))
        p++;
    return p == line.text.size() || line.text[p] == '#';
}

// ============================================================================
// Forward Declarations
// ============================================================================

// Findet einen Mapping-Trenner ": " bzw. ":" am Zeilenende (nicht in
// Quotes, Flows, Kommentaren). Liefert die Position des ':' oder npos.
static size_t scanMapColon(std::string_view text, size_t p)
{
    size_t len = text.size();
    while (p < len) {
        char c = text[p];
        if (c == '#' && (p == 0 || text[p - 1] == ' ' || text[p - 1] == '\t'))
            return std::string_view::npos;
        if (c == '"' || c == '\'') {
            p++;
            while (p < len && text[p] != c) {
                if (text[p] == '\\' && c == '"') p++;
                p++;
            }
            if (p >= len) return std::string_view::npos;
            p++;
            continue;
        }
        if (c == '[' || c == '{' || c == ']' || c == '}') return std::string_view::npos;
        if (c == ':') {
            if (p + 1 >= len || text[p + 1] == ' ' || text[p + 1] == '\t') return p;
            p++;
            continue;
        }
        p++;
    }
    return std::string_view::npos;
}

// Eine Zeile sieht aus wie ein Mapping (Key: Value oder Key:), wenn sie
// nach optionalem '- '-Präfix einen Mapping-Trenner enthält.
static bool looksLikeMappingAt(std::string_view text, size_t p)
{
    while (p < text.size() && (text[p] == ' ' || text[p] == '\t')) p++;
    if (p >= text.size()) return false;
    char c = text[p];
    if (c == '[' || c == '{' || c == '|' || c == '>' || c == '#' || c == '&' || c == '*' || c == '!')
        return false;
    return scanMapColon(text, p) != std::string_view::npos;
}

static bool looksLikeMappingLine(std::string_view text)
{
    size_t p = 0;
    while (p < text.size() && text[p] == ' ') p++;
    if (p + 1 < text.size() && text[p] == '-' && (text[p + 1] == ' ' || text[p + 1] == '\t')) {
        p += 2;
    }
    return looksLikeMappingAt(text, p);
}

// Zeile beginnt mit einem Block-Sequenz-Indikator ('- ' oder '-' am Zeilenende).
static bool looksLikeSequenceLine(std::string_view text)
{
    size_t p = firstContentPos(text);
    if (p == std::string_view::npos) return false;
    return text[p] == '-' && (p + 1 == text.size() || text[p + 1] == ' ' || text[p + 1] == '\t');
}

struct YamlParser
{
    YamlLineReader& reader;
    YamlCtx& ctx;
    YamlLine line; // aktuelle Zeile (nur gültig wenn hasLine)
    size_t pos = 0;
    bool hasLine = false;

    int curChar() const;
    void advance();
    void skipSpaces();
    bool skipBlanksAndComments(YamlLine& out);

    pplib::Variant parseDocument();
    pplib::Variant parseBlock(int indent, int depth);
    pplib::Variant parseMapping(int indent, int depth);
    pplib::Variant parseInlineMapping(int indent, int depth);
    void parseMappingEntry(pplib::AssocArray& dict, int indent, int depth);
    pplib::Variant parseSequence(int indent, int depth);
    pplib::Variant parseSequenceFrom(int indent, int depth, bool midline);
    pplib::Variant parseValueAfterIndicator(int col, int parentIndent, int depth);
    pplib::Variant parseQuoted(std::string_view content, size_t& p, char quote, int parentIndent, bool inFlow);
    pplib::Variant parsePlainScalar(std::string_view content, size_t& p, int parentIndent, bool inFlow);
    pplib::Variant parseBlockScalar(char style, char chomp, int parentIndent);
    pplib::Variant parseFlow(char open, int depth);
    pplib::Variant parseFlowInner(bool inSeq, int depth);
    pplib::Variant resolveScalar(std::string_view raw, std::string_view tag, bool quoted);
};

// ============================================================================
// YamlParser: zeilenorientierte Basisfunktionen
// ============================================================================

int YamlParser::curChar() const
{
    if (!hasLine || pos >= line.text.size()) return EOF;
    return static_cast<unsigned char>(line.text[pos]);
}

void YamlParser::advance()
{
    if (hasLine && pos < line.text.size()) pos++;
}

void YamlParser::skipSpaces()
{
    while (hasLine && pos < line.text.size() && (line.text[pos] == ' ' || line.text[pos] == '\t'))
        pos++;
}

// Überspringt Leerzeilen und reine Kommentarzeilen; die erste "sichtbare"
// Zeile wird in out zurückgegeben (ohne sie zu verbrauchten Lookahead zu verlieren).
bool YamlParser::skipBlanksAndComments(YamlLine& out)
{
    YamlLine peek;
    if (!reader.peekLine(peek)) return false;
    while (isBlankOrComment(peek)) {
        reader.readLine(peek);
        if (!reader.peekLine(peek)) return false;
    }
    out = peek;
    return true;
}

// ============================================================================
// Dokument- und Block-Parsing
// ============================================================================

// Prüft nach Abschluss des Dokuments auf Restmüll (nur Kommentare und "..." erlaubt).
static void expectYamlEnd(YamlParser& P)
{
    YamlLine peek;
    while (P.skipBlanksAndComments(peek)) {
        if (isDocMarker(peek, "...")) {
            P.reader.readLine(peek);
            // Nach "..." sind nur noch Kommentare und Leerzeilen erlaubt
            while (P.skipBlanksAndComments(peek)) {
                if (isDocMarker(peek, "---"))
                    throw pplib::UnexpectedCharacterException("Multiple documents are not supported at line %d", peek.number);
                throw pplib::UnexpectedCharacterException("Garbage after end of YAML document at line %d", peek.number);
            }
            return;
        }
        if (isDocMarker(peek, "---"))
            throw pplib::UnexpectedCharacterException("Multiple documents are not supported at line %d", peek.number);
        throw pplib::UnexpectedCharacterException("Garbage after end of YAML document at line %d", peek.number);
    }
}

pplib::Variant YamlParser::parseDocument()
{
    YamlLine first;
    if (!skipBlanksAndComments(first)) throw pplib::UnexpectedEndOfDataException();

    bool hadDocStart = false;
    if (isDocMarker(first, "---")) {
        hadDocStart = true;
        reader.readLine(first); // Marker verbrauchen
        if (!skipBlanksAndComments(first)) {
            // "---" ohne Inhalt
            throw pplib::UnexpectedEndOfDataException();
        }
    }
    (void)hadDocStart;

    checkIndentTabs(first);
    int ind = indentOf(first);

    pplib::Variant result;
    std::string_view t = first.text;
    // Einzelner Skalar auf Root-Ebene (inkl. Flow und Quoted), wenn die Zeile
    // weder Mapping noch Sequenz ist:
    bool isSeqLine = (static_cast<size_t>(ind) < t.size() && t[ind] == '-' &&
                      (ind + 1 == t.size() || t[ind + 1] == ' ' || t[ind + 1] == '\t'));
    bool isMapLine = false;
    if (!isSeqLine) {
        size_t p = firstContentPos(t);
        // Flow-Collection auf Root-Ebene ({a: 1} bzw. [1, 2])
        bool isFlowLine = (p < t.size() && (t[p] == '[' || t[p] == '{'));
        isMapLine = !isFlowLine && looksLikeMappingAt(t, p);
    }

    if (isSeqLine) {
        result = parseSequence(ind, 1);
    } else if (isMapLine) {
        result = parseMapping(ind, 1);
    } else {
        // Root-Skalar: Zeile als Value-Zeile behandeln
        pos = 0;
        hasLine = true;
        reader.readLine(line);
        result = parseValueAfterIndicator(ind, -1, 1);
    }

    expectYamlEnd(*this);
    return result;
}

pplib::Variant YamlParser::parseBlock(int indent, int depth)
{
    YamlLine peek;
    if (!skipBlanksAndComments(peek)) throw pplib::UnexpectedEndOfDataException();
    checkIndentTabs(peek);
    int ind = indentOf(peek);
    if (ind < indent) throw pplib::UnexpectedEndOfDataException();
    std::string_view t = peek.text;
    if (static_cast<size_t>(ind) < t.size() && t[ind] == '-' &&
        (ind + 1 == t.size() || t[ind + 1] == ' ' || t[ind + 1] == '\t')) {
        return parseSequence(ind, depth);
    }
    return parseMapping(ind, depth);
}

pplib::Variant YamlParser::parseMapping(int indent, int depth)
{
    if (depth > MAX_YAML_DEPTH) {
        throw pplib::OverflowException("YAML nesting depth exceeded maximum limit (%d)", MAX_YAML_DEPTH);
    }
    pplib::AssocArray dict;

    while (true) {
        YamlLine peek;
        if (!skipBlanksAndComments(peek)) break;
        checkIndentTabs(peek);
        int ind = indentOf(peek);
        if (ind < indent) break; // Ende dieses Blocks; Elternzeile gehört dem Aufrufer
        if (ind > indent) {
            throw pplib::UnexpectedCharacterException("Unexpected indentation at line %d", peek.number);
        }
        if (isDocMarker(peek, "---")) {
            throw pplib::UnexpectedCharacterException("Multiple documents are not supported at line %d", peek.number);
        }
        if (isDocMarker(peek, "...")) break;
        // Zeile gehört dazu: jetzt verbrauchen
        reader.readLine(peek);
        line = peek;
        hasLine = true;
        pos = indent;

        parseMappingEntry(dict, indent, depth);
    }

    return pplib::Variant(dict);
}

// Liest ein einzelnes "key: value"-Paar in der aktuellen Zeile (line/pos
// gesetzt, pos zeigt auf den Key-Start) und fügt es dem dict hinzu.
void YamlParser::parseMappingEntry(pplib::AssocArray& dict, int indent, int depth)
{
    std::string_view text = line.text;

    if (pos >= text.size()) {
        throw pplib::UnexpectedCharacterException("Expected key at line %d", line.number);
    }

    // Sequenz-Indicator anstelle eines Keys?
    if (text[pos] == '-' && (pos + 1 == text.size() || text[pos + 1] == ' ')) {
        throw pplib::UnexpectedCharacterException("Expected key but found sequence entry at line %d", line.number);
    }

    // --- Key lesen ---
    pplib::String key;
    if (text[pos] == '"' || text[pos] == '\'') {
        char quote = text[pos];
        size_t p = pos;
        pplib::Variant kv = parseQuoted(text, p, quote, indent, false);
        // Für Keys gilt: immer String
        key = kv.toString();
        pos = p;
        skipSpaces();
        if (curChar() != ':') {
            throw pplib::UnexpectedCharacterException("Expected ':' after key at line %d", line.number);
        }
        advance();
    } else {
        size_t colon = scanMapColon(text, pos);
        if (colon == std::string_view::npos) {
            throw pplib::UnexpectedCharacterException("Expected ':' after key at line %d", line.number);
        }
        std::string_view raw = text.substr(pos, colon - pos);
        while (!raw.empty() && (raw.back() == ' ' || raw.back() == '\t')) raw.remove_suffix(1);
        key = pplib::String(raw);
        if (key.isEmpty()) key = "_empty_";
        pos = colon + 1;
    }

    // --- Value lesen ---
    pplib::Variant val = parseValueAfterIndicator(indent, indent, depth + 1);
    dict.set(key, std::move(val));
}

// Mapping, dessen erster Eintrag schon auf der Sequenzzeile steht
// ("- key: value"). Fortsetzungseinträge müssen auf der gleichen Spalte
// wie der erste Key stehen.
pplib::Variant YamlParser::parseInlineMapping(int indent, int depth)
{
    if (depth > MAX_YAML_DEPTH) {
        throw pplib::OverflowException("YAML nesting depth exceeded maximum limit (%d)", MAX_YAML_DEPTH);
    }
    pplib::AssocArray dict;
    parseMappingEntry(dict, indent, depth);

    pplib::Variant rest = parseMapping(indent, depth);
    pplib::AssocArray& r = static_cast<pplib::AssocArray&>(rest);
    for (auto it = r.begin(); it != r.end(); ++it)
        dict.set((*it).first, *(*it).second);

    return pplib::Variant(dict);
}

pplib::Variant YamlParser::parseSequence(int indent, int depth)
{
    return parseSequenceFrom(indent, depth, false);
}

// midline == true: der erste Eintrag beginnt mitten in der bereits gelesenen Zeile 'line'
// an der Position 'pos' (z. B. "- - 1" -> verschachtelte Sequenz). Die Zeile ist vom Reader
// schon verbraucht, 'indent' ist die Spalte des ersten '-'.
pplib::Variant YamlParser::parseSequenceFrom(int indent, int depth, bool midline)
{
    if (depth > MAX_YAML_DEPTH) {
        throw pplib::OverflowException("YAML nesting depth exceeded maximum limit (%d)", MAX_YAML_DEPTH);
    }
    pplib::VariantArray arr;
    bool first = midline;

    while (true) {
        YamlLine peek;
        if (first) {
            peek = line;
        } else if (!skipBlanksAndComments(peek)) {
            break;
        }
        checkIndentTabs(peek);
        int ind = indent;
        if (!first) {
            ind = indentOf(peek);
            if (ind < indent) break;
            if (ind > indent) {
                throw pplib::UnexpectedCharacterException("Unexpected indentation at line %d", peek.number);
            }
        }
        if (isDocMarker(peek, "---")) {
            throw pplib::UnexpectedCharacterException("Multiple documents are not supported at line %d", peek.number);
        }
        if (isDocMarker(peek, "...")) break;
        std::string_view t = peek.text;
        size_t dash = first ? pos : static_cast<size_t>(ind);
        if (!(dash < t.size() && t[dash] == '-' &&
              (dash + 1 == t.size() || t[dash + 1] == ' ' || t[dash + 1] == '\t'))) {
            break; // kein Sequenzeintrag mehr; Zeile gehört dem Aufrufer
        }
        if (!first) reader.readLine(peek);
        line = peek;
        hasLine = true;
        pos = dash + 1; // hinter dem '-'
        first = false;
        skipSpaces();

        // "- key: value" -> Mapping, das auf der Sequenzzeile beginnt
        if (looksLikeMappingAt(line.text, pos)) {
            arr.add(parseInlineMapping(static_cast<int>(pos), depth + 1));
            continue;
        }

        // "- - ..." -> verschachtelte Sequenz, die auf derselben Zeile beginnt
        if (pos < line.text.size() && line.text[pos] == '-' &&
            (pos + 1 == line.text.size() || line.text[pos + 1] == ' ' || line.text[pos + 1] == '\t')) {
            arr.add(parseSequenceFrom(static_cast<int>(pos), depth + 1, true));
            continue;
        }

        pplib::Variant val = parseValueAfterIndicator(static_cast<int>(dash), indent, depth + 1);
        arr.add(std::move(val));
    }

    return pplib::Variant(arr);
}


// ============================================================================
// Scalar-Resolution
// ============================================================================

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
    } else if (codePoint <= 0x10FFFF) {
        char buf[4];
        buf[0] = static_cast<char>(0xF0 | (codePoint >> 18));
        buf[1] = static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F));
        buf[2] = static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
        buf[3] = static_cast<char>(0x80 | (codePoint & 0x3F));
        str.append(buf, 4);
    } else {
        throw pplib::CharacterEncodingException("Unicode code point 0x%X out of valid range (max 0x10FFFF)", codePoint);
    }
}

// Liest einen quotierten Skalar (single oder double) ab content[p].
// p zeigt beim Aufruf auf den öffnenden Quote und wird nach dem schließenden
// Quote zurückgegeben. Mehrzeilige Fortsetzung wird mit Leerzeichen gefaltet.
pplib::Variant YamlParser::parseQuoted(std::string_view content, size_t& p, char quote, int parentIndent, bool inFlow)
{
    (void)content;
    p++; // öffnendes Quote überspringen
    pplib::String str;
    str.reserve(64);
    bool closed = false;
    bool escapedNewline = false; // "\" am Zeilenende: Umbruch wird nicht gefaltet

    while (true) {
        std::string_view text = line.text;
        while (p < text.size()) {
            char c = text[p];
            if (c == quote) {
                if (quote == '\'' && p + 1 < text.size() && text[p + 1] == '\'') {
                    str.append('\'');
                    p += 2;
                    continue;
                }
                p++;
                closed = true;
                break;
            }
            if (quote == '"' && c == '\\') {
                p++;
                if (p >= text.size()) {
                    escapedNewline = true; // Fortsetzung am Zeilenende nach Escape
                    break;
                }
                char e = text[p++];
                switch (e) {
                case '0':
                    str.append('\0');
                    break;
                case 'a':
                    str.append('\a');
                    break;
                case 'b':
                    str.append('\b');
                    break;
                case 't':
                case '\t':
                    str.append('\t');
                    break;
                case 'n':
                    str.append('\n');
                    break;
                case 'v':
                    str.append('\v');
                    break;
                case 'f':
                    str.append('\f');
                    break;
                case 'r':
                    str.append('\r');
                    break;
                case 'e':
                    str.append('\x1B');
                    break;
                case 'N':
                    appendUtf8(str, 0x0085);
                    break;
                case '_':
                    appendUtf8(str, 0x00A0);
                    break;
                case 'L':
                    appendUtf8(str, 0x2028);
                    break;
                case 'P':
                    appendUtf8(str, 0x2029);
                    break;
                case 'x':
                case 'u':
                case 'U': {
                    int n = (e == 'x') ? 2 : (e == 'u') ? 4 : 8;
                    uint32_t cp = 0;
                    for (int i = 0; i < n; i++) {
                        if (p >= text.size()) {
                            throw pplib::UnexpectedEndOfDataException("Incomplete \\%c escape sequence at line %d", e, line.number);
                        }
                        char h = text[p++];
                        if (!isxdigit(static_cast<unsigned char>(h))) {
                            throw pplib::InvalidEscapeSequenceException("Invalid hex digit in \\%c escape sequence at line %d", e,
                                                                        line.number);
                        }
                        cp = (cp << 4) | static_cast<uint32_t>(isdigit(h) ? h - '0' : (tolower(h) - 'a' + 10));
                    }
                    // UTF-16 Surrogatpaare (\uD800..\"uDBFF \uDC00..\uDFFF) zu einem Codepunkt zusammenführen
                    if (e == 'u' && cp >= 0xD800 && cp <= 0xDBFF && p + 5 < text.size() && text[p] == '\\' && text[p + 1] == 'u') {
                        uint32_t lo = 0;
                        bool ok = true;
                        for (int i = 0; i < 4; i++) {
                            char h = text[p + 2 + i];
                            if (!isxdigit(static_cast<unsigned char>(h))) {
                                ok = false;
                                break;
                            }
                            lo = (lo << 4) | static_cast<uint32_t>(isdigit(h) ? h - '0' : (tolower(h) - 'a' + 10));
                        }
                        if (ok && lo >= 0xDC00 && lo <= 0xDFFF) {
                            p += 6;
                            cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                        }
                    }
                    appendUtf8(str, cp);
                    break;
                }
                case '"':
                    str.append('"');
                    break;
                case '/':
                    str.append('/');
                    break;
                case '\\':
                    str.append('\\');
                    break;
                default:
                    throw pplib::InvalidEscapeSequenceException("\\%c at line %d", e, line.number);
                }
                continue;
            }
            if (static_cast<unsigned char>(c) < 0x20 && c != '\t') {
                throw pplib::UnexpectedCharacterException("Unescaped control character (ASCII %d) in string at line %d", c, line.number);
            }
            str.append(c);
            p++;
        }
        if (closed) break;

        // Zeile zu Ende ohne Schluss-Quote: Lookahead auf Fortsetzungszeile prüfen
        YamlLine peek;
        if (!reader.peekLine(peek)) throw pplib::UnexpectedEndOfDataException("Unterminated string at line %d", line.number);
        int bindent = indentOf(peek);
        bool blankish = isBlankOrComment(peek);
        bool deeper = parentIndent < 0 ? bindent > 0 : bindent > parentIndent;
        if (!inFlow && parentIndent >= 0 && !blankish && !deeper) {
            throw pplib::UnexpectedEndOfDataException("Unterminated string at line %d", line.number);
        }
        reader.readLine(peek);
        line = peek;
        size_t first = firstContentPos(peek.text);
        bool empty = (first == std::string_view::npos);
        p = empty ? peek.text.size() : first;
        if (empty) {
            str.append("\n"); // echte Leerzeile => harter Umbruch
        } else if (peek.text[first] != '#') {
            if (!escapedNewline) str.append(" "); // Faltung; "\" am Zeilenende entfernt den Umbruch
        }
        escapedNewline = false;
        continue;
    }
    return pplib::Variant(str);
}

// Liest einen plain Skalar ab content[p] bis Zeilenende (Block) oder bis
// Flow-Delimiter (inFlow). Mehrzeilige Fortsetzung wird gefaltet.
pplib::Variant YamlParser::parsePlainScalar(std::string_view content, size_t& p, int parentIndent, bool inFlow)
{
    (void)content;
    pplib::String raw;
    raw.reserve(64);
    const pplib::String spaces(" \t");
    while (true) {
        std::string_view text = line.text;
        while (p < text.size()) {
            char c = text[p];
            if (inFlow) {
                if (c == ',' || c == ']' || c == '}') break;
                if (c == ':' && (p + 1 >= text.size() || text[p + 1] == ' ' || text[p + 1] == '\t' || text[p + 1] == ',' ||
                                 text[p + 1] == ']' || text[p + 1] == '}')) {
                    break;
                }
            } else {
                if (c == '#' && p > 0 && (text[p - 1] == ' ' || text[p - 1] == '\t')) break;
            }
            raw.append(c);
            p++;
        }
        raw.trimRight(spaces);
        if (inFlow) break;

        // Fortsetzung?
        YamlLine peek;
        if (!reader.peekLine(peek)) break;
        size_t first = firstContentPos(peek.text);
        bool blank = (first == std::string_view::npos);
        bool commentOnly = (!blank && peek.text[first] == '#');
        int bindent = indentOf(peek);
        if (blank || commentOnly) {
            // Leer- und Kommentarzeilen koennen noch zur Fortsetzung gehoeren.
            // peekLine liefert stets dieselbe Zeile, daher erst consumen und
            // danach die naechste Inhaltszeile beurteilen.
            size_t hardBreaks = 0;
            YamlLine skip;
            do {
                reader.readLine(skip);
                hardBreaks++; // echte Leerzeile wie Kommentarzeile wirken als Umbruch
            } while (reader.peekLine(skip) && isBlankOrComment(skip));
            if (!reader.peekLine(peek)) break;
            int b2 = indentOf(peek);
            if ((parentIndent < 0 ? b2 > 0 : b2 > parentIndent) && !looksLikeMappingLine(peek.text) && !looksLikeSequenceLine(peek.text)) {
                reader.readLine(peek); // Fortsetzungszeile konsumieren
                for (size_t i = 0; i < hardBreaks; i++) raw.append('\n');
                line = peek;
                p = firstContentPos(peek.text);
                continue;
            }
            break; // konsumierte Leer-/Kommentarzeilen hoeren den Skalar auf
        }
        if ((parentIndent < 0 ? bindent > 0 : bindent > parentIndent) && !looksLikeMappingLine(peek.text) && !looksLikeSequenceLine(peek.text)) {
            reader.readLine(peek);
            raw.append(' ');
            line = peek;
            p = firstContentPos(peek.text);
            continue;
        }
        break;
    }
    return pplib::Variant(raw);
}

// Liest einen Block-Skalar (| oder >) deren Header auf der aktuellen Zeile
// bereits gelesen wurde (pos nach dem Header). parentIndent ist die Einrückung
// des Blocks, in dem der Wert steht.
pplib::Variant YamlParser::parseBlockScalar(char style, char chomp, int parentIndent)
{
    std::vector<pplib::String> content;
    int blockIndent = -1;

    while (true) {
        YamlLine peek;
        if (!reader.peekLine(peek)) break;
        bool blank = isWhitespaceOnly(peek.text);
        if (blank) {
            reader.readLine(peek);
            content.push_back(pplib::String());
            continue;
        }
        int ind = indentOf(peek);
        int minIndent = parentIndent < 0 ? 0 : parentIndent + 1;
        if (ind <= minIndent) {
            if (blockIndent == -1 || ind < blockIndent) break; // Blockende
        }
        if (blockIndent == -1) blockIndent = ind;
        if (ind < blockIndent) break; // Blockende (Untereinrückung)
        reader.readLine(peek);
        content.push_back(peek.text.substr(blockIndent));
    }

    // Nachfolgende reine Leerzeilen gehören ggf. noch zum Block (chomping +)
    if (chomp == '+') {
        YamlLine peek;
        while (reader.peekLine(peek)) {
            bool blank = isWhitespaceOnly(peek.text);
            if (!blank) break;
            reader.readLine(peek);
            content.push_back(pplib::String());
        }
    }

    // Trailing leere Zeilen zählen
    size_t trailingBlanks = 0;
    while (!content.empty() && content.back().isEmpty()) {
        content.pop_back();
        trailingBlanks++;
    }

    // body: verbleibende Inhaltzeilen (ohne.trailing Leerzeilen), gefaltet bzw.
    // literal zusammengesetzt, ohne abschließenden Umbruch.
    pplib::String body;
    if (style == '|') {
        for (size_t i = 0; i < content.size(); i++) {
            if (i > 0) body.append("\n");
            body.append(content[i]);
        }
    } else {
        // gefaltet: aufeinanderfolgende nicht-leere Zeilen werden mit Leerzeichen
        // verbunden, Leerzeilen werden zu harten Umbrüchen, überstehende
        // Einrückungen bleiben erhalten.
        bool needSpace = false;
        bool needNewline = false;
        for (size_t i = 0; i < content.size(); i++) {
            const pplib::String& ln = content[i];
            bool moreIndent = !ln.isEmpty() && (ln[0] == ' ' || ln[0] == '\t');
            if (ln.isEmpty()) {
                needSpace = false;
                needNewline = true;
                continue;
            }
            if (needNewline) {
                body.append("\n");
                needNewline = false;
                needSpace = false;
            } else if (needSpace && !moreIndent) {
                body.append(" ");
            }
            body.append(ln);
            if (moreIndent) {
                body.append("\n");
                needSpace = false;
                needNewline = false;
            } else {
                needSpace = true;
            }
        }
    }

    pplib::String result;
    if (!body.isEmpty()) {
        result = body;
        if (chomp != '-') result.append("\n"); // clip: eine, keep: eine + Blank-Zeilen
    }
    if (chomp == '+') {
        for (size_t i = 0; i < trailingBlanks; i++)
            result.append("\n");
    }
    return pplib::Variant(result);
}

// ============================================================================
// Wert-Dispatch nach Indicator (':' oder '-') bzw. Root
// ============================================================================

pplib::Variant YamlParser::parseValueAfterIndicator(int col, int parentIndent, int depth)
{
    if (depth > MAX_YAML_DEPTH) {
        throw pplib::OverflowException("YAML nesting depth exceeded maximum limit (%d)", MAX_YAML_DEPTH);
    }
    skipSpaces();

    // Tag und/oder Anchor vor dem eigentlichen Wert
    pplib::String tag;
    pplib::String anchor;
    while (hasLine && pos < line.text.size() && (line.text[pos] == '!' || line.text[pos] == '&')) {
        if (line.text[pos] == '!') {
            size_t start = pos;
            while (pos < line.text.size() && line.text[pos] != ' ' && line.text[pos] != '\t') pos++;
            tag = line.text.mid(start, pos - start);
            skipSpaces();
        } else {
            pos++;
            size_t start = pos;
            while (pos < line.text.size() && line.text[pos] != ' ' && line.text[pos] != '\t') pos++;
            anchor = line.text.mid(start, pos - start);
            if (anchor.isEmpty()) throw pplib::UnexpectedCharacterException("Empty anchor name at line %d", line.number);
            skipSpaces();
        }
    }

    pplib::Variant value;

    // Kein Wert mehr auf dieser Zeile (Zeilenende oder nur ein Kommentar):
    // Block auf der nächsten Zeile suchen
    if (!hasLine || pos >= line.text.size() || line.text[pos] == '#') {
        YamlLine peek;
        if (reader.peekLine(peek)) {
            if (isDocMarker(peek, "---")) {
                throw pplib::UnexpectedCharacterException("Multiple documents are not supported at line %d", peek.number);
            }
            bool blank = isBlankOrComment(peek);
            int bindent = indentOf(peek);
            bool deeper = parentIndent < 0 ? bindent > 0 : bindent > parentIndent;
            // Sequenzeinträge direkt unter dem Key sind auf gleicher Ebene erlaubt
            bool seqSibling = parentIndent >= 0 && bindent == parentIndent &&
                              static_cast<size_t>(bindent) < peek.text.size() && peek.text[bindent] == '-' &&
                              (bindent + 1 == static_cast<int>(peek.text.size()) || peek.text[bindent + 1] == ' ' ||
                               peek.text[bindent + 1] == '\t');
            if (!blank && (deeper || seqSibling)) {
                value = parseBlock(bindent, depth);
            } else {
                value = resolveScalar("", tag, false); // leerer Wert => null
            }
        } else {
            value = resolveScalar("", tag, false);
        }
    } else {
        char c = line.text[pos];
        if (c == '*') {
            // Alias
            pos++;
            size_t start = pos;
            while (pos < line.text.size() && line.text[pos] != ' ' && line.text[pos] != '\t') pos++;
            pplib::String name = line.text.mid(start, pos - start);
            auto it = ctx.anchors.find(name);
            if (it == ctx.anchors.end()) {
                throw pplib::ItemNotFoundException("Unknown YAML alias '*%s' at line %d", name.c_str(), line.number);
            }
            value = it->second; // tiefe Kopie durch Variant-Kopie
        } else if (c == '"' || c == '\'') {
            pplib::Variant qv = parseQuoted(line.text, pos, c, parentIndent, false);
            value = resolveScalar(qv.toString(), tag, true);
        } else if (c == '[' || c == '{') {
            value = parseFlow(c, depth);
        } else if (c == '|' || c == '>') {
            char style = c;
            pos++;
            char chomp = 0;
            while (hasLine && pos < line.text.size() && (line.text[pos] == '-' || line.text[pos] == '+')) {
                chomp = line.text[pos++];
            }
            pplib::Variant bs = parseBlockScalar(style, chomp, col);
            value = resolveScalar(bs.toString(), tag, true);
        } else {
            pplib::Variant raw = parsePlainScalar(line.text, pos, parentIndent, false);
            value = resolveScalar(raw.toString(), tag, false);
        }
        // Rest der Zeile: nur Kommentare erlaubt
        skipSpaces();
        if (hasLine && pos < line.text.size() && line.text[pos] != '#') {
            throw pplib::UnexpectedCharacterException("Unexpected characters after value at line %d", line.number);
        }
    }

    if (!anchor.isEmpty()) ctx.anchors[anchor] = value;
    return value;
}

// ============================================================================
// Flow-Strukturen [ ... ] und { ... }
// ============================================================================

pplib::Variant YamlParser::parseFlow(char open, int depth)
{
    if (depth > MAX_YAML_DEPTH) {
        throw pplib::OverflowException("YAML nesting depth exceeded maximum limit (%d)", MAX_YAML_DEPTH);
    }
    pos++; // öffnende Klammer überspringen
    if (open == '[') {
        pplib::VariantArray arr;
        while (true) {
            skipSpaces();
            if (!hasLine) throw pplib::UnexpectedEndOfDataException();
            if (pos >= line.text.size()) {
                // Flow endet nicht in der Zeile: auf nächste Zeile umsteigen
                YamlLine peek;
                if (!reader.peekLine(peek)) throw pplib::UnexpectedEndOfDataException("Unterminated flow collection at line %d", line.number);
                reader.readLine(peek);
                line = peek;
                pos = 0;
                continue;
            }
            char c = line.text[pos];
            if (c == '#') {
                pos = line.text.size();
                continue;
            }
            if (c == ']') {
                pos++;
                break;
            }
            if (c == ',') {
                pos++;
                continue;
            }
            // Ein ':' an Wertposition in einer Flow-Sequenz ist fehlerhaft. Ohne
            // diese Prüfung würde der Skalar-Parser nichts konsumieren und die
            // Schleife endlos laufen.
            if (c == ':') {
                throw pplib::UnexpectedCharacterException("Unexpected ':' in flow sequence at line %d", line.number);
            }
            size_t before = pos;
            pplib::Variant item = parseFlowInner(true, depth + 1);
            if (pos == before) {
                throw pplib::UnexpectedCharacterException("Unexpected character '%c' in flow sequence at line %d", c, line.number);
            }
            arr.add(std::move(item));
        }
        return pplib::Variant(arr);
    }
    pplib::AssocArray dict;
    while (true) {
        skipSpaces();
        if (!hasLine) throw pplib::UnexpectedEndOfDataException();
        if (pos >= line.text.size()) {
            YamlLine peek;
            if (!reader.peekLine(peek)) throw pplib::UnexpectedEndOfDataException("Unterminated flow collection at line %d", line.number);
            reader.readLine(peek);
            line = peek;
            pos = 0;
            continue;
        }
        char c = line.text[pos];
        if (c == '#') {
            pos = line.text.size();
            continue;
        }
        if (c == '}') {
            pos++;
            break;
        }
        if (c == ',') {
            pos++;
            continue;
        }
        // Key lesen
        pplib::String key;
        if (c == '"' || c == '\'') {
            key = parseQuoted(line.text, pos, c, -1, true).toString();
        } else if (c == '[' || c == '{') {
            throw pplib::UnexpectedCharacterException("Complex mapping keys are not supported at line %d", line.number);
        } else {
            pplib::Variant kv = parsePlainScalar(line.text, pos, -1, true);
            key = kv.toString();
            key.trim();
        }
        skipSpaces();
        if (!hasLine || pos >= line.text.size()) throw pplib::UnexpectedEndOfDataException("Unterminated flow collection at line %d", line.number);
        if (line.text[pos] != ':') throw pplib::UnexpectedCharacterException("Expected ':' after key at line %d", line.number);
        pos++;
        pplib::Variant val = parseFlowInner(false, depth + 1);
        if (key.isEmpty()) key = "_empty_";
        dict.set(key, std::move(val));
    }
    return pplib::Variant(dict);
}

pplib::Variant YamlParser::parseFlowInner(bool inSeq, int depth)
{
    (void)inSeq;
    skipSpaces();
    if (!hasLine) throw pplib::UnexpectedEndOfDataException();
    while (pos >= line.text.size()) {
        YamlLine peek;
        if (!reader.peekLine(peek)) throw pplib::UnexpectedEndOfDataException("Unterminated flow collection at line %d", line.number);
        reader.readLine(peek);
        line = peek;
        pos = 0;
        skipSpaces();
    }
    char c = line.text[pos];

    // Tag/Anchor innerhalb des Flow
    pplib::String tag;
    pplib::String anchor;
    while (pos < line.text.size() && (line.text[pos] == '!' || line.text[pos] == '&')) {
        if (line.text[pos] == '!') {
            size_t start = pos;
            while (pos < line.text.size() && line.text[pos] != ' ' && line.text[pos] != '\t' && line.text[pos] != ',' &&
                   line.text[pos] != ']' && line.text[pos] != '}') {
                pos++;
            }
            tag = line.text.mid(start, pos - start);
            skipSpaces();
        } else {
            pos++;
            size_t start = pos;
            while (pos < line.text.size() && line.text[pos] != ' ' && line.text[pos] != '\t' && line.text[pos] != ',' &&
                   line.text[pos] != ']' && line.text[pos] != '}') {
                pos++;
            }
            anchor = line.text.mid(start, pos - start);
            skipSpaces();
        }
        if (pos >= line.text.size()) break;
        c = line.text[pos];
    }

    pplib::Variant value;
    if (c == '*') {
        pos++;
        size_t start = pos;
        while (pos < line.text.size() && line.text[pos] != ' ' && line.text[pos] != '\t' && line.text[pos] != ',' &&
               line.text[pos] != ']' && line.text[pos] != '}') {
            pos++;
        }
        pplib::String name = line.text.mid(start, pos - start);
        auto it = ctx.anchors.find(name);
        if (it == ctx.anchors.end()) {
            throw pplib::ItemNotFoundException("Unknown YAML alias '*%s' at line %d", name.c_str(), line.number);
        }
        value = it->second;
    } else if (c == '"' || c == '\'') {
        pplib::Variant qv = parseQuoted(line.text, pos, c, -1, true);
        value = resolveScalar(qv.toString(), tag, true);
    } else if (c == '[' || c == '{') {
        value = parseFlow(c, depth);
    } else {
        pplib::Variant raw = parsePlainScalar(line.text, pos, -1, true);
        value = resolveScalar(raw.toString(), tag, false);
    }
    if (!anchor.isEmpty()) ctx.anchors[anchor] = value;
    return value;
}

// ============================================================================
// Scalar-Resolution: Tag und Typauflösung
// ============================================================================

static bool allDigits(std::string_view s, size_t from, size_t to)
{
    if (from >= to) return false;
    for (size_t i = from; i < to; i++)
        if (!isdigit(static_cast<unsigned char>(s[i]))) return false;
    return true;
}

// Wandelt eine (per allDigits geprüfte) Ziffernfolge in einen int um.
static int digitsToInt(std::string_view s)
{
    int v = 0;
    for (char c : s) v = v * 10 + (c - '0');
    return v;
}

// Parsen von YAML-Integers (dezimal, 0x, 0o, 0b, Unterstriche, Vorzeichen).
// Zahlen außerhalb des int64-Bereichs ergeben false (es wird nicht geworfen):
// der Wert bleibt ein String (z. B. IBAN oder Kartennummern).
static bool yamlParseInt(std::string_view s, int64_t& result)
{
    if (s.empty()) return false;
    size_t i = 0;
    bool negative = false;
    if (s[0] == '+' || s[0] == '-') {
        negative = (s[0] == '-');
        i = 1;
    }
    if (i >= s.size()) return false;
    int radix = 10;
    if (s[i] == '0' && i + 1 < s.size()) {
        char c = s[i + 1];
        if (c == 'x' || c == 'X') {
            radix = 16;
            i += 2;
        } else if (c == 'o' || c == 'O') {
            radix = 8;
            i += 2;
        } else if (c == 'b' || c == 'B') {
            radix = 2;
            i += 2;
        }
    }
    if (i >= s.size()) return false;

    const uint64_t maxAbs = negative ? 9223372036854775808ULL : 9223372036854775807ULL;
    uint64_t v = 0;
    bool anyDigit = false;
    bool afterUnderscore = false;
    for (; i < s.size(); i++) {
        char c = s[i];
        if (c == '_') {
            // Unterstriche müssen von Ziffern eingeschlossen sein
            if (!anyDigit || afterUnderscore) return false;
            afterUnderscore = true;
            continue;
        }
        int d;
        if (c >= '0' && c <= '9')
            d = c - '0';
        else if (c >= 'a' && c <= 'f')
            d = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F')
            d = c - 'A' + 10;
        else
            return false;
        if (d >= radix) return false;
        if (v > (maxAbs - static_cast<uint64_t>(d)) / static_cast<uint64_t>(radix)) return false; // außerhalb int64
        v = v * static_cast<uint64_t>(radix) + static_cast<uint64_t>(d);
        anyDigit = true;
        afterUnderscore = false;
    }
    if (!anyDigit || afterUnderscore) return false;

    if (negative) {
        result = (v == 9223372036854775808ULL) ? std::numeric_limits<int64_t>::min() : -static_cast<int64_t>(v);
    } else {
        result = static_cast<int64_t>(v);
    }
    return true;
}

// Parsen von YAML-Floats (inkl. .inf/.nan, E-Notation).
static bool yamlParseFloat(std::string_view s, double& result)
{
    if (s.empty()) return false;
    bool negative = false;
    size_t i = 0;
    if (s[0] == '+' || s[0] == '-') {
        negative = (s[0] == '-');
        i = 1;
    }
    std::string_view rest = s.substr(i);
    if (rest == ".inf" || rest == ".Inf" || rest == ".INF") {
        result = negative ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();
        return true;
    }
    if (rest == ".nan" || rest == ".NaN" || rest == ".NAN") {
        result = std::numeric_limits<double>::quiet_NaN();
        return true;
    }
    if (rest.empty()) return false;
    // Unterstriche sind nur als Trenner zwischen Ziffern erlaubt (YAML 1.1).
    // "_123", "123_" oder "1__2" sind keine Zahlen und bleiben Strings.
    for (size_t j = 0; j < rest.size(); j++) {
        if (rest[j] != '_') continue;
        if (j == 0 || j + 1 >= rest.size()) return false;
        if (!std::isdigit(static_cast<unsigned char>(rest[j - 1])) || !std::isdigit(static_cast<unsigned char>(rest[j + 1]))) return false;
    }
    std::string cleaned;
    cleaned.reserve(rest.size());
    for (char c : rest) {
        if (c == '_') continue;
        cleaned.push_back(c);
    }
    double v = 0;
    auto res = std::from_chars(cleaned.data(), cleaned.data() + cleaned.size(), v);
    if (res.ec != std::errc() || res.ptr != cleaned.data() + cleaned.size()) return false;
    result = negative ? -v : v;
    return true;
}

static bool isBoolTrue(std::string_view s)
{
    return s == "true" || s == "True" || s == "TRUE";
}
static bool isBoolFalse(std::string_view s)
{
    return s == "false" || s == "False" || s == "FALSE";
}

pplib::Variant YamlParser::resolveScalar(std::string_view rawIn, std::string_view tag, bool quoted)
{
    // Führende/abschließende Leerzeichen sind nur bei unquotierten Sklaren
    // bedeutungslos; in Quotes (und Blocksklaren) sind sie Teil des Werts.
    std::string_view raw;
    if (quoted) {
        raw = rawIn;
    } else {
        size_t b = firstContentPos(rawIn);
        raw = (b == std::string_view::npos)
            ? std::string_view()
            : rawIn.substr(b, rawIn.find_last_not_of(" \t") - b + 1);
    }

    auto asString = [&](std::string_view s) { return pplib::Variant(pplib::String(s)); };

    // --- explizite Tags ---
    if (!tag.empty()) {
        if (tag == "!!str" || tag == "!str") return asString(raw);
        if (tag == "!!int" || tag == "!int") {
            int64_t v;
            if (!yamlParseInt(raw, v)) throw pplib::TypeConversionException("Invalid !!int value '%s'", pplib::String(raw).c_str());
            return pplib::Variant(v);
        }
        if (tag == "!!float" || tag == "!float") {
            double v;
            if (!yamlParseFloat(raw, v)) throw pplib::TypeConversionException("Invalid !!float value '%s'", pplib::String(raw).c_str());
            return pplib::Variant(v);
        }
        if (tag == "!!bool" || tag == "!bool") {
            if (isBoolTrue(raw)) return pplib::Variant(true);
            if (isBoolFalse(raw)) return pplib::Variant(false);
            throw pplib::TypeConversionException("Invalid !!bool value '%s'", pplib::String(raw).c_str());
        }
        if (tag == "!!null" || tag == "!null") return pplib::Variant(nullptr);
        if (tag == "!!binary" || tag == "!binary") {
            pplib::String b64;
            b64.reserve(raw.size());
            for (char c : raw) {
                if (c != ' ' && c != '\t' && c != '\n') b64.append(c);
            }
            // Ein leerer unquotierter !!binary-Wert ist ein Fehler (z.B. wenn
            // ein '#' als Kommentar beginnt und damit der Wert verdeckt).
            if (b64.isEmpty() && !quoted)
                throw pplib::TypeConversionException("Empty !!binary value at line %d", line.number);
            return pplib::Variant(pplib::ByteArray::fromBase64(b64));
        }
        if (tag == "!!timestamp" || tag == "!timestamp") {
            const char* bad = "Invalid !!timestamp value '%s'";
            if (raw.size() == 10 && raw[4] == '-' && raw[7] == '-' && allDigits(raw, 0, 4) && allDigits(raw, 5, 7) && allDigits(raw, 8, 10)) {
                try {
                    return pplib::Variant(pplib::Date(digitsToInt(raw.substr(0, 4)), digitsToInt(raw.substr(5, 2)), digitsToInt(raw.substr(8, 2))));
                } catch (const std::exception&) {
                    throw pplib::TypeConversionException(bad, pplib::String(raw).c_str());
                }
            }
            if (raw.size() > 10 && raw[4] == '-' && raw[7] == '-' && allDigits(raw, 0, 4) && allDigits(raw, 5, 7) && allDigits(raw, 8, 10) &&
                (raw[10] == 'T' || raw[10] == 't' || raw[10] == ' ')) {
                try {
                    return pplib::Variant(pplib::DateTime(pplib::String(raw)));
                } catch (const std::exception&) {
                    throw pplib::TypeConversionException(bad, pplib::String(raw).c_str());
                }
            }
            throw pplib::TypeConversionException(bad, pplib::String(raw).c_str());
        }
        if (tag == "!Time") {
            try {
                return pplib::Variant(pplib::Time(pplib::String(raw)));
            } catch (const std::exception&) {
                throw pplib::TypeConversionException("Invalid !Time value '%s'", pplib::String(raw).c_str());
            }
        }
        if (tag == "!TimeDelta") {
            try {
                return pplib::Variant(pplib::TimeDelta(pplib::String(raw)));
            } catch (const std::exception&) {
                throw pplib::TypeConversionException("Invalid !TimeDelta value '%s'", pplib::String(raw).c_str());
            }
        }
        if (tag == "!TimeZone") {
            // Format: [[±]HH:]MM  bzw. [[±]HHMM]  (pplib TimeZone toString: "HH:MM")
            const char* bad = "Invalid !TimeZone value '%s'";
            try {
                size_t i = 0;
                bool negative = false;
                if (!raw.empty() && (raw[0] == '+' || raw[0] == '-')) {
                    negative = (raw[0] == '-');
                    i = 1;
                }
                std::string_view body = raw.substr(i);
                int hours = 0, minutes = 0;
                size_t colon = body.find(':');
                if (colon != std::string_view::npos) {
                    if (!allDigits(body, 0, colon) || !allDigits(body, colon + 1, body.size()))
                        throw pplib::TypeConversionException(bad, pplib::String(raw).c_str());
                    hours = digitsToInt(body.substr(0, colon));
                    minutes = digitsToInt(body.substr(colon + 1));
                } else if (body.size() == 4 || body.size() == 2) {
                    if (!allDigits(body, 0, body.size()))
                        throw pplib::TypeConversionException(bad, pplib::String(raw).c_str());
                    hours = digitsToInt(body.substr(0, 2));
                    minutes = (body.size() == 4) ? digitsToInt(body.substr(2, 2)) : 0;
                } else {
                    throw pplib::TypeConversionException(bad, pplib::String(raw).c_str());
                }
                int offset = hours * 60 + minutes;
                if (negative) offset = -offset;
                return pplib::Variant(pplib::TimeZone(offset, pplib::String("")));
            } catch (const pplib::Exception&) {
                throw;
            } catch (const std::exception&) {
                throw pplib::TypeConversionException(bad, pplib::String(raw).c_str());
            }
        }
        if (tag == "!!seq" || tag == "!seq" || tag == "!!map" || tag == "!map") {
            if (quoted || !raw.empty()) return asString(raw);
            return pplib::Variant(nullptr);
        }
        // Unbekanntes Tag (z.B. !unknown): Inhalt normal behandeln
    }

    if (quoted) return asString(raw);

    // --- Typauflösung ohne Tag ---
    if (raw.empty() || raw == "~" || raw == "null" || raw == "Null" || raw == "NULL") return pplib::Variant(nullptr);
    if (isBoolTrue(raw)) return pplib::Variant(true);
    if (isBoolFalse(raw)) return pplib::Variant(false);

    // Date: YYYY-MM-DD
    if (raw.size() == 10 && raw[4] == '-' && raw[7] == '-' && allDigits(raw, 0, 4) && allDigits(raw, 5, 7) && allDigits(raw, 8, 10)) {
        try {
            return pplib::Variant(pplib::Date(digitsToInt(raw.substr(0, 4)), digitsToInt(raw.substr(5, 2)), digitsToInt(raw.substr(8, 2))));
        } catch (const std::exception&) {
            return asString(raw); // kein gültiges Datum -> String
        }
    }
    // DateTime: YYYY-MM-DDT... oder YYYY-MM-DD HH:MM:SS
    if (raw.size() > 10 && raw[4] == '-' && raw[7] == '-' && allDigits(raw, 0, 4) && allDigits(raw, 5, 7) && allDigits(raw, 8, 10) &&
        (raw[10] == 'T' || raw[10] == 't' || raw[10] == ' ')) {
        try {
            return pplib::Variant(pplib::DateTime(pplib::String(raw)));
        } catch (const std::exception&) {
            return asString(raw);
        }
    }
    // Time: HH:MM(:SS(.ffffff))?
    if (raw.size() >= 5 && allDigits(raw, 0, 2) && raw[2] == ':' && allDigits(raw, 3, 5) &&
        (raw.size() == 5 || raw[5] == '.' || raw[5] == ':')) {
        try {
            return pplib::Variant(pplib::Time(pplib::String(raw)));
        } catch (const std::exception&) {
            return asString(raw);
        }
    }

    int64_t iv;
    if (yamlParseInt(raw, iv)) return pplib::Variant(iv);
    double dv;
    if (yamlParseFloat(raw, dv)) return pplib::Variant(dv);

    return asString(raw);
}

// ============================================================================
// Writer-Hilfsfunktionen
// ============================================================================

static void writeDict(pplib::FileObject& file, const pplib::AssocArray& data, int indent, int level, int depth);
static void writeArray(pplib::FileObject& file, const pplib::VariantArray& data, int indent, int level, int depth);
static void writeClassicArray(pplib::FileObject& file, const pplib::Array& data, int indent, int level, int depth);
static void writeValueForKey(pplib::FileObject& file, const pplib::Variant& value, int indent, int level, int depth);

static void writeIndent(pplib::FileObject& file, int indent, int level)
{
    file.fputc('\n');
    for (int i = 0; i < level * indent; ++i) {
        file.fputc(' ');
    }
}

static void writeQuotedString(pplib::FileObject& file, const char* p, size_t len)
{
    file.fputc('"');
    for (size_t i = 0; i < len; ++i) {
        char c = p[i];
        switch (c) {
        case '"':
            file.fputs("\\\"");
            break;
        case '\\':
            file.fputs("\\\\");
            break;
        case '\b':
            file.fputs("\\b");
            break;
        case '\f':
            file.fputs("\\f");
            break;
        case '\n':
            file.fputs("\\n");
            break;
        case '\r':
            file.fputs("\\r");
            break;
        case '\t':
            file.fputs("\\t");
            break;
        default:
            if (static_cast<unsigned char>(c) < 32) {
                char ubuf[8];
                snprintf(ubuf, sizeof(ubuf), "\\u%04x", static_cast<unsigned char>(c));
                file.fputs(ubuf);
            } else {
                file.fputc(c);
            }
            break;
        }
    }
    file.fputc('"');
}

// Lexikalische Spiegelbilder der numerischen Literal-Erkennung des Parsers:
// geprüft wird nur die Form, nie der Wert. Ein String bleibt ein String und
// wird quotiert, wenn er beim Lesen als Zahl auflaufen würde (z.B. IBAN).
static bool looksLikeIntLiteral(std::string_view s)
{
    if (s.empty()) return false;
    size_t i = 0;
    if (s[0] == '+' || s[0] == '-') i = 1;
    if (i >= s.size()) return false;
    int radix = 10;
    if (s[i] == '0' && i + 1 < s.size()) {
        char c = s[i + 1];
        if (c == 'x' || c == 'X') { radix = 16; i += 2; }
        else if (c == 'o' || c == 'O') { radix = 8; i += 2; }
        else if (c == 'b' || c == 'B') { radix = 2; i += 2; }
    }
    if (i >= s.size()) return false;
    bool anyDigit = false;
    bool afterUnderscore = false;
    for (; i < s.size(); i++) {
        char c = s[i];
        if (c == '_') {
            if (!anyDigit || afterUnderscore) return false;
            afterUnderscore = true;
            continue;
        }
        int d;
        if (c >= '0' && c <= '9') d = c - '0';
        else if (c >= 'a' && c <= 'f') d = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') d = c - 'A' + 10;
        else return false;
        if (d >= radix) return false;
        anyDigit = true;
        afterUnderscore = false;
    }
    return anyDigit && !afterUnderscore;
}

static bool looksLikeFloatLiteral(std::string_view s)
{
    if (s.empty()) return false;
    size_t i = 0;
    if (s[0] == '+' || s[0] == '-') i = 1;
    std::string_view rest = s.substr(i);
    if (rest == ".inf" || rest == ".Inf" || rest == ".INF") return true;
    if (rest == ".nan" || rest == ".NaN" || rest == ".NAN") return true;
    if (rest.empty()) return false;
    size_t j = 0;
    bool anyDigit = false;
    while (j < rest.size() && (rest[j] == '_' || isdigit(static_cast<unsigned char>(rest[j])))) {
        if (rest[j] != '_') anyDigit = true;
        j++;
    }
    if (j < rest.size() && rest[j] == '.') {
        j++;
        while (j < rest.size() && (rest[j] == '_' || isdigit(static_cast<unsigned char>(rest[j])))) {
            if (rest[j] != '_') anyDigit = true;
            j++;
        }
    }
    if (!anyDigit) return false;
    if (j < rest.size() && (rest[j] == 'e' || rest[j] == 'E')) {
        j++;
        if (j < rest.size() && (rest[j] == '+' || rest[j] == '-')) j++;
        bool expDigit = false;
        while (j < rest.size() && isdigit(static_cast<unsigned char>(rest[j]))) { expDigit = true; j++; }
        if (!expDigit) return false;
    }
    return j == rest.size();
}

// Ergäbe ein unquotierter Skalar beim Lesen etwas anderes als einen String,
// muss der Writer quotieren.
static bool resolvesToNonString(std::string_view raw)
{
    if (raw.empty() || raw == "~" || raw == "null" || raw == "Null" || raw == "NULL") return true;
    if (isBoolTrue(raw) || isBoolFalse(raw)) return true;
    if (raw.size() >= 5 && allDigits(raw, 0, 2) && raw[2] == ':' && allDigits(raw, 3, 5) &&
        (raw.size() == 5 || raw[5] == '.' || raw[5] == ':')) {
        return true; // HH:MM(:SS...) wird zu Time
    }
    if (raw.size() == 10 && raw[4] == '-' && raw[7] == '-' && allDigits(raw, 0, 4) && allDigits(raw, 5, 7) && allDigits(raw, 8, 10)) {
        return true; // Date-Format
    }
    if (raw.size() > 10 && raw[4] == '-' && raw[7] == '-' && allDigits(raw, 0, 4) && allDigits(raw, 5, 7) && allDigits(raw, 8, 10) &&
        (raw[10] == 'T' || raw[10] == 't' || raw[10] == ' ')) {
        return true; // DateTime-Format
    }
    if (looksLikeIntLiteral(raw)) return true;
    if (looksLikeFloatLiteral(raw)) return true;
    return false;
}

// Strukturelle Sicherheit eines unquotierten Skalaren: erlaubte Zeichen,
// keine Indicator-/Flow-/Kommentar-Konflikte, keine Anführungszeichen.
static bool isPlainStructural(const pplib::String& s)
{
    if (s.isEmpty()) return false;
    const char* p = s.c_str();
    size_t n = s.size();
    if (p[0] == ' ' || p[0] == '\t' || p[n - 1] == ' ' || p[n - 1] == '\t') return false;
    switch (p[0]) {
    case '-':
    case '?':
    case ':':
    case '#':
    case ',':
    case '[':
    case ']':
    case '{':
    case '}':
    case '&':
    case '*':
    case '!':
    case '|':
    case '>':
    case '\'':
    case '"':
    case '%':
    case '@':
    case '`':
        // Reservierte Indikatoren am Anfang werden immer quotiert, damit der
        // Wert beim Lesen wieder als String ankommt.
        return false;
    default:
        break;
    }
    for (size_t i = 0; i < n; i++) {
        unsigned char c = static_cast<unsigned char>(p[i]);
        if (c < 32 || c == 127) return false;
        if (c >= 0x80) continue; // UTF-8 gilt als sicher
        if (p[i] == '"' || p[i] == '\'') return false;
        if (p[i] == ':' && (i + 1 == n || p[i + 1] == ' ' || p[i + 1] == '\t')) return false;
        if (p[i] == '#' && i > 0 && (p[i - 1] == ' ' || p[i - 1] == '\t')) return false;
    }
    return true;
}

// Keys werden beim Lesen immer als String interpretiert, daher ist nur die
// Struktur maßgeblich.
static bool isPlainSafeKey(const pplib::String& s)
{
    return isPlainStructural(s);
}

// Werte müssen zusätzlich beim Lesen wieder als String aufgelöst werden.
static bool isPlainSafeValue(const pplib::String& s)
{
    if (!isPlainStructural(s)) return false;
    std::string_view raw = s;
    return !resolvesToNonString(raw);
}

static void writeKey(pplib::FileObject& file, const pplib::String& key)
{
    if (isPlainSafeKey(key)) {
        file.fwrite(key.c_str(), 1, key.size());
    } else {
        writeQuotedString(file, key.c_str(), key.size());
    }
}

// Mehrzeilige Strings werden als literal block scalar |-(chomp) ausgegeben,
// sofern dabei keine Informationen verlorengehen.
static bool useBlockScalar(const pplib::String& s)
{
    if (!s.contains("\n")) return false;
    if (s.startsWith("\n")) return false;
    if (s.endsWith("\n")) return false;
    const char* p = s.c_str();
    size_t n = s.size();
    // Control-Zeichen (außer Zeilenumbruch) sind in block scalars nicht sicher
    // darstellbar -> quoted scalar.
    for (size_t i = 0; i < n; i++) {
        unsigned char c = static_cast<unsigned char>(p[i]);
        if (c < 32 && c != '\n') return false;
        if (c == 127) return false;
    }
    size_t lineStart = 0;
    for (size_t i = 0; i <= n; i++) {
        if (i == n || p[i] == '\n') {
            if (i > lineStart) {
                char last = p[i - 1];
                if (last == ' ' || last == '\t') return false;
            }
            if (i < n && i + 1 < n && (p[i + 1] == ' ' || p[i + 1] == '\t')) return false;
            lineStart = i + 1;
        }
    }
    return true;
}

static void writeBlockScalar(pplib::FileObject& file, const pplib::String& s, int indent, int level)
{
    file.fputs("|-");
    const char* p = s.c_str();
    size_t n = s.size();
    size_t lineStart = 0;
    for (size_t i = 0; i <= n; i++) {
        if (i == n || p[i] == '\n') {
            // Leere Zeilen bekommen kein Leerzeichen-Indent, aber ihren Umbruch
            if (i > lineStart) writeIndent(file, indent, level); else file.fputc('\n');
            file.fwrite(p + lineStart, 1, i - lineStart);
            lineStart = i + 1;
        }
    }
}

static void writeScalar(pplib::FileObject& file, const pplib::Variant& value)
{
    if (value.isString()) {
        const pplib::String& s = value.toString();
        if (isPlainSafeValue(s)) {
            file.fwrite(s.c_str(), 1, s.size());
        } else {
            writeQuotedString(file, s.c_str(), s.size());
        }
    } else if (value.isWideString()) {
        pplib::ByteArray ba = value.toWideString().toUtf8();
        writeQuotedString(file, reinterpret_cast<const char*>(ba.ptr()), ba.size());
    } else if (value.isInt64()) {
        char buf[32];
        auto [p, ec] = std::to_chars(buf, buf + sizeof(buf), value.toInt64());
        if (ec == std::errc()) {
            file.fwrite(buf, 1, p - buf);
        } else {
            file.putsf("%lld", static_cast<long long>(value.toInt64()));
        }
    } else if (value.isDouble()) {
        double d = value.toDouble();
        if (std::isnan(d)) {
            file.fputs(".nan");
        } else if (std::isinf(d)) {
            file.fputs(d > 0 ? ".inf" : "-.inf");
        } else {
            char buf[64];
            auto [p, ec] = std::to_chars(buf, buf + sizeof(buf), d);
            if (ec == std::errc()) {
                size_t len = static_cast<size_t>(p - buf);
                // Ganzzahlige Doubles bekommen ".0", damit sie beim Lesen wieder
                // ein Double und kein Integer werden.
                bool integral = true;
                for (size_t i = 0; i < len; i++) {
                    if (buf[i] == '.' || buf[i] == 'e' || buf[i] == 'E') { integral = false; break; }
                }
                if (integral) {
                    buf[len++] = '.';
                    buf[len++] = '0';
                }
                file.fwrite(buf, 1, len);
            } else {
                snprintf(buf, sizeof(buf), "%.17g", d);
                for (char* cp = buf; *cp; ++cp) {
                    if (*cp == ',') *cp = '.';
                }
                file.fputs(buf);
            }
        }
    } else if (value.isBool()) {
        file.fputs(value.toBool() ? "true" : "false");
    } else if (value.isNull()) {
        file.fputs("null");
    } else if (value.isByteArrayPtr() || value.isByteArray()) {
        const pplib::ByteArrayPtr& ba = value.toByteArrayPtr();
        pplib::String str = ba.toBase64();
        file.fputs("!!binary ");
        // Ein leeres !!binary ist ungültig (Parser wirft), daher quoted schreiben
        if (str.isEmpty()) {
            file.fputs("\"\"");
        } else {
            file.fwrite(str.c_str(), 1, str.size());
        }
    } else if (value.isDateTime()) {
        pplib::String str = value.toDateTime().getISO8601withUsec();
        file.fputs("!!timestamp ");
        file.fwrite(str.c_str(), 1, str.size());
    } else if (value.isDate()) {
        pplib::String str = value.toDate().toString();
        file.fputs("!!timestamp ");
        file.fwrite(str.c_str(), 1, str.size());
    } else if (value.isTime()) {
        pplib::String str = value.toTime().toString();
        file.fputs("!Time ");
        file.fwrite(str.c_str(), 1, str.size());
    } else if (value.isTimeDelta()) {
        pplib::String str = value.toTimeDelta().toString();
        file.fputs("!TimeDelta ");
        file.fwrite(str.c_str(), 1, str.size());
    } else if (value.isTimeZone()) {
        pplib::String str = value.toTimeZone().toString();
        file.fputs("!TimeZone ");
        file.fwrite(str.c_str(), 1, str.size());
    } else {
        throw pplib::UnsupportedDataTypeException("Variant Type >>%d<< cannot be serialized to YAML", value.type());
    }
}

static void writeSpaces(pplib::FileObject& file, int count)
{
    for (int i = 0; i < count; ++i) file.fputc(' ');
}

static bool useBlockScalarValue(const pplib::Variant& value)
{
    return value.isString() && useBlockScalar(value.toString());
}

// Schreibt den Wert eines Mapping-Eintrags direkt nach dem ':'. Sammlungen
// beginnen auf der nächsten Zeile; Sequenzen bleiben dabei auf der gleichen
// Einrückung wie der Key (YAML-Sequenz-Style).
static void writeValueForKey(pplib::FileObject& file, const pplib::Variant& value, int indent, int level, int depth)
{
    if (value.isAssocArray()) {
        const pplib::AssocArray& dict = value.toAssocArray();
        if (dict.size() == 0) {
            file.fputs(" {}");
        } else {
            file.fputc('\n');
            writeDict(file, dict, indent, level + 1, depth + 1);
        }
    } else if (value.isVariantArray()) {
        const pplib::VariantArray& arr = value.toVariantArray();
        if (arr.size() == 0) {
            file.fputs(" []");
        } else {
            file.fputc('\n');
            writeArray(file, arr, indent, level, depth + 1);
        }
    } else if (value.isArray()) {
        const pplib::Array& arr = value.toArray();
        if (arr.size() == 0) {
            file.fputs(" []");
        } else {
            file.fputc('\n');
            writeClassicArray(file, arr, indent, level, depth + 1);
        }
    } else {
        file.fputc(' ');
        writeScalar(file, value);
    }
}

// Ein Mapping-Eintrag an der aktuellen Position (ohne vorangehenden Umbruch
// und ohne Einrückung).
static void writeMapEntry(pplib::FileObject& file, const pplib::String& key, const pplib::Variant& value,
                          int indent, int level, int depth)
{
    writeKey(file, key);
    file.fputs(":");
    if (useBlockScalarValue(value)) {
        file.fputc(' ');
        writeBlockScalar(file, value.toString(), indent, level + 1);
    } else {
        writeValueForKey(file, value, indent, level, depth);
    }
}

// Besteht ein Mapping nur aus Skalaren und leeren Sammlungen, kann es inline
// hinter einem Sequenz-Indicator stehen ("- key: value").
static bool isInlineableDict(const pplib::AssocArray& dict)
{
    for (auto it = dict.begin(); it != dict.end(); ++it) {
        const pplib::Variant& v = *(*it).second;
        if (v.isAssocArray() && v.toAssocArray().size() > 0) return false;
        if (v.isVariantArray() && v.toVariantArray().size() > 0) return false;
        if (v.isArray() && v.toArray().size() > 0) return false;
    }
    return true;
}

static void writeDict(pplib::FileObject& file, const pplib::AssocArray& data, int indent, int level, int depth)
{
    if (depth > MAX_YAML_DEPTH) {
        throw pplib::OverflowException("YAML nesting depth exceeded maximum limit (%d)", MAX_YAML_DEPTH);
    }
    bool first = true;
    for (auto it = data.begin(); it != data.end(); ++it) {
        if (!first) file.fputc('\n');
        first = false;
        writeSpaces(file, level * indent);
        writeMapEntry(file, (*it).first, *(*it).second, indent, level, depth);
    }
}

static void writeArray(pplib::FileObject& file, const pplib::VariantArray& data, int indent, int level, int depth)
{
    if (depth > MAX_YAML_DEPTH) {
        throw pplib::OverflowException("YAML nesting depth exceeded maximum limit (%d)", MAX_YAML_DEPTH);
    }
    const int contColumn = level * indent + 2; // Spalte der Keys eines inline Mappings
    for (size_t i = 0; i < data.size(); ++i) {
        if (i > 0) file.fputc('\n');
        writeSpaces(file, level * indent);
        file.fputc('-');
        const pplib::Variant& v = data[i];
        if (v.isAssocArray()) {
            const pplib::AssocArray& dict = v.toAssocArray();
            if (dict.size() == 0) {
                file.fputs(" {}");
            } else if (isInlineableDict(dict)) {
                bool first = true;
                for (auto it = dict.begin(); it != dict.end(); ++it) {
                    if (!first) {
                        file.fputc('\n');
                        writeSpaces(file, contColumn);
                    } else {
                        file.fputc(' ');
                    }
                    first = false;
                    writeMapEntry(file, (*it).first, *(*it).second, indent, level, depth + 1);
                }
            } else {
                // Nicht inlinefaehig: Mapping beginnt in der naechsten Zeile
                file.fputc('\n');
                writeDict(file, dict, indent, level + 1, depth + 1);
            }
        } else if (v.isVariantArray() || v.isArray()) {
            const bool empty = (v.isVariantArray() && v.toVariantArray().size() == 0) || (v.isArray() && v.toArray().size() == 0);
            if (empty) {
                file.fputs(" []");
            } else {
                file.fputc('\n');
                if (v.isVariantArray()) writeArray(file, v.toVariantArray(), indent, level + 1, depth + 1);
                else writeClassicArray(file, v.toArray(), indent, level + 1, depth + 1);
            }
        } else if (useBlockScalarValue(v)) {
            file.fputc(' ');
            writeBlockScalar(file, v.toString(), indent, level + 1);
        } else {
            file.fputc(' ');
            writeScalar(file, v);
        }
    }
}

static void writeClassicArray(pplib::FileObject& file, const pplib::Array& data, int indent, int level, int depth)
{
    if (depth > MAX_YAML_DEPTH) {
        throw pplib::OverflowException("YAML nesting depth exceeded maximum limit (%d)", MAX_YAML_DEPTH);
    }
    for (size_t i = 0; i < data.size(); ++i) {
        if (i > 0) file.fputc('\n');
        writeSpaces(file, level * indent);
        file.fputs("- ");
        const pplib::String& s = data.get(i);
        if (isPlainSafeValue(s)) {
            file.fwrite(s.c_str(), 1, s.size());
        } else {
            writeQuotedString(file, s.c_str(), s.size());
        }
    }
}

// ============================================================================
// Public Parse-Methoden
// ============================================================================

void Yaml::load(pplib::Variant& data, pplib::FileObject& file)
{
    data = pplib::Variant();
    YamlCtx ctx;
    YamlLineReader reader(file);
    YamlParser parser{reader, ctx};
    data = parser.parseDocument();
}

void Yaml::loads(pplib::Variant& data, const pplib::String& yaml)
{
    if (yaml.isEmpty()) throw pplib::UnexpectedEndOfDataException();
    pplib::MemFile file(const_cast<char*>(yaml.c_str()), yaml.size(), false);
    load(data, file);
}

pplib::Variant Yaml::loads(const pplib::String& yaml)
{
    pplib::Variant result;
    loads(result, yaml);
    return result;
}

pplib::Variant Yaml::load(pplib::FileObject& file)
{
    pplib::Variant result;
    load(result, file);
    return result;
}

void Yaml::load(pplib::AssocArray& data, pplib::FileObject& file)
{
    pplib::Variant v;
    load(v, file);
    if (!v.isAssocArray()) {
        throw pplib::TypeConversionException("YAML root is not an AssocArray (type %d)", v.type());
    }
    data = std::move(v.toAssocArray());
}

void Yaml::loads(pplib::AssocArray& data, const pplib::String& yaml)
{
    if (yaml.isEmpty()) throw pplib::UnexpectedEndOfDataException();
    pplib::MemFile file(const_cast<char*>(yaml.c_str()), yaml.size(), false);
    load(data, file);
}

void Yaml::load(pplib::VariantArray& data, pplib::FileObject& file)
{
    pplib::Variant v;
    load(v, file);
    if (!v.isVariantArray()) {
        throw pplib::TypeConversionException("YAML root is not a VariantArray (type %d)", v.type());
    }
    data = std::move(v.toVariantArray());
}

void Yaml::loads(pplib::VariantArray& data, const pplib::String& yaml)
{
    if (yaml.isEmpty()) throw pplib::UnexpectedEndOfDataException();
    pplib::MemFile file(const_cast<char*>(yaml.c_str()), yaml.size(), false);
    load(data, file);
}

// ============================================================================
// Public Dump-Methoden
// ============================================================================

void Yaml::dump(pplib::FileObject& file, const pplib::Variant& data, int indent)
{
    if (data.isAssocArray()) {
        const pplib::AssocArray& dict = data.toAssocArray();
        if (dict.size() > 0) writeDict(file, dict, indent, 0, 0);
        else file.fputs("{}");
    } else if (data.isVariantArray()) {
        const pplib::VariantArray& arr = data.toVariantArray();
        if (arr.size() > 0) writeArray(file, arr, indent, 0, 0);
        else file.fputs("[]");
    } else if (data.isArray()) {
        const pplib::Array& arr = data.toArray();
        if (arr.size() > 0) writeClassicArray(file, arr, indent, 0, 0);
        else file.fputs("[]");
    } else {
        writeScalar(file, data);
    }
    file.fputc('\n');
}

void Yaml::dumps(pplib::String& yaml, const pplib::Variant& data, int indent)
{
    pplib::MemFile file;
    Yaml::dump(file, data, indent);
    size_t size = file.tell();
    yaml.set(reinterpret_cast<const char*>(file.map(0, size)), size);
}

pplib::String Yaml::dumps(const pplib::Variant& data, int indent)
{
    pplib::String result;
    Yaml::dumps(result, data, indent);
    return result;
}

void Yaml::dump(pplib::FileObject& file, const pplib::AssocArray& data, int indent)
{
    dump(file, pplib::Variant(data), indent);
}

void Yaml::dumps(pplib::String& yaml, const pplib::AssocArray& data, int indent)
{
    dumps(yaml, pplib::Variant(data), indent);
}

pplib::String Yaml::dumps(const pplib::AssocArray& data, int indent)
{
    return dumps(pplib::Variant(data), indent);
}

void Yaml::dump(pplib::FileObject& file, const pplib::VariantArray& data, int indent)
{
    dump(file, pplib::Variant(data), indent);
}

void Yaml::dumps(pplib::String& yaml, const pplib::VariantArray& data, int indent)
{
    dumps(yaml, pplib::Variant(data), indent);
}

pplib::String Yaml::dumps(const pplib::VariantArray& data, int indent)
{
    return dumps(pplib::Variant(data), indent);
}

} // end of namespace pplib
