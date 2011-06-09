/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 *
 *******************************************************************************
 * Copyright (c) 2011, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#ifndef PPL7EXCEPTIONS_H_
#define PPL7EXCEPTIONS_H_
#include <exception>

namespace ppl7 {

class String;

class Exception : std::exception
{
	private:
		char *ErrorText;
	public:
		Exception() throw();
		Exception(const Exception &other) throw();
		Exception& operator= (const Exception &other) throw();
		Exception(const char *msg, ...);
		virtual ~Exception() throw();
		virtual const char* what() const throw();
		const char* text() const throw();
		String toString() const throw();
		void print() const;
		void copyText(const char *str) throw();
		void copyText(const wchar_t *str) throw();
		void copyText(const char *fmt, va_list args) throw();
};

#define STR_VALUE(arg)      #arg
#define PPLNORMALEXCEPTION(name)	class name : public ppl7::Exception { public: \
		virtual const char* what() const throw() { return STR_VALUE(name); } \
	};
#define PPLPARAMETERISEDEXCEPTION(name)	class name : public ppl7::Exception { public: \
	name() {}; \
	name(const wchar_t *msg) { \
		copyText(msg); \
	}; \
	name(const char *msg, ...) {  \
		va_list args; va_start(args, msg); copyText(msg,args); \
		va_end(args); } \
		virtual const char* what() const throw() { return STR_VALUE(name); } \
	};


PPLNORMALEXCEPTION(UnknownException);
PPLNORMALEXCEPTION(OutOfMemoryException);
PPLNORMALEXCEPTION(NullPointerException);
PPLPARAMETERISEDEXCEPTION(UnsupportedFeatureException);
PPLNORMALEXCEPTION(CharacterEncodingException);
PPLNORMALEXCEPTION(UnsupportedCharacterEncodingException);
PPLNORMALEXCEPTION(OutOfBoundsEception);
PPLNORMALEXCEPTION(EmptyDataException);
PPLNORMALEXCEPTION(TypeConversionException);
PPLPARAMETERISEDEXCEPTION(IllegalArgumentException);
PPLNORMALEXCEPTION(IllegalRegularExpressionException);
PPLPARAMETERISEDEXCEPTION(OperationFailedException);
PPLPARAMETERISEDEXCEPTION(DuplicateInstanceException);
PPLNORMALEXCEPTION(ConnectionFailedException);
PPLNORMALEXCEPTION(SocketException);
PPLNORMALEXCEPTION(LoginRefusedException);
PPLNORMALEXCEPTION(AlreadyConnectedException);
PPLNORMALEXCEPTION(NoConnectionException);
PPLPARAMETERISEDEXCEPTION(TooManyInstancesException);
PPLPARAMETERISEDEXCEPTION(InvalidDateException);
PPLNORMALEXCEPTION(NoThreadSupportException);
PPLNORMALEXCEPTION(ThreadStartException);
PPLNORMALEXCEPTION(ThreadOperationFailedException);
PPLNORMALEXCEPTION(ItemNotFoundException);
PPLNORMALEXCEPTION(DuplicateItemException);
PPLNORMALEXCEPTION(UnsupportedDataTypeException);
PPLNORMALEXCEPTION(ItemNotFromThisListException);
PPLNORMALEXCEPTION(EndOfListException);
PPLNORMALEXCEPTION(IllegalMemoryAddressException);
PPLNORMALEXCEPTION(UnimplementedVirtualFunctionException);
PPLNORMALEXCEPTION(UnknownCompressionMethodException);



}	// EOF namespace ppl7

#endif /* PPL7EXCEPTIONS_H_ */
