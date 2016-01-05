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
#ifdef HAVE_LIBMCRYPT
#include <mcrypt.h>
#endif

#include "ppl7.h"
#include "ppl7-crypto.h"


namespace ppl7 {


/*!\class MCrypt
 * \ingroup PPL7_CRYPT
 * \brief Klasse zum Ver- und Entschlüsseln von Daten basierend auf MCrypt
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Die Klasse MCrypt dient zum Ver- und Entschlüsseln von Daten und basiert auf der
 * <a href="http://mcrypt.sourceforge.net/">MCrypt-Library</a>, die unter der
 * GNU General Public License (GPL) steht.
 * \par
 * Aus der Manpage mcrypt(3):
 * \par
 * The libmcrypt is a data encryption library.  The library is thread safe
 * and provides encryption and decryption functions.  This version of  the
 * library  supports many encryption algorithms and encryption modes. Some
 * algorithms which are supported: SERPENT, RIJNDAEL, 3DES, GOST,  SAFER+,
 * CAST-256, RC2, XTEA, 3WAY, TWOFISH, BLOWFISH, ARCFOUR, WAKE and more.
 * (see MCrypt::Algorithm)
 * \par
 * OFB, CBC, ECB, nOFB, nCFB and CFB are the modes that all algorithms may
 * function.  ECB, CBC, encrypt in blocks but CTR, nCFB, nOFB, CFB and OFB
 * in  bytes (streams).  Note that CFB and OFB in the rest of the document
 * represent the "8bit CFB or OFB" mode.  nOFB and nCFB modes represents a
 * n-bit  OFB/CFB mode, n is used to represent the algorithm's block size.
 * The library supports an extra STREAM mode to include some stream  algorithms
 * like WAKE or ARCFOUR.
 * (see MCrypt::Mode)
 * \par
 * In  this  version  of the library all modes and algorithms are modular,
 * which means that the algorithm and the  mode  is  loaded  at  run-time.
 * This way you can add algorithms and modes faster, and much easier.
 *
 * \note
 * Damit diese Klasse verwendet werden kann, muss vor Aufruf des configure-Scripts die
 * Library <a href="http://mcrypt.sourceforge.net/">libmcrypt</a> installiert sein.
 * Falls das configure die Library nicht automatisch findet, kann der Installationspfad
 * über den configure Parameter "\-\-with-libmcrypt-prefix=..." angegeben werden. Falls
 * libmcrypt nicht eingebunden wurde, werfen fast alle Funktionen eine
 * UnsupportedFeatureException.
 *
 * \example
 * Hier ein Beispiel, wie die Klasse zum Ver- und Entschlüsseln von Daten verwendet werden kann:
\code
#include <ppl7.h>
#include <ppl7-crypto.h>

int main (int argc, char **argv)
{
	// Instanz der Klasse erstellen
	ppl7::MCrypt MC;
	// Objekt mit dem gewünschten Algorithmus initialisieren
	MC.setAlgorithm(ppl7::MCrypt::Algo_TWOFISH, ppl7::MCrypt::Mode_CFB);

	// Wir legen ein Objekt für den IV an
	ppl7::ByteArray IV;
	if (MC.needIV()) {	// Prüfen, ob wir überhaupt einen IV brauchen
		size_t ivsize=MC.getIVSize();	// Ja, wir ermitteln die benötigte Größe
		ppl7::Random(IV,ivsize);		// Und erstellen entsprechend viele Zufallsdaten
		MC.setIV(IV);					// Diese übergeben wir an die Klasse
	}

	// Nun setzen wir das Passwort für die Verschlüsselung
	MC.setKey("uirfgreuifgewzifg");

	// Jetzt brauchen wir noch ein paar Daten, die wir verschlüsseln wollen
	ppl6::ByteArray bin="Hallo Welt";

	// Diese geben wir als HexDump aus, damit wir sehen, wie die Daten vor der
	// Verschlüsselung aussahen
	printf ("vorher:\n");
	bin.hexDump();

	// Jetzt wird verschlüsselt
	MC.crypt(bin);

	// Nun geben wir zum Vergleich die verschlüsselten Daten als Hexdump aus
	printf ("verschluesselt:\n");
	bin.hexDump();

	// Und nun entschlüsseln wir sie wieder
	MC.decrypt(bin);

	// Zu guter Letzt geben wir die entschlüsselten Daten wieder als Hexdump aus.
	// Dieser sollte identisch mit dem ersten sein
	printf ("entschluesselt:\n");
	bin.HexDump();
}
\endcode
 * Wenn das Programm ausgeführt wird, erhält man folgende Ausgabe:
\verbatim
vorher:
0x00000000: 48 61 6C 6C 6F 20 57 65 6C 74                   : Hallo Welt
verschluesselt:
0x00000000: 32 57 6E 7C E6 35 87 90 50 EE                   : 2Wn|.5..P.
entschluesselt:
0x00000000: 48 61 6C 6C 6F 20 57 65 6C 74                   : Hallo Welt
\endverbatim
 * Die zweite Zeile wird übrigens bei jedem Aufruf anderes aussehen, obwohl wir den gleichen
 * Schlüssel verwendet haben. Das liegt daran, dass wir den Initialisierungsvektor mit
 * Zufallsdaten gefüllt haben.
 * \par
 * Alternativ können auch die statischen Funktionen der Klasse verwendet werden. Ein Beispiel
 * dazu ist \ref MCrypt::crypt(ByteArrayPtr &, const Variant &, const Variant &, Algorithm, Mode) "hier" zu finden.
 */

/*!\var MCrypt::mcrypt
 * \brief Pointer auf ein initialisiertes Objekt der libmcrypt
 */

/*!\var MCrypt::IV
 * \brief Initialisierungsvektor
 */

/*!\var MCrypt::Key
 * \brief Schlüssel
 */

/*!\enum MCrypt::Mode
 * \brief Unterstütze Modi
 *
 * OFB, CBC, ECB, nOFB, nCFB and CFB are the modes that all algorithms may
 * function.  ECB, CBC, encrypt in blocks but CTR, nCFB, nOFB, CFB and OFB
 * in  bytes (streams).  Note that CFB and OFB in the rest of the document
 * represent the "8bit CFB or OFB" mode.  nOFB and nCFB modes represents a
 * n-bit  OFB/CFB mode, n is used to represent the algorithm's block size.
 * The library supports an extra STREAM mode to include some stream  algorithms
 * like WAKE or ARCFOUR.
 */

/*!\var MCrypt::Mode MCrypt::Mode_STREAM
 * The mode used with stream ciphers. In this mode the keystream from the cipher is
 * XORed with the plaintext. Thus you should NOT ever use the same key.
 */

/*!\var MCrypt::Mode MCrypt::Mode_ECB
 * The Electronic CodeBook mode. It is the simplest mode to use with a  block
 * cipher.  Encrypts  each block  independently. It is a block mode so plaintext
 * length should be a multiple of blocksize (n*block‐size).
 */

/*!\var MCrypt::Mode MCrypt::Mode_CBC
 * The Cipher Block Chaining mode. It is better than ECB since the plaintext
 * is XOR’ed with the previous  ciphertext.  A  random block should be placed
 * as the first block (IV) so the same block or messages always encrypt to
 * something different. It is a block mode so plaintext length should be
 * a multiple of blocksize (n*blocksize).
 */

/*!\var MCrypt::Mode MCrypt::Mode_CFB
 * The  Cipher-Feedback Mode (in 8bit). This is a self-synchronizing stream
 * cipher implemented from a block cipher. This is the best mode to use
 * for encrypting strings or streams. This mode requires an IV.
 */

/*!\var MCrypt::Mode MCrypt::Mode_OFB
 * The  Output-Feedback  Mode (in 8bit). This is a synchronous stream
 * cipher implemented from a block cipher. It is intended for use in noisy lines,
 * because corrupted ciphertext blocks do  not  corrupt  the plaintext  blocks
 * that follow. Insecure (because used in 8bit mode) so it is recommended not
 * to use it. Added just for completeness.
 */

/*!\var MCrypt::Mode MCrypt::Mode_nOFB
 * The Output-Feedback Mode (in nbit). n Is the size of the block of the
 * algorithm. This  is  a  synchronous  stream  cipher implemented from a
 * block cipher. It is intended for use in noisy lines, because corrupted
 * ciphertext blocks do not corrupt the plaintext blocks  that  follow.
 * This  mode  operates  in streams.
 */

/*!\var MCrypt::Mode MCrypt::Mode_nCFB
 * The  Cipher-Feedback  Mode (in nbit). n Is the size of the block of
 * the algorithm. This is a self synchronizing stream cipher implemented
 * from a block cipher. This mode operates in streams.
 */

/*!\var MCrypt::Mode MCrypt::Mode_CTR
 * The Counter Mode. This is a stream cipher implemented from a  block
 * cipher.  This  mode  uses  the cipher  to encrypt a set of input blocks,
 * called counters, to produce blocks that will be XORed with the plaintext.
 * In libmcrypt the counter is the given IV which is incremented at each step.
 * This mode operates in streams.
 */


/*!\enum MCrypt::Algorithm
 * \brief Unterstütze Algorithmen
 *
 * This version of  the
 * library  supports many encryption algorithms and encryption modes. Some
 * algorithms which are supported: SERPENT, RIJNDAEL, 3DES, GOST,  SAFER+,
 * CAST-256, RC2, XTEA, 3WAY, TWOFISH, BLOWFISH, ARCFOUR, WAKE and more.
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_DES
 * The traditional DES algorithm designed by IBM and US NSA.  Uses 56
 * bit key and 64 bit block. It is now considered a weak algorithm, due to
 * its  small  key  size  (it  was  never intended for use with classified
 * data).
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_TRIPLE_DES
 * 3DES or Triple DES: DES  but  with  multiple  (triple)  encryption.  It
 * encrypts  the plaintext once, then decrypts it with the second key, and
 * encrypts it again with the third key (outer cbc  mode  used  for  cbc).
 * Much  better  than traditional DES since the key is now 168 bits (actually
 * the effective key length is 112 bits due to the meet-in-the-middle attack).
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_CAST_128
 * CAST  was  designed in Canada by Carlisle Adams and Stafford
 * Tavares.  The original algorithm used a 64bit key and block. The  algorithm
 * here  is CAST-128 (also called CAST5) which has a 128bit key and
 * 64bit block size.
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_CAST_256
 * CAST-256 was designed by Carlisle Adams. It  is  a  symmetric
 * cipher  designed in accordance with the CAST design procedure. It is an
 * extention of the CAST-128, having a 128 bit block size, and up  to  256
 * bit key size.
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_xTEA
 * TEA  stands  for  the Tiny Encryption Algorithm. It is a feistel
 * cipher designed by David Wheeler & Roger M. Needham.  The original  TEA
 * was  intended  for use in applications where code size is at a premium,
 * or where it is necessary for someone to remember the algorithm and code
 * it on an arbitrary machine at a later time.  The algorithm used here is
 * extended TEA and has a 128bit key size and 64bit block size.
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_THREE_WAY
 * The 3way algorithm designed by Joan  Daemen.  It  uses  key  and
 * block size of 96 bits.
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_SKIPJACK
 * SKIPJACK was designed by the US NSA. It was part of the illfated
 * "Clipper" Escrowed Encryption Standard (EES) (FIPS 185) proposal.
 * It  operates  on  64bit  blocks  and uses a key of 80 bits. SKIPJACK is
 * provided only as an extra module to libmcrypt.
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_BLOWFISH
 * The Blowfish algorithm designed by Bruce Schneier. It is better
 * and faster than DES. It can use a key up to 448 bits.
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_TWOFISH
 * Twofish  was  designed  by Bruce Schneier, Doug Whiting, John
 * Kelsey, Chris Hall, David Wagner for Counterpane systems.  Intended  to
 * be  highly  secure and highly flexible. It uses a 128bit block size and
 * 128,192,256 bit key size.  (Twofish is the default algorithm)
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_LOKI97
 * LOKI97 was designed by Lawrie Brown and Josef Pieprzyk. It  has
 * a 128-bit block length and a 256bit key schedule, which can be initialized
 * using 128, 192 or 256 bit keys. It has evolved  from  the  earlier LOKI89
 * and LOKI91 64-bit block ciphers, with a strengthened key schedule and a
 * larger keyspace.
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_RC2
 * RC2 (RC stands for Rivest Cipher) was designed by Ron  Rivest.  It
 * uses  block  size  of  64 bit and a key size from 8 to 1024 bits. It is
 * optimized  for  16bit  microprocessors  (reflecting  its  age).  It  is
 * described in the RFC2268.
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_ARCFOUR
 * RC4  was designed by Ron Rivest. For several years this algorithm was
 * considered a trade secret and details were not available.  In
 * September  1994 someone posted the source code in the cypherpunks mailing
 * list. Although the source code is now available RC4 is  trademarked
 * by  RSADSI  so  a  compatible  cipher  named ARCFOUR is included in the
 * mcrypt distribution. It is a stream cipher and has  a  maximum  key  of
 * 2048 bits.
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_RC6
 * RC6  was  designed  by Ron Rivest for RSA labs. In mcrypt it uses
 * block size of 128 bit and a key size of 128/192/256 bits.  Refer to RSA
 * Labs and Ron Rivest for any copyright, patent or license issues for the
 * RC6 algorithm. RC6 is provided only as an extra module to libmcrypt.
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_RIJNDAEL_128
 * Rijndael is a block cipher, designed by Joan Daemen and  Vincent
 * Rijmen,  and  was approved for the USA’s NIST Advanced Encryption
 * Standard, FIPS-197.  The cipher has a variable  block  length  and  key
 * length. Rijndael can be implemented very efficiently on a wide range of
 * processors and in hardware. The design of Rijndael was strongly  influenced
 * by the design of the block cipher Square.  There exist three versions
 * of this algorithm, namely: RIJNDAEL-128 (the AES winner),  RIJNDAEL-192,
 * RIJNDAEL-256  The  numerals 128, 192 and 256 stand for the length of
 * the block size.
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_RIJNDAEL_192
 * \copydoc MCrypt::Algo_RIJNDAEL_128
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_RIJNDAEL_256
 * \copydoc MCrypt::Algo_RIJNDAEL_128
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_MARS
 * MARS is a 128-bit block cipher designed by IBM as a candidate for
 * the  Advanced  Encryption  Standard.  Refer  to  IBM for any copyright,
 * patent or license issues for the MARS algorithm. MARS is provided  only
 * as an extra module to libmcrypt.
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_PANAMA
 * PANAMA  is  a  cryptographic module that can be used both as a
 * cryptographic hash function and as a stream cipher. It designed by Joan
 * Daemen and Craig Clapp. PANAMA (the stream cipher) is included in libmcrypt.
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_WAKE
 * WAKE stands for Word Auto Key Encryption, and  is  an  encryption
 * system  for  medium  speed  encryption  of blocks and of high security.
 * WAKE was designed by David J. Wheeler. It is intended  to  be  fast  on
 * most  computers  and  relies  on  repeated table use and having a large
 * state space.
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_SERPENT
 * Serpent is a 128-bit block cipher designed by  Ross  Anderson,
 * Eli  Biham  and Lars Knudsen as a candidate for the Advanced Encryption
 * Standard.  Serpent’s design was limited to well understood  mechanisms,
 * so  that  could rely on the wide experience of block cipher cryptanalysis,
 * and achieve the highest  practical  level  of  assurance  that  no shortcut
 * attack will be found. Serpent has twice as many rounds as are
 * necessary, to block all currently known shortcut attacks. Despite these
 * exacting design constraints, Serpent is faster than DES.
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_IDEA
 * IDEA  stands for International Data Encryption Algorithm and was
 * designed by Xuejia Lai and James Massey. It operates  on  64bit  blocks
 * and  uses a key of 128 bits.  Refer to Ascom-Tech AG for any copyright,
 * patent or license issues for the IDEA algorithm. IDEA is provided  only
 * as an extra module to libmcrypt.
 */


/*!\var MCrypt::Algorithm MCrypt::Algo_ENIGMA
 * (UNIX  crypt):  A one-rotor machine designed along the lines of
 * Enigma but considerable trivialized. Very easy to break for  a  skilled
 * cryptanalyst.  I suggest against using it. Added just for completeness.
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_GOST
 * A former soviet union’s algorithm. An acronym for  "Gosudarstvennyi Standard"
 * or  Government Standard. It uses a 256 bit key and a 64 bit block.
 * The S-boxes used here are described in the Applied  Cryptography  book
 * by  Bruce  Schneier.  They  were used in an application for the Central
 * Bank of the Russian Federation.
 *
 * Some quotes from  gost.c:  The  standard  is  written  by  A.  Zabotin
 * (project  leader),  G.P. Glazkov, and V.B. Isaeva.  It was accepted and
 * introduced into use by the action of the State Standards  Committee  of
 * the  USSR  on  2 June 1989 as No. 1409.  It was to be reviewed in 1993,
 * but whether anyone wishes to take on this obligation from the  USSR  is
 * questionable.
 *
 * This  code is based on the 25 November 1993 draft translation by Aleksandr
 * Malchik, with Whitfield Diffie, of the Government Standard of the
 * U.S.S.R.   GOST  28149-89,  "Cryptographic  Transformation  Algorithm",
 * effective 1 July  1990.   (Whitfield.Diffie@eng.sun.com)  Some  details
 * have  been  cleared  up  by  the paper "Soviet Encryption Algorithm" by
 * Josef Pieprzyk and Leonid Tombak of the University of  Wollongong,  New
 * South Wales.
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_SAFER_SK64
 * SAFER  (Secure  And  Fast Encryption Routine) is a block cipher
 * developed by Prof. J.L. Massey at the Swiss Federal Institute of  Technology.
 * There  exist  four  versions of this algorithm, namely: SAFER K-64,
 * SAFER K-128 , SAFER SK-64 and SAFER SK-128.  The numerals 64 and
 * 128  stand  for the length of the user-selected key, ’K’ stands for the
 * original key schedule and ’SK’ stands for the strengthened key schedule
 * (in  which  some  of the "weaknesses" of the original key schedule have
 * been removed). In mcrypt only SAFER SK-64 and SAFER SK-128 are used.
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_SAFER_SK128
 * \copydoc MCrypt::Algo_SAFER_SK64
 */

/*!\var MCrypt::Algorithm MCrypt::Algo_SAFER_PLUS
 * SAFER+ was designed by  Prof.  J.L.  Massey,  Prof.  Gurgen  H.
 * Khachatrian  and  Dr. Melsik K. Kuregian for Cylink. SAFER+ is based on
 * the existing SAFER family of ciphers and provides for a block  size  of
 * 128bits and 128, 192 and 256 bits key length.
 */



static const char *ivs="h/fA_dH*Dsj?.2n87=löÜ'kb#~Cvö:ä(tgXcyz kjhd!%hd/(Gd5§,6F790=U";

MCrypt::MCrypt()
/*!\brief Konstruktor der Klasse
 *
 * \descr
 * Bei Verwendung dieses Konstruktors werden nur einige interne Variablen initialisert
 * und ein Default Initialisierungsvektor (IV) gesetzt (siehe MCrypt::setIV). Der
 * gewünschte Verschlüsselungsalgorithmus muss mittels der Funktion MCrypt::setAlgorithm
 * festgelegt werden.
 *
 */
{
	mcrypt=NULL;
	IV.copy((void*)ivs,strlen(ivs));
}

MCrypt::MCrypt(Algorithm algo, Mode mode)
/*!\brief Konstruktor mit Initialisierung des Algorithmus
 *
 * \descr
 * Bei Verwendung dieses Konstruktors wird die Klasse gleichzeitig mit einem bestimmten
 * Algorithmus und einem Verschlüsselungsmodus initialisiert, so dass die Funktion
 * MCrypt::setAlgorithm nicht mehr aufgerufen werden muss.
 *
 * @param algo Der gewünschte Algorithmus (siehe MCrypt::Algorithm)
 * @param mode Der gewünschte Modus (siehe MCrypt::Mode)
 *
 * \note Auch wenn dieser Konstruktor verwendet wurde, kann jederzeit mit der Funktion
 * MCrypt::setAlgorithm ein anderer Verschlüsselungsmechanismus ausgewählt werden.
 *
 * @throws UnsupportedFeatureException
 * @throws InvalidAlgorithmException
 */
{
	mcrypt=NULL;
	setAlgorithm(algo,mode);
	IV.copy((void*)ivs,strlen(ivs));
}

MCrypt::~MCrypt()
/*!\brief Destruktor der Klasse
 *
 * Der durch die Klasse belegte Speicher wird freigegeben.
 */
{
#ifdef HAVE_LIBMCRYPT
	if (mcrypt) mcrypt_module_close((MCRYPT)mcrypt);
#endif
}

void MCrypt::setAlgorithm(Algorithm algo, Mode mode)
/*!\brief Initialisierung der Klasse
 *
 * \descr
 * Mit dieser Funktion wird der gewünschte Verschlüsselungsalgorithmus und der
 * Modus ausgewählt wird. Beide Parameter sind optional.
 *
 * @param algo Der gewünschte Algorithmus (siehe MCrypt::Algorithm). Der Default ist
 * MCrypt::Algo_TWOFISH
 * @param mode Der gewünschte Modus (siehe MCrypt::Mode). Der Defalt ist
 * MCrypt::Mode_CFB
 *
 * @throws UnsupportedFeatureException
 * @throws InvalidAlgorithmException
 */
{
#ifndef HAVE_LIBMCRYPT
	throw UnsupportedFeatureException("MCrypt");
#else
	if (mcrypt) mcrypt_module_close((MCRYPT)mcrypt);
	mcrypt=NULL;
	const char *a=NULL, *m=NULL;
	switch (mode) {
		case Mode_STREAM: m="stream"; break;
		case Mode_ECB: m="ecb"; break;
		case Mode_CBC: m="cbc"; break;
		case Mode_CFB: m="cfb"; break;
		case Mode_OFB: m="ofb"; break;
		case Mode_nOFB: m="nofb"; break;
		case Mode_nCFB: m="ncfb"; break;
		case Mode_CTR: m="ctr"; break;
	};
	switch (algo) {
		case Algo_DES: a="des"; break;
		case Algo_TRIPLE_DES: a="tripledes"; break;
		case Algo_CAST_128: a="cast-128"; break;
		case Algo_CAST_256: a="cast-256"; break;
		case Algo_xTEA: a="xtea"; break;
		case Algo_THREE_WAY: a="threeway"; break;
		case Algo_SKIPJACK: a="skipjack"; break;
		case Algo_BLOWFISH: a="blowfish"; break;
		case Algo_TWOFISH: a="twofish"; break;
		case Algo_LOKI97: a="loki97"; break;
		case Algo_RC2: a="rc2"; break;
		case Algo_ARCFOUR: a="arcfour"; break;
		case Algo_RC6: a="rc6"; break;
		case Algo_RIJNDAEL_128: a="rijndael-128"; break;
		case Algo_RIJNDAEL_192: a="rijndael-192"; break;
		case Algo_RIJNDAEL_256: a="rijndael-256"; break;
		case Algo_MARS: a="mars"; break;
		case Algo_PANAMA: a="panama"; break;
		case Algo_WAKE: a="wake"; break;
		case Algo_SERPENT: a="serpent"; break;
		case Algo_IDEA: a="idea"; break;
		case Algo_ENIGMA: a="enigma"; break;
		case Algo_GOST: a="gost"; break;
		case Algo_SAFER_SK64: a="safer-sk64"; break;
		case Algo_SAFER_SK128: a="safer-sk128"; break;
		case Algo_SAFER_PLUS: a="saferplus"; break;
	}
	if (a==NULL || m==NULL) {
		throw InvalidAlgorithmException("MCrypt::setAlgorithm, algo=%i, mode=%i",algo,mode);
	}
	mcrypt=mcrypt_module_open((char*)a,NULL,(char*)m,NULL);
	if (mcrypt==MCRYPT_FAILED) {
		mcrypt=NULL;
		throw InvalidAlgorithmException("MCrypt::setAlgorithm, algo=%i, mode=%i",algo,mode);
	}
#endif
}


int MCrypt::getIVSize() const
/*!\brief Länge des Initialization Vector
 *
 * \descr
 * Mit dieser Funktion kann die Länge des Intilialisierungsvektors (IV) für den derzeit
 * ausgewählten Algorithmus ausgelesen werden.
 * \par
 * Die Länge des IV ist abhängig vom Algorithmus, daher liefert die Funktion erst nach der Initialisierung mit
 * MCrypt::setAlgorithm oder bei Verwendung des Konstruktors MCrypt::MCrypt(Algorithm algo, Mode mode) einen
 * aussagekräftigen Wert. Ob ein Algorithmus überhaupt einen IV benötigt, kann mit der Funktion
 * MCrypt::needIV geprüft werden.
 * \par
 * Weitere Informationen zum Thema "Initialization Vector" sind in der Wikipedia zu finden:
 * - http://en.wikipedia.org/wiki/Initialization_vector
 * - http://de.wikipedia.org/wiki/Initialisierungsvektor
 *
 * @return Liefert die Länge des IV zurück
 * @throws NoAlgorithmSpecifiedException
 * @throws UnsupportedFeatureException
 *
 * \see MCrypt::setIV
 */
{
#ifdef HAVE_LIBMCRYPT
	if (!mcrypt) {
		throw NoAlgorithmSpecifiedException();
	}
	return mcrypt_enc_get_iv_size((MCRYPT)mcrypt);
#else
	throw UnsupportedFeatureException("MCrypt");
#endif
}


int MCrypt::getMaxKeySize() const
/*!\brief Maximale Länge des Schlüssels
 * \descr
 * Mit dieser Funktion kann abgefragt werden, wie lang der zur Verschlüsselung verwendete
 * Schlüssel maximal sein darf. Die Länge ist abhängig vom verwendeten Algorithmus, daher
 * liefert die Funktion erst nach der Initialisierung mit MCrypt::setAlgorithm oder bei Verwendung
 * des Konstruktors MCrypt::MCrypt(Algorithm algo, Mode mode) einen Wert zurück.
 * \par
 * Der später verwendete Schlüssel darf jede beliebige Länge zwischen 1 und dem zurückgegebenen Wert lang sein.
 *
 * @return Die Funktion liefert die maximal erlaubte Länge des Schlüssels zurück
 *
 * @throws NoAlgorithmSpecifiedException
 * @throws UnsupportedFeatureException
 */
{
#ifdef HAVE_LIBMCRYPT
	if (!mcrypt) {
		throw NoAlgorithmSpecifiedException();
	}
	return mcrypt_enc_get_key_size((MCRYPT)mcrypt);
#else
	throw UnsupportedFeatureException("MCrypt");
#endif
}

void MCrypt::setIV(const void *buffer, size_t size)
/*!\brief Initialization Vector (IV) setzen
 *
 * \descr
 * Mit dieser Funktion wird der Initialization Vector (kurz: IV) gesetzt.
 * \par
 * Der Initialisierungsvektor (IV) ist ein Begriff aus der Kryptographie und bezeichnet einen Block
 * von Zufallsdaten, der in bestimmten Modi einiger Blockchiffren verwendet wird, wie dem
 * Cipher Block Chaining Mode (siehe MCrypt::Mode_CBC).
 * \par
 * Beim Verschlüsseln von Nachrichten muss vermieden werden, dass gleiche Klartextblöcke immer wieder
 * gleiche Geheimtextblöcke ergeben. Ein förmlicher Brief fängt im Deutschen in der Regel mit
 * "Sehr geehrter Herr/Frau" gefolgt vom Namen an. Aus diesem Wissen könnte ein Angreifer versuchen Rückschlüsse
 * auf den verwendeten Schlüssel zu ziehen. Um das zu vermeiden, wird der erste Klartextblock mit einem IV
 * XOR-verknüpft. Da der IV zufällig erzeugt wurde, unterscheiden sich die entstehenden Geheimtexte auch dann,
 * wenn die Klartexte mit identischen Daten beginnen.
 * \par
 * Da bei den Verschlüsselungsalgorithmen in der Regel Modi gewählt werden, bei denen der Geheimtext
 * eines Blocks vom Geheimtext seines Vorgängerblocks abhängt, muss der IV nicht geheim gehalten werden.
 * Im beschriebenen Fall würde der Geheimtext des Block Bn − 1 als IV des Blocks Bn fungieren, so dass für
 * die Kryptanalysten ohnehin n − 1 Initialisierungsvektoren bekannt wären.
 * \par
 * (Quelle: http://de.wikipedia.org/wiki/Initialisierungsvektor)
 *
 *
 * @param buffer Pointer auf den Speicherbereich, der den IV enthält
 * @param size Länge des Speicherbereichs
 *
 * \note
 * Falls der gewählte Algorithmus einen IV benötigt, aber keiner gesetzt wurde, verwendet die Klasse einen
 * statischen IV. Dieses Vorgehen ist aber nicht der Sicherheit dienlich, da der IV immer gleich ist
 * und dem Quellcode der Klasse entnommen werden kann. Es sollte daher immer ein eigener IV gesetzt werden!
 * \par
 * Die Klasse ist relativ tollerant, was die Länge des IV angeht. Zwar kann man mit der Funktion
 * MCrypt::GetIVSize abfragen, wir lang der IV für den gewählten Algorithmus sein muss, jedoch kann man
 * auch einen längeren oder kürzeren Wert angeben. Ist der Wert zu kurz, wird er einfach solange wiederholt,
 * bis die erforderliche Länge erreicht ist. Ist er zu lang, wird er an der notendigen Stelle abgeschnitten.
 * Dadurch kann es unter Umständen zu Problemen kommen, falls ein Datenblock mit einer Anwendung
 * verschlüsselt wurde oder entschlüsselt werden soll. Es wird daher empfohlen die erforderliche Länge
 * vorher mit MCrypt::GetIVSize abzufragen und dann mit dieser Funktion einen passenden Wert zu setzen.
 */
{
	if (buffer==NULL || size==0) throw IllegalArgumentException();
	IV.copy(buffer,size);
}

void MCrypt::setIV(const Variant &object)
/*!\brief Initialization Vector (IV) setzen
 *
 * \descr
 * Mit dieser Funktion wird der Initialization Vector (kurz: IV) gesetzt.
 * \par
 * Der Initialisierungsvektor (IV) ist ein Begriff aus der Kryptographie und bezeichnet einen Block
 * von Zufallsdaten, der in bestimmten Modi einiger Blockchiffren verwendet wird, wie dem
 * Cipher Block Chaining Mode (siehe MCrypt::Mode_CBC).
 * \par
 * Beim Verschlüsseln von Nachrichten muss vermieden werden, dass gleiche Klartextblöcke immer wieder
 * gleiche Geheimtextblöcke ergeben. Ein förmlicher Brief fängt im Deutschen in der Regel mit
 * "Sehr geehrter Herr/Frau" gefolgt vom Namen an. Aus diesem Wissen könnte ein Angreifer versuchen Rückschlüsse
 * auf den verwendeten Schlüssel zu ziehen. Um das zu vermeiden, wird der erste Klartextblock mit einem IV
 * XOR-verknüpft. Da der IV zufällig erzeugt wurde, unterscheiden sich die entstehenden Geheimtexte auch dann,
 * wenn die Klartexte mit identischen Daten beginnen.
 * \par
 * Da bei den Verschlüsselungsalgorithmen in der Regel Modi gewählt werden, bei denen der Geheimtext
 * eines Blocks vom Geheimtext seines Vorgängerblocks abhängt, muss der IV nicht geheim gehalten werden.
 * Im beschriebenen Fall würde der Geheimtext des Block Bn − 1 als IV des Blocks Bn fungieren, so dass für
 * die Kryptanalysten ohnehin n − 1 Initialisierungsvektoren bekannt wären.
 * \par
 * (Quelle: http://de.wikipedia.org/wiki/Initialisierungsvektor)
 *
 *
 * @param object Ein von CVar abgeleitetes Objekt, dass den IV enthält. Derzeit wird
 * CString, CWString und CBinary unterstützt.
 * @return Bei Erfolg wird 1 zurückgegeben, im Fehlerfall 0. Ein Fehler tritt auf, wenn
 * das Objekt leer ist oder kein Speicher mehr zur Verfügung steht.
 *
 * \note
 * Falls der gewählte Algorithmus einen IV benötigt, aber keiner gesetzt wurde, verwendet die Klasse einen
 * statischen IV. Dieses Vorgehen ist aber nicht der Sicherheit dienlich, da der IV immer gleich ist
 * und dem Quellcode der Klasse entnommen werden kann. Es sollte daher immer ein eigener IV gesetzt werden!
 * \par
 * Die Klasse ist relativ tollerant, was die Länge des IV angeht. Zwar kann man mit der Funktion
 * MCrypt::GetIVSize abfragen, wir lang der IV für den gewählten Algorithmus sein muss, jedoch kann man
 * auch einen längeren oder kürzeren Wert angeben. Ist der Wert zu kurz, wird er einfach solange wiederholt,
 * bis die erforderliche Länge erreicht ist. Ist er zu lang, wird er an der notendigen Stelle abgeschnitten.
 * Dadurch kann es unter Umständen zu Problemen kommen, falls ein Datenblock mit einer Anwendung
 * verschlüsselt wurde oder entschlüsselt werden soll. Es wird daher empfohlen die erforderliche Länge
 * vorher mit MCrypt::GetIVSize abzufragen und dann mit dieser Funktion einen passenden Wert zu setzen.
 */
{
	int type=object.type();
	if (type==Variant::TYPE_BYTEARRAY || type==Variant::TYPE_BYTEARRAYPTR) {
		const ByteArrayPtr &bin=object.toByteArrayPtr();
		IV.copy(bin);
	} else if (type==Variant::TYPE_STRING) {
		const String &str=object.toString();
		IV.copy(str.getPtr(),str.size());
	} else if (type==Variant::TYPE_WIDESTRING) {
		const WideString &str=object.toWideString();
		IV.copy(str.getPtr(),str.size());
	} else {
		throw UnsupportedDataTypeException();
	}
}

void MCrypt::setKey(const void *buffer, size_t size)
/*!\brief Schlüssel festlegen
 *
 * \descr
 * Mit dieser Funktion wird der Schlüssel definiert, mit dem die Daten verschlüsselt oder
 * entschlüsselt werden sollen. Die maximale Länge des Schlüssels hängt vom Algorithmus ab,
 * und muss daher vorher mit der Funktion MCrypt::GetMaxKeySize() abgefragt werden.
 *
 *
 * @param buffer Pointer auf den Speicherbereich, der den Schlüssel enthält
 * @param size Länge des Schlüssels in Bytes
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0
 *
 * \note
 * Die Funktion nimmt auch längere Schlüssel an, jedoch wird er bei Aufruf von Crypt
 * oder Decrypt bei der maximalen Länge abgeschnitten. Das kann unter umständen zu
 * Problemen führen, wenn zur Verschlüsselung oder Entschlüsselung auch andere Programme
 * eingesetzt werden. Es wird daher empfohlen die maximale Länge durch Aufruf von
 * MCrypt::GetMaxKeySize abzufragen und einen passenden Schlüssel zu verwenden.
 *
 */
{
	if (buffer==NULL || size==0) throw IllegalArgumentException();
	Key.copy(buffer,size);
}

void MCrypt::setKey(const Variant &object)
/*!\brief Schlüssel festlegen
 *
 * \descr
 * Mit dieser Funktion wird der Schlüssel definiert, mit dem die Daten verschlüsselt oder
 * entschlüsselt werden sollen. Die maximale Länge des Schlüssels hängt vom Algorithmus ab,
 * und muss daher vorher mit der Funktion MCrypt::GetMaxKeySize() abgefragt werden.
 *
 *
 * @param object Ein von CVar abgeleitetes Objekt, das den Schlüssel enthält.
 * Unterstützt werden CString, CWString und CBinary.
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0
 *
 * \note
 * Die Funktion nimmt auch längere Schlüssel an, jedoch wird er bei Aufruf von Crypt
 * oder Decrypt bei der maximalen Länge abgeschnitten. Das kann unter umständen zu
 * Problemen führen, wenn zur Verschlüsselung oder Entschlüsselung auch andere Programme
 * eingesetzt werden. Es wird daher empfohlen die maximale Länge durch Aufruf von
 * MCrypt::GetMaxKeySize abzufragen und einen passenden Schlüssel zu verwenden.
 *
 */
{
	int type=object.type();
	if (type==Variant::TYPE_BYTEARRAY || type==Variant::TYPE_BYTEARRAYPTR) {
		const ByteArrayPtr &bin=object.toByteArrayPtr();
		Key.copy(bin);
	} else if (type==Variant::TYPE_STRING) {
		const String &str=object.toString();
		Key.copy(str.getPtr(),str.size());
	} else if (type==Variant::TYPE_WIDESTRING) {
		const WideString &str=object.toWideString();
		Key.copy(str.getPtr(),str.size());
	} else {
		throw UnsupportedDataTypeException();
	}
}

bool MCrypt::needIV() const
/*!\brief Prüfen, ob ein Initialisierungsvektor (IV) benötigt wird.
 *
 * \descr
 * Der Initialisierungsvektor (IV) ist ein Begriff aus der Kryptographie und bezeichnet einen Block
 * von Zufallsdaten, der in bestimmten Modi einiger Blockchiffren verwendet wird, wie dem
 * Cipher Block Chaining Mode (siehe MCrypt::Mode_CBC).
 * \par
 * Beim Verschlüsseln von Nachrichten muss vermieden werden, dass gleiche Klartextblöcke immer wieder
 * gleiche Geheimtextblöcke ergeben. Ein förmlicher Brief fängt im Deutschen in der Regel mit
 * "Sehr geehrter Herr/Frau" gefolgt vom Namen an. Aus diesem Wissen könnte ein Angreifer versuchen Rückschlüsse
 * auf den verwendeten Schlüssel zu ziehen. Um das zu vermeiden, wird der erste Klartextblock mit einem IV
 * XOR-verknüpft. Da der IV zufällig erzeugt wurde, unterscheiden sich die entstehenden Geheimtexte auch dann,
 * wenn die Klartexte mit identischen Daten beginnen.
 * \par
 * Da bei den Verschlüsselungsalgorithmen in der Regel Modi gewählt werden, bei denen der Geheimtext
 * eines Blocks vom Geheimtext seines Vorgängerblocks abhängt, muss der IV nicht geheim gehalten werden.
 * Im beschriebenen Fall würde der Geheimtext des Block Bn − 1 als IV des Blocks Bn fungieren, so dass für
 * die Kryptanalysten ohnehin n − 1 Initialisierungsvektoren bekannt wären.
 * \par
 * (Quelle: http://de.wikipedia.org/wiki/Initialisierungsvektor)
 * \par
 * Ob ein Initialisierungsvektor benötigt wird, hängt vom verwendeten Verschlüsselungsalgorithmus ab.
 * Dieser muss daher zunächst mittels der Funktion MCrypt::setAlgorithm oder durch Verwendung des Konstruktors
 * MCrypt::MCrypt(Algorithm algo, Mode mode) ausgewählt worden sein. Anschließend kann dann mit
 * dieser Funktion geprüft werden, ob ein IV benötigt wird und mit MCrypt::GetIVSize wie lang
 * dieser sein muss.
 *
 * @return Wird ein IV benötigt, liefert die Funktion 1 zurück, sonst 0. Im Fehlerfall wird -1
 * zurückgegeben.
 */
{
#ifdef HAVE_LIBMCRYPT
	if (!mcrypt) {
		throw NoAlgorithmSpecifiedException();
	}
	return mcrypt_enc_mode_has_iv((MCRYPT)mcrypt);
#else
	throw UnsupportedFeatureException("MCrypt");
#endif
}

void MCrypt::crypt(void *buffer, size_t size)
/*!\brief Verschlüsseln eines Speicherbereiches
 *
 * \descr
 * Mit dieser Funktion wird der mit \p buffer angegebene Speicherbereich mit einer Länge
 * von \p size Bytes verschlüsselt.
 *
 * \param buffer Pointer auf den Beginn des zu verschlüsselnden Speicherbereiches
 * \param size Die Größe des zu verschlüsselnden Speicherbereiches in Bytes
 *
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 *
 * \note Vor Aufruf dieser Funktion muss zunächst der gewünschte Verschlüsselungsalgorithmus
 * mittels der Funktion MCrypt::setAlgorithm oder durch Verwendung des Konstruktors
 * MCrypt::MCrypt(Algorithm algo, Mode mode) ausgewählt worden sein. Ferner muss eventuell noch
 * ein Initialisierungsvektor (siehe MCrypt::NeedIV und MCrypt::SetIV) und ein Schlüssel
 * (siehe MCrypt::SetKey) angegeben worden sein.
 */
{
	ByteArrayPtr bin;
	bin.use(buffer,size);
	crypt(bin);
}

void MCrypt::crypt(const Variant &in, ByteArray &out)
/*!\brief Verschlüsseln eines Objekts
 *
 * \descr
 * Mit dieser Funktion wird das mit \p in übergebene von CVar abgeleitete Objekt eingelesen und
 * seine verschlüsselte Form in \p out gespeichert. Die Verschlüsselung erfolgt nicht innerhalb
 * des Objekts \p in, da die Verschlüsselung auf binärer Ebene abläuft und das Ergebnis mit
 * hoher Wahrscheinlichkeit kein gültiges Objekt mehr ist (einzige Ausnahme ist CBinary).
 *
 * \param[in] in Das zu verschlüsselnde Objekt. Gegenwärtig wird CString, CWString und CBinary
 * unterstützt.
 * \param[out] out Ein Objekt vom Typ CBinary, in dem die verschlüsselten Daten abgelegt werden.
 *
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 *
 * \note Vor Aufruf dieser Funktion muss zunächst der gewünschte Verschlüsselungsalgorithmus
 * mittels der Funktion MCrypt::setAlgorithm oder durch Verwendung des Konstruktors
 * MCrypt::MCrypt(Algorithm algo, Mode mode) ausgewählt worden sein. Ferner muss eventuell noch
 * ein Initialisierungsvektor (siehe MCrypt::NeedIV und MCrypt::SetIV) und ein Schlüssel
 * (siehe MCrypt::SetKey) angegeben worden sein.
 */
{
	int type=in.type();
	if (type==Variant::TYPE_BYTEARRAY || type==Variant::TYPE_BYTEARRAYPTR) {
		const ByteArrayPtr &bin=in.toByteArrayPtr();
		out.copy(bin);
	} else if (type==Variant::TYPE_STRING) {
		const String &str=in.toString();
		out.copy(str.getPtr(),str.size());
	} else if (type==Variant::TYPE_WIDESTRING) {
		const WideString &str=in.toWideString();
		out.copy(str.getPtr(),str.size());
	} else {
		throw UnsupportedDataTypeException();
	}
	crypt(out);
}

void MCrypt::crypt(ByteArrayPtr &buffer)
/*!\brief Verschlüsseln eines CBinary-Objekts
 *
 * \descr
 * Mit dieser Funktion wird der vom CBinary-Objekts \p buffer verwaltete Speicherbereich
 * verschlüsselt.
 *
 * \param[in,out] buffer Das CBinary Objekt, das den zu verschlüsselnden Speicherbereich enthält.
 *
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 *
 * \note Vor Aufruf dieser Funktion muss zunächst der gewünschte Verschlüsselungsalgorithmus
 * mittels der Funktion MCrypt::setAlgorithm oder durch Verwendung des Konstruktors
 * MCrypt::MCrypt(Algorithm algo, Mode mode) ausgewählt worden sein. Ferner muss eventuell noch
 * ein Initialisierungsvektor (siehe MCrypt::NeedIV und MCrypt::SetIV) und ein Schlüssel
 * (siehe MCrypt::SetKey) angegeben worden sein.
 */
{
#ifndef HAVE_LIBMCRYPT
	throw UnsupportedFeatureException("MCrypt");
#else
	char *myIV=NULL;
	// Wurde ein Key angegeben?
	if (Key.size()==0) throw NoKeySpecifiedException();
	// Benötigen wir IV?
	if (mcrypt_enc_mode_has_iv((MCRYPT)mcrypt)) {
		// Haben wir in IV irgendwas?
		size_t myIVSize=IV.size();
		if (myIVSize==0) throw NoIVSpecifiedException();
		int rest=mcrypt_enc_get_iv_size((MCRYPT)mcrypt);
		myIV = (char*)malloc(rest);
		if (!myIV) throw OutOfMemoryException();
		int p=0;
		while (rest) {
			int bytes=myIVSize;
			if (bytes>rest) bytes=rest;
			memcpy(myIV+p,IV.ptr(),bytes);
			p+=bytes;
			rest-=bytes;
		}
	}
    size_t keysize=mcrypt_enc_get_key_size((MCRYPT)mcrypt);
    if (Key.size()<keysize) keysize=Key.size();
    int ret=mcrypt_generic_init((MCRYPT)mcrypt,(void*)Key.ptr(), keysize, myIV);
    if (ret<0) {
    	const char *e=mcrypt_strerror(ret);
    	mcrypt_generic_deinit((MCRYPT)mcrypt);
    	if (myIV) free(myIV);
    	throw EncryptionFailedException("%s",e);
    }
	ret=mcrypt_generic((MCRYPT)mcrypt, (void*)buffer.ptr(),buffer.size());
	if (ret<0) {
		const char *e=mcrypt_strerror(ret);
    	mcrypt_generic_deinit((MCRYPT)mcrypt);
    	if (myIV) free(myIV);
    	throw EncryptionFailedException("%s",e);
	}
	mcrypt_generic_deinit((MCRYPT)mcrypt);
	if (myIV) free(myIV);
#endif
}

void MCrypt::decrypt(void *buffer, size_t size)
/*!\brief Entschlüsseln eines Speicherbereiches
 *
 * \descr
 * Mit dieser Funktion wird der mit \p buffer angegebene verschlüsselte Speicherbereich mit einer Länge
 * von \p size Bytes entschlüsselt.
 *
 * \param buffer Pointer auf den Beginn des verschlüsselten Speicherbereiches
 * \param size Die Größe des zu entschlüsselnden Speicherbereiches in Bytes
 *
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 *
 * \note Vor Aufruf dieser Funktion muss zunächst der gewünschte Verschlüsselungsalgorithmus
 * mittels der Funktion MCrypt::setAlgorithm oder durch Verwendung des Konstruktors
 * MCrypt::MCrypt(Algorithm algo, Mode mode) ausgewählt worden sein. Ferner muss eventuell noch
 * ein Initialisierungsvektor (siehe MCrypt::NeedIV und MCrypt::SetIV) und ein Schlüssel
 * (siehe MCrypt::SetKey) angegeben worden sein.
 */
{
	ByteArrayPtr bin(buffer,size);
	decrypt(bin);
}

void MCrypt::decrypt(const ByteArrayPtr &in, ByteArray &out)
/*!\brief Entschlüsseln eines Objekts
 *
 * \descr
 * Mit dieser Funktion wird der vom CBinary-Objekt \p in verwaltete und verschlüsselte Speicherbereich
 * entschlüsselt und das Ergebnis in \p out gespeichert.
 *
 * \param[in] in Das CBinary Objekt, das den zu entschlüsselnden Speicherbereich enthält.
 * \param[out] out Ein Objekt vom Typ CBinary, in dem die entschlüsselten Daten abgelegt werden.
 *
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 *
 * \note Vor Aufruf dieser Funktion muss zunächst der gewünschte Verschlüsselungsalgorithmus
 * mittels der Funktion MCrypt::setAlgorithm oder durch Verwendung des Konstruktors
 * MCrypt::MCrypt(Algorithm algo, Mode mode) ausgewählt worden sein. Ferner muss eventuell noch
 * ein Initialisierungsvektor (siehe MCrypt::NeedIV und MCrypt::SetIV) und ein Schlüssel
 * (siehe MCrypt::SetKey) angegeben worden sein.
 */
{
	out.copy(in);
	decrypt(out);
}

void MCrypt::decrypt(ByteArrayPtr &buffer)
/*!\brief Entschlüsseln eines CBinary-Objekts
 *
 * \descr
 * Mit dieser Funktion wird der vom CBinary-Objekt \p buffer verwaltete und verschlüsselte Speicherbereich
 * entschlüsselt.
 *
 * \param[in,out] buffer Das CBinary Objekt, das den zu entschlüsselnden Speicherbereich enthält.
 *
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 *
 * \note Vor Aufruf dieser Funktion muss zunächst der gewünschte Verschlüsselungsalgorithmus
 * mittels der Funktion MCrypt::setAlgorithm oder durch Verwendung des Konstruktors
 * MCrypt::MCrypt(Algorithm algo, Mode mode) ausgewählt worden sein. Ferner muss eventuell noch
 * ein Initialisierungsvektor (siehe MCrypt::NeedIV und MCrypt::SetIV) und ein Schlüssel
 * (siehe MCrypt::SetKey) angegeben worden sein.
 */
{
#ifndef HAVE_LIBMCRYPT
	throw UnsupportedFeatureException("MCrypt");
#else
	char *myIV=NULL;
	// Wurde ein Key angegeben?
	if (Key.size()==0) throw NoKeySpecifiedException();
	// Benötigen wir IV?
	if (mcrypt_enc_mode_has_iv((MCRYPT)mcrypt)) {
		// Haben wir in IV irgendwas?
		size_t myIVSize=IV.size();
		if (myIVSize==0) throw NoIVSpecifiedException();
		int rest=mcrypt_enc_get_iv_size((MCRYPT)mcrypt);
		myIV = (char*)malloc(rest);
		if (!myIV) throw OutOfMemoryException();
		int p=0;
		while (rest) {
			int bytes=myIVSize;
			if (bytes>rest) bytes=rest;
			memcpy(myIV+p,IV.ptr(),bytes);
			p+=bytes;
			rest-=bytes;
		}
	}
    size_t keysize=mcrypt_enc_get_key_size((MCRYPT)mcrypt);
    if (Key.size()<keysize) keysize=Key.size();
    int ret=mcrypt_generic_init((MCRYPT)mcrypt,(void*)Key.ptr(), keysize, myIV);
    if (ret<0) {
    	const char *e=mcrypt_strerror(ret);
    	mcrypt_generic_deinit((MCRYPT)mcrypt);
    	if (myIV) free(myIV);
    	throw DecryptionFailedException("%s",e);
    }
    ret=mdecrypt_generic((MCRYPT)mcrypt, (void*)buffer.ptr(),buffer.size());
    if (ret<0) {
    	const char *e=mcrypt_strerror(ret);
    	mcrypt_generic_deinit((MCRYPT)mcrypt);
    	if (myIV) free(myIV);
    	throw DecryptionFailedException("%s",e);
    }
    mcrypt_generic_deinit((MCRYPT)mcrypt);
	if (myIV) free(myIV);
#endif
}

void MCrypt::crypt(ByteArrayPtr &buffer, const Variant &key, const Variant &IV, Algorithm algo, Mode mode)
/*!\ingroup PPL7_CRYPT
 * \brief Daten verschlüsseln
 *
 * \header \#include <ppl6-crypt.h>
 *
 * \descr
 * Mit dieser statischen Funktion können die Daten eines CBinary-Objekts verschlüsselt werden, ohne
 * dass dafür eine Instanz von MCrypt erstellt werden muss.
 *
 * \param[in,out] buffer Das CBinary-Objekt, mit den zu verschlüsselnden Daten. Der Inhalt des Objekts
 * wird durch die Funktion mit den verschlüsselten Daten überschrieben
 * \param[in] key Der zu verwendende Schlüssel. Dieser kann aus Text in einem CString oder CWString bestehen,
 * eine als const char* übergebenen Text mit abschließendem 0-Byte,
 * aber auch aus binären Daten in einem CBinary Objekt
 * \param[in] algo Optionaler Parameter, der den zu verwendenden Algorithmus angibt. Default ist
 * MCrypt::Algo_TWOFISH (siehe MCrypt::Algorithm)
 * \param[in] mode Optionaler Parameter, der den zu verwendenden Verschlüsselungsmodus angibt.
 * Default ist MCrypt::Mode_CFB (siehe MCrypt::Mode)
 * \param[in] IV Initialisierungsvektor (IV), den einige Verschlüsselungsalgorithmen
 * benötigen (siehe dazu auch MCrypt::setIV). Defaultmäßig wird ein statischer IV verwendet,
 * der in der Library einkompiliert ist. Dies ist allerdings nicht besonders sicher.
 * \return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 *
 * \example
 * Hier ein Beispiel, wie mit den statischen Funktionen Daten zunächst verschlüsselt und
 * dann wieder entschlüsselt werden:
 * \code
ppl6::CBinary bin="Hallo Welt";		// Das zu verschlüsselnde Objekt
ppl6::CBinary r=ppl6::Random(32);	// Ein paar Zufallsdaten für den IV
ppl6::CString key="mein schlüssel";	// Der Schlüssel
bin.HexDump();						// Inhalt des Objekts vor der Verschlüsselung
// Daten verschlüsseln
if (!ppl6::MCrypt::Crypt(bin,key,ppl6::MCrypt::Algo_TWOFISH,ppl6::MCrypt::Mode_CFB,r)) {
	ppl6::PrintError();
	return;
}
bin.HexDump();						// Inhalt des Objekts nach der Verschlüsselung
// Daten entschlüsseln
if (!ppl6::MCrypt::Decrypt(bin,key,ppl6::MCrypt::Algo_TWOFISH,ppl6::MCrypt::Mode_CFB,r)) {
	ppl6::PrintError();
	return;
}
bin.HexDump();						// Inhalt des Objekts nach der Entschlüsselung
\endcode
 */
{
	MCrypt MC;
	MC.setAlgorithm(algo,mode);
	MC.setKey(key);
	MC.setIV(IV);
	MC.crypt(buffer);
}

void MCrypt::crypt(ByteArrayPtr &buffer, const Variant &key, Algorithm algo, Mode mode)
/*!\ingroup PPL7_CRYPT
 * \brief Daten verschlüsseln
 *
 * \header \#include <ppl6-crypt.h>
 *
 * \descr
 * Mit dieser statischen Funktion können die Daten eines CBinary-Objekts verschlüsselt werden, ohne
 * dass dafür eine Instanz von MCrypt erstellt werden muss.
 *
 * \param[in,out] buffer Das CBinary-Objekt, mit den zu verschlüsselnden Daten. Der Inhalt des Objekts
 * wird durch die Funktion mit den verschlüsselten Daten überschrieben
 * \param[in] key Der zu verwendende Schlüssel. Dieser kann aus Text in einem CString oder CWString bestehen,
 * eine als const char* übergebenen Text mit abschließendem 0-Byte,
 * aber auch aus binären Daten in einem CBinary Objekt
 * \param[in] algo Optionaler Parameter, der den zu verwendenden Algorithmus angibt. Default ist
 * MCrypt::Algo_TWOFISH (siehe MCrypt::Algorithm)
 * \param[in] mode Optionaler Parameter, der den zu verwendenden Verschlüsselungsmodus angibt.
 * Default ist MCrypt::Mode_CFB (siehe MCrypt::Mode)
 *
 * \example
 * Hier ein Beispiel, wie mit den statischen Funktionen Daten zunächst verschlüsselt und
 * dann wieder entschlüsselt werden:
 * \code
ppl6::CBinary bin="Hallo Welt";		// Das zu verschlüsselnde Objekt
ppl6::CBinary r=ppl6::Random(32);	// Ein paar Zufallsdaten für den IV
ppl6::CString key="mein schlüssel";	// Der Schlüssel
bin.HexDump();						// Inhalt des Objekts vor der Verschlüsselung
// Daten verschlüsseln
if (!ppl6::MCrypt::Crypt(bin,key,ppl6::MCrypt::Algo_TWOFISH,ppl6::MCrypt::Mode_CFB,r)) {
	ppl6::PrintError();
	return;
}
bin.HexDump();						// Inhalt des Objekts nach der Verschlüsselung
// Daten entschlüsseln
if (!ppl6::MCrypt::Decrypt(bin,key,ppl6::MCrypt::Algo_TWOFISH,ppl6::MCrypt::Mode_CFB,r)) {
	ppl6::PrintError();
	return;
}
bin.HexDump();						// Inhalt des Objekts nach der Entschlüsselung
\endcode
 */
{
	MCrypt MC;
	MC.setAlgorithm(algo,mode);
	MC.setKey(key);
	MC.crypt(buffer);
}


void MCrypt::decrypt(ByteArrayPtr &buffer, const Variant &key, const Variant &IV, Algorithm algo, Mode mode)
/*!\ingroup PPL7_CRYPT
 * \brief Daten entschlüsseln
 *
 * \header \#include <ppl6-crypt.h>
 *
 * \descr
 * Mit dieser statischen Funktion können die verschlüsselten Daten eines CBinary-Objekts wieder
 * entschlüsselt werden, ohne dass dafür eine Instanz von MCrypt erstellt werden muss.
 *
 * \param[in,out] buffer Das CBinary-Objekt, mit den verschlüsselten Daten. Der Inhalt des Objekts
 * wird durch die Funktion mit den entschlüsselten Daten überschrieben
 * \param[in] key Der zu verwendende Schlüssel. Dieser kann aus Text in einem CString oder CWString bestehen,
 * eine als const char* übergebenen Text mit abschließendem 0-Byte,
 * aber auch aus binären Daten in einem CBinary Objekt
 * \param[in] algo Optionaler Parameter, der den zu verwendenden Algorithmus angibt. Default ist
 * MCrypt::Algo_TWOFISH (siehe MCrypt::Algorithm)
 * \param[in] mode Optionaler Parameter, der den zu verwendenden Verschlüsselungsmodus angibt.
 * Default ist MCrypt::Mode_CFB (siehe MCrypt::Mode)
 * \param[in] IV Initialisierungsvektor (IV), den einige Verschlüsselungsalgorithmen
 * benötigen (siehe dazu auch MCrypt::SetIV). Defaultmäßig wird ein statischer IV verwendet,
 * der in der Library einkompiliert ist. Dies ist allerdings nicht besonders sicher.
 *
 * \example
 * Hier ein Beispiel, wie mit den statischen Funktionen Daten zunächst verschlüsselt und
 * dann wieder entschlüsselt werden:
 * \code
ppl6::CBinary bin="Hallo Welt";		// Das zu verschlüsselnde Objekt
ppl6::CBinary r=ppl6::Random(32);	// Ein paar Zufallsdaten für den IV
ppl6::CString key="mein schlüssel";	// Der Schlüssel
bin.HexDump();						// Inhalt des Objekts vor der Verschlüsselung
// Daten verschlüsseln
if (!ppl6::MCrypt::Crypt(bin,key,ppl6::MCrypt::Algo_TWOFISH,ppl6::MCrypt::Mode_CFB,r)) {
	ppl6::PrintError();
	return;
}
bin.HexDump();						// Inhalt des Objekts nach der Verschlüsselung
// Daten entschlüsseln
if (!ppl6::MCrypt::Decrypt(bin,key,ppl6::MCrypt::Algo_TWOFISH,ppl6::MCrypt::Mode_CFB,r)) {
	ppl6::PrintError();
	return;
}
bin.HexDump();						// Inhalt des Objekts nach der Entschlüsselung
\endcode
 */
{
	MCrypt MC;
	MC.setAlgorithm(algo,mode);
	MC.setKey(key);
	MC.setIV(IV);
	MC.decrypt(buffer);
}

void MCrypt::decrypt(ByteArrayPtr &buffer, const Variant &key, Algorithm algo, Mode mode)
/*!\ingroup PPL7_CRYPT
 * \brief Daten entschlüsseln
 *
 * \header \#include <ppl6-crypt.h>
 *
 * \descr
 * Mit dieser statischen Funktion können die verschlüsselten Daten eines CBinary-Objekts wieder
 * entschlüsselt werden, ohne dass dafür eine Instanz von MCrypt erstellt werden muss.
 *
 * \param[in,out] buffer Das CBinary-Objekt, mit den verschlüsselten Daten. Der Inhalt des Objekts
 * wird durch die Funktion mit den entschlüsselten Daten überschrieben
 * \param[in] key Der zu verwendende Schlüssel. Dieser kann aus Text in einem CString oder CWString bestehen,
 * eine als const char* übergebenen Text mit abschließendem 0-Byte,
 * aber auch aus binären Daten in einem CBinary Objekt
 * \param[in] algo Optionaler Parameter, der den zu verwendenden Algorithmus angibt. Default ist
 * MCrypt::Algo_TWOFISH (siehe MCrypt::Algorithm)
 * \param[in] mode Optionaler Parameter, der den zu verwendenden Verschlüsselungsmodus angibt.
 * Default ist MCrypt::Mode_CFB (siehe MCrypt::Mode)
 *
 * \example
 * Hier ein Beispiel, wie mit den statischen Funktionen Daten zunächst verschlüsselt und
 * dann wieder entschlüsselt werden:
 * \code
ppl6::CBinary bin="Hallo Welt";		// Das zu verschlüsselnde Objekt
ppl6::CBinary r=ppl6::Random(32);	// Ein paar Zufallsdaten für den IV
ppl6::CString key="mein schlüssel";	// Der Schlüssel
bin.HexDump();						// Inhalt des Objekts vor der Verschlüsselung
// Daten verschlüsseln
if (!ppl6::MCrypt::Crypt(bin,key,ppl6::MCrypt::Algo_TWOFISH,ppl6::MCrypt::Mode_CFB,r)) {
	ppl6::PrintError();
	return;
}
bin.HexDump();						// Inhalt des Objekts nach der Verschlüsselung
// Daten entschlüsseln
if (!ppl6::MCrypt::Decrypt(bin,key,ppl6::MCrypt::Algo_TWOFISH,ppl6::MCrypt::Mode_CFB,r)) {
	ppl6::PrintError();
	return;
}
bin.HexDump();						// Inhalt des Objekts nach der Entschlüsselung
\endcode
 */
{
	MCrypt MC;
	MC.setAlgorithm(algo,mode);
	MC.setKey(key);
	MC.decrypt(buffer);
}



}	// EOF namespace ppl7
