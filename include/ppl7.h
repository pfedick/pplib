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

#ifndef _PPL7_INCLUDE
#define _PPL7_INCLUDE

#define PPL7_VERSION_MAJOR 7
#define PPL7_VERSION_MINOR 2
#define PPL7_VERSION_BUILD 0
#define PPL7_RELEASEDATE 20260725
#define PPL7_COPYRIGHT "Copyright (c) 2026 by Patrick Fedick"

#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <set>
#include <list>
#include <vector>

#ifdef PPL_WITH_QT6
#include <QAnyStringView>
#ifndef WITH_QT
#define WITH_QT
#endif
#endif

#ifdef WITH_QT
#include <QString>
#include <QVariant>
#include <QByteArray>
#endif

#include <ppl7-exceptions.h>
#include <ppl7/core/memoryheap.h>
#include <ppl7/core/memorygroup.h>

// Inlcude PPL7 Data-Types
#include <ppl7/types.h>
// Inlcude PPL7 Functions
#include <ppl7/core/functions.h>

#include <ppl7/core/regex.h>
#include <ppl7/core/timer.h>
#include <ppl7/core/mutex.h>
#include <ppl7/core/signal.h>
#include <ppl7/core/threads.h>
#include <ppl7/core/fileobject.h>
#include <ppl7/core/memfile.h>
#include <ppl7/core/file.h>
#include <ppl7/core/pfpfile.h>
#include <ppl7/core/resource.h>

namespace ppl7
{

class PerlHelper
{
public:
    static String escapeString(const String& s);
    static String escapeRegExp(const String& s);
    static String toHash(const AssocArray& a, const String& s);
};

class PythonHelper
{
public:
    static String escapeString(const String& s);
    static String escapeRegExp(const String& s);
    static String toHash(const AssocArray& a, const String& name, int indention = 0);
};

#define NUMFACILITIES 9

class LogHandler;

//! \brief Allgemeine Logging-Klasse
class Logger
{
public:
    enum PRIORITY
    {
        EMERG = 1,
        ALERT = 2,
        CRIT = 3,
        ERR = 4,
        WARNING = 5,
        NOTICE = 6,
        INFO = 7,
        DEBUG = 8
    };

    enum SYSLOG_FACILITY
    {
        SYSLOG_AUTH = 1,
        SYSLOG_AUTHPRIV,
        SYSLOG_CONSOLE,
        SYSLOG_CRON,
        SYSLOG_DAEMON,
        SYSLOG_FTP,
        SYSLOG_KERN,
        SYSLOG_LPR,
        SYSLOG_MAIL,
        SYSLOG_NEWS,
        SYSLOG_NTP,
        SYSLOG_SECURITY,
        SYSLOG_SYSLOG,
        SYSLOG_USER,
        SYSLOG_UUCP,
        SYSLOG_LOCAL0,
        SYSLOG_LOCAL1,
        SYSLOG_LOCAL2,
        SYSLOG_LOCAL3,
        SYSLOG_LOCAL4,
        SYSLOG_LOCAL5,
        SYSLOG_LOCAL6,
        SYSLOG_LOCAL7
    };

private:
    Mutex mutex;
    AssocArray *FilterModule, *FilterFile;
    String ProgIdentity;
    int debuglevel[NUMFACILITIES];
    bool console_enabled;
    PRIORITY console_priority;
    int console_level;
    File logff[NUMFACILITIES];
    String logfilename[NUMFACILITIES];
    void *firsthandler, *lasthandler;
    bool logconsole;
    bool logThreadId;
    int rotate_mechanism;
    uint64_t maxsize;
    int generations;
    bool inrotate;
    bool useSyslog;
    SYSLOG_FACILITY syslogFacility;
    String syslogIdent;

    bool shouldPrint(const char* module, const char* function, const char* file, int line, PRIORITY prio, int level);
    int isFiltered(const char* module, const char* function, const char* file, int line, int level);
    void output(PRIORITY prio,
                int level,
                const char* module,
                const char* function,
                const char* file,
                int line,
                const String& buffer,
                bool printdate = true);
    void outputArray(PRIORITY prio,
                     int level,
                     const char* module,
                     const char* function,
                     const char* file,
                     int line,
                     const AssocArray& a,
                     const char* prefix,
                     String* Out = NULL);
    void checkRotate(PRIORITY prio);

public:
    Logger();
    ~Logger();
    void terminate();
    void addLogHandler(LogHandler* handler);
    void deleteLogHandler(LogHandler* handler);
    void setLogfile(PRIORITY prio, const String& filename);
    void setLogfile(PRIORITY prio, const String& filename, int level);
    void setLogLevel(PRIORITY prio, int level = 1);
    int getLogLevel(PRIORITY prio);
    String getLogfile(PRIORITY prio);
    void setLogRotate(uint64_t maxsize, int generations);
    void enableConsole(bool flag = true, PRIORITY prio = Logger::DEBUG, int level = 1);
    void openSyslog(const String& ident, SYSLOG_FACILITY facility = SYSLOG_USER);
    void closeSyslog();
    void printException(const Exception& e);
    void printException(const char* file, int line, const Exception& e);
    void printException(const char* file, int line, const char* module, const char* function, const Exception& e);
    void print(const String& text);
    void print(int level, const String& text);
    void print(PRIORITY prio, int level, const String& text);
    void print(PRIORITY prio, int level, const char* file, int line, const String& text);
    void print(PRIORITY prio, int level, const char* module, const char* function, const char* file, int line, const String& text);
    void printf(const String& text, ...);
    void printf(int level, const String& text, ...);
    void printf(PRIORITY prio, int level, const String& text, ...);
    void printf(PRIORITY prio, int level, const char* file, int line, const String& text, ...);
    void printf(PRIORITY prio, int level, const char* module, const char* function, const char* file, int line, const String& text, ...);
    void printArray(PRIORITY prio, int level, const AssocArray& a, const String& text);
    void printArray(PRIORITY prio,
                    int level,
                    const char* module,
                    const char* function,
                    const char* file,
                    int line,
                    const AssocArray& a,
                    const String& text);
    void printArraySingleLine(PRIORITY prio,
                              int level,
                              const char* module,
                              const char* function,
                              const char* file,
                              int line,
                              const AssocArray& a,
                              const String& text);
    void hexDump(PRIORITY prio, int level, const void* address, int bytes);
    void hexDump(const void* address, int bytes);
    void setFilter(const char* module, const char* function, int level);
    void setFilter(const char* file, int line, int level);
    void deleteFilter(const char* module, const char* function);
    void deleteFilter(const char* file, int line);
};

//! \brief Log-Handler
class LogHandler
{
public:
    virtual ~LogHandler() = 0;
    virtual void logMessage(Logger::PRIORITY prio, int level, const String& msg) = 0;
};

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

class Json
{
public:
    static void loads(ppl7::AssocArray& data, const ppl7::String& json);
    static void load(ppl7::AssocArray& data, ppl7::FileObject& file);
    static ppl7::AssocArray loads(const ppl7::String& json);
    static ppl7::AssocArray load(ppl7::FileObject& file);

    static void dumps(ppl7::String& json, const ppl7::AssocArray& data);
    static void dump(ppl7::FileObject& file, const ppl7::AssocArray& data);
    static ppl7::String dumps(const ppl7::AssocArray& data);
    static ppl7::String pp(const ppl7::String& json);
};

}; // namespace ppl7

#include <ppl7/core/compression.h>
#include <ppl7/core/iconv.h>

#endif // #ifndef _PPL7_INCLUDE
