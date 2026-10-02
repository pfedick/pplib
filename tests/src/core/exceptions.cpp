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

#include <gtest/gtest.h>

#include <pplib/types/string.h>
#include <pplib/exceptions.h>
#include <errno.h>

#ifdef _WIN32
#include <winsock2.h>
#include <winerror.h>
#endif

#include "pplib-tests.h"

namespace
{
TEST(ExceptionTest, EmptyException)
{
    try {
        throw pplib::Exception();
    }
    catch (const pplib::Exception& e) {
        EXPECT_STREQ(e.what(), "Exception");
        EXPECT_STREQ(e.className(), "Exception");
    }
    try {
        throw pplib::Exception("");
    }
    catch (const pplib::Exception& e) {
        EXPECT_STREQ(e.what(), "Exception");
        EXPECT_STREQ(e.className(), "Exception");
    }
}

TEST(ExceptionTest, ExceptionWithString)
{
    try {
        throw pplib::Exception("Test exception");
    }
    catch (const pplib::Exception& e) {
        EXPECT_STREQ(e.what(), "Exception: Test exception");
        EXPECT_STREQ(e.className(), "Exception");
    }
}

TEST(ExceptionTest, CopyConstructor)
{
    try {
        pplib::Exception original("Original exception");
        pplib::Exception copy(original);
        EXPECT_STREQ(copy.what(), "Exception: Original exception");
        EXPECT_STREQ(copy.className(), "Exception");
    }
    catch (const pplib::Exception& e) {
        FAIL() << "Exception thrown during copy construction: " << e.what();
    }
    try {
        pplib::Exception original2;
        pplib::Exception copy(original2);
        EXPECT_STREQ(copy.what(), "Exception");
        EXPECT_STREQ(copy.className(), "Exception");
    }
    catch (const pplib::Exception& e) {
        FAIL() << "Exception thrown during copy construction: " << e.what();
    }
}

TEST(ExceptionTest, MoveConstructor)
{
    try {
        pplib::Exception original("Original exception");
        pplib::Exception moved(std::move(original));
        EXPECT_STREQ(moved.what(), "Exception: Original exception");
        EXPECT_STREQ(moved.className(), "Exception");
    }
    catch (const pplib::Exception& e) {
        FAIL() << "Exception thrown during move construction: " << e.what();
    }
}

TEST(ExceptionTest, ConstructorWithFormat)
{
    ASSERT_NO_THROW({
        pplib::Exception e("Formatted %s", "message");
        EXPECT_STREQ(e.what(), "Exception: Formatted message");
        EXPECT_STREQ(e.className(), "Exception");
    });

    ASSERT_NO_THROW({
        pplib::Exception e(nullptr, "message");
        EXPECT_STREQ(e.what(), "Exception");
        EXPECT_STREQ(e.className(), "Exception");
    });
}

TEST(ExceptionTest, ConstructorWithString)
{
    ASSERT_NO_THROW({
        pplib::String msg("Test exception");
        pplib::Exception e(msg);
        EXPECT_STREQ(e.what(), "Exception: Test exception");
        EXPECT_STREQ(e.className(), "Exception");
    });
}

TEST(ExceptionTest, initFromFormat)
{
    ASSERT_NO_THROW({
        pplib::OutOfMemoryException e("Formatted %s", "message");
        EXPECT_STREQ(e.what(), "OutOfMemoryException: Formatted message");
        EXPECT_STREQ(e.className(), "OutOfMemoryException");
    });

    ASSERT_NO_THROW({
        pplib::OutOfMemoryException e(nullptr, "message");
        EXPECT_STREQ(e.what(), "OutOfMemoryException");
        EXPECT_STREQ(e.className(), "OutOfMemoryException");
    });
}

TEST(ExceptionTest, OperatorCopy)
{
    ASSERT_NO_THROW({
        pplib::Exception original("Original exception");
        pplib::Exception copy;
        copy = original;
        EXPECT_STREQ(copy.what(), "Exception: Original exception");
        EXPECT_STREQ(copy.className(), "Exception");
    });

    ASSERT_NO_THROW({
        pplib::OutOfMemoryException original;
        pplib::OutOfMemoryException copy;
        copy = original;
        EXPECT_STREQ(copy.what(), "OutOfMemoryException");
        EXPECT_STREQ(copy.className(), "OutOfMemoryException");
    });
}

TEST(ExceptionTest, OperatorSelfCopy)
{
    ASSERT_NO_THROW({
        pplib::Exception original("Original exception");
        original = original;
        EXPECT_STREQ(original.what(), "Exception: Original exception");
        EXPECT_STREQ(original.className(), "Exception");
    });
}

TEST(ExceptionTest, OperatorMove)
{
    ASSERT_NO_THROW({
        pplib::Exception original("Original exception");
        EXPECT_STREQ(original.what(), "Exception: Original exception");
        pplib::Exception copy;
        copy = std::move(original);
        EXPECT_STREQ(copy.what(), "Exception: Original exception");
        EXPECT_STREQ(copy.className(), "Exception");
    });
}

TEST(ExceptionTest, OperatorSelfMove)
{
    ASSERT_NO_THROW({
        pplib::Exception original("Original exception");
        original = std::move(original);
        EXPECT_STREQ(original.what(), "Exception: Original exception");
        EXPECT_STREQ(original.className(), "Exception");
    });
}

TEST(ExceptionTest, textMethod)
{
    ASSERT_NO_THROW({
        pplib::Exception e("Test exception");
        EXPECT_STREQ(e.text(), "Test exception");
    });
    ASSERT_NO_THROW({
        pplib::Exception e;
        EXPECT_STREQ(e.text(), "");
    });
}

TEST(ExceptionTest, toString)
{
    ASSERT_NO_THROW({
        pplib::Exception e("Test exception");
        EXPECT_STREQ(e.toString().c_str(), "Exception [Test exception]");
    });
    ASSERT_NO_THROW({
        pplib::Exception e;
        EXPECT_STREQ(e.toString().c_str(), "Exception");
    });
}

TEST(ExceptionTest, print)
{
    ASSERT_NO_THROW({
        pplib::Exception e("Test exception");
        testing::internal::CaptureStdout();
        e.print();
        pplib::String output = testing::internal::GetCapturedStdout();
        EXPECT_EQ(pplib::String("Exception: Exception [Test exception]\n"), output);
    });
}

TEST(ExceptionTest, ostringstream)
{

    pplib::Exception e("Test exception");
    testing::internal::CaptureStdout();
    std::cout << e;
    pplib::String output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(pplib::String("Exception [Test exception]"), output);
}

TEST(ExceptionTest, throwExceptionFromErrno)
{
    ASSERT_THROW({ pplib::throwExceptionFromErrno(ENOMEM, "Test"); }, pplib::OutOfMemoryException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EINVAL, "Test"); }, pplib::InvalidArgumentsException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(ENOTDIR, "Test"); }, pplib::InvalidFileNameException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(ENAMETOOLONG, "Test"); }, pplib::InvalidFileNameException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EACCES, "Test"); }, pplib::PermissionDeniedException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EPERM, "Test"); }, pplib::PermissionDeniedException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(ENOENT, "Test"); }, pplib::FileNotFoundException);
#ifdef ELOOP
    ASSERT_THROW({ pplib::throwExceptionFromErrno(ELOOP, "Test"); }, pplib::TooManySymbolicLinksException);
#endif
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EISDIR, "Test"); }, pplib::NoRegularFileException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EROFS, "Test"); }, pplib::ReadOnlyException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EMFILE, "Test"); }, pplib::TooManyOpenFilesException);
#ifdef EOPNOTSUPP
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EOPNOTSUPP, "Test"); }, pplib::UnsupportedFileOperationException);
#endif
    ASSERT_THROW({ pplib::throwExceptionFromErrno(ENOSPC, "Test"); }, pplib::FilesystemFullException);
#ifdef EDQUOT
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EDQUOT, "Test"); }, pplib::QuotaExceededException);
#endif
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EIO, "Test"); }, pplib::IOErrorException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EBADF, "Test"); }, pplib::BadFiledescriptorException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EFAULT, "Test"); }, pplib::BadAddressException);
#ifdef EOVERFLOW
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EOVERFLOW, "Test"); }, pplib::OverflowException);
#endif
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EEXIST, "Test"); }, pplib::FileExistsException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EAGAIN, "Test"); }, pplib::OperationBlockedException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EDEADLK, "Test"); }, pplib::DeadlockException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EINTR, "Test"); }, pplib::OperationInterruptedException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(ENOLCK, "Test"); }, pplib::TooManyLocksException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(ESPIPE, "Test"); }, pplib::IllegalOperationOnPipeException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(ETIMEDOUT, "Test"); }, pplib::TimeoutException);

    ASSERT_THROW({ pplib::throwExceptionFromErrno(ENETDOWN, "Test"); }, pplib::NetworkDownException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(ENETUNREACH, "Test"); }, pplib::NetworkUnreachableException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(ENETRESET, "Test"); }, pplib::NetworkDroppedConnectionOnResetException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(ECONNABORTED, "Test"); }, pplib::SoftwareCausedConnectionAbortException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(ECONNRESET, "Test"); }, pplib::ConnectionResetByPeerException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(ENOBUFS, "Test"); }, pplib::NoBufferSpaceException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EISCONN, "Test"); }, pplib::SocketIsAlreadyConnectedException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(ENOTCONN, "Test"); }, pplib::NotConnectedException);
#ifdef ESHUTDOWN
    ASSERT_THROW({ pplib::throwExceptionFromErrno(ESHUTDOWN, "Test"); }, pplib::CantSendAfterSocketShutdownException);
#endif
#ifdef ETOOMANYREFS
    ASSERT_THROW({ pplib::throwExceptionFromErrno(ETOOMANYREFS, "Test"); }, pplib::TooManyReferencesException);
#endif
    ASSERT_THROW({ pplib::throwExceptionFromErrno(ECONNREFUSED, "Test"); }, pplib::ConnectionRefusedException);
#ifdef EHOSTDOWN
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EHOSTDOWN, "Test"); }, pplib::HostDownException);
#endif
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EHOSTUNREACH, "Test"); }, pplib::NoRouteToHostException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(ENOTSOCK, "Test"); }, pplib::InvalidSocketException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(ENOPROTOOPT, "Test"); }, pplib::UnknownOptionException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EPIPE, "Test"); }, pplib::BrokenPipeException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EINPROGRESS, "Test"); }, pplib::OperationBlockedException);
#ifdef ERROR_LOCK_VIOLATION
    ASSERT_THROW({ pplib::throwExceptionFromErrno(ERROR_LOCK_VIOLATION, "Test"); }, pplib::OperationBlockedException);
#endif
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EALREADY, "Test"); }, pplib::OperationAlreadyInProgressException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EDESTADDRREQ, "Test"); }, pplib::DestinationAddressRequiredException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EMSGSIZE, "Test"); }, pplib::MessageTooLongException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(EPROTOTYPE, "Test"); }, pplib::ProtocolWrongTypeForSocketException);

    // Default (mit und ohne Zusatzinfo)
    ASSERT_THROW({ pplib::throwExceptionFromErrno(99999, "Test"); }, pplib::UnknownException);
    ASSERT_THROW({ pplib::throwExceptionFromErrno(99999, ""); }, pplib::UnknownException);
}

#ifdef _WIN32
TEST(ExceptionTest, throwExceptionFromWinError)
{
    ASSERT_THROW({ pplib::throwExceptionFromWinError(ERROR_FILE_NOT_FOUND, "Test"); }, pplib::FileNotFoundException);
    ASSERT_THROW({ pplib::throwExceptionFromWinError(ERROR_PATH_NOT_FOUND, "Test"); }, pplib::FileNotFoundException);
    ASSERT_THROW({ pplib::throwExceptionFromWinError(ERROR_ACCESS_DENIED, "Test"); }, pplib::PermissionDeniedException);
    ASSERT_THROW({ pplib::throwExceptionFromWinError(ERROR_NOT_ENOUGH_MEMORY, "Test"); }, pplib::OutOfMemoryException);
    ASSERT_THROW({ pplib::throwExceptionFromWinError(ERROR_OUTOFMEMORY, "Test"); }, pplib::OutOfMemoryException);
    ASSERT_THROW({ pplib::throwExceptionFromWinError(ERROR_SHARING_VIOLATION, "Test"); }, pplib::OperationBlockedException);
    ASSERT_THROW({ pplib::throwExceptionFromWinError(ERROR_LOCK_VIOLATION, "Test"); }, pplib::OperationBlockedException);
    ASSERT_THROW({ pplib::throwExceptionFromWinError(ERROR_HANDLE_DISK_FULL, "Test"); }, pplib::FilesystemFullException);
    ASSERT_THROW({ pplib::throwExceptionFromWinError(ERROR_DISK_FULL, "Test"); }, pplib::FilesystemFullException);
    ASSERT_THROW({ pplib::throwExceptionFromWinError(ERROR_ALREADY_EXISTS, "Test"); }, pplib::FileExistsException);
    ASSERT_THROW({ pplib::throwExceptionFromWinError(ERROR_FILE_EXISTS, "Test"); }, pplib::FileExistsException);
    ASSERT_THROW({ pplib::throwExceptionFromWinError(ERROR_INVALID_PARAMETER, "Test"); }, pplib::InvalidArgumentsException);
    ASSERT_THROW({ pplib::throwExceptionFromWinError(ERROR_INVALID_DATA, "Test"); }, pplib::InvalidArgumentsException);

    // Default
    ASSERT_THROW({ pplib::throwExceptionFromWinError(0xFFFFFFFF, "Test"); }, pplib::IOErrorException);
}
#endif

TEST(ExceptionTest, throwSocketException)
{
#ifdef _WIN32
    ASSERT_THROW({ pplib::throwSocketException(WSA_INVALID_HANDLE, "Test"); }, pplib::InvalidArgumentsException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_NOT_ENOUGH_MEMORY, "Test"); }, pplib::OutOfMemoryException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_INVALID_PARAMETER, "Test"); }, pplib::InvalidArgumentsException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_OPERATION_ABORTED, "Test"); }, pplib::OperationAbortedException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_IO_INCOMPLETE, "Test"); }, pplib::ObjectNotInSignaledStateException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_IO_PENDING, "Test"); }, pplib::OverlappedOperationPendingException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEINTR, "Test"); }, pplib::OperationInterruptedException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEBADF, "Test"); }, pplib::BadFiledescriptorException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEACCES, "Test"); }, pplib::PermissionDeniedException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEFAULT, "Test"); }, pplib::BadAddressException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEINVAL, "Test"); }, pplib::InvalidArgumentsException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEMFILE, "Test"); }, pplib::TooManyOpenFilesException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEWOULDBLOCK, "Test"); }, pplib::OperationBlockedException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEINPROGRESS, "Test"); }, pplib::OperationInProgressException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEALREADY, "Test"); }, pplib::OperationAlreadyInProgressException);
    ASSERT_THROW({ pplib::throwSocketException(WSAENOTSOCK, "Test"); }, pplib::SocketOperationOnNonSocketException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEDESTADDRREQ, "Test"); }, pplib::DestinationAddressRequiredException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEMSGSIZE, "Test"); }, pplib::MessageTooLongException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEPROTOTYPE, "Test"); }, pplib::ProtocolWrongTypeForSocketException);
    ASSERT_THROW({ pplib::throwSocketException(WSAENOPROTOOPT, "Test"); }, pplib::ProtocolNotAvailableException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEPROTONOSUPPORT, "Test"); }, pplib::ProtocolNotSupportedException);
    ASSERT_THROW({ pplib::throwSocketException(WSAESOCKTNOSUPPORT, "Test"); }, pplib::SocketTypeNotSupportedException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEOPNOTSUPP, "Test"); }, pplib::UnsupportedFileOperationException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEPFNOSUPPORT, "Test"); }, pplib::ProtocolFamilyNotSupportedException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEAFNOSUPPORT, "Test"); }, pplib::AddressFamilyNotSupportedException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEADDRINUSE, "Test"); }, pplib::AddressAlreadyInUseException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEADDRNOTAVAIL, "Test"); }, pplib::AddressNotAvailableException);
    ASSERT_THROW({ pplib::throwSocketException(WSAENETDOWN, "Test"); }, pplib::NetworkDownException);
    ASSERT_THROW({ pplib::throwSocketException(WSAENETUNREACH, "Test"); }, pplib::NetworkUnreachableException);
    ASSERT_THROW({ pplib::throwSocketException(WSAENETRESET, "Test"); }, pplib::ConnectionAbortedByNetworkException);
    ASSERT_THROW({ pplib::throwSocketException(WSAECONNABORTED, "Test"); }, pplib::ConnectionAbortedException);
    ASSERT_THROW({ pplib::throwSocketException(WSAECONNRESET, "Test"); }, pplib::ConnectionResetException);
    ASSERT_THROW({ pplib::throwSocketException(WSAENOBUFS, "Test"); }, pplib::NoBufferSpaceAvailableException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEISCONN, "Test"); }, pplib::SocketIsConnectedException);
    ASSERT_THROW({ pplib::throwSocketException(WSAENOTCONN, "Test"); }, pplib::SocketNotConnectedException);
    ASSERT_THROW({ pplib::throwSocketException(WSAESHUTDOWN, "Test"); }, pplib::TransportEndpointHasShutdownException);
    ASSERT_THROW({ pplib::throwSocketException(WSAETOOMANYREFS, "Test"); }, pplib::TooManyReferencesException);
    ASSERT_THROW({ pplib::throwSocketException(WSAETIMEDOUT, "Test"); }, pplib::ConnectionTimeoutException);
    ASSERT_THROW({ pplib::throwSocketException(WSAECONNREFUSED, "Test"); }, pplib::ConnectionRefusedException);
    ASSERT_THROW({ pplib::throwSocketException(WSAELOOP, "Test"); }, pplib::InvalidFileNameException);
    ASSERT_THROW({ pplib::throwSocketException(WSAENAMETOOLONG, "Test"); }, pplib::InvalidFileNameException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEHOSTDOWN, "Test"); }, pplib::HostDownException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEHOSTUNREACH, "Test"); }, pplib::HostIsUnreachableException);
    ASSERT_THROW({ pplib::throwSocketException(WSAENOTEMPTY, "Test"); }, pplib::DirectoryNotEmptyException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEPROCLIM, "Test"); }, pplib::ProcessLimitException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEUSERS, "Test"); }, pplib::TooManyUsersException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEDQUOT, "Test"); }, pplib::QuotaExceededException);
    ASSERT_THROW({ pplib::throwSocketException(WSAESTALE, "Test"); }, pplib::StaleFileHandleException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEREMOTE, "Test"); }, pplib::ObjectIsRemoteException);
    ASSERT_THROW({ pplib::throwSocketException(WSASYSNOTREADY, "Test"); }, pplib::NetworkSubsystemUnavailableException);
    ASSERT_THROW({ pplib::throwSocketException(WSAVERNOTSUPPORTED, "Test"); }, pplib::UnsupportedWinsockVersionException);
    ASSERT_THROW({ pplib::throwSocketException(WSANOTINITIALISED, "Test"); }, pplib::NotInitializedException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEDISCON, "Test"); }, pplib::GracefulShutdownInProgressException);
    ASSERT_THROW({ pplib::throwSocketException(WSAENOMORE, "Test"); }, pplib::NoMoreResultsException);
    ASSERT_THROW({ pplib::throwSocketException(WSAECANCELLED, "Test"); }, pplib::CallHasBeenCanceledException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEINVALIDPROCTABLE, "Test"); }, pplib::ProcedureCallTableIsInvalidException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEINVALIDPROVIDER, "Test"); }, pplib::ServiceProviderIsInvalidException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEPROVIDERFAILEDINIT, "Test"); }, pplib::ServiceProviderFailedToInitializeException);
    ASSERT_THROW({ pplib::throwSocketException(WSASYSCALLFAILURE, "Test"); }, pplib::SystemCallFailureException);
    ASSERT_THROW({ pplib::throwSocketException(WSASERVICE_NOT_FOUND, "Test"); }, pplib::ServiceNotFoundException);
    ASSERT_THROW({ pplib::throwSocketException(WSATYPE_NOT_FOUND, "Test"); }, pplib::ClassTypeNotFoundException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_E_NO_MORE, "Test"); }, pplib::NoMoreResultsException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_E_CANCELLED, "Test"); }, pplib::CallHasBeenCanceledException);
    ASSERT_THROW({ pplib::throwSocketException(WSAEREFUSED, "Test"); }, pplib::QueryRefusedException);
    ASSERT_THROW({ pplib::throwSocketException(WSAHOST_NOT_FOUND, "Test"); }, pplib::HostNotFoundException);
    ASSERT_THROW({ pplib::throwSocketException(WSATRY_AGAIN, "Test"); }, pplib::NonauthoritativeHostNotFound);
    ASSERT_THROW({ pplib::throwSocketException(WSANO_RECOVERY, "Test"); }, pplib::UnrecoverableErrorException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_RECEIVERS, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_SENDERS, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_NO_SENDERS, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_NO_RECEIVERS, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_REQUEST_CONFIRMED, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_ADMISSION_FAILURE, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_POLICY_FAILURE, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_BAD_STYLE, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_BAD_OBJECT, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_TRAFFIC_CTRL_ERROR, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_GENERIC_ERROR, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_ESERVICETYPE, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_EFLOWSPEC, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_EPROVSPECBUF, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_EFILTERSTYLE, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_EFILTERTYPE, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_EFILTERCOUNT, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_EOBJLENGTH, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_EUNKOWNPSOBJ, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_EPOLICYOBJ, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_EFLOWDESC, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_EPSFLOWSPEC, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_EPSFILTERSPEC, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_ESDMODEOBJ, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_ESHAPERATEOBJ, "Test"); }, pplib::QoSException);
    ASSERT_THROW({ pplib::throwSocketException(WSA_QOS_RESERVED_PETYPE, "Test"); }, pplib::QoSException);

    // Default: delegiert an throwExceptionFromErrno
    ASSERT_THROW({ pplib::throwSocketException(ENOMEM, "Test"); }, pplib::OutOfMemoryException);
    ASSERT_THROW({ pplib::throwSocketException(99999, "Test"); }, pplib::UnknownException);
#else
    // Auf Nicht-Windows delegiert throwSocketException direkt an throwExceptionFromErrno
    ASSERT_THROW({ pplib::throwSocketException(ENOMEM, "Test"); }, pplib::OutOfMemoryException);
    ASSERT_THROW({ pplib::throwSocketException(99999, "Test"); }, pplib::UnknownException);
#endif
}

} // namespace