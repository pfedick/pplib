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
 * Copyright (c) 2013, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright notice, this
 *       list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
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

void throwExceptionFromErrno(int e,const String &info);
void throwExceptionFromEaiError(int ecode, const String &info);

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

std::ostream& operator<<(std::ostream& s, const Exception &e);


#define STR_VALUE(arg)      #arg
#define PPLNORMALEXCEPTION(name)	class name : public ppl7::Exception { public: \
		virtual const char* what() const throw() { return (STR_VALUE(name)); } \
	};

#define PPLPARAMETERISEDEXCEPTION(name)	class name : public ppl7::Exception { public: \
	name() {}; \
	name(const wchar_t *msg) { \
		copyText(msg); \
	}; \
	name(const char *msg, ...) {  \
		va_list args; va_start(args, msg); copyText(msg,args); \
		va_end(args); } \
		virtual const char* what() const throw() { return (STR_VALUE(name)); } \
	};


#define PPLEXCEPTION(name,inherit)	class name : public ppl7::inherit { public: \
	name() {}; \
	name(const wchar_t *msg) { \
		copyText(msg); \
	}; \
	name(const char *msg, ...) {  \
		va_list args; va_start(args, msg); copyText(msg,args); \
		va_end(args); } \
		virtual const char* what() const throw() { return (STR_VALUE(name)); } \
	};

PPLPARAMETERISEDEXCEPTION(UnknownException);
PPLNORMALEXCEPTION(OutOfMemoryException);
PPLNORMALEXCEPTION(NullPointerException);
PPLPARAMETERISEDEXCEPTION(UnsupportedFeatureException);
PPLNORMALEXCEPTION(CharacterEncodingException);
PPLNORMALEXCEPTION(UnsupportedCharacterEncodingException);
PPLNORMALEXCEPTION(OutOfBoundsEception);
PPLPARAMETERISEDEXCEPTION(EmptyDataException);
PPLNORMALEXCEPTION(TypeConversionException);
PPLPARAMETERISEDEXCEPTION(IllegalArgumentException);
PPLPARAMETERISEDEXCEPTION(MissingArgumentException);
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
PPLPARAMETERISEDEXCEPTION(DateOutOfRangeException);
PPLNORMALEXCEPTION(NoThreadSupportException);
PPLNORMALEXCEPTION(ThreadStartException);
PPLNORMALEXCEPTION(ThreadAlreadyRunningException);
PPLNORMALEXCEPTION(ThreadOperationFailedException);
PPLNORMALEXCEPTION(ItemNotFoundException);
PPLNORMALEXCEPTION(DuplicateItemException);
PPLNORMALEXCEPTION(UnsupportedDataTypeException);
PPLNORMALEXCEPTION(ItemNotFromThisListException);
PPLNORMALEXCEPTION(EndOfListException);
PPLNORMALEXCEPTION(IllegalMemoryAddressException);
PPLNORMALEXCEPTION(UnimplementedVirtualFunctionException);
PPLNORMALEXCEPTION(UnknownCompressionMethodException);
PPLNORMALEXCEPTION(IllegalChunkException);
PPLNORMALEXCEPTION(ChunkNotFoundException);
PPLNORMALEXCEPTION(EmptyFileException);
PPLNORMALEXCEPTION(CompressionFailedException);
PPLNORMALEXCEPTION(DecompressionFailedException);
PPLPARAMETERISEDEXCEPTION(InvalidFormatException);
PPLNORMALEXCEPTION(AccessDeniedByInstanceException);
PPLNORMALEXCEPTION(BufferTooSmallException);
PPLNORMALEXCEPTION(CorruptedDataException);
PPLNORMALEXCEPTION(FailedToLoadResourceException);
PPLNORMALEXCEPTION(InvalidResourceException);
PPLNORMALEXCEPTION(ResourceNotFoundException);
PPLNORMALEXCEPTION(OperationUnavailableException);
PPLNORMALEXCEPTION(UnavailableException);
PPLPARAMETERISEDEXCEPTION(InitializationFailedException);
PPLPARAMETERISEDEXCEPTION(KeyNotFoundException);

PPLNORMALEXCEPTION(NoSectionSelectedException);
PPLPARAMETERISEDEXCEPTION(UnknownSectionException);

PPLPARAMETERISEDEXCEPTION(SSLException);
PPLPARAMETERISEDEXCEPTION(SSLContextInUseException);
PPLNORMALEXCEPTION(SSLContextUninitializedException);
PPLNORMALEXCEPTION(SSLContextReferenceCounterMismatchException);
PPLPARAMETERISEDEXCEPTION(InvalidSSLCertificateException);
PPLPARAMETERISEDEXCEPTION(InvalidSSLCipherException);
PPLPARAMETERISEDEXCEPTION(SSLPrivatKeyException);


//! @name IO-Exceptions
//@{
PPLEXCEPTION(IOException,Exception);

PPLEXCEPTION(FileNotOpenException, IOException);
PPLEXCEPTION(FileSeekException, IOException);
PPLEXCEPTION(ReadException, IOException);
PPLEXCEPTION(WriteException, IOException);
PPLEXCEPTION(EndOfFileException, IOException);
PPLEXCEPTION(FileOpenException, IOException);
PPLEXCEPTION(FileNotFoundException, IOException);					// ENOENT
PPLEXCEPTION(InvalidArgumentsException, IOException);				// EINVAL
PPLEXCEPTION(InvalidFileNameException, IOException);				// ENOTDIR, ENAMETOOLONG, ELOOP
PPLEXCEPTION(PermissionDeniedException, IOException);				// EACCESS, EPERM
PPLEXCEPTION(ReadOnlyException, IOException);						// EROFS
PPLEXCEPTION(NoRegularFileException, IOException);					// EISDIR
PPLEXCEPTION(TooManyOpenFilesException, IOException);				// EMFILE
PPLEXCEPTION(UnsupportedFileOperationException, IOException);		// EOPNOTSUPP
PPLEXCEPTION(TooManySymbolicLinksException, IOException);			// ELOOP
PPLEXCEPTION(FilesystemFullException, IOException);					// ENOSPC
PPLEXCEPTION(QuotaExceededException, IOException);					// EDQUOT
PPLEXCEPTION(IOErrorException, IOException);						// EIO
PPLEXCEPTION(BadFiledescriptorException, IOException);				// EABDF
PPLEXCEPTION(BadAddressException, IOException);						// EFAULT
PPLEXCEPTION(OverflowException, IOException);						// EOVERFLOW
PPLEXCEPTION(FileExistsException, IOException);						// EEXIST
PPLEXCEPTION(OperationBlockedException, IOException);				// EAGAIN
PPLEXCEPTION(DeadlockException, IOException);						// EDEADLK
PPLEXCEPTION(OperationInterruptedException, IOException);			// EINTR
PPLEXCEPTION(TooManyLocksException, IOException);					// ENOLCK
PPLEXCEPTION(IllegalOperationOnPipeException, IOException);			// ESPIPE
PPLEXCEPTION(BufferExceedsLimitException, IOException);

//@}


PPLPARAMETERISEDEXCEPTION(HostNotFoundException);
PPLNORMALEXCEPTION(TryAgainException);
PPLPARAMETERISEDEXCEPTION(NoResultException);
PPLEXCEPTION(TimeoutException,Exception);



}	// EOF namespace ppl7

#endif /* PPL7EXCEPTIONS_H_ */
