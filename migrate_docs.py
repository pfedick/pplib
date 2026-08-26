#!/usr/bin/env python3
# -*- coding: utf-8 -*-
r"""
migrate_docs.py - Verschiebt Doxygen-Dokumentationen aus C++-Quellcodedateien (.cpp)
in die zugehoerigen Headerdateien (.h).

Verhalten:
  * Doxygen-Kommentare, die direkt UBER oder UNTER einer Klassenmethodendefinition
    stehen, werden aus der Quellcodedatei entfernt und in der Headerdatei direkt
    UBER der Methodendeklaration eingefuegt.
  * Kommentare zu Klassen (\class / @class), Member-Variablen oder frei stehende
    Funktionen bleiben unveraendert.
  * Falls in der Headerdatei bereits ein Doxygen-Kommentar ueber der Deklaration
    existiert, wird dieser NICHT ueberschrieben; der Kommentar aus der
    Quellcodedatei wird trotzdem geloescht.

Syntax-Transformation des Dokukommentars:
  * Einleitung immer im Format "/** @<Tag> ..."
  * Alle Doxygen-Tags beginnen mit "@" statt "\"
  * Der propraetäre Tag "\desc" wird komplett entfernt (inklusive Zeilenumbruch)
  * Der Tag "\par" / "@par" wird entfernt und durch eine Leerzeile ersetzt

Aufruf:
    python3 migrate_docs.py <Quellcodedatei> <Headerdatei> [--dry-run]

Temporaere Dateien (Backup, Diff) werden unter ./tmp abgelegt.
"""

import argparse
import difflib
import os
import re
import shutil
import sys


# --------------------------------------------------------------------------- #
#  Kleine Hilfsfunktionen fuer Kommentarzeilen
# --------------------------------------------------------------------------- #

def _strip_comment(line):
    """Entfernt das Kommentar-Praefix (/*, /*!, /**, * usw.) und Rueckrueckraum."""
    s = re.sub(r'^\s*/?\*+!?', '', line).lstrip()
    return s.rstrip()


def _inner_content(line):
    """Inhalt einer Kommentarzeile ohne Praefix und ohne schliessendes */."""
    idx = line.find('*/')
    if idx != -1:
        line = line[:idx]
    return _strip_comment(line)


def _first_tag(line):
    """Gibt den ersten Doxygen-Tag der Kommentarzeile zurueck (ohne \\ / @)."""
    m = re.match(r'^\s*[*]?\s*[\\@]([A-Za-z][A-Za-z0-9]*)', line)
    if not m:
        return None
    return m.group(1)


def _is_tag_line(line):
    return _first_tag(line) is not None


# --------------------------------------------------------------------------- #
#  Normalisierung von Doxygen-Kommentaren
# --------------------------------------------------------------------------- #

def format_doc(raw_lines):
    r"""
    Wandelt einen rohen Doxygen-Kommentar (Liste von Zeilen, inkl. /*- und */-Zeile)
    in die Ziel-Syntax um:

        /** @<Tag> ...
         * ...
         */

    Regeln:
      * Einleitung immer "/** @..."
      * Alle Doxygen-Tags beginnen mit "@" statt "\"
      * "\desc" wird komplett entfernt (inklusive Zeilenumbruch); der folgende
        Text wird als normaler Fliesstext uebernommen. Leerzeilen innerhalb des
        \desc-Blocks werden verworfen.
      * "\par" / "@par" wird durch eine Leerzeile ersetzt.

    Rueckgabe: Liste von Zeilen OHNE Einrueckung, z.B.
        ['/** @brief ...', ' *', ' * Text', ' */']
    """
    contents = [_inner_content(l) for l in raw_lines]

    out = []
    skip_desc = False   # True, waehrend wir einen \desc-Block ueberspringen
    for s in contents:
        if s == '':     # leere Kommentarzeile (im \desc-Block verworfen)
            if not skip_desc:
                out.append('')
            continue

        tag = _first_tag(s)

        if skip_desc:
            if tag is None:
                out.append(s)          # Fliesstext nach \desc uebernehmen
                continue
            skip_desc = False          # naechster echter Tag beendet den Block

        if tag == 'desc':
            skip_desc = True           # kompletten \desc-Block verwerfen
            continue

        if tag == 'par':
            out.append('')             # Leerzeile hinterlassen
            continue

        s = re.sub(r'\\([A-Za-z][A-Za-z0-9]*)', lambda m: '@' + m.group(1), s)
        out.append(s)

    # Leerzeilen am Anfang und Ende des Kommentars entfernen
    while out and out[0] == '':
        out.pop(0)
    while out and out[-1] == '':
        out.pop()

    if not out:
        return None

    result = []
    for i, l in enumerate(out):
        if i == 0:
            result.append('/** ' + l)
        elif l == '':
            result.append(' *')
        else:
            result.append(' * ' + l)
    result.append(' */')
    return result


# --------------------------------------------------------------------------- #
#  Parameter-Normalisierung fuer das Matching von Definition/Deklaration
# --------------------------------------------------------------------------- #

def norm_params(params):
    """Normalisiert eine Parameterliste zum Vergleich (ohne Namen/Defaultwerte)."""
    params = re.sub(r'\bconst\b', ' ', params)
    params = re.sub(r'\bvolatile\b', ' ', params)
    params = re.sub(r'\bnoexcept\b', ' ', params)

    # Default-Werte entfernen (nur auf oberster Ebene, d.h. nicht in <...> / (...) / [...])
    depth = 0
    cut = None
    for i, c in enumerate(params):
        if c in '<([':
            depth += 1
        elif c in '>)]':
            depth -= 1
        elif c == '=' and depth == 0:
            cut = i
            break
    if cut is not None:
        params = params[:cut]

    # Parameter einzeln aufteilen (nur Kommas auf oberster Ebene)
    parts, buf, depth = [], '', 0
    for c in params:
        if c in '<([':
            depth += 1
        elif c in '>)]':
            depth -= 1
        if c == ',' and depth == 0:
            parts.append(buf)
            buf = ''
        else:
            buf += c
    if buf.strip():
        parts.append(buf)

    norm = []
    for p in parts:
        p = re.sub(r'\bstatic\b', ' ', p)
        p = re.sub(r'\binline\b', ' ', p)
        p = re.sub(r'\bconstexpr\b', ' ', p)
        p = re.sub(r'\bregister\b', ' ', p)
        # letzen Bezeichner (Parametername) entfernen
        p = re.sub(r'\s+[A-Za-z_]\w*\s*$', '', p.strip())
        p = re.sub(r'&{2,}', '&', p)
        p = re.sub(r'\*{2,}', '*', p)
        p = re.sub(r'\s+', ' ', p).strip()
        norm.append(p)

    return '|'.join(norm)


def norm_name(name):
    """Methode normalisieren (Leerzeichen entfernen, z.B. "operator =" -> "operator=")."""
    return re.sub(r'\s+', '', name)


# --------------------------------------------------------------------------- #
#  Gemeinsamer Parser: von einer Zeile mit "name(" bis zum schliessenden ")"
# --------------------------------------------------------------------------- #

def _collect_until_paren_close(lines, i):
    """
    lines[i] enthaelt das oeffnende "(" der gesuchten Signatur. Liefert
    (buf, j), wobei buf vom Anfang von lines[i] bis zum ENDE der Zeile reicht,
    in der das passende ")" steht (inklusive des Texts nach dem ")", z.B. ";"
    oder "const"). Bei Fehlschlag (None, None).
    """
    n = len(lines)
    depth = 0
    started = False
    done = False
    buf = ''
    j = i
    while j < n and not done:
        for c in lines[j]:
            if not done:
                if c == '(':
                    depth += 1
                    started = True
                elif c == ')':
                    depth -= 1
                    if started and depth == 0:
                        done = True
            buf += c
        if not done:
            buf += '\n'
            j += 1
    if not (started and depth == 0):
        return None, None
    return buf, j


def _tail_after_paren(buf, lines, j):
    """Text nach dem schliessenden ")" bis zum naechsten ";" oder "{"."""
    tail = buf[buf.rfind(')') + 1:]
    n = len(lines)
    k = j
    while (';' not in tail and '{' not in tail) and k + 1 < n:
        k += 1
        tail += '\n' + lines[k]
    return tail


# --------------------------------------------------------------------------- #
#  Quellcodedatei: Doxygen-Kommentarbloecke und Methodendefinitionen finden
# --------------------------------------------------------------------------- #

def find_doc_blocks(lines):
    """
    Findet alle Doxygen-Kommentarbloecke in der Datei.
    Rueckgabe: Liste von (start, end, raw_lines), Zeilennummern 0-basiert, end inkl.
    Ein Block gilt nur als Doku, wenn er mindestens ein Doxygen-Tag enthaelt.
    """
    blocks = []
    i = 0
    n = len(lines)
    while i < n:
        stripped = lines[i].strip()
        if re.match(r'^/\*+!?', stripped):
            start = i
            j = i
            found_close = False
            while j < n:
                if '*/' in lines[j]:
                    found_close = True
                    break
                j += 1
            if not found_close:
                i += 1
                continue
            raw = lines[start:j + 1]
            if any(_is_tag_line(l) for l in raw):
                blocks.append((start, j, raw))
            i = j + 1
        else:
            i += 1
    return blocks


_SRC_DEF_RE = re.compile(
    r'^\s*[\w:<>,\*&\s]*?::'
    r'(~?[A-Za-z_]\w*|operator\s*[<>=!+\-*/%&|^~\[\]]+)\s*\(')


def find_source_defs(lines):
    """
    Findet alle Klassenmethodendefinitionen im Format  Typ Klassename::Methode( ... )
    Rueckgabe: Liste von (zeilen_nr, methode, norm_params, trailing_const),
    Zeilennummern 0-basiert.
    """
    defs = []
    for i, line in enumerate(lines):
        if not _SRC_DEF_RE.match(line):
            continue
        buf, j = _collect_until_paren_close(lines, i)
        if buf is None:
            continue
        pstart = buf.index('(') + 1
        pend = buf.rfind(')')
        paramstr = buf[pstart:pend]
        tail = _tail_after_paren(buf, lines, j)
        cut = re.search(r'[;{]', tail)
        tail_sig = tail[:cut.start()] if cut else tail
        trailing_const = bool(re.search(r'\bconst\b', tail_sig))
        m = _SRC_DEF_RE.match(line)
        defs.append((i, norm_name(m.group(1)), norm_params(paramstr), trailing_const))
    return defs


# --------------------------------------------------------------------------- #
#  Headerdatei: Deklarationen finden und vorhandene Kommentare pruefen
# --------------------------------------------------------------------------- #

_HDR_DECL_RE = re.compile(
    r'^\s*(?:[\w:<>,\*&\s]+?\b)?'
    r'(~?[A-Za-z_]\w*|operator\s*[<>=!+\-*/%&|^~\[\]]+)\s*\(')


def find_header_decls(lines):
    """
    Findet Methodendeklarationen in der Headerdatei (Ende mit ";", kein "{...}"-Body).
    Rueckgabe: Liste von (zeilen_nr, methode, norm_params, trailing_const),
    Zeilennummern 0-basiert.

    Ausgeschlossen werden:
      * Inline-Methoden mit "{...}"-Body
      * Member-Aufrufketten wie  createTree(key)->set(value);   (enthalt " -> ")
    """
    decls = []
    n = len(lines)
    for i, line in enumerate(lines):
        if '{' in line or '}' in line:
            continue
        m = _HDR_DECL_RE.match(line)
        if not m or '(' not in line:
            continue
        buf, j = _collect_until_paren_close(lines, i)
        if buf is None:
            continue
        tail = _tail_after_paren(buf, lines, j)
        semi = tail.find(';')
        brace = tail.find('{')
        # Inline-Definition (Body "{" vor ";") -> keine reine Deklaration
        if brace != -1 and (semi == -1 or brace < semi):
            continue
        stmt_end = semi if semi != -1 else len(tail)
        stmt = buf + tail[:stmt_end]
        # Member-Aufrufkette wie createTree(key)->set(value); -> keine Deklaration
        if '->' in stmt:
            continue
        pstart = buf.index('(') + 1
        pend = buf.rfind(')')
        paramstr = buf[pstart:pend]
        trailing_const = bool(re.search(r'\bconst\b', tail[:stmt_end]))
        decls.append((i, norm_name(m.group(1)), norm_params(paramstr), trailing_const))
    return decls


def has_existing_doc_above(lines, idx):
    """True, wenn direkt ueber Zeile idx (leere Zeilen werden ignoriert) ein
    Doxygen-Kommentarblock mit mindestens einem Tag steht."""
    k = idx - 1
    for _ in range(30):
        if k < 0:
            return False
        s = lines[k].strip()
        if s == '':
            k -= 1
            continue
        if s.startswith('/*'):
            b = k
            while b >= 0 and '*/' not in lines[b]:
                b -= 1
            raw = lines[max(0, b):k + 1]
            return any(_is_tag_line(l) for l in raw)
        return False
    return False


def indent_of(line):
    return len(line) - len(line.lstrip())


# --------------------------------------------------------------------------- #
#  Hauptlogik
# --------------------------------------------------------------------------- #

def migrate(src_path, hdr_path, dry_run=False):
    with open(src_path, 'r', encoding='utf-8') as f:
        src_lines = f.read().splitlines()
    with open(hdr_path, 'r', encoding='utf-8') as f:
        hdr_lines = f.read().splitlines()

    # --- 1. Doxygen-Kommentare in der Quellcodedatei sammeln -------------- #
    doc_blocks = find_doc_blocks(src_lines)

    # --- 2. Methodendefinitionen in der Quellcodedatei -------------------- #
    src_defs = find_source_defs(src_lines)
    def_by_line = {d[0]: (d[1], d[2], d[3]) for d in src_defs}

    matched_docs = {}   # (name, norm_params, trailing_const) -> formatierter Kommentar
    delete_ranges = []  # Zeilenbereiche in src, die geloescht werden

    for b_start, b_end, raw in doc_blocks:
        # naechste nicht-leere Zeile nach dem Block
        nxt = b_end + 1
        while nxt < len(src_lines) and src_lines[nxt].strip() == '':
            nxt += 1
        # vorherige nicht-leere Zeile vor dem Block
        prv = b_start - 1
        while prv >= 0 and src_lines[prv].strip() == '':
            prv -= 1

        def_key = None
        if nxt < len(src_lines) and nxt in def_by_line:
            def_key = def_by_line[nxt]
        elif prv in def_by_line:
            def_key = def_by_line[prv]

        if def_key is None:
            # keine Methodendefinition gefunden -> ignorieren (Klasse/Member/etc.)
            continue

        formatted = format_doc(raw)
        if formatted is None:
            delete_ranges.append((b_start, b_end))
            continue

        matched_docs[def_key] = formatted
        delete_ranges.append((b_start, b_end))

    # Quellcodedatei: Kommentare loeschen (inkl. einer direkt folgenden Leerzeile)
    new_src = list(src_lines)
    for b_start, b_end in sorted(delete_ranges, reverse=True):
            end = b_end
            if end + 1 < len(new_src) and new_src[end + 1].strip() == '':
                end += 1
            del new_src[b_start:end + 1]

    # --- 3. Headerdatei: Deklarationen finden und Kommentare einfuegen ---- #
    hdr_decls = find_header_decls(hdr_lines)

    insertions = []   # (zeilen_nr, [comment lines])
    for key, formatted in matched_docs.items():
        positions = [d[0] for d in hdr_decls if (d[1], d[2], d[3]) == key]
        if not positions:
            continue
        # aufsteigend sortierte Gruppen direkt aufeinanderfolgender Deklarationen
        groups = []
        for p in sorted(positions):
            if groups and p == groups[-1][1] + 1:
                groups[-1][1] = p
            else:
                groups.append([p, p])
        # Einfuegen vor der ersten Deklaration der letzten Gruppe
        d_idx = groups[-1][0]
        if has_existing_doc_above(hdr_lines, d_idx):
            continue  # existierender Kommentar wird NICHT ueberschrieben
        indent = ' ' * indent_of(hdr_lines[d_idx])
        block = [indent + l for l in formatted]
        # Leerzeile vor dem Kommentarblock sicherstellen (außer Datei-Anfang)
        if d_idx > 0 and hdr_lines[d_idx - 1].strip() != '':
            block.insert(0, '')
        insertions.append((d_idx, block))

    # Einfuegen (von unten nach oben, damit Indexe gueltig bleiben)
    new_hdr = list(hdr_lines)
    for d_idx, block in sorted(insertions, reverse=True):
        new_hdr[d_idx:d_idx] = block

    # --- 4. Ausgabe ------------------------------------------------------- #
    src_text = '\n'.join(new_src) + '\n'
    hdr_text = '\n'.join(new_hdr) + '\n'

    if dry_run:
        print('--- Dry-Run: Quellcodedatei (Aenderungen mit +/- markiert) ---')
        for line in difflib.unified_diff(src_lines, new_src, lineterm=''):
            print(line)
        print('--- Dry-Run: Headerdatei (Aenderungen mit +/- markiert) ---')
        for line in difflib.unified_diff(hdr_lines, new_hdr, lineterm=''):
            print(line)
        return

    # Backup der Originaldateien unter tmp/
    os.makedirs('tmp', exist_ok=True)
    base_src = os.path.basename(src_path)
    base_hdr = os.path.basename(hdr_path)
    shutil.copy2(src_path, os.path.join('tmp', base_src + '.bak'))
    shutil.copy2(hdr_path, os.path.join('tmp', base_hdr + '.bak'))

    with open(src_path, 'w', encoding='utf-8') as f:
        f.write(src_text)
    with open(hdr_path, 'w', encoding='utf-8') as f:
        f.write(hdr_text)

    # Diff unter tmp/ ablegen
    with open(os.path.join('tmp', base_src + '.diff'), 'w', encoding='utf-8') as f:
        f.write('\n'.join(difflib.unified_diff(
            src_lines, new_src, fromfile='a/' + base_src, tofile='b/' + base_src)))
    with open(os.path.join('tmp', base_hdr + '.diff'), 'w', encoding='utf-8') as f:
        f.write('\n'.join(difflib.unified_diff(
            hdr_lines, new_hdr, fromfile='a/' + base_hdr, tofile='b/' + base_hdr)))

    print('Fertig.')
    print('  Quellcode : %s (Backup: tmp/%s.bak)' % (src_path, base_src))
    print('  Header    : %s (Backup: tmp/%s.bak)' % (hdr_path, base_hdr))
    print('  Diff      : tmp/%s.diff, tmp/%s.diff' % (base_src, base_hdr))


def main():
    parser = argparse.ArgumentParser(
        description='Verschiebt Doxygen-Dokumentationen aus einer C++-Quellcodedatei '
                    'in die zugehoerige Headerdatei.')
    parser.add_argument('source', help='Pfad zur Quellcodedatei (.cpp)')
    parser.add_argument('header', help='Pfad zur Headerdatei (.h)')
    parser.add_argument('--dry-run', action='store_true',
                        help='Nur anzeigen, was geaendert wuerde (Dateien bleiben unveraendert)')
    args = parser.parse_args()

    if not os.path.isfile(args.source):
        sys.exit('Quellcodedatei nicht gefunden: %s' % args.source)
    if not os.path.isfile(args.header):
        sys.exit('Headerdatei nicht gefunden: %s' % args.header)

    migrate(args.source, args.header, dry_run=args.dry_run)


if __name__ == '__main__':
    main()
