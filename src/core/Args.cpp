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
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include <errno.h>
#include <limits.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN // Keine MFCs
#include <windows.h>
#endif

#include <pplib/core/functions.h>
#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/bytearrayptr.h>

namespace pplib
{
static bool MatchArg(const char* arg, const String& argument, size_t argl)
{
    if (strncmp(arg, argument, argl) != 0) return false;
    // Wenn argument mit "--" beginnt (lange Option) und nicht mit '=' endet:
    // arg darf an Position argl nur '\0' (exakter Treffer) oder '=' (Trenner für Wert) haben.
    if (argl >= 2 && argument[0] == '-' && argument[1] == '-' && argument[argl - 1] != '=') {
        if (arg[argl] != '\0' && arg[argl] != '=') {
            return false;
        }
    }
    return true;
}

String GetArgv(int argc, char* argv[], const String& argument)
{
    if (!argv || argc <= 1 || argument.isEmpty()) return String();
    size_t argl = strlen(argument);
    for (int i = 1; i < argc; i++) {
        if (!argv[i]) continue;
        if (MatchArg(argv[i], argument, argl)) {
            size_t l = strlen(argv[i]);
            bool hasEqual = (strchr(argv[i], '=') != nullptr);
            if (l > argl || hasEqual || i + 1 >= argc || argv[i + 1] == NULL) {
                const char* ret = (argv[i] + argl);
                if (ret[0] == '=' && (argl == 0 || argument[argl - 1] != '=')) {
                    ret++;
                }
                return String(ret);
            } else {
                const char* ret = (argv[i + 1]);
                if (ret[0] == '-') return String();
                if (ret[0] == '\\' && ret[1] == '-') return ret + 1;
                return String(ret);
            }
        }
    }
    return String();
}

bool HaveArgv(int argc, char* argv[], const String& argument)
{
    if (!argv || argc <= 1 || argument.isEmpty()) return false;
    size_t argl = strlen(argument);
    for (int i = 1; i < argc; i++) {
        if (!argv[i]) continue;
        if (MatchArg(argv[i], argument, argl)) {
            return true;
        }
    }
    return false;
}

} // namespace pplib
