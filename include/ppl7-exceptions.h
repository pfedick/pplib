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
		Exception(const char *msg, ...) throw();
		virtual ~Exception() throw();
		virtual const char* what() const throw();
		const char* text() const throw();
		String toString() const throw();
		void print() const;
		void copyText(const char *str) throw();
		void copyText(const char *fmt, va_list args) throw();
};

std::ostream& operator<<(std::ostream& s, const Exception &e);


#define STR_VALUE(arg)      #arg
#define PPLEXCEPTION(name,inherit)	class name : public ppl7::inherit { public: \
	name() throw() {}; \
	name(const char *msg, ...) throw() {  \
		va_list args; va_start(args, msg); copyText(msg,args); \
		va_end(args); } \
		virtual const char* what() const throw() { return (STR_VALUE(name)); } \
	};

PPLEXCEPTION(UnknownException, Exception);
PPLEXCEPTION(OutOfMemoryException, Exception);
PPLEXCEPTION(NullPointerException, Exception);
PPLEXCEPTION(UnsupportedFeatureException, Exception);
PPLEXCEPTION(CharacterEncodingException, Exception);
PPLEXCEPTION(UnsupportedCharacterEncodingException, Exception);
PPLEXCEPTION(OutOfBoundsEception, Exception);
PPLEXCEPTION(EmptyDataException, Exception);
PPLEXCEPTION(TypeConversionException, Exception);
PPLEXCEPTION(IllegalArgumentException, Exception);
PPLEXCEPTION(MissingArgumentException, Exception);
PPLEXCEPTION(IllegalRegularExpressionException, Exception);
PPLEXCEPTION(OperationFailedException, Exception);
PPLEXCEPTION(DuplicateInstanceException, Exception);
PPLEXCEPTION(ConnectionFailedException, Exception);
PPLEXCEPTION(SocketException, Exception);
PPLEXCEPTION(LoginRefusedException, Exception);
PPLEXCEPTION(AlreadyConnectedException, Exception);
PPLEXCEPTION(NoConnectionException, Exception);
PPLEXCEPTION(TooManyInstancesException, Exception);
PPLEXCEPTION(InvalidDateException, Exception);
PPLEXCEPTION(DateOutOfRangeException, Exception);
PPLEXCEPTION(NoThreadSupportException, Exception);
PPLEXCEPTION(ThreadStartException, Exception);
PPLEXCEPTION(ThreadAlreadyRunningException, Exception);
PPLEXCEPTION(ThreadOperationFailedException, Exception);
PPLEXCEPTION(ThreadAlreadyInPoolException, Exception);
PPLEXCEPTION(ThreadNotInPoolException, Exception);
PPLEXCEPTION(ItemNotFoundException, Exception);
PPLEXCEPTION(DuplicateItemException, Exception);
PPLEXCEPTION(UnsupportedDataTypeException, Exception);
PPLEXCEPTION(ItemNotFromThisListException, Exception);
PPLEXCEPTION(EndOfListException, Exception);
PPLEXCEPTION(IllegalMemoryAddressException, Exception);
PPLEXCEPTION(UnimplementedVirtualFunctionException, Exception);
PPLEXCEPTION(UnknownCompressionMethodException, Exception);
PPLEXCEPTION(IllegalChunkException, Exception);
PPLEXCEPTION(ChunkNotFoundException, Exception);
PPLEXCEPTION(EmptyFileException, Exception);
PPLEXCEPTION(CompressionFailedException, Exception);
PPLEXCEPTION(DecompressionFailedException, Exception);
PPLEXCEPTION(InvalidFormatException, Exception);
PPLEXCEPTION(AccessDeniedByInstanceException, Exception);
PPLEXCEPTION(BufferTooSmallException, Exception);
PPLEXCEPTION(CorruptedDataException, Exception);
PPLEXCEPTION(FailedToLoadResourceException, Exception);
PPLEXCEPTION(InvalidResourceException, Exception);
PPLEXCEPTION(ResourceNotFoundException, Exception);
PPLEXCEPTION(OperationUnavailableException, Exception);
PPLEXCEPTION(UnavailableException, Exception);
PPLEXCEPTION(InitializationFailedException, Exception);
PPLEXCEPTION(KeyNotFoundException, Exception);
PPLEXCEPTION(InvalidTimezoneException, Exception);
PPLEXCEPTION(CharacterEncodingNotInitializedException, Exception);

PPLEXCEPTION(SyntaxException, Exception);

PPLEXCEPTION(NoSectionSelectedException, Exception);
PPLEXCEPTION(UnknownSectionException, Exception);

PPLEXCEPTION(SSLException, Exception);
PPLEXCEPTION(SSLContextInUseException, Exception);
PPLEXCEPTION(SSLContextUninitializedException, Exception);
PPLEXCEPTION(SSLContextReferenceCounterMismatchException, Exception);
PPLEXCEPTION(InvalidSSLCertificateException, Exception);
PPLEXCEPTION(InvalidSSLCipherException, Exception);
PPLEXCEPTION(SSLPrivatKeyException, Exception);



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


PPLEXCEPTION(HostNotFoundException, Exception);
PPLEXCEPTION(TryAgainException, Exception);
PPLEXCEPTION(NoResultException, Exception);
PPLEXCEPTION(TimeoutException, Exception);

PPLEXCEPTION(QueryFailedException, Exception);
PPLEXCEPTION(EscapeFailedException, Exception);
PPLEXCEPTION(FieldNotInResultSetException, Exception);



}	// EOF namespace ppl7

#endif /* PPL7EXCEPTIONS_H_ */
