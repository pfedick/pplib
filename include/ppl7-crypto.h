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

#ifndef PPL7CRYPTO_H_
#define PPL7CRYPTO_H_

#ifndef _PPL7_INCLUDE
    #ifdef PPL7LIB
        #include "ppl7.h"
    #else
        #include <ppl7.h>
    #endif
#endif

namespace ppl7 {

PPLEXCEPTION (InvalidAlgorithmException, Exception);
PPLEXCEPTION (NoAlgorithmSpecifiedException, Exception);
PPLEXCEPTION (InvalidBlocksizeException, Exception);
PPLEXCEPTION (HashFailedException, OperationFailedException);
PPLEXCEPTION (NoKeySpecifiedException, Exception);
PPLEXCEPTION (NoIVSpecifiedException, Exception);
PPLEXCEPTION (EncryptionFailedException, OperationFailedException);
PPLEXCEPTION (DecryptionFailedException, OperationFailedException);

class MCrypt
{

	private:
		void *mcrypt;
		ByteArray IV;
		ByteArray Key;


	public:
		enum Mode {
			Mode_STREAM,
			Mode_ECB,
			Mode_CBC,
			Mode_CFB,
			Mode_OFB,
			Mode_nOFB,
			Mode_nCFB,
			Mode_CTR
		};


		enum Algorithm {
			Algo_DES,
			Algo_TRIPLE_DES,
			Algo_CAST_128,
			Algo_CAST_256,
			Algo_xTEA,
			Algo_THREE_WAY,
			Algo_SKIPJACK,
			Algo_BLOWFISH,
			Algo_TWOFISH,
			Algo_LOKI97,
			Algo_RC2,
			Algo_ARCFOUR,
			Algo_RC6,
			Algo_RIJNDAEL_128,
			Algo_RIJNDAEL_192,
			Algo_RIJNDAEL_256,
			Algo_MARS,
			Algo_PANAMA,
			Algo_WAKE,
			Algo_SERPENT,
			Algo_IDEA,
			Algo_ENIGMA,
			Algo_GOST,
			Algo_SAFER_SK64,
			Algo_SAFER_SK128,
			Algo_SAFER_PLUS
		};

		MCrypt();
		MCrypt(Algorithm algo, Mode mode);
		~MCrypt();
		void setAlgorithm(Algorithm algo=MCrypt::Algo_TWOFISH, Mode mode=MCrypt::Mode_CFB);
		int getIVSize() const;
		int getMaxKeySize() const;
		void setIV(const void *buffer, size_t size);
		void setIV(const Variant &object);
		void setKey(const void *buffer, size_t size);
		void setKey(const Variant &object);
		bool needIV() const;
		//int SupportKeySize(size_t size);

		void crypt(void *buffer, size_t size);
		void crypt(const Variant &in, ByteArray &out);
		void crypt(ByteArrayPtr &buffer);

		void decrypt(void *buffer, size_t size);
		void decrypt(const ByteArrayPtr &in, ByteArray &out);
		void decrypt(ByteArrayPtr &buffer);

		static void crypt(ByteArrayPtr &buffer, const Variant &key, Algorithm algo, Mode mode);
		static void crypt(ByteArrayPtr &buffer, const Variant &key, const Variant &IV, Algorithm algo, Mode mode);
		static void decrypt(ByteArrayPtr &buffer, const Variant &key, Algorithm algo, Mode mode);
		static void decrypt(ByteArrayPtr &buffer, const Variant &key, const Variant &IV, Algorithm algo, Mode mode);

};

class MHash
{
	public:
		enum Algorithm {
			Algo_CRC32,
			Algo_CRC32B,
			Algo_ADLER32,
			Algo_MD2,
			Algo_MD4,
			Algo_MD5,
			Algo_SHA1,
			Algo_SHA224,
			Algo_SHA256,
			Algo_SHA384,
			Algo_SHA512,
			Algo_WHIRLPOOL,
			Algo_HAVAL256,
			Algo_HAVAL224,
			Algo_HAVAL192,
			Algo_HAVAL160,
			Algo_HAVAL128,
			Algo_RIPEMD128,
			Algo_RIPEMD160,
			Algo_RIPEMD256,
			Algo_RIPEMD320,
			Algo_TIGER128,
			Algo_TIGER160,
			Algo_TIGER192,
			Algo_GOST,
			Algo_SNEFRU128,
			Algo_SNEFRU256
		};

		enum Bits {
			Bits_Default,
			Bits_128,
			Bits_160,
			Bits_192,
			Bits_224,
			Bits_256,
			Bits_320,
			Bits_384,
			Bits_512
		};

	private:
		void *handle;
		int blocksize;
		Algorithm algo;
		ppluint64 bytesHashed;

	public:
		MHash();
		MHash(Algorithm algorithm);
		~MHash();
		void setAlgorithm(Algorithm algorithm);
		void addData(const void *data, size_t size);
		void addData(const Variant &data);
		void addData(FileObject &file);
		void addFile(const String &filename);
		int getBlockSize() const;
		void saveDigest(ByteArray &result);
		void saveDigest(String &result);
		void saveDigest(WideString &result);
		ByteArray getDigest();
		int getInt();
		void reset();

		static Algorithm getAlgorithmFromName(const String &name);

		static ByteArray hash(const Variant &data, Algorithm algorithm);
		static ByteArray hash(const Variant &data, const String &algorithm);
		static int crc32(const Variant &data);
		static int crc32b(const Variant &data);
		static int adler32(const Variant &data);
		static ByteArray md4(const Variant &data);
		static ByteArray md5(const Variant &data);
		static ByteArray sha1(const Variant &data);
		static ByteArray sha2(const Variant &data, Bits bits=Bits_256);
		static ByteArray sha224(const Variant &data);
		static ByteArray sha256(const Variant &data);
		static ByteArray sha384(const Variant &data);
		static ByteArray sha512(const Variant &data);
};

class Digest
{
	private:
		const void *m;
		void *ctx;
		unsigned char *ret;
		ppluint64 bytecount;

	public:
		enum Algorithm {
			Algo_MD4,
			Algo_MD5,
			Algo_SHA1,
			Algo_SHA224,
			Algo_SHA256,
			Algo_SHA384,
			Algo_SHA512,
			Algo_WHIRLPOOL,
			Algo_RIPEMD160,
			Algo_ECDSA
		};


		Digest();
		Digest(const String &name);
		Digest(Algorithm algorithm);
		~Digest();

		void setAlgorithm(Algorithm algorithm);
		void setAlgorithm(const String &name);
		void addData(const void *data, size_t size);
		void addData(const Variant &data);
		void addData(FileObject &file);
		void addFile(const String &filename);
		ByteArray getDigest();
		void saveDigest(ByteArray &result);
		void saveDigest(String &result);
		void saveDigest(WideString &result);

		void reset();
		ppluint64 bytesHashed() const;

		static ByteArray hash(const Variant &data, Algorithm algorithm);
		static ByteArray hash(const Variant &data, const String &name);
		static ByteArray md4(const Variant &data);
		static ByteArray md5(const Variant &data);
		static ByteArray sha1(const Variant &data);
		static ByteArray sha224(const Variant &data);
		static ByteArray sha256(const Variant &data);
		static ByteArray sha384(const Variant &data);
		static ByteArray sha512(const Variant &data);
		static ByteArray ecdsa(const Variant &data);

};


} // EOF namespace ppl7

#endif /* PPL7CRYPTO_H_ */
