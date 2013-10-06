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
PPLNORMALEXCEPTION(InvalidFormatException);
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
PPLNORMALEXCEPTION(FileNotOpenException);
PPLNORMALEXCEPTION(FileSeekException);
PPLNORMALEXCEPTION(ReadException);
PPLNORMALEXCEPTION(WriteException);
PPLNORMALEXCEPTION(EndOfFileException);
PPLPARAMETERISEDEXCEPTION(FileOpenException);
PPLPARAMETERISEDEXCEPTION(FileNotFoundException);				// ENOENT
PPLNORMALEXCEPTION(InvalidArgumentsException);					// EINVAL
PPLPARAMETERISEDEXCEPTION(InvalidFileNameException);			// ENOTDIR, ENAMETOOLONG, ELOOP
PPLPARAMETERISEDEXCEPTION(PermissionDeniedException);			// EACCESS, EPERM
PPLPARAMETERISEDEXCEPTION(ReadOnlyException);					// EROFS
PPLPARAMETERISEDEXCEPTION(NoRegularFileException);				// EISDIR
PPLNORMALEXCEPTION(TooManyOpenFilesException);					// EMFILE
PPLPARAMETERISEDEXCEPTION(UnsupportedFileOperationException);	// EOPNOTSUPP
PPLPARAMETERISEDEXCEPTION(TooManySymbolicLinksException);		// ELOOP
PPLNORMALEXCEPTION(FilesystemFullException);					// ENOSPC
PPLNORMALEXCEPTION(QuotaExceededException);						// EDQUOT
PPLNORMALEXCEPTION(IOErrorException);							// EIO
PPLNORMALEXCEPTION(BadFiledescriptorException);					// EABDF
PPLNORMALEXCEPTION(BadAddressException);						// EFAULT
PPLNORMALEXCEPTION(OverflowException);							// EOVERFLOW
PPLNORMALEXCEPTION(FileExistsException);						// EEXIST
PPLNORMALEXCEPTION(OperationBlockedException);					// EAGAIN
PPLNORMALEXCEPTION(DeadlockException);							// EDEADLK
PPLNORMALEXCEPTION(OperationInterruptedException);				// EINTR
PPLNORMALEXCEPTION(TooManyLocksException);						// ENOLCK
PPLNORMALEXCEPTION(IllegalOperationOnPipeException);			// ESPIPE
PPLNORMALEXCEPTION(BufferExceedsLimitException);

//@}


PPLPARAMETERISEDEXCEPTION(HostNotFoundException);
PPLNORMALEXCEPTION(TryAgainException);
PPLPARAMETERISEDEXCEPTION(NoResultException);



}	// EOF namespace ppl7

#endif /* PPL7EXCEPTIONS_H_ */
