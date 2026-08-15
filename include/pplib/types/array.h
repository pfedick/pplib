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

#ifndef PPLIB_TYPES_ARRAY_H_
#define PPLIB_TYPES_ARRAY_H_

#include <stdint.h>
#include <iterator>

#include "pplib/types/string.h"

namespace pplib
{

class String;

/** @class Array
 * @ingroup PPLGroupDataTypes
 * @brief Ein Array mit Strings
 *
 * Diese Klasse repräsentiert ein Array aus Strings, die über einen Index angesprochen werden
 * können. Die Zählung der Elemente beginnt dabei bei 0, das heisst das erste Element hat den
 * Index 0 (vergleichbar mit Arrays in C/C++).
 */
class Array
{
private:
    typedef struct
    {
        String* value;
    } ROW;

    /// Anzahl Elemente im Array
    size_t numElements;

    /// Maximale Anzahl Elemente im Array, für die bereits Speicher reserviert wurde
    size_t numCapacity;

    /** @brief Array mit den Datenelementen
     *
     * Diese Variable enthält einen Pointer auf die interne Datenstruktur, die die Werte
     * der einzelnen Elemente enthält. Die Größe des Speicherbereichs ergibt sich aus der
     * maximalen Anzahl Elemente im Array (Array::numCapacity) multipliziert mit der größe der
     * Datenstruktur für jedes Element.
     */
    void* rows;

public:
    //! @name Konstruktoren und Destruktor
    //@{

    /** @brief Konstruktor mit leerem Array
     */
    Array();

    /** @brief Copy-Konstruktor
     *
     * Mit dem Copy-Konstruktor wird der Inhalt des Arrays \p other 1:1 kopiert.
     *
     * @param other Anderes Array
     */
    Array(const Array& other);

    /** @brief Move-Konstruktor
     *
     * Mit dem Move-Konstruktor wird der Inhalt des Arrays \p other 1:1 übernommen.
     *
     * @param other Anderes Array
     */
    Array(Array&& other);

    /** @brief Konstruktor aus String
     *
     * Mit diesem Konstruktor wird der String \p str anhand des Trennzeichens
     * \p delimiter in einzelne Elemente zerlegt
     *
     * @param str String
     * @param delimiter Trennzeichen oder Trennstring
     * @param limit Maximale Anzahl Elemente, normalerweise unbegrenzt
     * @param skipemptylines Leere Elemente überspringen. Folgen zwei Trennzeichen hintereinander, würde
     * normalerweise ein leeres Element in das Array eingefügt. Durch setzen dieses Parameters auf \c true
     * werden keine leeren Elemente eingefügt.
     */
    Array(const String& str, const String& delimiter = String("\n"), size_t limit = 0, bool skipemptylines = false);

    /** @brief Destruktor
     *
     * Mit dem Destruktor wird das Array gelöscht und der Speicher wieder freigegeben.
     */
    ~Array();
    //@}

    //! @name Elemente hinzufügen/verändern
    //@{

    /** @brief String Element hinzufügen
     *
     * Mit dieser Funktion wird ein Element an das Ende des Arrays angefügt.
     *
     * @param value Wert des Elements
     */
    void add(const String& value);

    /** @brief String Element hinzufügen mit bestimmter Länge
     *
     * Mit dieser Funktion wird ein Element an das Ende des Arrays angefügt. Es werden nur die ersten \p size Zeichen übernommen.
     *
     * @param value Wert des Elements
     * @param size Anzahl Zeichen, die übernommen werden sollen
     */
    void add(const String& value, size_t size);

    /** @brief Array hinzufügen
     *
     * Der Inhalt des Arrays \p other wird an das Array angehangen.
     *
     * @param other Zu kopierendes Array
     */
    void add(const Array& other);

    /** @brief String Element hinzufügen mit Formatierung
     *
     * Mit dieser Funktion wird ein Element an das Ende des Arrays angefügt. Der Wert des Elements wird anhand des Formatstrings \p fmt und
     * den optionalen Parametern \p ... erzeugt.
     *
     * @param fmt Formatstring
     * @param ... Optionale Parameter, die im Formatstring verwendet werden können
     */
    void addf(const char* fmt, ...);

    /** @brief Array kopieren
     *
     * Der bisherige Inhalt des Arrays wird gelöscht und mit dem das Arrays \p other gefüllt. Falls der bestehende
     * Inhalt nicht gelöscht werden soll, muss die Funktion Array::add(const Array &other) verwendet werden.
     *
     * @param other Zu kopierendes Array
     */
    void copy(const Array& other);

    /** @brief Element im Array setzen
     *
     * Mit dieser Funktion wird das Element an Position \p index des Arrays auf den Wert \p value gesetzt.
     * Der bisherige Wert an dieser Position wird überschrieben. Ist \p index größer als die Anzahl Elemente
     * des Arrays, wird das Array zunächst mit leeren Elementen bis zu \p index aufgefüllt
     * und anschließend das Element an Position \p index gesetzt.
     *
     * @param index Position innerhalb des Arrays, beginnend mit 0
     * @param value String
     */
    void set(size_t index, const String& value);

    /** @brief Formatierten String setzen
     *
     * Mittels Formatstring \p fmt und der optionalen Parameter wird ein neuer String gebildet, der an
     * die Position \p index des Arrays gesetzt wird. Der vorherige Wert
     * des Arrays an dieser Stelle wird überschrieben.
     * Ist \p index größer als die Anzahl Elemente des Arrays, wird das Array zunächst mit leeren Elementen
     * bis zu \p index aufgefüllt und anschließend das Element an Position \p index gesetzt.
     *
     * @param index Position innerhalb des Arrays, beginnend mit 0
     * @param fmt Formatstring
     * @param ... Optionale Parameter
     */
    void setf(size_t index, const char* fmt, ...);

    /** @brief Element an bestimmter Position einfügen
     *
     * Alle vorhandenen Elemente des Arrays ab Position \p index werden um eins nach hinten
     * verschoben. Anschließend wird der Wert \p value an der Position \p index eingefügt.
     * Falls \p index größer als die Anzahl Elemente des Arrays ist, wird das
     * Array entsprechend mit leeren Elementen aufgefüllt und anschließend das Element
     * an Position \p index eingefügt.
     *
     * @param index Position, an der das Element eingefügt werden soll
     * @param value Einzufügendes Element
     */
    void insert(size_t index, const String& value);

    /** @brief Array an bestimmter Position einfügen
     *
     * Alle vorhandenen Elemente des Arrays ab Position \p index werden um die größe des anderen
     * Arrays nach hinten verschoben. Anschließend wird der Inhalt des Arrays \p other an der Position \p index eingefügt.
     * Falls \p index größer als die Anzahl Elemente des Arrays ist, wird das
     * Array entsprechend mit leeren Elementen aufgefüllt und anschließend das andere Array
     * an Position \p index eingefügt.
     *
     * @param index Position, an der das Element eingefügt werden soll
     * @param other Einzufügendes Array
     */
    void insert(size_t index, const Array& other);

    /** @brief Formatierten String an bestimmter Position einfügen
     *
     * Alle vorhandenen Elemente des Arrays ab Position \p index werden um eins nach hinten
     * verschoben. Anschließend wird ein neuer Wert mittels des Formatstrings \p fmt
     * und den zusätzlichen Parametern gebildet und an der Position \p index
     * eingefügt.
     *
     * @param index Position, an der das Element eingefügt werden soll
     * @param fmt Formatstring
     * @param ... Zusätzliche optionale Parameter
     */
    void insertf(size_t index, const char* fmt, ...);

    /** @brief Array aus den Aufrufparametern des Programms erzeugen
     *
     * Ein Array wird aus den Aufrufparametern des Programms erstellt.
     *
     * @param argc Anzahl Parameter
     * @param argv Pointer auf ein Array mit C-Strings
     * @return Gibt eine Referenz auf das Array zurück.
     */
    Array& fromArgs(int argc, const char** argv);

    /** @brief Array aus den Aufrufparametern des Programms erzeugen
     *
     * Ein Array wird aus den Aufrufparametern des Programms erstellt.
     *
     * @param args Aufrufstring
     * @return Gibt eine Referenz auf das Array zurück.
     */
    Array& fromArgs(const String& args);

    /** @brief String in Array zerlegen
     *
     * Mit dieser Funktion wird der String \p text anhand des Trennzeichens \p delimiter in einzelne Elemente zerlegt.
     * Die einzelnen Elemente werden in das Array eingefügt. Falls das Array bereits Elemente enthält, werden diese
     * nicht gelöscht, sondern die neuen Elemente angehangen.
     *
     * @param text String, der zerlegt werden soll
     * @param delimiter Trennzeichen oder Trennstring
     * @param limit Maximale Anzahl Elemente, normalerweise unbegrenzt
     * @param skipemptylines Leere Elemente überspringen. Folgen zwei Trennzeichen hintereinander, würde
     * normalerweise ein leeres Element in das Array eingefügt. Durch setzen dieses Parameters auf \c true
     * werden keine leeren Elemente eingefügt.
     */
    Array& explode(const String& text, const String& delimiter = "\n", size_t limit = 0, bool skipemptylines = false);
    //@}

    //! @name Elemente löschen
    //@{

    /** @brief Array löschen
     *
     * Der durch das Array belegte Speicher wird freigegeben. Das Array ist danach leer und kann erneut befüllt werden.
     */
    void clear();

    /** @brief Element löschen
     *
     * Das Element an Position \p index wird gelöscht. Alle nachfolgenden Elemente werden um eins nach vorne verschoben.
     * Ist \p index größer als die Anzahl Elemente des Arrays, wird eine Exception geworfen.
     *
     * @param index Position des zu löschenden Elements
     * @return Wert des gelöschten Elements
     * \exception OutOfBoundsException: Wird geworfen, wenn \p index größer als die Anzahl Elemente des Arrays ist
     */
    String erase(size_t index);

    /** @brief Das letzte Element aus dem Array holen
     *
     * Das letzte Element des Arrays (also das mit dem höchsten Index) wird aus
     * dem Array entfernt und als String zurückgegeben. Ist das Array leer, wird eine
     * Exception geworfen.
     *
     * @return String mit dem Wert, der aus dem Array entfernt wurde
     * \exception EmptyDataException: Wird geworfen, wenn das Array leer ist
     */
    String pop();

    /** @brief Das erste Element aus dem Array holen
     *
     * Das erste Element des Arrays (also das mit dem Index 0) wird aus
     * dem Array entfernt und als String zurückgegeben. Der Rest des Arrays wird um
     * eine Position nach vorne gerückt. Ist das Array leer, wird eine
     * Exception geworfen.
     *
     * @return String mit dem Wert, der aus dem Array entfernt wurde
     * @exception EmptyDataException: Wird geworfen, wenn das Array leer ist
     * @note
     * Bei großen Arrays ist diese Operation recht teuer, da alle nachfolgenden Elemente
     * um eine Position nach vorne gerückt werden müssen.
     */
    String shift();

    //@}

    //! @name Elemente auslesen
    //@{

    /** @brief Element als Konstante auslesen
     *
     * Gibt das Element an Position \p index des Arrays als Referenz zurück, dessen Inhalt nicht
     * verändert werden kann. Ist \p index größer als die Anzahl Elemente des Arrays, wird eine Exception geworfen.
     *
     * @param index Gewünschtes Element
     * @return Referenz auf den Inhalt des Elements
     * @exception OutOfBoundsException: Wird geworfen, wenn \p index größer als die Anzahl Elemente des Arrays ist
     */
    const String& get(ssize_t index) const;

    /** @brief Element auslesen
     *
     * Gibt das Element an Position \p index des Arrays als Referenz zurück. Ist \p index größer als die Anzahl
     * Elemente des Arrays, wird eine Exception geworfen.
     *
     * @param index Gewünschtes Element
     * @return Referenz auf den Inhalt des Elements
     * \exception OutOfBoundsException: Wird geworfen, wenn \p index größer als die Anzahl Elemente des Arrays ist
     */
    String& get(ssize_t index);

    /** @brief Zufälliges Element als Konstante auslesen
     *
     * Gibt eine Referenz auf ein zufälliges Element des Arrays zurück, dessen Inhalt nicht verändert
     * werden kann.
     *
     * @return Referenz auf ein zufälliges Elements des Arrays.
     * Ist das Array leer, wird immer ein leerer String zurückgegeben.
     */
    const String& getRandom() const;

    /** @brief Zufälliges Element auslesen
     *
     * Gibt eine Referenz auf ein zufälliges Element des Arrays zurück.
     *
     * @return Referenz auf ein zufälliges Elements des Arrays.
     * Ist das Array leer, wird immer ein leerer String zurückgegeben.
     */
    String& getRandom();

    /** @brief Inhalt des Arrays ab einer bestimmten Position als String zurückgeben
     *
     * Inhalt des Arrays ab einer bestimmten Position als String zurückgeben
     *
     * @param index Position im Array
     * @param delimiter Trennzeichen, mit dem die Elemente des Arrays im String zusammengefügt
     *        werden sollen
     * @return String
     */
    String getRest(size_t index, const String& delimiter = " ");

    /** @brief Array zu einem String zusammenfügen
     *
     * Der Inhalt des Arrays wird zu einem String zusammengefügt, wobei das im Parameter \p delimiter
     * angegebene Zeichen oder String als Trennzeichen verwendet wird.
     *
     * @param delimiter Trennzeichen oder String
     * @return Zusammengesetzter String mit dem Inhalt des Arrays
     */
    String implode(const String& delimiter = "\n") const;
    //@}

    //! @name Sonstiges
    //@{

    /** @brief Anzahl Elemente im Array
     *
     * @return Anzahl Elemente im Array
     */
    inline size_t count() const
    {
        return numElements;
    }

    /** @brief Größe des Arrays
     *
     * @return Anzahl Elemente im Array
     */
    inline size_t size() const
    {
        return numElements;
    }

    /** @brief Kapazität des Arrays
     *
     * Gibt die maximale Anzahl Elemente zurück, für die im Array bereits Speicher reserviert wurde.
     * Die Kapazität kann größer sein als die Anzahl der Elemente im Array. Die Kapazität kann nachträglich
     * nicht verkleinert werden.
     *
     * @return Maximale Anzahl Elemente, für die Speicher reserviert wurde
     */
    inline size_t capacity() const
    {
        return numCapacity;
    }

    /** @brief Prüfen, ob das Array leer ist
     *
     * @return \c true, wenn das Array keine Elemente enthält, sonst \c false
     */
    inline bool isEmpty() const
    {
        return numElements == 0;
    }

    /** @brief Platz reservieren
     *
     * Durch Aufruf dieser Funktion wird vorab Speicher allokiert, um die durch \p size angegebene
     * Anzahl Elemente aufnehmen zu können. Die Funktion sollte immer dann aufgerufen werden, wenn
     * eine große Anzahl Elemente in das Array eingefügt werden soll, um die Anzahl der Speicherallokationen
     * zu reduzieren.
     *
     * @param size Anzahl Elemente, für die Speicher reserviert werden soll
     * @note Ist die Kapazität des Arrays bei Aufruf der Funktion bereits höher als der angegebene Wert
     * \p size, bleibt das Array unverändert. Die Kapazität kann nachträglich nicht verkleinert werden.
     */
    void reserve(size_t size);

    /** @brief Array ausgeben
     *
     * Mit dieser Funktion wird der Inhalt des Arrays auf die Standardausgabe ausgegeben. Optional kann
     * ein Präfix angegeben werden, das vor jedem Element ausgegeben wird.
     *
     * @param prefix Präfix, das vor jedem Element ausgegeben wird
     */
    void list(const String& prefix = String()) const;

    /** @brief Elemente nach ihrem Wert sortieren
     *
     * Die einzelnen Elemente des Arrays werden alphabetisch sortiert. Duplikate bleiben erhalten
     */
    void sort();

    /** @brief Elemente nach ihrem Wert in umgekehrter Reihenfolge sortieren
     *
     * Die einzelnen Elemente des Arrays werden in umgekehrter alphabetischer Reihenfolge
     * sortiert. Duplikate bleiben erhalten.
     */
    void sortReverse();

    /** @brief Elemente nach ihrem Wert sortieren, Duplikate entfernen
     *
     * Die einzelnen Elemente des Arrays werden alphabetisch sortiert. Duplikate werden entfernt.
     */
    void sortUnique();

    /** @brief Duplikate entfernen
     *
     * Elemente, die mehrfach im Array vorkommen, werden entfernt. Die Reihenfolge der Elemente
     * bleibt bestehen.
     */
    void makeUnique();

    /** @brief Index eines Elements suchen
     *
     * Mit dieser Funktion wird der Index des ersten Vorkommens des Strings \p search im Array gesucht.
     * Wird der String nicht gefunden, wird \c -1 zurückgegeben.
     *
     * @param search String, dessen Index gesucht werden soll
     * @return Index des Strings im Array oder \c -1, wenn der String nicht gefunden wurde
     */
    ssize_t indexOf(const String& search);

    /** @brief Prüfen, ob ein Element im Array enthalten ist
     *
     * Mit dieser Funktion wird geprüft, ob der String \p search im Array enthalten ist.
     *
     * @param search String, der gesucht werden soll
     * @return \c true, wenn der String im Array enthalten ist, sonst \c false
     */
    bool has(const String& search);
    //@}

    //! @name Operatoren
    //@{

    /** @brief Element aus dem Array auslesen
     *
     * Gibt das Element an Position \p index des Arrays als Referenz zurück. Ist \p index größer als die Anzahl
     * Elemente des Arrays, wird eine Exception geworfen.
     *
     * @param index Gewünschtes Element
     * @return Referenz auf den Inhalt des Elements
     * @exception OutOfBoundsException: Wird geworfen, wenn \p index größer als die Anzahl Elemente des Arrays ist
     */
    String& operator[](ssize_t index);

    /** @brief Element aus dem Array als Konstante auslesen
     *
     * Gibt das Element an Position \p index des Arrays als Referenz zurück, dessen Inhalt nicht verändert werden
     * kann. Ist \p index größer als die Anzahl Elemente des Arrays, wird eine Exception geworfen.
     *
     * @param index Gewünschtes Element
     * @return Referenz auf den Inhalt des Elements
     * @exception OutOfBoundsException: Wird geworfen, wenn \p index größer als die Anzahl Elemente des Arrays ist
     */
    const String& operator[](ssize_t index) const;

    /** @brief Inhalt eines anderen Arrays kopieren
     *
     * Wie bei der Funktion Array::copy wird der aktuelle Inhalt des Arrays gelöscht und der
     * Inhalt des Arrays \p other übernommen.
     *
     * @param other Zu kopierendes Array
     * @return Referenz auf das Array
     */
    Array& operator=(const Array& other);

    /** @brief Move-Operator
     *
     * Mit dem Move-Operator wird der Inhalt des Arrays \p other 1:1 übernommen
     * und das Array \p other anschließend geleert.
     *
     * @param other Zu kopierendes Array
     * @return Referenz auf das Array
     */
    Array& operator=(Array&& other);

    /** @brief Inhalt eines anderen Arrays hinzufügen
     *
     * Wie bei der Funktion Array::add wird der Inhalt des Arrays \p other am Ende des
     * bestehenden Arrays angefügt.
     *
     * @param other Zu kopierendes Array
     * @return Referenz auf das Array
     * \see Array::add(const Array &other)
     */
    Array& operator+=(const Array& other);

    /** @brief Prüfen, ob zwei Arrays identisch sind
     *
     * Mit dem Operator "==" wird geprüft, ob zwei Arrays inhaltlich identisch sind.
     * Dazu wird die Gesamtlänge des Arrays sowie jedes einzelne Element miteinander
     * verglichen.
     *
     * @param other Referenz auf ein zweites Array
     * @return Liefert \c true zurück, wenn beide Arrays identisch sind, sonst \c false.
     */
    bool operator==(const Array& other) const;

    /** @brief Prüfen, ob zwei Arrays unterschiedlich sind
     *
     * Mit dem Operator "!=" wird geprüft, ob zwei Arrays inhaltlich unterschiedlich sind.
     * Dazu wird die Gesamtlänge des Arrays sowie jedes einzelne Element miteinander
     * verglichen.
     *
     * @param other Referenz auf ein zweites Array
     * @return Liefert \c true zurück, wenn beide Arrays unterschiedlich sind, sonst \c false.
     */
    bool operator!=(const Array& other) const;
    //@}

    //! @name Iteratoren
    //@{
    class ptr_iterator
    {
        void* ptr;

    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef String value_type;
        typedef std::ptrdiff_t difference_type;
        typedef String* pointer;
        typedef String& reference;
        ptr_iterator(void* p)
            : ptr(p)
        {
        }
        reference operator*() const
        {
            return *((*(ROW*)ptr).value);
        }
        pointer operator->() const
        {
            return (*(ROW*)ptr).value;
        }
        ptr_iterator& operator++()
        {
            ptr = (ROW*)ptr + 1;
            return *this;
        }
        ptr_iterator operator++(int)
        {
            ptr_iterator tmp = *this;
            ptr = (ROW*)ptr + 1;
            return tmp;
        }
        bool operator==(const ptr_iterator& other) const
        {
            return ptr == other.ptr;
        }
        bool operator!=(const ptr_iterator& other) const
        {
            return ptr != other.ptr;
        }
    };

    typedef ptr_iterator iterator;
    typedef const ptr_iterator const_iterator;

    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    iterator end() noexcept;
    const_iterator end() const noexcept;
    //@}
};

/** @brief Zwei Arrays zusammenaddieren
 *
 * Mit diesem Operator wird der Inhalt zweier Arrays zu einem neuen Array
 * zusammenaddiert
 *
 * @param a1 Erstes Array
 * @param a2 Zweites Array
 * @return Neues Array mit den Werten beider Arrays
 * @relates Array
 */
Array operator+(const Array& a1, const Array& a2);

/** @brief Inhalt eines Arrays alphabetisch sortieren
 *
 * Die einzelnen Elemente des Arrays \p array werden alphabetisch sortiert.
 *
 * @param array Das zu sortierende Array
 * @param unique Optionaler Parameter: true=Duplikate werden entfernt
 * false=Duplikate bleiben erhalten (Default)
 * @return Sortieres Array
 * @relates Array
 */
Array Sort(const Array& array, bool unique);

/** @brief Inhalt eines Arrays in umgekehrter Reihenfolge sortieren
 *
 * Die einzelnen Elemente des Arrays \p array werden in alphabetisch umgekehrter Reihenfolge sortiert.
 *
 * @param array Das zu sortierende Array
 * @param unique Optionaler Parameter: true=Duplikate werden entfernt
 * false=Duplikate bleiben erhalten (Default)
 * @return Sortieres Array
 * @relates Array
 */
Array SortReverse(const Array& array, bool unique);

} // namespace pplib

#endif /* PPLIB_TYPES_ARRAY_H_ */