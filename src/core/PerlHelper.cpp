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

#include <pplib/core/functions.h>
#include <pplib/core/regex.h>

namespace pplib
{

String PerlHelper::escapeString(const String& s)
{
    String ret = s;
    ret.replace("\\", "\\\\");
    ret.replace("\"", "\\\"");
    ret.replace("@", "\\@");
    return ret;
}

String PerlHelper::escapeRegExp(const String& s)
{
    return RegEx::escape(s);
}

static String toHashRecurse(const AssocArray& a, const String& name)
{
    String r;
    String key;
    for (auto it = a.begin(); it != a.end(); ++it) {
        const String& key = it->first;
        const Variant& res = *it->second;
        if (res.isAssocArray()) {
            String newName;
            newName = name + "{" + key + "}";
            r += toHashRecurse(res.toAssocArray(), newName);
        } else {
            r += name + "{" + key + "}=\"" + PerlHelper::escapeString(res.toString()) + "\";\n";
        }
    }
    return r;
}

String PerlHelper::toHash(const AssocArray& a, const String& name)
{
    String ret;
    if (name.isEmpty()) return ret;
    ret = "my %" + name + ";\n";
    String n;
    n = "$" + name;
    ret += toHashRecurse(a, n);
    return ret;
}

} // namespace pplib
