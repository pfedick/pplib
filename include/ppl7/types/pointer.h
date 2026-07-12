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
#ifndef PPL7_TYPES_POINTER_H_
#define PPL7_TYPES_POINTER_H_

namespace ppl7
{

/**!class Pointer
 * @deprecated This class is deprecated and will be removed in a future version of PPL7. Use a different pointer type instead.
 */
class [[deprecated("Use a different pointer type instead.")]] Pointer
{
private:
    void* myptr;

public:
    Pointer();
    Pointer(const Pointer& other);
    Pointer(void* ptr);
    bool isNull() const;
    const void* ptr() const;
    operator const void*() const;
    operator const char*() const;
    operator const unsigned char*() const;
    void set(const void* ptr);
    Pointer& operator=(const Pointer& other);
    Pointer& operator=(const void* ptr);
    bool operator<(const Pointer& other) const;
    bool operator<=(const Pointer& other) const;
    bool operator==(const Pointer& other) const;
    bool operator!=(const Pointer& other) const;
    bool operator>=(const Pointer& other) const;
    bool operator>(const Pointer& other) const;
};

} // namespace ppl7

#endif /* PPL7_TYPES_POINTER_H_ */
