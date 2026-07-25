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

#ifndef PPL7_CORE_CONFIGPARSER_H_
#define PPL7_CORE_CONFIGPARSER_H_

#include <ppl7/types/string.h>
#include <ppl7/types/assocarray.h>
#include <ppl7/core/fileobject.h>

namespace ppl7
{
class ConfigParser
{
private:
    String separator;
    void *first, *last, *section;
    AssocArray sections;
    AssocArray::Iterator it;

    void* findSection(const String& sectionname) const;

public:
    ConfigParser();
    ConfigParser(const String& filename);
    ConfigParser(FileObject& file);
    ~ConfigParser();
    void load(const String& filename);
    void load(FileObject& file);
    void loadFromString(const String& string);
    void loadFromMemory(const void* buffer, size_t bytes);
    void loadFromMemory(const ByteArrayPtr& ptr);
    void save(const String& filename);
    void save(FileObject& file);
    void unload();
    void setSeparator(const String& string);
    const String& getSeparator() const;
    void selectSection(const String& section);
    int firstSection();
    int nextSection();
    const String& getSectionName() const;
    const String& getSection(const String& name) const;
    void copySection(AssocArray& target, const String& section) const;
    void createSection(const String& name);
    void setSection(const String& name);
    void deleteSection(const String& name);
    bool hasSection(const String& sectionname) const;
    void add(const String& section, const String& key, const String& value);
    void add(const String& section, const String& key, const char* value);
    void add(const String& key, const String& value);
    void add(const String& key, const char* value);
    void add(const String& key, int value);
    void add(const String& key, bool value);
    void add(const String& section, const String& key, int value);
    void add(const String& section, const String& key, bool value);
    void deleteKey(const String& key);
    void deleteKey(const String& section, const String& key);
    bool hasKey(const String& key) const;
    bool hasKey(const String& section, const String& key) const;
    const String& get(const String& key, const String& defaultvalue = String()) const;
    bool getBool(const String& key, bool defaultvalue = false) const;
    int getInt(const String& key, int defaultvalue = 0) const;
    const String& getFromSection(const String& section, const String& key, const String& defaultvalue = String()) const;
    bool getBoolFromSection(const String& section, const String& key, bool defaultvalue = false) const;
    int getIntFromSection(const String& section, const String& key, int defaultvalue = 0) const;
    void reset(); // Zum Auslesen einer kompletten Section
    bool getFirst(String& key, String& value);
    bool getNext(String& key, String& value);
    void print() const;
};

} // namespace ppl7
#endif /* PPL7_CORE_CONFIGPARSER_H_ */