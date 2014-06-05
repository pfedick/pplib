/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (ppl7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 660 $
 * $Date: 2013-05-19 11:36:13 +0200 (So, 19 Mai 2013) $
 * $Id: MHash.cpp 660 2013-05-19 09:36:13Z pafe $
 *
 *******************************************************************************
 * Copyright (c) 2010, Patrick Fedick <patrick@pfp.de>
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

#include "ppl7.h"
#include "ppl7-crypto.h"


namespace ppl7 {

/*!\class MHash
 * \ingroup PPL7_CRYPT
 * \brief Klasse zum Erstellen von Hashes nach verschiedenen Algorithmen
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Klasse können Hashes von beliebigen Daten mit verschiedenen Algorithmen, wie z.B.
 * MD5 oder SHA256 erstellt werden. Die Klasse basiert auf Funktionen der
 * <a href="http://mhash.sourceforge.net/">MHash-Library</a>, die unter der
 * GNU General Public License (GPL) steht.
 *
 * \note
 * Damit diese Klasse verwendet werden kann, muss vor Aufruf des configure-Scripts die
 * Library <a href="http://mhash.sourceforge.net/">libmhash</a> installiert sein.
 * Falls das configure die Library nicht automatisch findet, kann der Installationspfad
 * über den configure Parameter "--with-libhash-prefix=..." angegeben werden. Falls
 * libmhash nicht eingebunden wurde, liefern fast alle Funktionen einen Fehler
 * zurück und setzen den Fehlercode "527".
 * \par
 * Mit der statischen Funktion Cppl7Core::haveMHash kann zur Laufzeit
 * geprüft werden, ob libmhash eingebunden wurde.
 * \par
 * \code
 * if (ppl7::Cppl7Core::haveMHash()!=1) {
 *    printf("MHash steht nicht zur Verfügung!\n");
 *    return;
 * }
 * \endcode
 *
 * \example
 * Das folgende Beispiel zeigt, wie die Klasse verwendet werden kann:
\code
#include <ppl7.h>
#include <ppl7-crypto.h>
int mhash_demo_class()
{
	// Zuerst erstellen wir mal ein Binary-Objekt, das unsere zu hashende Daten enthält
	ppl7::CBinary data="The quick brown fox jumps over the lazy dog";
	// Nun erstellen wir unsere Klasse
	ppl7::MHash Hash;
	// Wir wollen einen SHA256 Hash
	if (!Hash.Init(ppl7::MHash::Algo_SHA256)) {
		ppl7::PrintError();
		return 0;
	}
	// Jetzt übergeben wir unsere Daten zum Hashen
	if (!Hash.AddData(data)) {
		ppl7::PrintError();
		return 0;
	}
	// Und holen uns das Ergebnis als String
	ppl7::CString result;
	if (!Hash.GetResult(result)) {
		ppl7::PrintError();
		return 0;
	}
	// Den geben wir auf stdout aus
	result.Print(true);
	return 1;
}
\endcode
Ergebnis:
\verbatim
patrick@server: ./ppltest
d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592
\endverbatim

 *
 * \example
 * Das nachfolgende Beispiel zeigt, wie die statische Funktion MHash::CRC32B verwendet
 * wird, um eine CRC-Prüfsumme zu berechnen:
\code
#include <ppl7.h>
#include <ppl7-crypto.h>
int mhash_demo_static_crc32()
{
	// Zuerst erstellen wir ein Binary-Objekt, für das wir die Prüfsumme berechnen wollen
	ppl7::CBinary data="The quick brown fox jumps over the lazy dog";
	// Das Ergebnis wollen wir als 32-Bit Integer
	ppl7::CInt result;
	if (!ppl7::MHash::CRC32B(data,result)) {	// MD5-Hashfunktion aufrufen
		ppl7::PrintError();
		return 0;
	}
	// Jetzt das Ergebnis ausgeben
	printf ("Die CRC32-Prüfsumme lautet: %u (0x%X)\n",(unsigned int)result, (unsigned int)result);
	return 1;
}
\endcode
Ergebnis:
\verbatim
patrick@server: ./ppltest
Die CRC32-Prüfsumme lautet: 1095738169 (0x414FA339)
\endverbatim

 * \example
 * Und zuletzt noch ein Beispiel mit MHash::MD5:
\code
#include <ppl7.h>
#include <ppl7-crypto.h>
int mhash_demo_static_md5()
{
	// Zuerst erstellen wir mal ein Binary-Objekt, das unsere zu hashende Daten enthält
	ppl7::CBinary data="The quick brown fox jumps over the lazy dog";
	// Das Ergebnis wollen wir als Hexadezimalwert in einem String
	ppl7::CString result;
	if (!ppl7::MHash::MD5(data,result)) {	// MD5-Hashfunktion aufrufen
		ppl7::PrintError();
		return 0;
	}
	result.Print(true);	// Ergebnis ausgeben
	return 1;
}
\endcode
Ergebnis:
\verbatim
patrick@server: ./ppltest
9e107d9d372bb6826bd81d3542a419d6
\endverbatim

 */

/*!\enum MHash::Algorithm
 * \brief Unterstütze Algorithmen
 *
 * Die Klasse unterstützt folgende Algorithmen zu Erzeugen der Hashes:
 */

/*!\var MHash::Algorithm MHash::Algo_CRC32
 * Die zyklische Redundanzprüfung (engl. cyclic redundancy check, daher meist CRC)
 * ist ein Verfahren (bzw. eine bestimmte Klasse von Verfahren) aus der
 * Informationstechnik zur Bestimmung eines Prüfwerts für Daten (z. B.
 * Datenübertragung in Rechnernetzen oder eine Datei), um Fehler bei der Übertragung
 * oder Duplizierung von Daten erkennen zu können. Diese Klasse unterstützt zwei
 * Varianten des Algorithmus: MHash::Algo_CRC32 (wird im Netzwerkbereich verwendet)
 * und MHash::Algo_CRC32B (wird z.B. in ZIP Programmen verwendet).
 */

/*!\var MHash::Algorithm MHash::Algo_CRC32B
 * \copydoc MHash::Algo_CRC32
 * Dies ist auch der Algorithmus, der in ppl7::crc32 und ppl7::CSocketMessage
 * verwendet wird.
 */

/*!\var MHash::Algorithm MHash::Algo_ADLER32
 * Adler-32 ist ein einfacher, von Mark Adler entwickelter Prüfsummenalgorithmus.
 * Er wird unter anderem von der zlib-Bibliothek benutzt, um (zufällige Übertragungs-)Fehler
 * im komprimierten Datenstrom zu erkennen. In RFC 1950 wird der Algorithmus genau beschrieben.
 *
 * Der Adler-32-Algorithmus ist einfacher und lässt sich schneller berechnen als
 * die bekannte Zyklische Redundanzprüfung (siehe MHash::Algo_CRC32), bietet aber auch
 * weniger Sicherheit beim Erkennen von zufälligen Bitfehlern.
 */

/*!\var MHash::Algorithm MHash::Algo_MD2
 * Message-Digest Algorithm 2 (MD2) ist eine von Ronald L. Rivest im Jahr 1988
 * veröffentlichte Hash-Funktion. Der Algorithmus wurde für 8-Bit Rechner optimiert.
 * MD2 ist in RFC 1319 spezifiziert. Bereits 1997 wurden erste Kollisionsmöglichkeiten
 * im Algorithmus entdeckt und seit 2004 gilt er offiziell als unsicher.
 */

/*!\var MHash::Algorithm MHash::Algo_MD5
 * MD5 (Message-Digest Algorithm 5) ist eine weit verbreitete kryptographische
 * Hashfunktion, die einen 128-Bit-Hashwert erzeugt. MD5 wurde 1991 von
 * Ronald L. Rivest entwickelt. Die errechneten MD5-Summen (kurz md5sum) werden
 * zum Beispiel zur Integritätsprüfung von Dateien eingesetzt.
 * Inzwischen ist es möglich anhand der Quelldaten Kollisionen zu finden, daher
 * gilt MD5 inzwischen nicht mehr als 100% sicher. Ist nur der Hash-Wert
 * bekannt, ist es nach wie vor schwierig.
 */

/*!\var MHash::Algorithm MHash::Algo_MD4
 * MD4 (engl. Message-Digest Algorithm 4) wurde 1990 von Ronald L. Rivest veröffentlicht.
 * Der MD4 Hash-Algorithmus wurde mit dem Anspruch entwickelt, auf 32 Bit-Rechnern
 * besonders schnell zu laufen und gleichzeitig in der Implementierung einfach zu sein.
 * Dabei sollten natürlich die grundlegenden Anforderungen an Hash-Funktionen erhalten
 * bleiben. MD4 erzeugt einen Hashwert mit einer Länge von 128 Bit.
 *
 * Trotz aller Sorgfalt im Design zeigte sich bald, dass das Verfahren unsicher ist.
 * Als besonders problematisch stellte sich die mangelnde Kollisionsbeständigkeit heraus.
 * Im Cryptobytes Journal der Firma RSA wurde eine Methode veröffentlicht, welche
 * innerhalb einer Stunde zwei bis auf ein Zeichen identische Nachrichten erzeugen
 * konnte, die denselben Hashwert ergaben. Rivest selbst bestätigte die Unsicherheit
 * im RFC 1321, so dass selbst RSA vom Einsatz dieses Message-Digest abrät. MD4
 * wurde als Public Domain lizenziert, worauf wohl zurückzuführen ist, dass das
 * verwendete Prinzip zur Basis weiterer Hash-Funktionen geworden ist.
 */

/*!\var MHash::Algorithm MHash::Algo_SHA1
 * SHA1 wurde vom Amerikanischen NIST (National Institute of Standards and Technology) zusammen
 * mit der NSA (National Security Agency) 1995 entwickelt und hjat einen Hash-Wert von
 * 160 Bit Länge für beliebige digitale Daten von maximal \f$ 2^{64}−1\f$ Bit (≈ 2 Exbibyte)
 * Länge. Der Algorithmus ähnelt im Aufbau dem von Ronald L. Rivest entwickelten MD4.
 * Im Sommer 2006 wurde eine wesentliche Schwäche dieses Algorithmus entdeckt und publik
 * gemacht. Grundsätzlich sollte SHA-1 daher nicht mehr in neuen Entwicklungen als sicherer
 * Hash-Algorithmus vorgesehen werden.
 */

/*!\var MHash::Algorithm MHash::Algo_SHA256
 * Das NIST (National Institute of Standards and Technology) hat im August 2002 drei
 * weitere Varianten des Algorithmus veröffentlicht, die größere Hash-Werte als SHA1
 * erzeugen. Es handelt sich dabei um den SHA-256, SHA-384 und SHA-512 wobei die
 * angefügte Zahl jeweils die Länge des Hash-Werts (in Bit) angibt.
 * Mit SHA-384 und SHA-512 können (theoretisch) Daten bis zu einer Größe von
 * \f$ 2^{128} \f$ Bit verarbeitet werden. In der Praxis sind Dateien mit mehr
 * als \f$ 2^{64} \f$ Bit jedoch unrealistisch. Im Februar 2004 wurde eine weitere
 * Version, SHA-224, veröffentlicht. Die 4 Varianten werden zusammenfassend als
 * SHA-2 bezeichnet.
 */

/*!\var MHash::Algorithm MHash::Algo_SHA224
 * \copydoc MHash::Algo_SHA256
 */

/*!\var MHash::Algorithm MHash::Algo_SHA384
 * \copydoc MHash::Algo_SHA256
 */

/*!\var MHash::Algorithm MHash::Algo_SHA512
 * \copydoc MHash::Algo_SHA256
 */

/*!\var MHash::Algorithm MHash::Algo_WHIRLPOOL
 * WHIRLPOOL ist eine kryptologische Hash-Funktion, die von Vincent Rijmen und
 * Paulo S. L. M. Barreto entworfen wurde. Sie wurde nach der Whirlpool-Galaxie
 * im Sternbild der Jagdhunde benannt. Whirlpool funktioniert mit Dateien bis zu
 * \f$ 2^{256}\f$ Bit Größe und gibt einen Hash-Wert von 512 Bit aus. Bislang sind
 * keine Schwächen des Algorithmus bekannt, was allerdings relativiert werden muss,
 * da der Algorithmus noch sehr jung ist und bislang wenig untersucht wurde.
 * Die Autoren haben erklärt, dass Whirlpool nicht patentiert ist und es nie sein wird.
 * Whirlpool darf kostenlos zu jedem Zweck verwendet werden.
 *
 * Whirlpool gehört zu den vom Projekt NESSIE empfohlenen kryptografischen
 * Algorithmen und wurde von der ISO mit ISO/IEC 10118-3:2004 standardisiert.
 * (Quelle: http://de.wikipedia.org/wiki/Whirlpool_(Algorithmus) )
 */

/*!\var MHash::Algorithm MHash::Algo_HAVAL256
 * HAVAL ist eine kryptologische Hash-Funktion. Im Gegensatz zu MD5 ist es mit HAVAL,
 * wie mit anderen moderneren Algorithmen, möglich, Hashes variabler Länge zu erzeugen,
 * vor allem von 128 Bit, 160 Bit, 192 Bit, 224 Bit und 256 Bit. HAVAL erlaubt
 * außerdem eine variable Anzahl von Runden (3, 4, oder 5). HAVAL wurde von
 * Yuliang Zheng, Josef Pieprzyk, und Jennifer Seberry im Jahr 1992 erfunden.
 * Die MHash-Klasse unterstützt alle genannten Hash-Längen:
 * Algo_HAVAL256, Algo_HAVAL224, Algo_HAVAL192, Algo_HAVAL160 and Algo_HAVAL128
 */

/*!\var MHash::Algorithm MHash::Algo_HAVAL224
 * \copydoc MHash::Algo_HAVAL256
 */

/*!\var MHash::Algorithm MHash::Algo_HAVAL192
 * \copydoc MHash::Algo_HAVAL256
 */

/*!\var MHash::Algorithm MHash::Algo_HAVAL160
 * \copydoc MHash::Algo_HAVAL256
 */

/*!\var MHash::Algorithm MHash::Algo_HAVAL128
 * \copydoc MHash::Algo_HAVAL256
 */

/*!\var MHash::Algorithm MHash::Algo_RIPEMD160
 * RIPEMD-160 ist eine kryptologische Hash-Funktion und wurde von Hans Dobbertin,
 * Antoon Bosselaers und Bart Preneel in Europa entwickelt und 1996 erstmals publiziert.
 * Es handelt sich dabei um eine verbesserte Version von RIPEMD, welcher wiederum
 * auf den Designprinzipien von MD4 basiert und in Hinsicht auf seine Stärke und
 * Performance dem populäreren SHA-1 gleicht.
 *
 * Es existieren auch 128-, 256- und 320-Bit-Versionen dieses Algorithmus
 * (Algo_RIPEMD128, Algo_RIPEMD256, Algo_RIPEMD320). Die 128-Bit-Version war ursprünglich
 * als Ersatz für den RIPEMD gedacht, der ebenfalls 128 Bit stark war, aber eine
 * fragwürdige Sicherheit bot. Die 256- und 320-Bit-Versionen reduzieren lediglich
 * die Wahrscheinlichkeit von Hash-Wert-Kollisionen, bieten aber keine höhere Sicherheit
 * als RIPEMD-128 oder RIPEMD-160.
 *
 * Da die Entwicklung von RIPEMD-160 offener war als die von SHA-1, ist es wahrscheinlicher,
 * dass dieser Algorithmus weniger Sicherheitslücken aufweist. Da er jedoch weniger
 * populär ist, haben weniger Kryptologen versucht, Schwächen zu finden, was wiederum
 * die Wahrscheinlichkeit für unentdeckte Sicherheitslücken steigen lässt.
 */

/*!\var MHash::Algorithm MHash::Algo_RIPEMD128
 * \copydoc MHash::Algo_RIPEMD160
 */

/*!\var MHash::Algorithm MHash::Algo_RIPEMD256
 * \copydoc MHash::Algo_RIPEMD160
 */

/*!\var MHash::Algorithm MHash::Algo_RIPEMD320
 * \copydoc MHash::Algo_RIPEMD160
 */

/*!\var MHash::Algorithm MHash::Algo_TIGER128
 * Tiger ist eine schneller kryptografische Hashfunktion die von Ross Anderson und Eli
 * Biham im Jahr 1996 entwickelt wurde. Der von Tiger erzeugte Hashwert kann Längen
 * von 128, 160 oder 192 Bit haben. Der Tiger-Algorithmus ist nicht patentiert.
 * Die MHash-Klasse unterstützt alle drei Bittiefen:
 * Algo_TIGER128, Algo_TIGER160, Algo_TIGER192
 */

/*!\var MHash::Algorithm MHash::Algo_TIGER160
 * \copydoc MHash::Algo_TIGER128
 */

/*!\var MHash::Algorithm MHash::Algo_TIGER192
 * \copydoc MHash::Algo_TIGER128
 */

/*!\var MHash::Algorithm MHash::Algo_GOST
 * GOST ist ein in den 1970er Jahren von der Sowjetische bzw. Russischen Regierung entwickelter Hash
 * Algorithmus, definiert im Standard "GOST 28147-89". Er galt zunächst als "Top Secret", wurde
 * aber nach dem Ende der UdSSR im Jahr 1990 für die Öffentlichkeit freigegeben. Er war
 * als alternative zum amerikanischen DES Algorithmus entwickelt worden, weshalb er diesem
 * sehr ähnlich ist.
 */

/*!\var MHash::Algorithm MHash::Algo_SNEFRU128
 * Snefru (benannt nach dem ägyptischen Pharao Sneferu) ist eine von Ralph Merkle
 * entwickelte kryptologische Hash-Funktion, die für beliebig lange Nachrichten einen
 * Hash-Wert von 128 bzw. 256 Bit Länge berechnet. Eli Biham und Adi Shamir konnten mit
 * Hilfe der differentiellen Kryptoanalyse die Unsicherheit des ursprünglichen Designs
 * von Snefru zeigen, in dem sie Nachrichtenpaare mit gleichem Hash-Wert innerhalb
 * von Minuten fanden.
 *
 * Das Design wurde daraufhin so geändert, dass die Zahl der Wiederholungen des
 * Hauptdurchlaufs des Algorithmus erhöht wurde. Aus Sicherheitsgründen wird der Einsatz
 * von Snefru mit acht Durchläufen empfohlen. Allerdings ist der Algorithmus dann
 * wesentlich langsamer als andere gängige Hash-Verfahren.
 */

/*!\var MHash::Algorithm MHash::Algo_SNEFRU256
 * \copydoc MHash::Algo_SNEFRU128
 */

MHash::MHash()
/*!\brief Konstruktor der Klasse
 *
 * \descr
 * Bei Verwendung dieses Konstruktors werden nur einige interne Variablen initialisert.
 * Der gewünschte Hash-Algorithmus muss mittels der Funktion MHash::Init
 * festgelegt werden.
 */
{
	handle=NULL;
	blocksize=0;
	bytesHashed=0;
}

MHash::MHash(Algorithm algorithm)
/*!\brief Konstruktor mit Initialisierung des Algorithmus
 *
 * \descr
 * Bei Verwendung dieses Konstruktors wird die Klasse gleichzeitig mit einem bestimmten
 * Hash-Algorithmus initialisiert, so dass die Funktion MHash::Init nicht mehr aufgerufen
 * werden muss.
 *
 * @param method Der gewünschte Algorithmus (siehe MHash::Algorithm)
 *
 * \note Auch wenn dieser Konstruktor verwendet wurde, kann jederzeit mit der Funktion
 * MHash::init ein anderer Hash-Algorithmus ausgewählt werden.
 */
{
	handle=NULL;
	blocksize=0;
	bytesHashed=0;
	init(algorithm);
}

MHash::~MHash()
/*!\brief Destruktor der Klasse
 *
 * \descr
 * Der Destruktor gibt den intern reservierten Speicher wieder frei.
 */
{
#ifdef HAVE_LIBMHASH
	if (handle) mhash_deinit((MHASH) handle, NULL);
#endif
}

void MHash::init(Algorithm algorithm)
/*!\brief Auswahl des gewünschten Hash-Algorithmus
 *
 * \descr
 * Mit dieser Funktion wird der gewünschte Hash-Algorithmus ausgewählt.
 *
 * @param method Der gewünschte Algorithmus (siehe MHash::Algorithm).
 *
 * \return Bei Erfolg liefert die Funktion 1 zurück, sonst 0. Falls die libmhash
 * bei der Erstellung der Library nicht eingebunden wurde, liefert die Funktion
 * ebenfalls 0 zurück und setzt den Fehlercode 527.
 *
 */
{
#ifndef HAVE_LIBMHASH
	throw UnsupportedFeatureException("MHash");
#else
	if (handle) mhash_deinit((MHASH) handle, NULL);
	handle=NULL;
	blocksize=0;
	bytesHashed=0;
	hashid type=MHASH_CRC32;
	switch (algorithm) {
		case Algo_CRC32: type=MHASH_CRC32; break;
		case Algo_CRC32B: type=MHASH_CRC32B; break;
		case Algo_ADLER32: type=MHASH_ADLER32; break;
		case Algo_MD2: type=MHASH_MD2; break;
		case Algo_MD4: type=MHASH_MD4; break;
		case Algo_MD5: type=MHASH_MD5; break;
		case Algo_SHA1: type=MHASH_SHA1; break;
		case Algo_SHA224: type=MHASH_SHA224; break;
		case Algo_SHA256: type=MHASH_SHA256; break;
		case Algo_SHA384: type=MHASH_SHA384; break;
		case Algo_SHA512: type=MHASH_SHA512; break;
		case Algo_WHIRLPOOL: type=MHASH_WHIRLPOOL; break;
		case Algo_HAVAL256: type=MHASH_HAVAL256; break;
		case Algo_HAVAL224: type=MHASH_HAVAL224; break;
		case Algo_HAVAL192: type=MHASH_HAVAL192; break;
		case Algo_HAVAL160: type=MHASH_HAVAL160; break;
		case Algo_HAVAL128: type=MHASH_HAVAL128; break;
		case Algo_RIPEMD128: type=MHASH_RIPEMD128; break;
		case Algo_RIPEMD160: type=MHASH_RIPEMD160; break;
		case Algo_RIPEMD256: type=MHASH_RIPEMD256; break;
		case Algo_RIPEMD320: type=MHASH_RIPEMD320; break;
		case Algo_TIGER128: type=MHASH_TIGER128; break;
		case Algo_TIGER160: type=MHASH_TIGER160; break;
		case Algo_TIGER192: type=MHASH_TIGER192; break;
		case Algo_GOST: type=MHASH_GOST; break;
		case Algo_SNEFRU128: type=MHASH_SNEFRU128; break;
		case Algo_SNEFRU256: type=MHASH_SNEFRU256; break;
		default:
			throw UnknownAlgorithmException("%i",algorithm);
	}
	handle=mhash_init(type);
	if (handle!=MHASH_FAILED) {
		blocksize=mhash_get_block_size(type);
		algo=algorithm;
		return;
	}
	throw UnsupportedAlgorithmException("%i",algorithm);

#endif
}


MHash::Algorithm MHash::getAlgorithmFromName(const String &name)
{
	String a=name;
	a.upperCase();
	if (a=="CRC32") return Algo_CRC32;
	if (a=="CRC32B") return Algo_CRC32B;
	if (a=="ADLER32") return Algo_ADLER32;
	if (a=="MD2") return Algo_MD2;
	if (a=="MD4") return Algo_MD4;
	if (a=="MD5") return Algo_MD5;
	if (a=="SHA1") return Algo_SHA1;
	if (a=="SHA224") return Algo_SHA224;
	if (a=="SHA256") return Algo_SHA256;
	if (a=="SHA384") return Algo_SHA384;
	if (a=="SHA512") return Algo_SHA512;
	if (a=="WHIRLPOOL") return Algo_WHIRLPOOL;
	if (a=="HAVAL256") return Algo_HAVAL256;
	if (a=="HAVAL224") return Algo_HAVAL224;
	if (a=="HAVAL192") return Algo_HAVAL192;
	if (a=="HAVAL160") return Algo_HAVAL160;
	if (a=="HAVAL128") return Algo_HAVAL128;
	if (a=="RIPEMD128") return Algo_RIPEMD128;
	if (a=="RIPEMD160") return Algo_RIPEMD160;
	if (a=="RIPEMD256") return Algo_RIPEMD256;
	if (a=="RIPEMD320") return Algo_RIPEMD320;
	if (a=="TIGER128") return Algo_TIGER128;
	if (a=="TIGER160") return Algo_TIGER160;
	if (a=="TIGER192") return Algo_TIGER192;
	if (a=="GOST") return Algo_GOST;
	if (a=="SNEFRU128") return Algo_SNEFRU128;
	if (a=="SNEFRU256") return Algo_SNEFRU256;
	throw UnknownAlgorithmException("%s",(const char*)name);
}


void MHash::reset()
/*!\brief Interne Datenstrukturen zurücksetzen
 *
 * \descr
 * Falls bereits Daten mit MHash::AddData zur Berechnung an die Klasse übergeben wurden,
 * können die internen Datenstrukturen mit dieser Funktion wieder in den Anfangszustand
 * zurückversetzt werden.
 */
{
	init(algo);
}

int MHash::getBlockSize() const
/*!\brief Größe des gewählten Hash-Algorithmus
 *
 * \descr
 * Diese Funktion liefert die Länge des gewählten Hash-Algorithmus in Bytes zurück.
 * Die Funktion liefert nur einen brauchbaren Wert, wenn zuvor MHash::Init aufgerufen
 * oder der Konstruktor MHash(Algorithm method) verwendet wurde.
 *
 * @return Bei Erfolg liefert die Funktion die Länge des Algorithmus zurück, im
 * Fehlerfall -1.
 */
{
#ifndef HAVE_LIBMHASH
	throw UnsupportedFeatureException("MHash");
#else
	if (!handle) {
		throw MissingInitializationException("MHash::GetBlockSize");
	}
	return blocksize;
#endif
}

void MHash::getResult(Variant &result)
/*!\brief Ergebnis auslesen
 *
 * \descr
 * Durch Aufruf dieser Funktion wird der bis dahin berechnete Hash-Wert ausgelesen und im
 * Objekt \p result gespeichert. Gleichzeitig werden die internen Datenstrukturen reinitialisiert,
 * so dass ein weiterer Aufruf dieser Funktion fehlschlagen wird. Es kann jedoch sofort
 * ein neuer Hash-Wert berechnet werden.
 * \par
 * Das Ergebnis kann auf verschiedene Arten zurückgegeben werden und hängt vom Typ des \p result
 * Objekts ab. Handelt es sich um ein CBinary, wird das Ergebnis im Binärformat übergeben und
 * hat exakt die Länge des Algorithmus. Wird CString oder CWString verwendet, wird das
 * Ergebnis als Hexadezimalwerte in lesbarer Form zurückgegeben. Bei den Algorithmen, die
 * einen 32-Bit Wert zurückliefern (z.B. Algo_CRC32), kann auch CInt verwendet werden.
 *
 * @param[out] result Ein von CVar abgeleitetes Objekt, in dem das Ergebnis abgelegt werden
 * soll. Unterstützt werden CBinary, CString, CWString und CInt. Der vorherige Inhalt
 * des Objekts wird überschrieben.
 *
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
{
#ifndef HAVE_LIBMHASH
	throw UnsupportedFeatureException("MHash");
#else
	if (!handle) {
		throw MissingInitializationException("MHash::GetBlockSize");
	}
	if (bytesHashed==0) {
		throw EmptyDataException();
	}
	// Wir brauchen einen Buffer für das Ergebnis
	unsigned char *buffer=(unsigned char*) malloc(blocksize);
	if (!buffer) {
		throw OutOfMemoryException();
	}
	mhash_deinit((MHASH)handle, buffer);
	handle=NULL;
	// Bereit machen, für nächste Hash-Berechnung
	init(algo);
	// Nun schauen wir mal, worin wir das Ergebnis speichern sollen
	int type=result.dataType();
	if (type==Variant::BYTEARRAY) {
		ByteArray &bin=static_cast<ByteArray&>(result);
		bin.clear();
		bin.copy(buffer,blocksize);
	} else if (type==Variant::STRING) {
		String &str=static_cast<String&>(result);
		str.clear();
		for(int i=0;i<blocksize;i++) str.appendf("%02x",buffer[i]);
	} else if (type==Variant::WIDESTRING) {
		WideString &str=static_cast<WideString&>(result);
		str.clear();
		for(int i=0;i<blocksize;i++) str.appendf("%02x",buffer[i]);
	} else {
		free(buffer);
		throw UnsupportedDataTypeException();
	}
	free(buffer);
#endif
}

int MHash::getIntResult()
/*!\brief Ergebnis einer CRC32-Berechnung oder Adler32-Berechnung
 *
 * @return
 */
{
#ifndef HAVE_LIBMHASH
	throw UnsupportedFeatureException("MHash");
#else
	if (!handle) {
		throw MissingInitializationException("MHash::GetBlockSize");
	}
	if (bytesHashed==0) {
		throw EmptyDataException();
	}
	if (blocksize!=4) {
		throw InvalidBlocksizeException("%i != 4",blocksize);
	}
	// Wir brauchen einen Buffer für das Ergebnis
	unsigned char *buffer=(unsigned char*) malloc(blocksize);
	if (!buffer) {
		throw OutOfMemoryException();
	}
	mhash_deinit((MHASH)handle, buffer);
	handle=NULL;
	// Bereit machen, für nächste Hash-Berechnung
	init(algo);
	int i=*((int*)buffer);
	free(buffer);
	return i;
#endif
}


void MHash::addData(const void *data, size_t size)
/*!\brief Daten zur Berechnung hinzufügen
 *
 * \descr
 * Mit dieser Funktion werden die Daten übergeben, aus denen der Hash-Wert berechnet werden
 * soll. Die Funktion kann beliebig oft aufgerufen werden, jeder neue Datenblock wird dem
 * Hash-Wert hinzugefügt.
 *
 * @param data Pointer auf den Speicherbereich mit den zu berechnenden Daten
 * @param size Größe des Speicherbereichs in Bytes
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
{
#ifndef HAVE_LIBMHASH
	throw UnsupportedFeatureException("MHash");
#else
	if (!handle) {
		throw MissingInitializationException("MHash::GetBlockSize");
	}
	if (mhash((MHASH)handle, data, size)) {
		throw HashFailedException();
	}
	bytesHashed+=size;
#endif
}

void MHash::addData(const Variant &data)
/*!\brief Daten-Objekt zur Berechnung hinzufügen
 *
 * \descr
 * Mit dieser Funktion wird der Inhalt eines von CVar-Abgeleiteten Objekts zur Berechnung
 * des Hash-Wertes übergeben. Die Funktion kann beliebig oft aufgerufen werden, jeder
 * neue Datenblock wird dem Hash-Wert hinzugefügt.
 *
 * @param data Ein von CVar abgeleitetes Objekt. Derzeit werden CString, CWString und CBinary
 * unterstützt.
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
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


void MHash::addFile(const String &filename)
/*!\brief Hash-Wert einer Datei berechnen
 *
 * \descr
 * Mit dieser Funktion wird der Hash-Wert des Inhaltes einer kompletten Datei berechnet.
 * Dazu wird diese in 10 MB große Blöcke aufgeteilt und nach und nach eingelesen.
 * Die Funktion kann beliebig oft aufgerufen werden, jeder
 * neue Datenblock wird dem Hash-Wert hinzugefügt.
 *
 * @param filename String mit dem Dateinamen.
 */
{
	File ff;
	ff.open(filename,File::READ);
	addData(ff);
}

void MHash::addData(FileObject &file)
/*!\brief Hash-Wert einer geöffneten Datei berechnen
 *
 * \descr
 * Mit dieser Funktion wird der Hash-Wert des Inhaltes einer kompletten Datei berechnet.
 * Dazu wird diese in 10 MB große Blöcke aufgeteilt und nach und nach eingelesen.
 * Die Funktion kann beliebig oft aufgerufen werden, jeder
 * neue Datenblock wird dem Hash-Wert hinzugefügt.
 *
 * @param file Referenz auf ein von CFileObject abgeleitetes Objekt
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
{
#ifndef HAVE_LIBMHASH
	throw UnsupportedFeatureException("MHash");
#else
	if (!handle) {
		throw MissingInitializationException("MHash::GetBlockSize");
	}
	file.seek(0);
	size_t bsize=1024*1024*10;		// We allocate 10 MB maximum
	ppluint64 fsize=file.size();
	if (fsize<bsize) bsize=fsize;	// or filesize if file is < 10 MB
	void *buffer=malloc(bsize);
	if (!buffer) {
		throw OutOfMemoryException();
	}
	ppluint64 rest=fsize;
	size_t bytes;
	try {
		while(rest) {
			bytes=rest;
			if (bytes>bsize) bytes=bsize;
			if (!file.read(buffer,bytes)) {
				throw ReadException();
			}
			if (mhash((MHASH)handle, buffer, bytes)) {
				throw HashFailedException();
			}
			bytesHashed+=bytes;
			rest-=bytes;
		}
	} catch (...) {
		free(buffer);
		throw;
	}
	free(buffer);
#endif
}

void MHash::hash(const Variant &data, Variant &result, Algorithm algorithm)
/*!\ingroup PPL7_CRYPT
 * \brief Hash-Wert mit einem beliebigen Algorithmus berechnen.
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein Hash-Wert für die Daten \p data mit dem Algorithmus
 * \p algo berechnet und das Ergebnis in \p result gespeichert.
 * \par
 * Handelt es sich bei \p result um ein CBinary, wird das Ergebnis im Binärformat übergeben und
 * hat exakt die Länge des Algorithmus (siehe MHash::GetBlockSize). Wird CString oder CWString
 * verwendet, wird das Ergebnis als Hexadezimalwerte in lesbarer Form zurückgegeben.
 * Bei den Algorithmen, die einen 32-Bit Wert zurückliefern (z.B. Algo_CRC32), kann
 * auch CInt übergeben werden.
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @param[out] result Objekt, in dem das Ergebnis gespeichert werden soll. Auch hier kann es
 * sich um ein CBinary, CString oder CWString handeln, bei Algorithmen, die einen 32-Bit-Wert
 * zurückgeben (z.B. Algo_CRC32) auch CInt.
 * @param[in] algo Der zu verwendende Algorithmus (siehe MHash::Algorithm)
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	MHash MH(algorithm);
	MH.addData(data);
	MH.getResult(result);
}

/*!\ingroup PPL7_CRYPT
 * \brief Hash-Wert mit einem beliebigen Algorithmus berechnen.
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein Hash-Wert für die Daten \p data mit dem Algorithmus
 * \p algo berechnet und das Ergebnis in \p result gespeichert.
 * \par
 * Handelt es sich bei \p result um ein CBinary, wird das Ergebnis im Binärformat übergeben und
 * hat exakt die Länge des Algorithmus (siehe MHash::GetBlockSize). Wird CString oder CWString
 * verwendet, wird das Ergebnis als Hexadezimalwerte in lesbarer Form zurückgegeben.
 * Bei den Algorithmen, die einen 32-Bit Wert zurückliefern (z.B. Algo_CRC32), kann
 * auch CInt übergeben werden.
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @param[out] result Objekt, in dem das Ergebnis gespeichert werden soll. Auch hier kann es
 * sich um ein CBinary, CString oder CWString handeln, bei Algorithmen, die einen 32-Bit-Wert
 * zurückgeben (z.B. Algo_CRC32) auch CInt.
 * @param[in] algo Der zu verwendende Algorithmus als String
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
void MHash::hash(const Variant &data, Variant &result, const String &algo)
{
	MHash MH(MHash::getAlgorithmFromName(algo));
	MH.addData(data);
	MH.getResult(result);
}

#ifdef DONE

int MHash::CRC32(const CVar &data, CVar &result)
/*!\ingroup PPL7_CRYPT
 * \brief CRC32-Summe berechnen, wie sie im Netzwerkbereich verwendet wird
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein CRC32-Prüfwert für die Daten \p data berechnet und das
 * Ergebnis in \p result gespeichert.
 * \par
 * Handelt es sich bei \p result um ein CBinary, wird das Ergebnis im Binärformat übergeben und
 * hat exakt die Länge des Algorithmus (siehe MHash::GetBlockSize). Wird CString oder CWString
 * verwendet, wird das Ergebnis als Hexadezimalwerte in lesbarer Form zurückgegeben.
 * Bei diesem Algorithmus kann auch CInt als \p result verwendet werden.
 * \par
 * \copydoc Algo_CRC32
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @param[out] result Objekt, in dem das Ergebnis gespeichert werden soll. Hier kann es
 * sich um ein CBinary, CString, CWString oder CInt handeln.
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	return Hash(data,result,Algo_CRC32);
}

int MHash::CRC32(const CVar &data)
/*!\ingroup PPL7_CRYPT
 * \brief CRC32-Summe berechnen, wie sie im Netzwerkbereich verwendet wird
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein CRC32-Prüfwert für die Daten \p data berechnet und das
 * Ergebnis als 32-Bit Integer zurückgegeben.
 * \par
 * \copydoc Algo_CRC32
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @return Bei Erfolg liefert die Funktion die Prüfsumme zurück, im Fehlerfall 0.
 */
{
	CInt result;
	if (!Hash(data,result,Algo_CRC32)) return 0;
	return result;
}


int MHash::CRC32B(const CVar &data, CVar &result)
/*!\ingroup PPL7_CRYPT
 * \brief CRC32-Summe berechnen, wie sie in ZIP-Programmen verwendet wird
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein CRC32-Prüfwert für die Daten \p data berechnet und das
 * Ergebnis in \p result gespeichert.
 * \par
 * Handelt es sich bei \p result um ein CBinary, wird das Ergebnis im Binärformat übergeben und
 * hat exakt die Länge des Algorithmus (siehe MHash::GetBlockSize). Wird CString oder CWString
 * verwendet, wird das Ergebnis als Hexadezimalwerte in lesbarer Form zurückgegeben.
 * Bei diesem Algorithmus kann auch CInt als \p result verwendet werden.
 * \par
 * \copydoc Algo_CRC32B
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @param[out] result Objekt, in dem das Ergebnis gespeichert werden soll. Hier kann es
 * sich um ein CBinary, CString, CWString oder CInt handeln.
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	return Hash(data,result,Algo_CRC32B);
}

int MHash::CRC32B(const CVar &data)
/*!\ingroup PPL7_CRYPT
 * \brief CRC32-Summe berechnen, wie sie in ZIP-Programmen verwendet wird
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein CRC32-Prüfwert für die Daten \p data berechnet und das
 * Ergebnis als 32-Bit Integer zurückgegeben.
 * \par
 * \copydoc Algo_CRC32B
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @return Bei Erfolg liefert die Funktion die Prüfsumme zurück, im Fehlerfall 0.
 */
{
	CInt result;
	if (!Hash(data,result,Algo_CRC32B)) return 0;
	return result;
}

int MHash::ADLER32(const CVar &data, CVar &result)
/*!\ingroup PPL7_CRYPT
 * \brief ADLER32 berechnen
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein ADLER32-Prüfwert für die Daten \p data berechnet und das
 * Ergebnis in \p result gespeichert.
 * \par
 * Handelt es sich bei \p result um ein CBinary, wird das Ergebnis im Binärformat übergeben und
 * hat exakt die Länge des Algorithmus (siehe MHash::GetBlockSize). Wird CString oder CWString
 * verwendet, wird das Ergebnis als Hexadezimalwerte in lesbarer Form zurückgegeben.
 * Bei diesem Algorithmus kann auch CInt als \p result verwendet werden.
 * \par
 * \copydoc Algo_ADLER32
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @param[out] result Objekt, in dem das Ergebnis gespeichert werden soll. Hier kann es
 * sich um ein CBinary, CString, CWString oder CInt handeln.
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	return Hash(data,result,Algo_ADLER32);
}

int MHash::ADLER32(const CVar &data)
/*!\ingroup PPL7_CRYPT
 * \brief ADLER32 berechnen
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein ADLER32-Prüfwert für die Daten \p data berechnet und das
 * Ergebnis als 32-Bit Integer zurückgegeben.
 * \par
 * \copydoc Algo_ADLER32
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @return Bei Erfolg liefert die Funktion die Prüfsumme zurück, im Fehlerfall 0.
 */
{
	CInt result;
	if (!Hash(data,result,Algo_ADLER32)) return 0;
	return result;
}


int MHash::MD4(const CVar &data, CVar &result)
/*!\ingroup PPL7_CRYPT
 * \brief MD4-Hash berechnen
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein Hashwert nach dem MD4-Algorithmus für die Daten \p data
 * berechnet und das Ergebnis in \p result gespeichert.
 * \par
 * Handelt es sich bei \p result um ein CBinary, wird das Ergebnis im Binärformat übergeben und
 * hat exakt die Länge des Algorithmus (siehe MHash::GetBlockSize). Wird CString oder CWString
 * verwendet, wird das Ergebnis als Hexadezimalwerte in lesbarer Form zurückgegeben.
 * \par
 * \copydoc Algo_MD4
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @param[out] result Objekt, in dem das Ergebnis gespeichert werden soll. Auch hier kann es
 * sich um ein CBinary, CString oder CWString handeln.
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	return Hash(data,result,Algo_MD4);
}

int MHash::MD5(const CVar &data, CVar &result)
/*!\ingroup PPL7_CRYPT
 * \brief MD5-Hash berechnen
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein Hashwert nach dem MD5-Algorithmus für die Daten \p data
 * berechnet und das Ergebnis in \p result gespeichert.
 * \par
 * Handelt es sich bei \p result um ein CBinary, wird das Ergebnis im Binärformat übergeben und
 * hat exakt die Länge des Algorithmus (siehe MHash::GetBlockSize). Wird CString oder CWString
 * verwendet, wird das Ergebnis als Hexadezimalwerte in lesbarer Form zurückgegeben.
 * \par
 * \copydoc Algo_MD5
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @param[out] result Objekt, in dem das Ergebnis gespeichert werden soll. Auch hier kann es
 * sich um ein CBinary, CString oder CWString handeln.
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	return Hash(data,result,Algo_MD5);
}

int MHash::SHA1(const CVar &data, CVar &result)
/*!\ingroup PPL7_CRYPT
 * \brief SHA1-Hash berechnen
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein Hashwert nach dem SHA1-Algorithmus für die Daten \p data
 * berechnet und das Ergebnis in \p result gespeichert.
 * \par
 * Handelt es sich bei \p result um ein CBinary, wird das Ergebnis im Binärformat übergeben und
 * hat exakt die Länge des Algorithmus (siehe MHash::GetBlockSize). Wird CString oder CWString
 * verwendet, wird das Ergebnis als Hexadezimalwerte in lesbarer Form zurückgegeben.
 * \par
 * \copydoc Algo_SHA1
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @param[out] result Objekt, in dem das Ergebnis gespeichert werden soll. Auch hier kann es
 * sich um ein CBinary, CString oder CWString handeln.
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	return Hash(data,result,Algo_SHA1);
}

int MHash::SHA224(const CVar &data, CVar &result)
/*!\ingroup PPL7_CRYPT
 * \brief SHA224-Hash berechnen
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein Hashwert nach dem SHA224-Algorithmus für die Daten \p data
 * berechnet und das Ergebnis in \p result gespeichert.
 * \par
 * Handelt es sich bei \p result um ein CBinary, wird das Ergebnis im Binärformat übergeben und
 * hat exakt die Länge des Algorithmus (siehe MHash::GetBlockSize). Wird CString oder CWString
 * verwendet, wird das Ergebnis als Hexadezimalwerte in lesbarer Form zurückgegeben.
 * \par
 * \copydoc Algo_SHA224
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @param[out] result Objekt, in dem das Ergebnis gespeichert werden soll. Auch hier kann es
 * sich um ein CBinary, CString oder CWString handeln.
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	return Hash(data,result,Algo_SHA224);
}

int MHash::SHA256(const CVar &data, CVar &result)
/*!\ingroup PPL7_CRYPT
 * \brief SHA256-Hash berechnen
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein Hashwert nach dem SHA256-Algorithmus für die Daten \p data
 * berechnet und das Ergebnis in \p result gespeichert.
 * \par
 * Handelt es sich bei \p result um ein CBinary, wird das Ergebnis im Binärformat übergeben und
 * hat exakt die Länge des Algorithmus (siehe MHash::GetBlockSize). Wird CString oder CWString
 * verwendet, wird das Ergebnis als Hexadezimalwerte in lesbarer Form zurückgegeben.
 * \par
 * \copydoc Algo_SHA256
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @param[out] result Objekt, in dem das Ergebnis gespeichert werden soll. Auch hier kann es
 * sich um ein CBinary, CString oder CWString handeln.
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	return Hash(data,result,Algo_SHA256);
}

int MHash::SHA384(const CVar &data, CVar &result)
/*!\ingroup PPL7_CRYPT
 * \brief SHA384-Hash berechnen
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein Hashwert nach dem SHA384-Algorithmus für die Daten \p data
 * berechnet und das Ergebnis in \p result gespeichert.
 * \par
 * Handelt es sich bei \p result um ein CBinary, wird das Ergebnis im Binärformat übergeben und
 * hat exakt die Länge des Algorithmus (siehe MHash::GetBlockSize). Wird CString oder CWString
 * verwendet, wird das Ergebnis als Hexadezimalwerte in lesbarer Form zurückgegeben.
 * \par
 * \copydoc Algo_SHA384
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @param[out] result Objekt, in dem das Ergebnis gespeichert werden soll. Auch hier kann es
 * sich um ein CBinary, CString oder CWString handeln.
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	return Hash(data,result,Algo_SHA384);
}

int MHash::SHA512(const CVar &data, CVar &result)
/*!\ingroup PPL7_CRYPT
 * \brief SHA512-Hash berechnen
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein Hashwert nach dem SHA512-Algorithmus für die Daten \p data
 * berechnet und das Ergebnis in \p result gespeichert.
 * \par
 * Handelt es sich bei \p result um ein CBinary, wird das Ergebnis im Binärformat übergeben und
 * hat exakt die Länge des Algorithmus (siehe MHash::GetBlockSize). Wird CString oder CWString
 * verwendet, wird das Ergebnis als Hexadezimalwerte in lesbarer Form zurückgegeben.
 * \par
 * \copydoc Algo_SHA512
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @param[out] result Objekt, in dem das Ergebnis gespeichert werden soll. Auch hier kann es
 * sich um ein CBinary, CString oder CWString handeln.
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	return Hash(data,result,Algo_SHA512);
}

int MHash::SHA2(const CVar &data, CVar &result, Bits bits)
/*!\ingroup PPL7_CRYPT
 * \brief SHA2-Hash berechnen
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein Hashwert nach dem SHA-2-Algorithmus mit einer Bittiefe
 * von \p bits für die Daten \p data
 * berechnet und das Ergebnis in \p result gespeichert.
 * \par
 * Handelt es sich bei \p result um ein CBinary, wird das Ergebnis im Binärformat übergeben und
 * hat exakt die Länge des Algorithmus (siehe MHash::GetBlockSize). Wird CString oder CWString
 * verwendet, wird das Ergebnis als Hexadezimalwerte in lesbarer Form zurückgegeben.
 * \par
 * \copydoc Algo_SHA256
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @param[out] result Objekt, in dem das Ergebnis gespeichert werden soll. Auch hier kann es
 * sich um ein CBinary, CString oder CWString handeln.
 * @param[in] bits Die gewünschte Bittiefe für die Berechnung. Möglich sind:
 * Bits_224, Bits_256, Bits_384 und Bits_512
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	if (bits==Bits_Default || bits==Bits_256) return Hash(data,result,Algo_SHA256);
	if (bits==Bits_224) return Hash(data,result,Algo_SHA224);
	if (bits==Bits_384) return Hash(data,result,Algo_SHA384);
	if (bits==Bits_512) return Hash(data,result,Algo_SHA512);
	SetError(526);
	return 0;
}


int MHash::HAVAL(const CVar &data, CVar &result, Bits bits)
/*!\ingroup PPL7_CRYPT
 * \brief HAVAL-Hash berechnen
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein Hashwert nach dem HAVAL-Algorithmus mit einer Bittiefe
 * von \p bits für die Daten \p data
 * berechnet und das Ergebnis in \p result gespeichert.
 * \par
 * Handelt es sich bei \p result um ein CBinary, wird das Ergebnis im Binärformat übergeben und
 * hat exakt die Länge des Algorithmus (siehe MHash::GetBlockSize). Wird CString oder CWString
 * verwendet, wird das Ergebnis als Hexadezimalwerte in lesbarer Form zurückgegeben.
 * \par
 * \copydoc Algo_HAVAL256
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @param[out] result Objekt, in dem das Ergebnis gespeichert werden soll. Auch hier kann es
 * sich um ein CBinary, CString oder CWString handeln.
 * @param[in] bits Die gewünschte Bittiefe für die Berechnung. Möglich sind:
 * Bits_128, Bits_160, Bits_192, Bits_224 und Bits_256
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 *
 * \copydoc Algo_HAVAL256
 *
 * @param data
 * @param result
 * @param bits
 * @return
 */
{
	if (bits==Bits_Default || bits==Bits_256) return Hash(data,result,Algo_HAVAL256);
	if (bits==Bits_128) return Hash(data,result,Algo_HAVAL128);
	if (bits==Bits_160) return Hash(data,result,Algo_HAVAL160);
	if (bits==Bits_192) return Hash(data,result,Algo_HAVAL192);
	SetError(526);
	return 0;
}

int MHash::TIGER(const CVar &data, CVar &result, Bits bits)
/*!\ingroup PPL7_CRYPT
 * \brief TIGER-Hash berechnen
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein Hashwert nach dem TIGER-Algorithmus mit einer Bittiefe
 * von \p bits für die Daten \p data
 * berechnet und das Ergebnis in \p result gespeichert.
 * \par
 * Handelt es sich bei \p result um ein CBinary, wird das Ergebnis im Binärformat übergeben und
 * hat exakt die Länge des Algorithmus (siehe MHash::GetBlockSize). Wird CString oder CWString
 * verwendet, wird das Ergebnis als Hexadezimalwerte in lesbarer Form zurückgegeben.
 * \par
 * \copydoc Algo_TIGER192
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @param[out] result Objekt, in dem das Ergebnis gespeichert werden soll. Auch hier kann es
 * sich um ein CBinary, CString oder CWString handeln.
 * @param[in] bits Die gewünschte Bittiefe für die Berechnung. Möglich sind:
 * Bits_128, Bits_160 und Bits_192
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	if (bits==Bits_Default || bits==Bits_192) return Hash(data,result,Algo_TIGER192);
	if (bits==Bits_128) return Hash(data,result,Algo_TIGER128);
	if (bits==Bits_160) return Hash(data,result,Algo_TIGER160);
	SetError(526);
	return 0;

}

int MHash::RIPEMD(const CVar &data, CVar &result, Bits bits)
/*!\ingroup PPL7_CRYPT
 * \brief RIPEMD-Hash berechnen
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein Hashwert nach dem RIPEMD-Algorithmus mit einer Bittiefe
 * von \p bits für die Daten \p data
 * berechnet und das Ergebnis in \p result gespeichert.
 * \par
 * Handelt es sich bei \p result um ein CBinary, wird das Ergebnis im Binärformat übergeben und
 * hat exakt die Länge des Algorithmus (siehe MHash::GetBlockSize). Wird CString oder CWString
 * verwendet, wird das Ergebnis als Hexadezimalwerte in lesbarer Form zurückgegeben.
 * \par
 * \copydoc Algo_RIPEMD160
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @param[out] result Objekt, in dem das Ergebnis gespeichert werden soll. Auch hier kann es
 * sich um ein CBinary, CString oder CWString handeln.
 * @param[in] bits Die gewünschte Bittiefe für die Berechnung. Möglich sind:
 * Bits_128, Bits_160, Bits_256 und Bits_320
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	if (bits==Bits_Default || bits==Bits_160) return Hash(data,result,Algo_RIPEMD160);
	if (bits==Bits_128) return Hash(data,result,Algo_RIPEMD128);
	if (bits==Bits_256) return Hash(data,result,Algo_RIPEMD256);
	if (bits==Bits_320) return Hash(data,result,Algo_RIPEMD320);
	SetError(526);
	return 0;

}

int MHash::SNEFRU(const CVar &data, CVar &result, Bits bits)
/*!\ingroup PPL7_CRYPT
 * \brief SNEFRU-Hash berechnen
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein Hashwert nach dem SNEFRU-Algorithmus mit einer Bittiefe
 * von \p bits für die Daten \p data
 * berechnet und das Ergebnis in \p result gespeichert.
 * \par
 * Handelt es sich bei \p result um ein CBinary, wird das Ergebnis im Binärformat übergeben und
 * hat exakt die Länge des Algorithmus (siehe MHash::GetBlockSize). Wird CString oder CWString
 * verwendet, wird das Ergebnis als Hexadezimalwerte in lesbarer Form zurückgegeben.
 * \par
 * \copydoc Algo_SNEFRU128
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @param[out] result Objekt, in dem das Ergebnis gespeichert werden soll. Auch hier kann es
 * sich um ein CBinary, CString oder CWString handeln.
 * @param[in] bits Die gewünschte Bittiefe für die Berechnung. Möglich sind:
 * Bits_128 und Bits_256
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	if (bits==Bits_Default || bits==Bits_128) return Hash(data,result,Algo_SNEFRU128);
	if (bits==Bits_256) return Hash(data,result,Algo_SNEFRU256);
	SetError(526);
	return 0;

}


int MHash::GOST(const CVar &data, CVar &result)
/*!\ingroup PPL7_CRYPT
 * \brief GOST-Hash berechnen
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein Hashwert nach dem GOST-Algorithmus für die Daten \p data
 * berechnet und das Ergebnis in \p result gespeichert.
 * \par
 * Handelt es sich bei \p result um ein CBinary, wird das Ergebnis im Binärformat übergeben und
 * hat exakt die Länge des Algorithmus (siehe MHash::GetBlockSize). Wird CString oder CWString
 * verwendet, wird das Ergebnis als Hexadezimalwerte in lesbarer Form zurückgegeben.
 * \par
 * \copydoc Algo_GOST
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @param[out] result Objekt, in dem das Ergebnis gespeichert werden soll. Auch hier kann es
 * sich um ein CBinary, CString oder CWString handeln.
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	return Hash(data,result,Algo_GOST);
}

int MHash::WHIRLPOOL(const CVar &data, CVar &result)
/*!\ingroup PPL7_CRYPT
 * \brief WHIRLPOOL-Hash berechnen
 *
 * \header \#include <ppl7-crypto.h>
 *
 * \descr
 * Mit dieser Funktion wird ein Hashwert nach dem WHIRLPOOL-Algorithmus für die Daten \p data
 * berechnet und das Ergebnis in \p result gespeichert.
 * \par
 * Handelt es sich bei \p result um ein CBinary, wird das Ergebnis im Binärformat übergeben und
 * hat exakt die Länge des Algorithmus (siehe MHash::GetBlockSize). Wird CString oder CWString
 * verwendet, wird das Ergebnis als Hexadezimalwerte in lesbarer Form zurückgegeben.
 * \par
 * \copydoc Algo_WHIRLPOOL
 *
 * @param[in] data Objekt, dem die zu berechnenden Daten zu entnehmen sind. Dabei kann es sich um
 * ein CBinary, CString oder CWString handeln
 * @param[out] result Objekt, in dem das Ergebnis gespeichert werden soll. Auch hier kann es
 * sich um ein CBinary, CString oder CWString handeln.
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	return Hash(data,result,Algo_WHIRLPOOL);
}

#endif

}	// EOF namespace ppl7
