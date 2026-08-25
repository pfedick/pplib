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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// #include <set>
// #include <ostream>

#include <pplib/types/array.h>
#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/assocarray.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/bytearrayptr.h>
#include "pplib/types/datetime.h"
#include "pplib/types/date.h"
#include "pplib/types/time.h"
#include "pplib/types/timedelta.h"
#include "pplib/types/timezone.h"
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>
#include <pplib/core/iconv.h>

#include <config_pplib.h>

namespace pplib
{

/*!\brief Konstruktor des Assoziativen Arrays
 *
 * \desc
 * Initialisiert die Instanz mit 0 und initialisiert den AVL-Baum.
 */
AssocArray::AssocArray()
{
    maxint = 0;
}

/*!\brief Copy-Konstruktor des Assoziativen Arrays
 *
 * \desc
 * Macht eine Kopie des Assoziativen Arrays \p other.
 *
 * \param[in] other Referenz auf zu kopierendes Assoziatives Array
 *
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 */
AssocArray::AssocArray(const AssocArray& other)
{
    maxint = 0;
    add(other);
}

AssocArray::AssocArray(AssocArray&& other)
{
    maxint = other.maxint;
    Tree = std::move(other.Tree);
    other.maxint = 0;
    other.Tree.clear();
}

/*!\brief Destruktor der Klasse
 *
 * \desc
 * Der Destruktor ruft die Funktion AssocArray::clear auf, um alle vorhandenen Elemente zu
 * löschen.
 */
AssocArray::~AssocArray()
{
    clear();
}

/*!\brief Inhalt des Arrays löschen
 *
 * \desc
 * Mit dieser Funktion wird der komplette Inhalt des Arrays gelöscht. Dabei der Destruktor für jedes
 * vorhandene Element aufgerufen, der wiederum sicherstellt, dass die darin enthaltenen Daten
 * ordnungsgemäß gelöscht werden.
 *
 */
void AssocArray::clear()
{
    iterator it;
    for (it = Tree.begin(); it != Tree.end(); ++it) {
        delete (*it).second;
    }
    Tree.clear();
    maxint = 0;
}

/*!\brief Interne Funktion zum Suchen eines Elements
 *
 * \desc
 * Diese Funktion zerlegt den angegebenen Schlüssel (\p key) in seine einzelnen Elemente.
 * Als Trennzeichen wird wie bei einer Unix-Pfadangabe der Slash (/) verwendet. Die Funktion
 * sucht zunächst nach dem erste Element des Schlüssels im eigenen Baum. Ist dies vorhanden
 * und handelt es sich bei dessen Datentyp wieder um ein AssocArray, wird deren
 * findInternal-Funktion mit dem restlichen Schlüssel rekursiv aufgerufen. Dies geschieht
 * solange, bis das letzte Element des Keys gefunden wurde.
 *
 * \param[in] key String mit dem gesuchten Schlüssel
 * \return Konnte der Schlüssel gefunden werden, wir der Pointer auf das Element (Variant)
 * zurückgegeben. Wurde der Schlüssel nicht gefunden, wird NULL zurückgegeben
 * \exception InvalidKeyException: Wird geworfen, wenn der Schlüssel ungültig oder leer ist
 * \note
 * Die Funktion wird von allen Get...- und Concat-Funktionen verwendet.
 */
Variant* AssocArray::findInternal(const String& key) const
{
    // printf ("AssocArray::findInternal (key=%ls)\n",(const wchar_t*)key);
    Array tok(key, "/", 0, true);
    if (tok.count() == 0) throw InvalidKeyException(key);
    String firstkey = tok.shift();
    String rest = tok.implode("/");

    const_iterator it = Tree.find(firstkey);
    if (it == Tree.end()) return NULL;
    // Ist noch was im Pfad rest?
    if (tok.count() > 0) { // Ja, koennen wir iterieren?
        if (it->second != NULL && it->second->isAssocArray()) {
            return it->second->toAssocArray().findInternal(rest);
        } else {
            return NULL;
        }
    }
    return it->second;
}

/*!\brief Interne Funktion, die ein Element im Baum sucht oder anlegt
 *
 * \desc
 * Diese Funktion durchsucht den Baum nach dem gewünschten Element. Ist es vorhanden, wird dessen Pointer
 * zurückgeliefert, wenn nicht, wird es angelegt, jedoch ohne Daten. Bei verschachtelten Schlüsseln wird
 * die Funktion rekursiv aufgerufen, bis das letzte Element erreicht ist. Die Funktion wird intern von allen
 * Funktionen verwendet, die Daten in das Array speichern.
 *
 * \param[in] key Pointer auf den Namen des Schlüssels
 * \param[in] var Pointer auf die Daten, die unter diesem Schlüssel abgelegt werden sollen
 * \return Bei Erfolg liefert die Funktion einen Pointer auf das gewünschte Element zurück.
 * Im Fehlerfall wird eine Exception geworfen.
 *
 * \exception InvalidKeyException: Wird geworfen, wenn der Schlüssel ungültig oder leer ist
 * \exception std::bad_alloc: Kein Speicher mehr frei
 *
 * \remarks Bei der Angabe eines verschachtelten Schlüssels kann es vorkommen, dass bereits vorhandene
 * Elemente überschrieben werden. Beispiel:
 *
 * Das Element <tt>ebene1/schlüssel1</tt> ist im Baum bereits vorhanden und beinhaltet einen String. Nun wird
 * das neue Element <tt>ebene1/schlüssel1/unterschlüssel1</tt> angelegt. Da Schlüssel eindeutig sein müssen,
 * wird der String <tt>ebene1/schlüssel1</tt> gelöscht und in ein Array umgewandelt.
 */
Variant* AssocArray::createTree(const String& key)
{
    // TODO: das sollte keine Exception werfen, sondern mit Returncode arbeiten!
    Array tok(key, "/", 0, true);
    if (tok.count() == 0) throw InvalidKeyException(key);
    String firstkey = tok.shift();
    String rest = tok.implode("/");
    // printf ("firstkey=%ls, rest=%ls\n",(const wchar_t *)firstkey,(const wchar_t *)rest);
    if (firstkey == "[]") {
        firstkey.setf("%llu", maxint);
        maxint++;
    }
    // Beginnt Firstkey mit einer Zahl?
    if (firstkey.isNumeric()) {
        uint64_t keyint = firstkey.toInt64();
        if (keyint >= maxint) maxint = keyint + 1;
    }

    iterator it = Tree.find(firstkey);
    if (it != Tree.end()) {
        // Ist noch was im Pfad rest?
        if (tok.count() > 0) { // Ja, koennen wir iterieren?
            if (it->second->isAssocArray() == false) {
                // Nein, wir loeschen daher diesen Zweig und machen ein Array draus
                it->second->set(pplib::AssocArray());
            }
            return it->second->toAssocArray().createTree(rest);
        }
        // Nein, wir haben die Zielposition gefunden
        it->second->clear();
        return it->second;
    }

    // Key ist nicht in diesem Array, wir legen ihn an

    // Ist noch was im Pfad rest?
    if (tok.count() > 0) { // Ja, wir erstellen ein Array und iterieren
        // printf ("Iteration\n");
        Variant* newnode = new Variant(pplib::AssocArray());
        Tree.insert(std::pair<String, Variant*>(firstkey, newnode));
        return newnode->toAssocArray().createTree(rest);
    } else {
        Variant* newnode = new Variant();
        Tree.insert(std::pair<String, Variant*>(firstkey, newnode));
        return newnode;
    }
}

/*!\brief Anzahl Schlüssel zählen
 *
 * \desc
 * Diese Funktion liefert die Anzahl Schlüssel auf dieser Ebene des Array zurück.
 *
 * \param[in] recursive Falls recursive auf true gesetzt wird die Funktion rekusriv für jeden
 * Schlüssel aufgerufen, dessen Wert ebenfalls ein Array ist.
 *
 * \returns Anzahl Schlüssel
 */
size_t AssocArray::count(bool recursive) const
{
    if (!recursive) return Tree.size();
    const_iterator it;
    size_t num = Tree.size();
    for (it = Tree.begin(); it != Tree.end(); ++it) {
        if (it->second->isAssocArray()) num += it->second->toAssocArray().count(recursive);
    }
    return num;
}

/*!\brief Anzahl Elemente
 *
 * \desc
 * Diese Funktion liefert die Anzahl Elemente auf dieser Ebene des Arrays zurück.
 *
 * \returns Anzahl Elemente
 */
size_t AssocArray::size() const
{
    return Tree.size();
}

/*!\brief Anzahl Schlüssel für ein bestimmtes Element zählen
 *
 * \desc
 * Diese Funktion liefert die Anzahl Schlüssel zurück, die in dem angegebenen Key enthalten sind.
 *
 * \param[in] key Schlüssel-Name eines Assoziativen Arrays innerhalb dieses Arrays
 * \param[in] recursive Falls recursive auf true gesetzt wird die Funktion rekusriv für jeden
 * Schlüssel aufgerufen, dessen Wert ebenfalls ein Array ist.
 *
 * \returns Anzahl Schlüssel
 */
size_t AssocArray::count(const String& key, bool recursive) const
{
    const Variant* p = findInternal(key);
    if (!p) return (size_t)0;
    if (p->isAssocArray()) return p->toAssocArray().count(recursive);
    return 1;
}

/*!\brief Inhalt des Arrays ausgeben
 *
 * \desc
 * Diese Funktion dient Debugging-Zwecken. Der Aufruf bewirkt, dass der Inhalt des kompletten Arrays auf
 * STDOUT ausgegeben wird.
 *
 * \param[in] prefix Optionaler Text, der bei der Ausgabe jedem Element vorangestellt wird
 *
 * \par Beispiel:
 * \code
pplib::AssocArray a;
pplib::Binary bin;
bin.load("main.cpp");

a.set("key1","value1");
a.set("array1/unterkey1","value2");
a.set("array1/unterkey2","value3");
a.set("array1/noch ein array/unterkey1","value4");
a.set("array1/unterkey2","value5");
a.set("key2","value6");
a.set("dateien/main.cpp",bin);
a.set("array2/unterkey1","value7");
a.set("array2/unterkey2","value8");
a.set("array2/unterkey1","value9");
a.list("prefix");
\endcode
    Ausgabe:
\code
prefix/array1/noch ein array/unterkey1=value4
prefix/array1/unterkey1=value2
prefix/array1/unterkey2=value5
prefix/array2/unterkey1=value9
prefix/array2/unterkey2=value8
prefix/dateien/main.cpp=Binary, 806 Bytes
prefix/key1=value1
prefix/key2=value6
\endcode
 *
 * \remarks Die Funktion gibt nur "lesbare" Element aus. Enthält das Array Pointer oder Binaries, wird das Element zwar
 * ausgegeben, jedoch werden als Wert nur Meta-Informationen ausgegeben (Datentyp, Pointer, Größe).
 */
void AssocArray::list(const String& prefix) const
{
    String key;
    String pre;
    if (prefix.notEmpty()) key = prefix + "/";

    const_iterator it;
    for (it = Tree.begin(); it != Tree.end(); ++it) {
        Variant* p = it->second;
        if (p->isString()) {
            PrintDebug("%s%s=%s\n", (const char*)key, (const char*)it->first, (const char*)p->toString().getPtr());
        } else if (p->isWideString()) {
            PrintDebug("%s%s=%ls\n", (const char*)key, (const char*)it->first, (const wchar_t*)p->toWideString().getPtr());
        } else if (p->isByteArray()) {
            PrintDebug("%s%s=ByteArray, %zu Bytes\n", (const char*)key, (const char*)it->first, p->toByteArray().size());
        } else if (p->isByteArrayPtr()) {
            PrintDebug("%s%s=ByteArrayPtr, %zu Bytes\n", (const char*)key, (const char*)it->first, p->toByteArrayPtr().size());
        } else if (p->isAssocArray()) {
            pre.setf("%s%s", (const char*)key, (const char*)it->first);
            p->toAssocArray().list(pre);
        } else if (p->isArray()) {
            const Array& a = (const Array&)*p;
            for (size_t i = 0; i < a.size(); i++) {
                PrintDebug("%s%s/Array(%zu)=%s\n", (const char*)key, (const char*)it->first, i, (const char*)a[i]);
            }
        } else if (p->isDateTime()) {
            PrintDebug("%s%s=DateTime %s\n", (const char*)key, (const char*)it->first, (const char*)p->toDateTime().getISO8601withMsec());
        } else {
            PrintDebug("%s%s=UnknownDataType Id=%i\n", (const char*)key, (const char*)it->first, p->type());
        }
    }
}
/*!\brief Formatierten String hinzufügen
 *
 * \desc
 * Diese Funktion fügt den Inhalt eines formatierten Strings dem Array hinzu.
 *
 * \param[in] key Name des Schlüssels
 * \param[in] fmt Pointer auf den Format-String des Wertes
 * \param[in] ... Beliebig viele Parameter, die vom Formatstring verwendet werden
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 */
void AssocArray::setf(const String& key, const char* fmt, ...)
{
    String value;
    va_list args;
    va_start(args, fmt);
    value.vasprintf(fmt, args);
    va_end(args);
    createTree(key)->set(value);
}

/*!\brief %String verlängern
 *
 * \desc
 * Diese Funktion fügt den Inhalt des Strings \p value an den bereits vorhandenen
 * Wert des Schlüssels \p key an. Falls der optionale Parameter \p concat einen Wert
 * enthält, wird dieser als Trennung zwischen bestehendem und neuem String verwendet.
 * War der Schlüssel bisher nicht vorhanden, wird ein neuer angelegt.
 *
 * \param[in] key Name des Schlüssels
 * \param[in] value Wert
 * \param[in] concat Trennzeichen (Optional, Default=keins)
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 * \exception TypeConversionException: Schlüssel ist bereits vorhanden, enthält aber keinen String
 */
void AssocArray::append(const String& key, const String& value, const String& concat)
{
    Variant* node = findInternal(key);
    if (!node) {
        set(key, value);
        return;
    }
    String& str = node->toString();
    if (concat.notEmpty()) str.append(concat);
    str.append(value);
}

/*!\brief %String mit Formatiertem String verlängern
 *
 * \desc
 * Diese Funktion erstellt zuerst einen neuen String anhand des Formatstrings
 * \p fmt und der zusätzlichen optionalen Parameter. Dieser wird an den bereits vorhandenen
 * Wert des Schlüssels \p key angehangen. Falls der optionale Parameter \p concat einen Wert
 * enthält, wird dieser als Trennung zwischen bestehendem und neuem String verwendet.
 * War der Schlüssel bisher nicht vorhanden, wird ein neuer angelegt.
 *
 * \param[in] key Name des Schlüssels
 * \param[in] concat Trennzeichen (Optional, Default=keins)
 * \param[in] fmt Formatstring
 * \param[in] ... Optionale Parameter
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 * \exception TypeConversionException: Schlüssel ist bereits vorhanden, enthält aber keinen String
 */
void AssocArray::appendf(const String& key, const String& concat, const char* fmt, ...)
{
    String var;
    va_list args;
    va_start(args, fmt);
    var.vasprintf(fmt, args);
    va_end(args);
    append(key, var, concat);
}

/*!\brief %AssocArray kopieren
 *
 * \desc
 * Mit dieser Funktion wird der komplette Inhalt des Assoziativen Arrays \p other
 * in dieses hineinkopiert. Das Array wird vorher nicht gelöscht, so dass vorhandene
 * Schlüssel erhalten bleiben. Gibt es in \p other jedoch gleichnamige Schlüssel,
 * werden die bisherigen Werte überschrieben.
 *
 * \param[in] a Das zu kopierende AssocArray
 *
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 */
void AssocArray::add(const AssocArray& other)
{
    const_iterator it;
    for (it = other.Tree.begin(); it != other.Tree.end(); ++it) {
        Variant* existing = findInternal(it->first);
        if (existing && existing->isAssocArray() && it->second->isAssocArray()) {
            existing->toAssocArray().add(it->second->toAssocArray());

        } else
            set(it->first, *it->second);
    }
}

/*!\brief Schlüssel auslesen
 *
 * \desc
 * Diese Funktion liefert den Wert des Schlüssels \p key als Variant zurück. Dieser kann
 * von der aufrufenden Anwendung in den jeweiligen Datentyp umgewandelt werden.
 *
 * @param key Name des Schlüssels
 * @return Referenz auf einen Variant mit dem Wert des Schlüssels
 * \exception InvalidKeyException: Ungültiger Schlüssel
 * \exception KeyNotFoundException: Schlüssel wurde nicht gefunden
 * \example
 * Der Variant kann z.B. folgendermaßen in einen String umgewandelt werden:
 * \code
pplib::String &str=a.get(L"key1").toString();
\endcode
 */
Variant& AssocArray::get(const String& key) const
{
    Variant* node = findInternal(key);
    if (!node) throw KeyNotFoundException(key);
    return (*node);
}

/*!\brief Schlüssel vorhanden
 *
 * \desc
 * Diese Funktion prüft, ob der Schlüssels \p key im Assoziativen Array enthalten ist.
 *
 * @param key Name des Schlüssels
 * @return Liefert \c true zurück, wenn der Schlüssel vorhanden ist, sonst \c false
 * \exception InvalidKeyException: Ungültiger Schlüssel
 */
bool AssocArray::exists(const String& key) const
{
    return findInternal(key) != nullptr;
}

/*!\brief String auslesen
 *
 * \desc
 * Diese Funktion liefert den Wert des Schlüssels \p key als String zurück, sofern
 * der Schlüssel auch tatsächlich einen String enthält.
 *
 * @param key Name des Schlüssels
 * @return Referenz auf einen String mit dem Wert des Schlüssels
 * \exception InvalidKeyException: Ungültiger Schlüssel
 * \exception KeyNotFoundException: Schlüssel wurde nicht gefunden
 */
String& AssocArray::getString(const String& key)
{
    Variant* node = findInternal(key);
    if (!node) throw KeyNotFoundException(key);
    if (!node->isString()) throw TypeConversionException("%s is not a String", (const char*)key);
    return node->toString();
}

const String& AssocArray::getString(const String& key) const
{
    Variant* node = findInternal(key);
    if (!node) throw KeyNotFoundException(key);
    if (!node->isString()) throw TypeConversionException("%s is not a String", (const char*)key);
    return node->toString();
}

String AssocArray::getString(const String& key, const String& default_value) const
{
    Variant* node = findInternal(key);
    if (!node) return default_value;
    if (node->isString()) return node->toString();
    return default_value;
}

bool AssocArray::getBoolean(const String& key, bool default_value) const
{
    Variant* node = findInternal(key);
    if (!node) return default_value;
    if (node->isString()) return node->toString().isTrue();
    if (node->isWideString()) return node->toWideString().isTrue();
    return default_value;
}

int AssocArray::getInt(const String& key, int default_value) const
{
    Variant* node = findInternal(key);
    if (!node) return default_value;
    if (node->isString()) return node->toString().toInt();
    if (node->isWideString()) return node->toWideString().toInt();
    return default_value;
}

int64_t AssocArray::getInt64t(const String& key, int64_t default_value) const
{
    Variant* node = findInternal(key);
    if (!node) return default_value;
    if (node->isString()) return node->toString().toInt64();
    if (node->isWideString()) return node->toWideString().toInt64();
    return default_value;
}

/*!\brief Key vorhanden und True
 *
 * \desc
 * Liefert True zurück, wenn der Schlüssel \b key vorhanden ist,
 * und dessen Value einen String oder WideString enthält, dessen
 * Boolean Wert True entspricht.
 *
 * @param key Name des Schlüssels
 * @return True oder False
 */
bool AssocArray::isTrue(const String& key) const
{
    Variant* node = findInternal(key);
    if (!node) return false;
    if (node->isString()) return node->toString().isTrue();
    if (node->isWideString()) return node->toWideString().isTrue();
    return false;
}

/*!\brief AssocArray auslesen
 *
 * \desc
 *
 * @param key Name des Schlüssels
 * @return Referenz auf einen String mit dem Wert des Schlüssels
 * \exception InvalidKeyException: Ungültiger Schlüssel
 * \exception KeyNotFoundException: Schlüssel wurde nicht gefunden
 */
AssocArray& AssocArray::getAssocArray(const String& key)
{
    Variant* node = findInternal(key);
    if (!node) throw KeyNotFoundException(key);
    if (!node->isAssocArray()) throw TypeConversionException("%s is not an AssocArray", (const char*)key);
    return node->toAssocArray();
}

const AssocArray& AssocArray::getAssocArray(const String& key) const
{
    Variant* node = findInternal(key);
    if (!node) throw KeyNotFoundException(key);
    if (!node->isAssocArray()) throw TypeConversionException("%s is not an AssocArray", (const char*)key);
    return node->toAssocArray();
}

Array& AssocArray::getArray(const String& key)
{
    Variant* node = findInternal(key);
    if (!node) throw KeyNotFoundException(key);
    if (!node->isArray()) throw TypeConversionException("%s is not an Array", (const char*)key);
    return node->toArray();
}

const Array& AssocArray::getArray(const String& key) const
{
    Variant* node = findInternal(key);
    if (!node) throw KeyNotFoundException(key);
    if (!node->isArray()) throw TypeConversionException("%s is not an Array", (const char*)key);
    return node->toArray();
}

/*!\brief Einzelnen Schlüssel löschen
 *
 * \desc
 * Mit dieser Funktion wird ein einzelner Schlüssel aus dem Array gelöscht.
 *
 * \param[in] key String mit dem Namen des zu löschenden Schlüssels
 *
 */
void AssocArray::erase(const String& key)
{
    Array tok(key, "/", 0, true);
    if (tok.count() == 0) throw InvalidKeyException(key);
    String firstkey = tok.shift();
    String rest = tok.implode("/");
    iterator it = Tree.find(firstkey);
    if (it == Tree.end()) return; // nothing to do
    // Ist noch was im Pfad rest?
    if (tok.count() > 0) { // Ja, koennen wir iterieren?
        if (it->second != NULL && it->second->isAssocArray()) {
            it->second->toAssocArray().erase(rest);
            return;
        } else {
            return;
        }
    }
    delete it->second;
    Tree.erase(it);
}

/*!\brief Einzelnen Schlüssel löschen
 *
 * \desc
 * Mit dieser Funktion wird ein einzelner Schlüssel aus dem Array gelöscht.
 *
 * \param[in] key String mit dem Namen des zu löschenden Schlüssels
 *
 */
void AssocArray::remove(const String& key)
{
    erase(key);
}

AssocArray::iterator AssocArray::begin()
{
    return Tree.begin();
}

AssocArray::const_iterator AssocArray::begin() const
{
    return Tree.begin();
}

AssocArray::iterator AssocArray::end()
{
    return Tree.end();
}

AssocArray::const_iterator AssocArray::end() const
{
    return Tree.end();
}

AssocArray::reverse_iterator AssocArray::rbegin()
{
    return Tree.rbegin();
}

AssocArray::const_reverse_iterator AssocArray::rbegin() const
{
    return Tree.rbegin();
}

AssocArray::reverse_iterator AssocArray::rend()
{
    return Tree.rend();
}

AssocArray::const_reverse_iterator AssocArray::rend() const
{
    return Tree.rend();
}

/*!\brief Liefert Anzahl Bytes, die für exportBinary erforderlich sind
 *
 * \desc
 * Diese Funktion liefert die Anzahl Bytes zurück, die für den Buffer der Funktion AssocArray::exportBinary
 * erforderlich sind. Es kann dadurch ein ausreichend großer Puffer vor Aufruf der Funktion exportBinary
 * angelegt werden.
 *
 * \return Anzahl Bytes oder 0 im Fehlerfall
 *
 * \see
 * - AssocArray::exportBinary
 * - AssocArray::importBinary
 */
size_t AssocArray::binarySize() const
{
    return exportBinary(NULL, 0);
}

/*!\brief Inhalt des Arrays in einem plattform-unabhängigen Binären-Format exportieren
 *
 * \desc
 * Mit dieser Funktion kann der komplette Inhalt des Arrays in einem plattform-unabhängigem binären Format abgelegt
 * werden, das sich zum Speichern in einer Datei oder zum Übertragen über das Internet eignet.
 *
 * \param[in] buffer Pointer auf einen ausreichend großen Puffer. Die Größe des benötigten Puffers
 *            kann zuvor mit der Funktion AssocArray::binarySize ermittelt werden. Wird als Buffer NULL
 *            übergeben, wird in der Variable \p realsize ebenfalls die Anzahl Bytes zurückgegeben
 * \param[in] buffersize Die Größe des Puffers in Bytes
 * \param[out] realsize In dieser Variable wird gespeichert, wieviele Bytes tatsächlich für den Export
 *            verwendet wurden
 * \exception ExportBufferToSmallException: Wird geworfen, wenn \p buffersize nicht groß genug ist, um
 * das Assoziative Array vollständig exportieren zu können.
 *
 * \attention
 * Es muss daran gedacht werden, dass nicht alle Datentypen exportiert werden können. Gegenwärtig
 * werden folgende Typen unterstützt:
 * - String (Wird als UTF-8 exportiert)
 * - Array
 * - AssocArray
 * - ByteArray
 * - ByteArrayPtr (wird in ein ByteArray umgewandelt!)
 * - DateTime
 * \see
 * - AssocArray::binarySize
 * - AssocArray::importBinary
 *
 * \note
 * Das exportierte Binary ist komptibel mit dem Assoziativen Array der PPL-Version 6
 */
size_t AssocArray::exportBinary(void* buffer, size_t buffersize) const
{
    char* ptr = (char*)buffer;
    size_t p = 0;
    size_t vallen = 0;
    String key;
    ByteArray ba;
    if (!buffer) buffersize = 0;
    if (p + 7 < buffersize) memcpy(ptr, "PPLASOC", 7);
    p += 7;
    AssocArray::const_iterator it;
    for (it = Tree.begin(); it != Tree.end(); ++it) {
        const Variant* a = it->second;
        if (p < buffersize) {
            if (a->isByteArrayPtr())
                PokeN8(ptr + p, Variant::TYPE_BYTEARRAY);
            else
                PokeN8(ptr + p, a->type());
        }
        p++;
        key = it->first;
        size_t keylen = key.size();
        if (p + 4 < buffersize) PokeN16(ptr + p, (int)keylen);
        p += 2;
        if (p + keylen < buffersize) strncpy(ptr + p, (const char*)key, (int)keylen);
        p += keylen;
        if (a->isString()) {
            String string = a->toString();
            vallen = string.size();
            if (p + 4 < buffersize) PokeN32(ptr + p, (int)vallen);
            p += 4;
            if (p + vallen < buffersize) strncpy(ptr + p, (const char*)string, vallen);
            p += vallen;
        } else if (a->isWideString()) {
            String string(a->toWideString()); // Konvertierung in UTF-8
            vallen = string.size();
            if (p + 4 < buffersize) PokeN32(ptr + p, (int)vallen);
            p += 4;
            if (p + vallen < buffersize) strncpy(ptr + p, (const char*)string, vallen);
            p += vallen;

        } else if (a->isAssocArray()) {
            if (!buffer)
                p += a->toAssocArray().exportBinary(NULL, 0);
            else
                p += a->toAssocArray().exportBinary(ptr + p, buffersize - p);
        } else if (a->isArray()) {
            pplib::Array aaa(a->toArray());
            if (p + 4 < buffersize) PokeN32(ptr + p, (int)aaa.size());
            p += 4;
            for (ssize_t i = 0; i < (ssize_t)aaa.size(); i++) {
                const String s = aaa.get(i);
                if (p + 4 < buffersize) PokeN32(ptr + p, (int)s.size());
                p += 4;
                vallen = s.size();
                if (p + vallen < buffersize) strncpy(ptr + p, (const char*)s, vallen);
                p += vallen;
            }
        } else if (a->isDateTime()) {
            const DateTime& dt = a->toDateTime();
            // DateTime könnte invalid sein
            if (dt.isEmpty()) {
                if (p + 4 < buffersize) PokeN32(ptr + p, 0);
                p += 4;
            } else {
                vallen = 10;                           // PPL8 speichert Microseconds in 8 und Zeitzone in 2 Bytes,
                vallen += dt.timeZone().name().size(); // plus die Länge des Zeitzonen-Namens, der aber leer sein kann.
                if (p + 4 < buffersize) PokeN32(ptr + p, (int)vallen);
                p += 4;
                if (p + vallen < buffersize) {
                    PokeN64(ptr + p, dt.toMicroseconds());
                    PokeN16(ptr + p + 8, dt.timeZone().offsetMinutes());
                    memcpy(ptr + p + 10, (const char*)dt.timeZone().name(), dt.timeZone().name().size());
                }
                p += vallen;
            }
        } else if (a->isByteArray() == true || a->isByteArrayPtr() == true) {
            vallen = a->toByteArray().size();
            if (p + 4 < buffersize) PokeN32(ptr + p, (int)vallen);
            p += 4;
            if (p + vallen < buffersize) memcpy(ptr + p, a->toByteArrayPtr().adr(), vallen);
            p += vallen;
        } else if (a->isDate() == true) {
            vallen = 4; // Date exportiert das Datum als 32Bit Integer (YYYYMMDD)
            if (p + 4 < buffersize) PokeN32(ptr + p, (int)vallen);
            p += 4;
            if (p + vallen < buffersize) {
                PokeN32(ptr + p, a->toDate().toInt());
            }
            p += vallen;
        } else if (a->isTime() == true) {
            vallen = 8; // Time exportiert die Zeit in Microseconds als 64Bit Integer
            if (p + 4 < buffersize) PokeN32(ptr + p, (int)vallen);
            p += 4;
            if (p + vallen < buffersize) {
                PokeN64(ptr + p, a->toTime().toMicroseconds());
            }
            p += vallen;
        } else if (a->isTimeDelta()) {
            vallen = 8; // TimeDelta exportiert die Zeit in Microseconds als 64Bit Integer
            if (p + 4 < buffersize) PokeN32(ptr + p, (int)vallen);
            p += 4;
            if (p + vallen < buffersize) {
                PokeN64(ptr + p, a->toTimeDelta().toMicroseconds());
            }
            p += vallen;
        } else if (a->isTimeZone()) {
            const TimeZone& tz = a->toTimeZone();
            vallen = 2;                 // TimeZone exportiert die OffsetMinutes als 16Bit Integer,
            vallen += tz.name().size(); // und den Namen als String, der aber leer sein kann
            if (p + 4 < buffersize) PokeN32(ptr + p, (int)vallen);
            p += 4;
            if (p + vallen < buffersize) {
                PokeN16(ptr + p, tz.offsetMinutes());
                memcpy(ptr + p + 2, (const char*)tz.name(), tz.name().size());
            }
            p += vallen;
        } else {
            vallen = 0;
            if (p + 4 < buffersize) PokeN32(ptr + p, 0);
            p += 4;
        }
    }
    if (p < buffersize) PokeN8(ptr + p, 0);
    p++;
    if (buffersize == 0 || p <= buffersize) return p;
    throw ExportBufferToSmallException("%zd < %zd", buffersize, p);
}

/*!\brief Inhalt des Arrays in einem plattform-unabhängigen Binären-Format exportieren
 *
 * \desc
 * Mit dieser Funktion kann der komplette Inhalt des Arrays in einem plattform-unabhängigem binären Format abgelegt
 * werden, das sich zum Speichern in einer Datei oder zum Übertragen über das Internet eignet.
 *
 * \param[in,out] buffer %ByteArray, in dem die exportierten Daten gespeichert werden sollen
 *
 * \attention
 * Es muss daran gedacht werden, dass nicht alle Datentypen exportiert werden können. Gegenwärtig
 * werden folgende Typen unterstützt:
 * - String (Wird als UTF-8 exportiert)
 * - Array
 * - AssocArray
 * - ByteArray
 * - ByteArrayPtr (wird in ein ByteArray umgewandelt!)
 * - DateTime
 * \see
 * - AssocArray::binarySize
 * - AssocArray::importBinary
 *
 * \note
 * Das exportierte Binary ist komptibel mit dem Assoziativen Array der PPL-Version 6
 */
void AssocArray::exportBinary(ByteArray& buffer) const
{
    buffer.free();
    size_t size = binarySize();
    buffer.malloc(size);
    exportBinary((void*)buffer.adr(), buffer.size());
}

ByteArray AssocArray::exportBinary() const
{
    ByteArray buffer;
    exportBinary(buffer);
    return buffer;
}

/*!\brief Daten aus einem vorherigen Export wieder importieren
 *
 * \desc
 * Mit dieser Funktion kann ein zuvor mit AssocArray::exportBinary exportiertes Assoziatives %Array wieder
 * importiert werden. Falls im %Array bereits Daten vorhanden sind, werden diese nicht gelöscht, können aber
 * überschrieben werden, wenn es im Export gleichnamige Schlüssel gibt.
 *
 * \param[in] bin Referenz auf ByteArray oder ByteArrayPtr mit den zu importierenden Daten
 *
 * \see
 * - CAssocArray::exportBinary
 * - CAssocArray::binarySize
 */
void AssocArray::importBinary(const ByteArrayPtr& bin)
{
    importBinary(bin.adr(), bin.size());
}

/*!\brief Daten aus einem vorherigen Export wieder importieren
 *
 * \desc
 * Mit dieser Funktion kann ein zuvor mit AssocArray::exportBinary exportiertes Assoziatives %Array wieder
 * importiert werden. Falls im %Array bereits Daten vorhanden sind, werden diese nicht gelöscht, können aber
 * überschrieben werden, wenn es im Export gleichnamige Schlüssel gibt.
 *
 * \param[in] buffer Pointer auf den Puffer, der die zu importierenden Daten enthält
 * \param[in] buffersize Größe des Puffers
 * \exception ImportFailedException
 *
 * \see
 * - AssocArray::exportBinary
 * - AssocArray::binarySize
 */
size_t AssocArray::importBinary(const void* buffer, size_t buffersize)
{
    if (!buffer) throw IllegalArgumentException();
    if (buffersize == 0) throw IllegalArgumentException();
    const char* ptr = (const char*)buffer;
    size_t p = 0;
    if (buffersize < 8 || strncmp((const char*)ptr, "PPLASOC", 7) != 0) {
        throw ImportFailedException("Not an AssocArray binary export");
    }
    p += 7;
    int type;
    size_t vallen, bytes;
    String key;
    while (p < buffersize && (type = PeekN8(ptr + p)) != 0) {
        p++;
        size_t keylen = PeekN16(ptr + p);
        p += 2;
        key.set(ptr + p, keylen);
        p += keylen;
        switch (type) {
        case Variant::TYPE_STRING:
            vallen = PeekN32(ptr + p);
            p += 4;
            set(key, String((const char*)ptr + p, vallen));
            p += vallen;
            break;
        case Variant::TYPE_WIDESTRING:
            vallen = PeekN32(ptr + p);
            p += 4;
            set(key, WideString((const char*)ptr + p, vallen));
            p += vallen;
            break;
        case Variant::TYPE_ASSOCARRAY: {
            AssocArray na;
            bytes = na.importBinary(ptr + p, buffersize - p);
            p += bytes;
            set(key, na);

        } break;
        case Variant::TYPE_ARRAY: {
            size_t elements = PeekN32(ptr + p);
            p += 4;
            Array stringarray;
            stringarray.reserve(elements);
            for (size_t i = 0; i < elements; i++) {
                String str(ptr + p + 4, PeekN32(ptr + p));
                p += PeekN32(ptr + p) + 4;
                stringarray.add(str);
            }
            set(key, stringarray);
        } break;
        case Variant::TYPE_BYTEARRAY: {
            vallen = PeekN32(ptr + p);
            p += 4;
            ByteArray nb(ptr + p, vallen);
            set(key, nb);
            p += vallen;
        } break;
        case Variant::TYPE_DATETIME: {
            vallen = PeekN32(ptr + p);
            p += 4;
            DateTime dt;
            if (vallen == 8) { // Legacy PPL7-Format
                dt.setLongInt(PeekN64(ptr + p));
            } else if (vallen >= 10) { // PPL8, mit Microseconds und Timezone (Offset in Minutes)
                int64_t us = (int64_t)PeekN64(ptr + p);
                int16_t tz_offset = (int16_t)PeekN16(ptr + p + 8);
                dt.setMicroseconds(us, TimeZone(tz_offset));
                if (vallen > 10) { // Wir haben auch einen Zeitzonen-Namen
                    dt.timeZone().setName(String(ptr + p + 10, vallen - 10));
                }
            }
            // vallen könnte auch 0 sein, wenn das DateTime invalid ist
            p += vallen;
            set(key, dt);
        } break;
        case Variant::TYPE_DATE: {
            vallen = PeekN32(ptr + p);
            p += 4;
            set(key, Date::fromInt(PeekN32(ptr + p)));
            p += vallen;
        } break;
        case Variant::TYPE_TIME: {
            vallen = PeekN32(ptr + p);
            p += 4;
            set(key, Time::fromMicroseconds(PeekN64(ptr + p)));
            p += vallen;
        } break;
        case Variant::TYPE_TIMEDELTA: {
            vallen = PeekN32(ptr + p);
            p += 4;
            set(key, TimeDelta::fromMicroseconds((int64_t)PeekN64(ptr + p)));
            p += vallen;
        } break;
        case Variant::TYPE_TIMEZONE: {
            vallen = PeekN32(ptr + p);
            p += 4;
            int16_t offset = (int16_t)PeekN16(ptr + p);
            String name;
            if (vallen > 2) {
                name.set((const char*)ptr + p + 2, vallen - 2);
            }
            set(key, TimeZone(offset, name));
            p += vallen;
        } break;
        default:
            vallen = PeekN32(ptr + p);
            throw ImportFailedException("unknown datatype in AssocArray binary export [type=%d, size=%zu]", type, vallen);
        };
    }
    p++;
    return p;
}

/*!\brief Schlüssel auslesen
 *
 * \desc
 * Dieser Operator liefert den Wert des Schlüssels \p key als Variant zurück. Dieser kann
 * von der aufrufenden Anwendung in den jeweiligen Datentyp umgewandelt werden.
 *
 * @param key Name des Schlüssels
 * @return Referenz auf den einen Variant mit dem Wert des Schlüssels
 * \exception InvalidKeyException: Ungültiger Schlüssel
 * \exception KeyNotFoundException: Schlüssel wurde nicht gefunden
 */
const Variant& AssocArray::operator[](const String& key) const
{
    Variant* node = findInternal(key);
    if (!node) throw KeyNotFoundException(key);
    return *node;
}

Variant& AssocArray::operator[](const String& key)
{
    Variant* node = findInternal(key);
    if (!node) throw KeyNotFoundException(key);
    return *node;
}

/*!\brief Assoziatives Array kopieren
 *
 * \desc
 * Mit diesem Operator wird der Inhalt das Assoziativen Arrays \p other übernommen.
 * Der bisherige Inhalt dieses Arrays geht verloren.
 *
 * @param other Zu kopierendes assoziatives Array
 * @return Referenz auf dieses Array
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 *
 */
AssocArray& AssocArray::operator=(const AssocArray& other)
{
    clear();
    add(other);
    return *this;
}

/*!\brief Assoziatives Array hinzufügen
 *
 * \desc
 * Mit diesem Operator wird der Inhalt das Assoziativen Arrays \p other dem eigenen
 * Array hinzugefügt. Das Array wird vorher nicht gelöscht, so dass vorhandene
 * Schlüssel erhalten bleiben. Gibt es in \p other jedoch gleichnamige Schlüssel,
 * werden die bisherigen Werte überschrieben.
 *
 * @param other Zu kopierendes assoziatives Array
 * @return Referenz auf dieses Array
 * \exception std::bad_alloc: Kein Speicher mehr frei
 * \exception OutOfMemoryException: Kein Speicher mehr frei
 * \exception InvalidKeyException: Ungültiger Schlüssel
 *
 */
AssocArray& AssocArray::operator+=(const AssocArray& other)
{
    add(other);
    return *this;
}

bool AssocArray::operator==(const AssocArray& other) const
{
    ByteArray b1, b2;
    exportBinary(b1);
    other.exportBinary(b2);
    if (b1 == b2) return true;
    return false;
}

bool AssocArray::operator!=(const AssocArray& other) const
{
    if (*this == other) return false;
    return true;
}

AssocArray operator+(const AssocArray& a1, const AssocArray& a2)
{
    AssocArray a(a1);
    a.add(a2);
    return a;
}

#ifdef OLDCODE
/*! \brief Wandelt ein Key-Value Template in ein Assoziatives Array um
 *
 * \desc
 * Diese Funktion wandelt einen Text mit Key-Value-Paaren in ein
 * Assoziatives Array um. Leere Zeilen oder Zeilen mit Raute (#)
 * am Anfang (Kommentarzeilen) werden ignoriert.
 *
 * \param[in] templ String mit den Key-Value-Paaren
 * \param[in] linedelimiter Das Zeichen, was als Zeilenende interpretiert werden soll. Default ist \c Newline
 * \param[in] splitchar Das Zeichen, was als Trennzeichen zwischen Schlüssel (Key) und Wert (Value)
 * interpretiert werden soll. Der Default ist das Gleichheitszeichen (=)
 * \param[in] concat Ist concat gesetzt und kommen im Text mehrere identische Schlüssel vor, werden die Werte
 * zu einem String zusammengeführt, wobei als Trennzeichen \c concat verwendet wird. Ist concat leer,
 * wird ein vorhandener Schlüssel überschrieben. Der Default ist, dass Werte mit gleichem Schlüssel mit
 * Newline aneinander gehangen werden.
 * \param[in] dotrim Ist \c dotrim=true, werden einzelnen Werte vor dem Einfügen ins Array mit der Funktion
 * Trim getrimmt, also Leerzeilen, Tabs und Zeilenumbrüche am Anfang und Ende gelöscht. Der Default
 * ist \c false.
 *
 * \return Die Funktion gibt die Anzahl gelesener Key-Value-Paare zurück, oder 0, wenn der Text
 * keine verwertbaren Zeilen enthielt.
 *
 * \note Falls das %Array vor dem Aufruf dieser Funktion bereits Datensätze enthielt, werden diese
 * nicht gelöscht. Die Funktion kann also benutzt werden, um Werte aus verschiedenen Templates in ein
 * einziges %Array einzulesen. Soll das %Array geleert werden, muß vorher die Funktion AssocArray::clear
 * aufgerufen werden.
 *
 * \see Um Konfigurationsdateien mit verschiedenen Abschnitten (z.B. .ini-Dateien) in ein
 * Assoziatives %Array einzulesen, gibt es die Member-Funktion
 * AssocArray::fromConfig
 *
 */
size_t AssocArray::fromTemplate(
    const String& templ, const String& linedelimiter, const String& splitchar, const String& concat, bool dotrim)
{
    String Row, Line;

    String Key, Value;
    size_t rows = 0;
    ssize_t p;
    size_t ssc = splitchar.size();
    Array a;
    a.explode(templ, linedelimiter);

    Array::const_iterator it;
    for (it = a.begin(); it != a.end(); ++it) {
        Line = *it;
        Row = Trim(Line);
        if (Row.len() > 0 && Row[0] != L'#') { // Leere Zeilen und Kommentare ignorieren
            Row = Line;
            p = Row.instr(splitchar);
            if (p > 0) {
                Key = Trim(Row.left(p));
                Value = Row.mid(p + ssc);
                if (dotrim) Value.trim();
                // printf ("Key=%ls\nValue=%ls\n",(const wchar_t *)Key, (const wchar_t *)Value);
                if (concat.notEmpty()) {
                    append(Key, Value, concat);
                } else {
                    set(Key, Value);
                }
                rows++;
            }
        }
    }
    return rows;
}

/*! \brief Wandelt eine Konfigurationsdatei in ein Assoziatives Array um
 *
 * \desc
 * Diese Funktion wandelt einen Konfigurations-Text mit mehreren Abschnitten
 * im Key-Value-Format in ein Assoziatives %Array um. Ein Abschnitt beginnt immer mit einem Keywort
 * in Eckigen klammern und enthält Key-Value-Paare. Zeilen mit Raute (#) am Anfang werden als
 * Kommentarzeilen interpretiert und ignoriert.
 * \par Beispiel einer Konfigurationsdatei
 * \code
[Abschnitt_1]
# Kommentarzeile, die überlesen wird
key1: value1
key2: value2
[Abschnitt_2]
key1: value1
key2: value2
\endcode
 *
 * \param[in] content Ein String, dre die zu parsende Konfiguration enthält.
 * \param[in] linedelimiter Das Zeichen, was als Zeilenende interpretiert werden soll. Default ist \c Newline
 * \param[in] splitchar Das Zeichen, was als Trennzeichen zwischen Schlüssel (Key) und Wert (Value)
 * interpretiert werden soll. Der Default ist das Gleichheitszeichen (=)
 * \param[in] concat Ist concat gesetzt und kommen im Text mehrere identische Schlüssel vor, werden die Werte
 * zu einem String zusammengeführt, wobei als Trennzeichen \c concat verwendet wird. Ist concat NULL,
 * wird ein vorhandener Schlüssel überschrieben. Der Default ist, dass gleiche Schlüssel mit Newline
 * aneinander gehangen werden.
 * \param[in] dotrim Ist \c dotrim=true, werden einzelnen Werte vor dem Einfügen ins Array mit der Funktion
 * Trim getrimmt, also Leerzeilen, Tabs und Zeilenumbrüche am Anfang und Ende gelöscht. Der Default
 * ist \c false.
 *
 * \return Die Funktion gibt die Anzahl gelesener Key-Value-Paare zurück, oder 0, wenn der Text
 * keine verwertbaren Zeilen enthielt.
 *
 * \note Falls das %Array vor dem Aufruf dieser Funktion bereits Datensätze enthielt, werden diese
 * nicht gelöscht. Die Funktion kann also benutzt werden, um Werte aus verschiedenen Templates in ein
 * einziges %Array einzulesen. Soll das %Array geleert werden, muß vorher die Funktion AssocArray::clear
 * aufgerufen werden.
 */
size_t AssocArray::fromConfig(
    const String& content, const String& linedelimiter, const String& splitchar, const String& concat, bool dotrim)
{
    String Row, Line, Section;
    Array a;
    String Key, Value;
    size_t rows = 0;
    ssize_t p;
    size_t ssc = splitchar.size();
    a.explode(content, linedelimiter);
    pplib::Array::const_iterator it;
    for (it = a.begin(); it != a.end(); ++it) {
        Line = *it;
        Row = Trim(Line);
        if (Row.len() > 0 && Row[0] != L'#') { // Leere Zeilen und Kommentare ignorieren
            if (Row[0] == L'[' && Row[-1] == L']') {
                Section = Row.mid(1, Row.len() - 2);
                Section.lowerCase();
                Section.trim();
            } else {
                Row = Line;
                p = Row.instr(splitchar);
                if (p > 0) {
                    Key = Section;
                    if (Key.notEmpty()) Key += "/";
                    Key += Trim(Row.left(p));
                    Value = Row.mid(p + ssc);
                    if (dotrim) Value.trim();
                    // printf ("Key=%ls\nValue=%ls\n",(const wchar_t *)Key, (const wchar_t *)Value);
                    if (concat.notEmpty()) {
                        append(Key, Value, concat);
                    } else {
                        set(Key, Value);
                    }
                    rows++;
                }
            }
        }
    }
    return rows;
}

/*!\brief Inhalt des Assoziativen Arrays in ein Template exportieren
 *
 * \desc
 * Mit dieser Funktion wird der textuelle Inhalt des Arrays als Template im Key-Value-Format in einem String
 * abgelegt.
 * Pointer oder Binäre Daten werden ignoriert.
 *
 * \param[out] s %String, in dem das Template gespeichert werden soll. Der %String wird von der Funktion nicht gelöscht,
 * der Inhalt des Arrays wird angehangen!
 * \param[in] prefix Optionaler Prefix, der jedem Key vorangestellt werden soll
 * \param[in] linedelimiter Optionaler Pointer auf einen String, der am Zeilenende ausgegeben werden soll. Der
 *            Default ist ein einzelnes Newline.
 * \param[in] splitchar Optionaler Pointer auf einen String, der als Trennzeichen zwischen Schlüssel und Wert
 *            verwendet werden soll. Der Default ist ein Gleichheitszeichen.
 * \par Beispiel
\code
#include <stdio.h>
#include <string.h>
#include <pplib.h>

int main(int argc, char **argv)
{
    pplib::AssocArray a;
    pplib::ByteArray bin;
    pplib::String out;
    bin.load("main.cpp");
    a.set("key1","Dieser Wert geht über\nmehrere Zeilen");
    a.set("array1/unterkey1","value2");
    a.set("array1/unterkey2","value3");
    a.set("array1/noch ein array/unterkey1","value4");
    a.set("array1/unterkey2","value5");
    a.set("key2","value6");
    a.set("dateien/main.cpp",&bin);
    a.set("array2/unterkey1","value7");
    a.set("array2/unterkey2","value8");
    a.set("array2/unterkey1","value9");
    a.toTemplate(&out,"foo");
    out.printnl();
}
\endcode
Ergebnis:
\code
foo/array1/noch ein array/unterkey1=value4
foo/array1/unterkey1=value2
foo/array1/unterkey2=value5
foo/array2/unterkey1=value9
foo/array2/unterkey2=value8
foo/key1=Dieser Wert geht über
foo/key1=mehrere Zeilen
foo/key2=value6
\endcode
    An diesem Beispiel sieht man, dass Pointer- und ByteArray-Werte nicht exportiert werden und Werte, die Zeilenumbrüche
    enthalten, werden auf mehrere Key-Value-Paare aufgesplittet. Die Importfunktion (AssocArray::fromTemplate,
    AssocArray::fromConfig) fügen diese wieder zu einer einzelnen Variable mit Zeilenumbruch
    zusammen.
 */
void AssocArray::toTemplate(String& s, const String& prefix, const String& linedelimiter, const String& splitchar) const
{
    String key, pre, value, index;
    Array Tok;
    if (prefix.notEmpty()) key = prefix + "/";
    pplib::AssocArray::const_iterator it;
    for (it = Tree.begin(); it != Tree.end(); ++it) {
        Variant* p = it->second;
        if (p->isString()) {
            Tok.clear();
            Tok.explode(p->toString(), "\n");
            for (size_t i = 0; i < Tok.size(); i++) {
                s += key + it->first + splitchar + Tok[i] + linedelimiter;
            }
        } else if (p->isAssocArray()) {
            pre.setf("%s%s", (const char*)key, (const char*)it->first);
            p->toAssocArray().toTemplate(s, pre, linedelimiter, splitchar);
        } else if (p->isArray()) {
            const Array& a = (const Array&)*p;
            for (size_t i = 0; i < a.size(); i++) {
                Tok.clear();
                Tok.explode(a[i], "\n");
                index.setf("%zu", i);
                for (size_t z = 0; z < Tok.size(); z++) {
                    s += key + it->first + "/" + index + splitchar + Tok[z] + linedelimiter;
                }
            }
        } else if (p->isDateTime()) {
            s += key + it->first + splitchar + p->toDateTime().getISO8601withMsec() + linedelimiter;
        }
    }
}

#endif

} // namespace pplib
