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

#include "prolog.h"
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_LIBMHASH
#define PROTOTYPES 1
#undef HAVE__BOOL
#include <mutils/mhash.h>
#endif

#ifdef HAVE_OPENSSL
#include <openssl/evp.h>
#endif

#include "ppl7.h"
#include "ppl7-crypto.h"


namespace ppl7 {

bool __OpenSSLDigestAdded = false;

Mutex __OpenSSLGlobalMutex;

void InitOpenSSLDigest()
{
#ifndef HAVE_OPENSSL
	throw UnsupportedFeatureException("OpenSSL");
#else
	__OpenSSLGlobalMutex.lock();
	if(!__OpenSSLDigestAdded) {
		::OpenSSL_add_all_digests();
		__OpenSSLDigestAdded=true;
	}
	__OpenSSLGlobalMutex.unlock();
#endif
}


Digest::Digest()
{
	bytecount=0;
	m=NULL;
	ret=NULL;
	ctx=NULL;
#ifndef HAVE_OPENSSL
	throw UnsupportedFeatureException("OpenSSL");
#else
	if (!__OpenSSLDigestAdded) {
		InitOpenSSLDigest();
	}
#endif
}

Digest::~Digest()
{
#ifdef HAVE_OPENSSL
	free(ret);
	if (ctx) EVP_MD_CTX_destroy((EVP_MD_CTX*)ctx);
#endif
}

Digest::Digest(const String &name)
{
	bytecount=0;
	m=NULL;
	ret=NULL;
	ctx=NULL;
#ifndef HAVE_OPENSSL
	throw UnsupportedFeatureException("OpenSSL");
#else
	if (!__OpenSSLDigestAdded) {
		InitOpenSSLDigest();
	}
	setAlgorithm(name);
#endif
}

Digest::Digest(Algorithm algorithm)
{
	bytecount=0;
	m=NULL;
	ret=NULL;
	ctx=NULL;
#ifndef HAVE_OPENSSL
	throw UnsupportedFeatureException("OpenSSL");
#else
	if (!__OpenSSLDigestAdded) {
		InitOpenSSLDigest();
	}
	setAlgorithm(algorithm);
#endif
}

void Digest::setAlgorithm(const String &name)
{
#ifndef HAVE_OPENSSL
	throw UnsupportedFeatureException("OpenSSL");
#else
	m=EVP_get_digestbyname((const char*)name);
	if (!m) {
		throw InvalidAlgorithmException("%s",(const char*)name);
	}
	if (!ctx) {
		ctx=EVP_MD_CTX_create();
		if (!ctx) throw OutOfMemoryException();
	} else {
		reset();
	}
	EVP_DigestInit_ex((EVP_MD_CTX*)ctx,(const EVP_MD*)m, NULL);
#endif
}

void Digest::setAlgorithm(Algorithm algorithm)
{
#ifndef HAVE_OPENSSL
	throw UnsupportedFeatureException("OpenSSL");
#else

	switch(algorithm) {
#ifndef OPENSSL_NO_MD4
		case Algo_MD4: m=EVP_md4(); break;
#endif
#ifndef OPENSSL_NO_MD5
		case Algo_MD5: m=EVP_md5(); break;
#endif
#ifndef OPENSSL_NO_SHA
		case Algo_SHA1: m=EVP_sha1(); break;
		case Algo_ECDSA: m=EVP_ecdsa(); break;

#endif
#ifndef OPENSSL_NO_SHA256
		case Algo_SHA224: m=EVP_sha224(); break;
		case Algo_SHA256: m=EVP_sha256(); break;
#endif
#ifndef OPENSSL_NO_SHA512
		case Algo_SHA384: m=EVP_sha384(); break;
		case Algo_SHA512: m=EVP_sha512(); break;
#endif
#ifndef OPENSSL_NO_WHIRLPOOL
#if OPENSSL_VERSION_NUMBER >= 0x10001000L
		case Algo_WHIRLPOOL: m=EVP_whirlpool(); break;
#endif
#endif
#ifndef OPENSSL_NO_RIPEMD
		case Algo_RIPEMD160: m=EVP_ripemd160(); break;
#endif

		default: throw InvalidAlgorithmException();
	}
	if (!m) {
		throw InvalidAlgorithmException("%i",algorithm);
	}
	if (!ctx) {
		ctx=EVP_MD_CTX_create();
		if (!ctx) throw OutOfMemoryException();
	} else {
		reset();
	}
	EVP_DigestInit_ex((EVP_MD_CTX*)ctx,(const EVP_MD*)m, NULL);
#endif
}

void Digest::addData(const void *data, size_t size)
{
#ifndef HAVE_OPENSSL
	throw UnsupportedFeatureException("OpenSSL");
#else
	if (!m) throw NoAlgorithmSpecifiedException();
	EVP_DigestUpdate((EVP_MD_CTX*)ctx,data,size);
	bytecount+=size;
#endif
}

void Digest::addData(const Variant &data)
{
	int type=data.dataType();
	if (type==Variant::BYTEARRAY || type==Variant::BYTEARRAYPTR) {
		const ByteArrayPtr &bin=static_cast<const ByteArrayPtr&>(data);
		addData(bin.ptr(),bin.size());
	} else if (type==Variant::STRING) {
		const String &str=static_cast<const String&>(data);
		addData(str.getPtr(),str.size());
	} else if (type==Variant::WIDESTRING) {
		const WideString &wstr=static_cast<const WideString&>(data);
		addData(wstr.getPtr(),wstr.size());
	} else {
		throw UnsupportedDataTypeException();
	}
}

void Digest::addData(FileObject &file)
{
	file.seek(0);
	size_t bsize=1024*1024*1;		// We allocate 1 MB maximum
	ppluint64 fsize=file.size();
	if (fsize<bsize) bsize=fsize;	// or filesize if file is < 1 MB
	void *buffer=malloc(bsize);
	if (!buffer) {
		throw OutOfMemoryException();
	}
	ppluint64 rest=fsize;
	try {
		while(rest) {
			size_t bytes=rest;
			if (bytes>bsize) bytes=bsize;
			if (!file.read(buffer,bytes)) {
				throw ReadException();
			}
			addData(buffer,bytes);
			rest-=bytes;
		}
	} catch (...) {
		free(buffer);
		throw;
	}
	free(buffer);

}

void Digest::addFile(const String &filename)
{
	File ff;
	ff.open(filename,File::READ);
	addData(ff);
}

ppluint64 Digest::bytesHashed() const
{
	return bytecount;
}

void Digest::saveDigest(Variant &result)
{
#ifndef HAVE_OPENSSL
	throw UnsupportedFeatureException("OpenSSL");
#else
	ByteArray ba=getDigest();
	int type=result.dataType();
	if (type==Variant::BYTEARRAY) {
		ByteArray &bin=static_cast<ByteArray&>(result);
		bin=ba;
	} else if (type==Variant::STRING) {
		String &str=static_cast<String&>(result);
		str=ba.toHex();
	} else if (type==Variant::WIDESTRING) {
		WideString &str=static_cast<WideString&>(result);
		str=ba.toHex();
	} else {
		throw UnsupportedDataTypeException();
	}
#endif
}

ByteArray Digest::getDigest()
{
#ifndef HAVE_OPENSSL
	throw UnsupportedFeatureException("OpenSSL");
#else
	unsigned int len;
	if (!ret) {
		ret=(unsigned char*)malloc(EVP_MAX_MD_SIZE);
		if (!ret) throw OutOfMemoryException();
	}
	EVP_DigestFinal((EVP_MD_CTX*)ctx,ret,&len);
	EVP_MD_CTX_cleanup((EVP_MD_CTX*)ctx);
	EVP_DigestInit_ex((EVP_MD_CTX*)ctx,(const EVP_MD*)m, NULL);
	bytecount=0;
	return ByteArray(ret,len);
#endif
}

void Digest::reset()
{
#ifndef HAVE_OPENSSL
	throw UnsupportedFeatureException("OpenSSL");
#else
	if (!m) throw NoAlgorithmSpecifiedException();
	if (!ctx) throw NoAlgorithmSpecifiedException();
	EVP_MD_CTX_cleanup((EVP_MD_CTX*)ctx);
	EVP_DigestInit((EVP_MD_CTX*)ctx,(const EVP_MD*)m);
	bytecount=0;
#endif
}


ByteArray Digest::hash(const Variant &data, Algorithm algorithm)
{
	Digest dig(algorithm);
	dig.addData(data);
	return dig.getDigest();
}

ByteArray Digest::hash(const Variant &data, const String &name)
{
	Digest dig(name);
	dig.addData(data);
	return dig.getDigest();
}

ByteArray Digest::md4(const Variant &data)
{
	return Digest::hash(data,Algo_MD4);
}

ByteArray Digest::md5(const Variant &data)
{
	return Digest::hash(data,Algo_MD5);
}

ByteArray Digest::sha1(const Variant &data)
{
	return Digest::hash(data,Algo_SHA1);
}

ByteArray Digest::sha224(const Variant &data)
{
	return Digest::hash(data,Algo_SHA224);
}

ByteArray Digest::sha256(const Variant &data)
{
	return Digest::hash(data,Algo_SHA256);
}

ByteArray Digest::sha384(const Variant &data)
{
	return Digest::hash(data,Algo_SHA384);
}

ByteArray Digest::sha512(const Variant &data)
{
	return Digest::hash(data,Algo_SHA512);
}

ByteArray Digest::ecdsa(const Variant &data)
{
	return Digest::hash(data,Algo_ECDSA);
}



}
