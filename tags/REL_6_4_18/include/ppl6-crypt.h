/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
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

#ifndef PPL6CRYPT_H_
#define PPL6CRYPT_H_

#ifndef _PPL6_INCLUDE
    #ifdef PPL6LIB
        #include "ppl6.h"
    #else
        #include <ppl6.h>
    #endif
#endif

namespace ppl6 {


class CMCrypt{

	private:
		void *mcrypt;
		CBinary IV;
		CBinary Key;


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

		CMCrypt();
		CMCrypt(Algorithm algo, Mode mode);
		~CMCrypt();
		int Init(Algorithm algo=CMCrypt::Algo_TWOFISH, Mode mode=CMCrypt::Mode_CFB);
		int GetIVSize() const;
		int GetMaxKeySize() const;
		int SetIV(const void *buffer, size_t size);
		int SetIV(const CVar &object);
		int SetIV(const char *iv);
		int SetKey(const void *buffer, size_t size);
		int SetKey(const char *key);
		int SetKey(const CVar &object);
		int NeedIV() const;
		//int SupportKeySize(size_t size);

		int Crypt(void *buffer, size_t size);
		int Crypt(const CVar &in, CBinary &out);
		int Crypt(CBinary &buffer);

		int Decrypt(void *buffer, size_t size);
		int Decrypt(const CBinary &in, CBinary &out);
		int Decrypt(CBinary &buffer);

		static int Crypt(CBinary &buffer, const CVar &key, Algorithm algo=CMCrypt::Algo_TWOFISH, Mode mode=CMCrypt::Mode_CFB, const CVar &IV=CBinary());
		static int Decrypt(CBinary &buffer, const CVar &key, Algorithm algo=CMCrypt::Algo_TWOFISH, Mode mode=CMCrypt::Mode_CFB, const CVar &IV=CBinary());
		static int Crypt(CBinary &buffer, const char *key, Algorithm algo=CMCrypt::Algo_TWOFISH, Mode mode=CMCrypt::Mode_CFB, const CVar &IV=CBinary());
		static int Decrypt(CBinary &buffer, const char *key, Algorithm algo=CMCrypt::Algo_TWOFISH, Mode mode=CMCrypt::Mode_CFB, const CVar &IV=CBinary());

};

class CMHash
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
		CMHash();
		CMHash(Algorithm method);
		~CMHash();
		int Init(Algorithm method);
		int AddData(const void *data, size_t size);
		int AddData(const CVar &data);
		int AddData(CFileObject &file);
		int AddFile(const char *filename);
		int GetBlockSize() const;
		int GetResult(CVar &result);
		void Reset();

		static int Hash(const CVar &data, CVar &result, Algorithm algo);
		static int Hash(const CVar &data, CVar &result, const CString &algo);
		static int CRC32(const CVar &data, CVar &result);
		static int CRC32B(const CVar &data, CVar &result);
		static int ADLER32(const CVar &data, CVar &result);
		static int CRC32(const CVar &data);
		static int CRC32B(const CVar &data);
		static int ADLER32(const CVar &data);
		static int MD4(const CVar &data, CVar &result);
		static int MD5(const CVar &data, CVar &result);
		static int SHA1(const CVar &data, CVar &result);
		static int SHA2(const CVar &data, CVar &result, Bits bits=Bits_256);
		static int SHA224(const CVar &data, CVar &result);
		static int SHA256(const CVar &data, CVar &result);
		static int SHA384(const CVar &data, CVar &result);
		static int SHA512(const CVar &data, CVar &result);
		static int HAVAL(const CVar &data, CVar &result, Bits bits=Bits_Default);
		static int TIGER(const CVar &data, CVar &result, Bits bits=Bits_Default);
		static int RIPEMD(const CVar &data, CVar &result, Bits bits=Bits_160);
		static int SNEFRU(const CVar &data, CVar &result, Bits bits=Bits_160);
		static int GOST(const CVar &data, CVar &result);
		static int WHIRLPOOL(const CVar &data, CVar &result);
};

} // EOF namespace ppl6


#endif /* PPL6CRYPT_H_ */
