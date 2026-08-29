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

#ifndef PPLIBCRYPTO_H_
#define PPLIBCRYPTO_H_

#include <pplib.h>
#include <pplib/crypto/digest.h>

namespace pplib
{

PPLIBEXCEPTION(UnsupportedAlgorithmException, Exception);
PPLIBEXCEPTION(InvalidAlgorithmException, Exception);
PPLIBEXCEPTION(NoAlgorithmSpecifiedException, Exception);
PPLIBEXCEPTION(InvalidBlocksizeException, Exception);
PPLIBEXCEPTION(HashFailedException, OperationFailedException);
PPLIBEXCEPTION(NoKeySpecifiedException, Exception);
PPLIBEXCEPTION(NoIVSpecifiedException, Exception);
PPLIBEXCEPTION(EncryptionFailedException, OperationFailedException);
PPLIBEXCEPTION(DecryptionFailedException, OperationFailedException);
PPLIBEXCEPTION(InvalidKeyLengthException, Exception);

class Crypt
{
    friend class Encrypt;
    friend class Decrypt;

private:
    void* ctx;

public:
    enum Mode
    {
        Mode_ECB,
        Mode_CBC,
        Mode_CFB,
        Mode_OFB
    };

    enum Algorithm
    {
        Algo_AES_128,
        Algo_AES_192,
        Algo_AES_256,
        Algo_ARIA_128,
        Algo_ARIA_192,
        Algo_ARIA_256,
        Algo_BLOWFISH,
        Algo_CAMELLIA_128,
        Algo_CAMELLIA_192,
        Algo_CAMELLIA_256,
        Algo_CAST5,
        Algo_DES,
        Algo_TRIPLE_DES,
        Algo_IDEA,
        Algo_RC2,
        Algo_RC5,
    };
    Crypt();
    ~Crypt();
    int keyLength() const;
    int maxKeyLength() const;
    int ivLength() const;
    int blockSize() const;
    void setPadding(bool enabled);
    void setKeyLength(int keylen);
};

class Encrypt : public Crypt
{
public:
    Encrypt(Algorithm algo, Mode mode);
    void setAlgorithm(Algorithm algo, Mode mode);
    void setKey(const ByteArrayPtr& key);
    void setIV(const ByteArrayPtr& iv);
    void update(const ByteArrayPtr& in, ByteArray& out);
    void final(ByteArray& out);
    void encrypt(const ByteArrayPtr& in, ByteArray& out);
    ByteArray encrypt(const ByteArrayPtr& in);
};

class Decrypt : public Crypt
{
public:
    Decrypt(Algorithm algo, Mode mode);
    void setAlgorithm(Algorithm algo, Mode mode);
    void setKey(const ByteArrayPtr& key);
    void setIV(const ByteArrayPtr& iv);
    void update(const ByteArrayPtr& in, ByteArray& out);
    void final(ByteArray& out);
    void decrypt(const ByteArrayPtr& in, ByteArray& out);
    ByteArray decrypt(const ByteArrayPtr& in);
};

// TODO: Signaturen Generierung und Prüfung
// Key-Pairs

// Draft
class KeyPair
{
public:
    enum Type
    {
        Type_RSA,
        Type_EC_P256,
        Type_Ed25519
    };

    static KeyPair generate(Type type);
    void loadPrivateKeyPEM(const String& pem);
    void loadPublicKeyPEM(const String& pem);
    String savePrivateKeyPEM() const;
    String savePublicKeyPEM() const;
};

// Draft
class Signature
{
public:
    static ByteArray sign(const ByteArrayPtr& data, const KeyPair& key);
    static bool verify(const ByteArrayPtr& data, const ByteArrayPtr& signature, const KeyPair& key);
};

// Draft
class KDF
{
public:
    static ByteArray pbkdf2_sha256(const String& password, const ByteArrayPtr& salt, int iterations, size_t keyLen);
};

// Draft
class HMAC
{
public:
    static ByteArray sha256(const ByteArrayPtr& data, const ByteArrayPtr& key);
};

} // namespace pplib

#endif /* PPLIBCRYPTO_H_ */
