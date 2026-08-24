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
#ifndef PPLIB_TYPES_ASSOCARRAY_H_
#define PPLIB_TYPES_ASSOCARRAY_H_

#include <stdint.h>
#include <map>

#include <pplib/types/variant.h>
#include <pplib/types/string.h>
#include <pplib/core/baseexception.h>

namespace pplib
{

class Variant;

/** @class AssocArray
 * @ingroup PPLGroupDataTypes
 * @brief Komplexes mehrdimensionales %Array mit Strings als Schlüssel
 *
 * Die Klasse AssocArray dient als Container für beliebige Key-Value-Paare. Ein Schlüssel
 * (Key) besteht aus einem String, der aus beliebigen Zeichen bestehen kann. Ein Value kann
 * veschiedene Datentypen enthalten. Gegenwärtig werden folgende Datentypen unterstützt:
 * - String
 * - WideString
 * - Array
 * - ByteArray
 * - ByteArrayPtr
 * - AssocArray
 * - DateTime
 * - Date
 * - Time
 * - TimeDelta
 * - TimeZone
 *
 * Die Schlüssel werden sortiert in einer std::map verwaltet, so dass auch bei
 * sehr großen Arrays eine schnelle Verarbeitung gewährleistet ist. Gross-/Kleinschreibung wird
 * ignoriert, der Schlüssel "TEST" wäre also identisch mit "test" oder "Test".
 *
 * Mehrdimensionale Arrays sind möglich, indem einem Schlüssel als Wert einfach ein anderes Array
 * zugeordnet wird. In einem solchen Array kann jedes Element direkt angesprochen werden, indem man
 * die einzelnen Schlüssel durch Slash (/) getrennt zu einem einzigen Schlüssel zusammenfasst.
 *
 * Mehrdimensionale Arrays werden automatisch generiert. Gibt man bei einem leeren %Array dem Schlüssel
 * <tt>"ebene1/ebene2/key"</tt> einen Wert, werden automatisch folgende Aktionen ausgeführt:
 * - Es wird ein neues AssocArray generiert und mit dem Schlüssel "ebene1" in das %Array eingefügt
 * - In das %Array "ebene1" wird ein weiteres neues %Array mit dem Schlüssel "ebene2" eingefügt
 * - In das %Array "ebene2" wird der eigentliche Wert unter dem Schlüssel "key" eingefügt
 */
class AssocArray
{
private:
    /** @class AssocArray::ArrayKey
     * @brief Datentyp für Schlüssel
     *
     * Das AssocArray verwendet einen eigenen von der String-Klasse abgeleiteten Datentyp. Dieser
     * unterscheidet sich von der String-Klasse nur durch die Vergleichoperatoren. Diese behandelt
     * rein nummerische Schlüssel anders als alphanummerische.
     *
     */
    class ArrayKey : public String
    {
    private:
        int compare(const ArrayKey& str) const;

    public:
        ArrayKey();
        inline ArrayKey(const String& other)
            : String(other)
        {
        }

        inline ArrayKey& operator=(const String& str)
        {
            set(str);
            return *this;
        }
        inline bool operator<(const ArrayKey& str) const
        {
            return compare(str) < 0;
        }
        inline bool operator<=(const ArrayKey& str) const
        {
            return compare(str) <= 0;
        }

        inline bool operator==(const ArrayKey& str) const
        {
            return compare(str) == 0;
        }

        inline bool operator!=(const ArrayKey& str) const
        {
            return compare(str) != 0;
        }

        inline bool operator>=(const ArrayKey& str) const
        {
            return compare(str) >= 0;
        }

        inline bool operator>(const ArrayKey& str) const
        {
            return compare(str) > 0;
        }
    };

    std::map<ArrayKey, Variant*> Tree;
    uint64_t maxint;

    Variant* findInternal(const ArrayKey& key) const;
    void createTree(const ArrayKey& key, Variant* var);

public:
    PPLIBEXCEPTION(InvalidKeyException, Exception);
    PPLIBEXCEPTION(ExportBufferToSmallException, Exception);
    PPLIBEXCEPTION(ImportFailedException, Exception);

    typedef std::map<ArrayKey, Variant*>::iterator iterator;
    typedef std::map<ArrayKey, Variant*>::const_iterator const_iterator;
    typedef std::map<ArrayKey, Variant*>::reverse_iterator reverse_iterator;
    typedef std::map<ArrayKey, Variant*>::const_reverse_iterator const_reverse_iterator;

    class Iterator
    {
    private:
        friend class AssocArray;
        const_iterator it;
        Variant empty;
        bool reset;

    public:
        Iterator()
        {
            reset = true;
        }
        const String& key()
        {
            return (*it).first;
        }
        const Variant& value()
        {
            if ((*it).second == NULL) return empty;
            return *(*it).second;
        };
    };
    class ReverseIterator
    {
    private:
        friend class AssocArray;
        const_reverse_iterator it;
        Variant empty;
        bool reset;

    public:
        ReverseIterator()
        {
            reset = true;
        }
        const String& key()
        {
            return (*it).first;
        }
        const Variant& value()
        {
            if ((*it).second == NULL) return empty;
            return *(*it).second;
        };
    };

    //!\name Konstruktoren und Destruktoren
    //@{
    AssocArray();
    AssocArray(const AssocArray& other);
    ~AssocArray();
    //@}

    //!\name Informationen ausgeben/auslesen
    //@{
    size_t count(bool recursive = false) const;
    size_t count(const String& key, bool recursive = false) const;
    size_t size() const;
    void list(const String& prefix = "") const;

    //@}

    //!\name Werte setzen
    //@{
    void add(const AssocArray& other);
    void set(const String& key, const String& value);
    void set(const String& key, const String& value, size_t size);
    void set(const String& key, const WideString& value);
    void set(const String& key, const Array& value);
    void set(const String& key, const DateTime& value);
    void set(const String& key, const ByteArray& value);
    void set(const String& key, const ByteArrayPtr& value);
    void set(const String& key, const AssocArray& value);
    void set(const String& key, const Variant& value);
    void setf(const String& key, const char* fmt, ...);
    //@}

    //!\name Werte erweitern (nur Strings)
    //@{
    void append(const String& key, const String& value, const String& concat = "");
    void appendf(const String& key, const String& concat, const char* fmt, ...);
    //@}

    //!\name Werte löschen
    //@{
    void clear();
    void erase(const String& key);
    void remove(const String& key);
    //@}

    //!\name Import und Export von Daten
    //@{
    size_t fromTemplate(const String& templ,
                        const String& linedelimiter = "\n",
                        const String& splitchar = "=",
                        const String& concat = "\n",
                        bool dotrim = false);
    size_t fromConfig(const String& content,
                      const String& linedelimiter = "\n",
                      const String& splitchar = "=",
                      const String& concat = "\n",
                      bool dotrim = false);
    void toTemplate(String& s, const String& prefix = "", const String& linedelimiter = "\n", const String& splitchar = "=") const;
    size_t binarySize() const;
    void exportBinary(void* buffer, size_t buffersize, size_t* realsize) const;
    void exportBinary(ByteArray& buffer) const;
    size_t importBinary(const void* buffer, size_t buffersize);
    void importBinary(const ByteArrayPtr& buffer);
    //@}

    //!\name Werte direkt auslesen
    //@{
    Variant& get(const String& key) const;
    String& getString(const String& key) const;
    String& getString(const String& key, String& default_value) const;
    const String& getString(const String& key, const String& default_value) const;
    int getInt(const String& key) const;
    int getInt(const String& key, int default_value) const;
    long long getLongLong(const String& key) const;
    long long getLongLong(const String& key, long long default_value) const;
    AssocArray& getAssocArray(const String& key) const;
    AssocArray& getAssocArray(const String& key, AssocArray& default_value) const;
    Array& getArray(const String& key) const;
    Array& getArray(const String& key, Array& default_value) const;
    bool getBoolean(const String& key, bool default_value) const;
    bool exists(const String& key) const;
    bool isTrue(const String& key) const;

    //@}

    //!\name Array durchwandern
    //@{
    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
    reverse_iterator rbegin();
    const_reverse_iterator rbegin() const;
    reverse_iterator rend();
    const_reverse_iterator rend() const;

    // TODO: brauchen wir die noch?
    void reset(Iterator& it) const;
    void reset(ReverseIterator& it) const;
    bool getFirst(Iterator& it, Variant::DataType type = Variant::TYPE_UNKNOWN) const;
    bool getNext(Iterator& it, Variant::DataType type = Variant::TYPE_UNKNOWN) const;
    bool getLast(ReverseIterator& it, Variant::DataType type = Variant::TYPE_UNKNOWN) const;
    bool getPrevious(ReverseIterator& it, Variant::DataType type = Variant::TYPE_UNKNOWN) const;

    bool getFirst(Iterator& it, String& key, String& value) const;
    bool getNext(Iterator& it, String& key, String& value) const;
    bool getLast(ReverseIterator& it, String& key, String& value) const;
    bool getPrevious(ReverseIterator& it, String& key, String& value) const;
    //@}

    //!\name Operatoren
    //@{
    Variant& operator[](const String& key);
    const Variant& operator[](const String& key) const;
    AssocArray& operator=(const AssocArray& other);
    AssocArray& operator+=(const AssocArray& other);

    bool operator==(const AssocArray& other) const;
    bool operator!=(const AssocArray& other) const;
    //@}
};
AssocArray operator+(const AssocArray& a1, const AssocArray& a2);

} // namespace pplib

#endif /* PPLIB_TYPES_ASSOCARRAY_H_ */