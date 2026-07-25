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

#ifndef PPL7_CORE_LOGGING_H_
#define PPL7_CORE_LOGGING_H_

#include <ppl7/types/string.h>
#include <ppl7/types/assocarray.h>
#include <ppl7/core/mutex.h>
#include <ppl7/core/file.h>

namespace ppl7
{
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

} // namespace ppl7
#endif /* PPL7_CORE_LOGGING_H_ */