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

#ifndef PPLIB_EXCEPTIONS_H_
#define PPLIB_EXCEPTIONS_H_

#include <pplib/types/string.h>
#include <pplib/core/baseexception.h>

namespace pplib
{

void throwExceptionFromErrno(int e, const String& info);
void throwSocketException(int e, const String& info);
void throwExceptionFromEaiError(int ecode, const String& info);

PPLIBEXCEPTION(UnknownException, Exception);
PPLIBEXCEPTION(OutOfMemoryException, Exception);
PPLIBEXCEPTION(NullPointerException, Exception);
PPLIBEXCEPTION(UnsupportedFeatureException, Exception);
PPLIBEXCEPTION(CharacterEncodingException, Exception);
PPLIBEXCEPTION(UnsupportedCharacterEncodingException, Exception);
PPLIBEXCEPTION(OutOfBoundsException, Exception);
PPLIBEXCEPTION(EmptyDataException, Exception);
PPLIBEXCEPTION(TypeConversionException, Exception);
PPLIBEXCEPTION(IllegalArgumentException, Exception);
PPLIBEXCEPTION(MissingArgumentException, Exception);
PPLIBEXCEPTION(IllegalRegularExpressionException, Exception);
PPLIBEXCEPTION(OperationFailedException, Exception);
PPLIBEXCEPTION(OperationAbortedException, Exception);
PPLIBEXCEPTION(DuplicateInstanceException, Exception);
PPLIBEXCEPTION(ConnectionFailedException, Exception);
PPLIBEXCEPTION(SocketException, Exception);
PPLIBEXCEPTION(LoginRefusedException, Exception);
PPLIBEXCEPTION(AlreadyConnectedException, Exception);
PPLIBEXCEPTION(NoConnectionException, Exception);
PPLIBEXCEPTION(TooManyInstancesException, Exception);
PPLIBEXCEPTION(InvalidDateException, Exception);
PPLIBEXCEPTION(DateOutOfRangeException, Exception);
PPLIBEXCEPTION(NoThreadSupportException, Exception);
PPLIBEXCEPTION(ThreadStartException, Exception);
PPLIBEXCEPTION(ThreadAlreadyRunningException, Exception);
PPLIBEXCEPTION(ThreadOperationFailedException, Exception);
PPLIBEXCEPTION(ThreadAlreadyInPoolException, Exception);
PPLIBEXCEPTION(ThreadNotInPoolException, Exception);
PPLIBEXCEPTION(ItemNotFoundException, Exception);
PPLIBEXCEPTION(DuplicateItemException, Exception);
PPLIBEXCEPTION(UnsupportedDataTypeException, Exception);
PPLIBEXCEPTION(ItemNotFromThisListException, Exception);
PPLIBEXCEPTION(EndOfListException, Exception);
PPLIBEXCEPTION(IllegalMemoryAddressException, Exception);
PPLIBEXCEPTION(UnimplementedVirtualFunctionException, Exception);
PPLIBEXCEPTION(UnknownCompressionMethodException, Exception);
PPLIBEXCEPTION(IllegalChunkException, Exception);
PPLIBEXCEPTION(ChunkNotFoundException, Exception);
PPLIBEXCEPTION(EmptyFileException, Exception);
PPLIBEXCEPTION(CompressionFailedException, Exception);
PPLIBEXCEPTION(DecompressionFailedException, Exception);
PPLIBEXCEPTION(InvalidFormatException, Exception);
PPLIBEXCEPTION(AccessDeniedByInstanceException, Exception);
PPLIBEXCEPTION(BufferTooSmallException, Exception);
PPLIBEXCEPTION(CorruptedDataException, Exception);
PPLIBEXCEPTION(FailedToLoadResourceException, Exception);
PPLIBEXCEPTION(InvalidResourceException, Exception);
PPLIBEXCEPTION(ResourceNotFoundException, Exception);
PPLIBEXCEPTION(OperationUnavailableException, Exception);
PPLIBEXCEPTION(UnavailableException, Exception);
PPLIBEXCEPTION(InitializationFailedException, Exception);
PPLIBEXCEPTION(KeyNotFoundException, Exception);
PPLIBEXCEPTION(InvalidTimezoneException, Exception);
PPLIBEXCEPTION(CharacterEncodingNotInitializedException, Exception);
PPLIBEXCEPTION(MutexException, Exception);
PPLIBEXCEPTION(MutexLockingException, MutexException);
PPLIBEXCEPTION(MutexNotLockedException, MutexLockingException);
PPLIBEXCEPTION(UnexpectedEndOfDataException, Exception);
PPLIBEXCEPTION(InvalidEscapeSequenceException, Exception);
PPLIBEXCEPTION(UnexpectedCharacterException, Exception);
PPLIBEXCEPTION(SyntaxException, Exception);
PPLIBEXCEPTION(IllegalStateException, Exception);

PPLIBEXCEPTION(NoSectionSelectedException, Exception);
PPLIBEXCEPTION(UnknownSectionException, Exception);

PPLIBEXCEPTION(SSLException, Exception);
PPLIBEXCEPTION(SSLContextInUseException, Exception);
PPLIBEXCEPTION(SSLContextUninitializedException, Exception);
PPLIBEXCEPTION(SSLContextReferenceCounterMismatchException, Exception);
PPLIBEXCEPTION(InvalidSSLCertificateException, Exception);
PPLIBEXCEPTION(InvalidSSLCipherException, Exception);
PPLIBEXCEPTION(SSLPrivatKeyException, Exception);
PPLIBEXCEPTION(SSLFailedToReadDHParams, Exception);

//! @name IO-Exceptions
//@{
PPLIBEXCEPTION(IOException, Exception);

PPLIBEXCEPTION(FileNotOpenException, IOException);
PPLIBEXCEPTION(FileSeekException, IOException);
PPLIBEXCEPTION(ReadException, IOException);
PPLIBEXCEPTION(WriteException, IOException);
PPLIBEXCEPTION(EndOfFileException, IOException);
PPLIBEXCEPTION(FileOpenException, IOException);
PPLIBEXCEPTION(FileNotFoundException, IOException);                      // ENOENT
PPLIBEXCEPTION(InvalidArgumentsException, IOException);                  // EINVAL
PPLIBEXCEPTION(InvalidFileNameException, IOException);                   // ENOTDIR, ENAMETOOLONG, ELOOP
PPLIBEXCEPTION(PermissionDeniedException, IOException);                  // EACCESS, EPERM
PPLIBEXCEPTION(ReadOnlyException, IOException);                          // EROFS
PPLIBEXCEPTION(NoRegularFileException, IOException);                     // EISDIR
PPLIBEXCEPTION(TooManyOpenFilesException, IOException);                  // EMFILE
PPLIBEXCEPTION(UnsupportedFileOperationException, IOException);          // EOPNOTSUPP
PPLIBEXCEPTION(TooManySymbolicLinksException, IOException);              // ELOOP
PPLIBEXCEPTION(FilesystemFullException, IOException);                    // ENOSPC
PPLIBEXCEPTION(QuotaExceededException, IOException);                     // EDQUOT
PPLIBEXCEPTION(IOErrorException, IOException);                           // EIO
PPLIBEXCEPTION(BadFiledescriptorException, IOException);                 // EABDF
PPLIBEXCEPTION(BadAddressException, IOException);                        // EFAULT
PPLIBEXCEPTION(OverflowException, IOException);                          // EOVERFLOW
PPLIBEXCEPTION(FileExistsException, IOException);                        // EEXIST
PPLIBEXCEPTION(OperationBlockedException, IOException);                  // EAGAIN
PPLIBEXCEPTION(OperationInProgressException, IOException);               // EINPROGRESS
PPLIBEXCEPTION(DeadlockException, IOException);                          // EDEADLK
PPLIBEXCEPTION(OperationInterruptedException, IOException);              // EINTR
PPLIBEXCEPTION(TooManyLocksException, IOException);                      // ENOLCK
PPLIBEXCEPTION(IllegalOperationOnPipeException, IOException);            // ESPIPE
PPLIBEXCEPTION(NotInitializedException, IOException);                    // WSANOTINITIALISED
PPLIBEXCEPTION(SocketOperationOnNonSocketException, IOException);        // ENOTSOCK
PPLIBEXCEPTION(OperationAlreadyInProgressException, IOException);        // EALREADY
PPLIBEXCEPTION(DestinationAddressRequiredException, IOException);        // EDESTADDRREQ
PPLIBEXCEPTION(MessageTooLongException, IOException);                    // EMSGSIZE
PPLIBEXCEPTION(ProtocolWrongTypeForSocketException, IOException);        // EPROTOTYPE
PPLIBEXCEPTION(ProtocolNotAvailableException, IOException);              // ENOPROTOOPT
PPLIBEXCEPTION(ProtocolFamilyNotSupportedException, IOException);        // EPFNOSUPPORT
PPLIBEXCEPTION(ProtocolNotSupportedException, IOException);              // EPROTONOSUPPORT
PPLIBEXCEPTION(SocketTypeNotSupportedException, IOException);            // ESOCKTNOSUPPORT
PPLIBEXCEPTION(AddressFamilyNotSupportedException, IOException);         // EAFNOSUPPORT
PPLIBEXCEPTION(AddressAlreadyInUseException, IOException);               // EADDRINUSE
PPLIBEXCEPTION(AddressNotAvailableException, IOException);               // EADDRNOTAVAIL
PPLIBEXCEPTION(NetworkDownException, IOException);                       // ENETDOWN
PPLIBEXCEPTION(ConnectionAbortedByNetworkException, IOException);        // ENETRESET
PPLIBEXCEPTION(ConnectionAbortedException, IOException);                 // ECONNABORTED
PPLIBEXCEPTION(ConnectionResetException, IOException);                   // ECONNRESET
PPLIBEXCEPTION(NoBufferSpaceAvailableException, IOException);            // ENOBUFS
PPLIBEXCEPTION(SocketIsConnectedException, IOException);                 // EISCONN
PPLIBEXCEPTION(SocketNotConnectedException, IOException);                // ENOTCONN
PPLIBEXCEPTION(TransportEndpointHasShutdownException, IOException);      // ESHUTDOWN
PPLIBEXCEPTION(HostIsUnreachableException, IOException);                 // EHOSTUNREACH
PPLIBEXCEPTION(DirectoryNotEmptyException, IOException);                 // ENOTEMPTY
PPLIBEXCEPTION(ProcessLimitException, IOException);                      // EPROCLIM
PPLIBEXCEPTION(TooManyUsersException, IOException);                      // EUSERS
PPLIBEXCEPTION(StaleFileHandleException, IOException);                   // ESTALE
PPLIBEXCEPTION(ObjectIsRemoteException, IOException);                    // EREMOTE
PPLIBEXCEPTION(NetworkSubsystemUnavailableException, IOException);       // WSASYSNOTREADY
PPLIBEXCEPTION(UnsupportedWinsockVersionException, IOException);         // WSAVERNOTSUPPORTED
PPLIBEXCEPTION(GracefulShutdownInProgressException, IOException);        // WSAEDISCON
PPLIBEXCEPTION(NoMoreResultsException, IOException);                     // WSAENOMORE, WSA_E_NO_MORE
PPLIBEXCEPTION(CallHasBeenCanceledException, IOException);               // WSAECANCELLED
PPLIBEXCEPTION(ProcedureCallTableIsInvalidException, IOException);       // WSAEINVALIDPROCTABLE
PPLIBEXCEPTION(ServiceProviderIsInvalidException, IOException);          // WSAEINVALIDPROVIDER
PPLIBEXCEPTION(ServiceProviderFailedToInitializeException, IOException); // WSAEPROVIDERFAILEDINIT
PPLIBEXCEPTION(SystemCallFailureException, IOException);                 // WSASYSCALLFAILURE
PPLIBEXCEPTION(ServiceNotFoundException, IOException);                   // WSASERVICE_NOT_FOUND
PPLIBEXCEPTION(ClassTypeNotFoundException, IOException);                 // WSATYPE_NOT_FOUND
PPLIBEXCEPTION(CallWasCanceledException, IOException);                   // WSA_E_CANCELLED
PPLIBEXCEPTION(QueryRefusedException, IOException);                      // WSAEREFUSED
PPLIBEXCEPTION(NonauthoritativeHostNotFound, IOException);               // WSATRY_AGAIN
PPLIBEXCEPTION(UnrecoverableErrorException, IOException);                // WSANO_RECOVERY
PPLIBEXCEPTION(ObjectNotInSignaledStateException, IOException);          // WSA_IO_INCOMPLETE
PPLIBEXCEPTION(OverlappedOperationPendingException, IOException);        // WSA_IO_PENDING
PPLIBEXCEPTION(QoSException, IOException);                               // Windows WSA_QOS_*
PPLIBEXCEPTION(BufferExceedsLimitException, IOException);
PPLIBEXCEPTION(CouldNotOpenDirectoryException, IOException);

//@}

PPLIBEXCEPTION(HostNotFoundException, Exception); // WSAHOST_NOT_FOUND
PPLIBEXCEPTION(TryAgainException, Exception);
PPLIBEXCEPTION(NoResultException, Exception);
PPLIBEXCEPTION(TimeoutException, Exception);

PPLIBEXCEPTION(QueryFailedException, Exception);
PPLIBEXCEPTION(EscapeFailedException, Exception);
PPLIBEXCEPTION(FieldNotInResultSetException, Exception);

//! @name Network-Exceptions
//@{
PPLIBEXCEPTION(NetworkException, Exception);
PPLIBEXCEPTION(WinsockInitialisationFailed, Exception);
PPLIBEXCEPTION(IdnConversionException, Exception);

PPLIBEXCEPTION(NotConnectedException, NetworkException);
PPLIBEXCEPTION(CouldNotOpenSocketException, NetworkException);      // 393
PPLIBEXCEPTION(IllegalPortException, NetworkException);             // 393
PPLIBEXCEPTION(CouldNotBindToInterfaceException, NetworkException); // 394
PPLIBEXCEPTION(ConnectionRefusedException, NetworkException);       // ECONNREFUSED
PPLIBEXCEPTION(NetworkUnreachableException, NetworkException);      // ENETUNREACH
PPLIBEXCEPTION(NetworkDroppedConnectionOnResetException, NetworkException);
PPLIBEXCEPTION(SoftwareCausedConnectionAbortException, NetworkException);
PPLIBEXCEPTION(ConnectionResetByPeerException, NetworkException);
PPLIBEXCEPTION(NoBufferSpaceException, NetworkException);
PPLIBEXCEPTION(SocketIsAlreadyConnectedException, NetworkException);
PPLIBEXCEPTION(CantSendAfterSocketShutdownException, NetworkException);
PPLIBEXCEPTION(TooManyReferencesException, NetworkException); // ETOOMANYREFS
PPLIBEXCEPTION(HostDownException, NetworkException);          // EHOSTDOWN
PPLIBEXCEPTION(NoRouteToHostException, NetworkException);
PPLIBEXCEPTION(InvalidSocketException, NetworkException);
PPLIBEXCEPTION(UnknownOptionException, NetworkException);
PPLIBEXCEPTION(OutOfBandDataReceivedException, NetworkException);
PPLIBEXCEPTION(BrokenPipeException, NetworkException);
PPLIBEXCEPTION(SSLNotStartedException, NetworkException);
PPLIBEXCEPTION(SSLConnectionFailedException, NetworkException);
PPLIBEXCEPTION(SettingSocketOptionException, NetworkException);
PPLIBEXCEPTION(InvalidIpAddressException, NetworkException);
PPLIBEXCEPTION(InvalidNetworkAddressException, NetworkException);
PPLIBEXCEPTION(InvalidNetmaskOrPrefixlenException, NetworkException);
PPLIBEXCEPTION(ConnectionTimeoutException, NetworkException); // ETIMEDOUT
//@}

} // namespace pplib

#endif /* PPLIBEXCEPTIONS_H_ */
