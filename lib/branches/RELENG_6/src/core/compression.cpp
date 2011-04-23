/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.3 $
 * $Date: 2010/07/15 21:20:07 $
 * $Id: compression.cpp,v 1.3 2010/07/15 21:20:07 pafe Exp $
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "ppl6.h"

#ifdef HAVE_LIBZ
#include <zlib.h>
#endif

#ifdef HAVE_BZIP2
#include <bzlib.h>
#endif


namespace ppl6 {

/*!\class CCompression
 * \ingroup PPL6_COMPRESSION
 * \brief Komprimierung und Dekomprimierung von Daten
 *
 * \descr
 * Mit dieser Klasse können Daten komprimiert und dekomprimiert werden. Zur Zeit werden zwei
 * verschiedene Komprimierungsmethoden unterstüzt:
 * - ZLib (siehe http://www.zlib.net/)
 * - BZip2 (siehe http://www.bzip.org/)
 * \par
 * Um die gewünschte Methode auszuwählen, muss diese entweder im Konstruktor übergeben werden, oder durch
 * Aufruf von CCompression::Init, was den Vorteil hat, das man hier auch gleich einen Fehlercode
 * gemeldet bekommt, wenn die gewünschte Methode nicht einkompiliert ist.
 * \par
 * Anschließend können durch Aufrufe von Compress und Uncompress Daten komprimiert bzw. entpackt
 * werden.
 *
 * \example
\code
int main (int argc, char **argv)
{
	ppl6::CCompression comp;
	ppl6::CBinary uncompressed;				// Wir verwenden einfach die Quelldatei des
	if (!uncompressed.Load("main.cpp")) {	// Beispiels zum Komprimieren
		ppl6::PrintError();
		return 0;
	}
	printf ("Größe der Datei unkomprimiert: %u Bytes\n",uncompressed.Size());
	// Komprimierungsmethode und Level auswählen
	if (!comp.Init(ppl6::CCompression::Algo_ZLIB, ppl6::CCompression::Level_High)) {
		ppl6::PrintError();
		return 0;
	}
	// Die komprimierten Daten speichern wir in einem CBinary Objekt
	ppl6::CBinary compressed;
	if (!comp.Compress(compressed,data,false)) {
		ppl6::PrintError();
		return 0;
	}
	printf ("Größe der Datei komprimiert ohne Prefix: %u Bytes\n",compressed.Size());

	// Jetzt wieder dekomprimieren
	ppl6::CBinary recovered;
	if (!comp.Uncompress(recovered,compressed,false)) {
		ppl6::PrintError();
		return 0;
	}
	printf ("Größe nach Dekomprimierung:      %u Bytes, MD5: %s\n",recovered.Size(),(char*)chk2);
	return 1;
}
\endcode

 * \section CCompression_Prefix Komprimierungsprefix
 *
 * Über die Funktion CCompression::UsePrefix kann eingestellt werden, ob bei der Komprimierung noch ein
 * Header vorangestellt werden soll oder nicht. Der Header hat den Vorteil, dass man ihm die Komprimierungs-
 * Methode und die Länge der ursprünglichen unkomprimierten Daten entnehmen kann. Nicht alle Variationen
 * von Compress und Uncompress unterstützen den Prefix, daher ist bei der jeweiligen Funktion vermerkt,
 * ob der Prefix beachtet wird oder nicht.
 *
 * Es gibt zwei Versionen des Headers:
 *
 * \par Version 1 Prefix
 * Bei Version 1 gibt es einen 9-Byte großen Header mit folgendem Aufbau:
 *
\verbatim
Byte 0: Kompressions-Flag (siehe oben)
        Bits 0-2: Kompressionsart
                  0=keine
                  1=Zlib
                  2=Bzip2
        Bits 3-7: unbenutzt, müssen 0 sein
Byte 1: Bytes Unkomprimiert (4 Byte)
Byte 5: Bytes Komprimiert (4 Byte)
\endverbatim
 * Der erste Wert gibt an, wieviele Bytes der Datenblock unkomprimiert benötigt, der zweite gibt an,
 * wie gross er komprimiert ist. Nach dem Header folgen dann soviele Bytes, wie in "Bytes Komprimiert"
 * angegeben ist.
 *
 * \par Version 2 Prefix
 * Die Länge des Version 2 Headers ist variabel. Er beginnt wieder mit dem Kompressionsflag, diesmal
 * ist jedoch Bit 3 gesetzt und die Bits 4-7 werden ebenfalls verwendet:
 *
\verbatim
Byte 0: Kompression-Flag
        Bits 0-2: Kompressionsart
                  0=keine
                  1=Zlib
                  2=Bzip2
        Bit 3:    Headerversion
        Bits 4-5: Bytezahl Uncompressed Value
                  0=1 Byte, 1=2 Byte, 2=3 Byte, 3=4 Byte
        Bits 6-7: Bytezahl Compressed Value
                  0=1 Byte, 1=2 Byte, 2=3 Byte, 3=4 Byte
Byte 1: Bytes Unkomprimiert (1-4 Byte)
Byte n: Bytes Komprimiert (1-4 Byte)
\endverbatim
 * Bei Version 2 folgen eine variable Anzahl von Bytes für die beiden Werte "Bytes Unkomprimiert" und
 * "Bytes Komprimiert". Wieviele Bytes das sind, ist jeweils den Bits 4-5 und 6-7 des
 * Kompressions-Flags zu entnehmen. Bei kleinen Datenblöcken, die unkomprimiert weniger als 255 Bytes
 * benötigen, schrumpft der Prefix somit von 9 auf 3 Byte im Vergleich zum Version 1 Prefix.
 *
 */


/*!\enum CCompression::Algorithm
 * \brief Unterstütze Komprimierungsmethoden
 *
 * Die Klasse unterstützt folgende Komprimierungsmethoden:
 */

/*!\var CCompression::Algorithm CCompression::Algo_NONE
 * Keine Komprimierung. Bei Verwendung dieser Methode werden die Daten einfach nur unverändert kopiert.
 */

/*!\var CCompression::Algorithm CCompression::Algo_ZLIB
 * Zlib ist eine freie Programmbibliothek von Jean-Loup Gailly und Mark Adler (http://www.zlib.net/).
 * Sie verwendet wie gzip den Deflate-Algorithmus um den Datenstrom blockweise zu komprimieren.
 * Die ausgegebenen Blöcke werden durch Adler-32-Prüfsummen geschützt.
 * Das Format ist in den RFC 1950, RFC 1951 und RFC 1952 definiert und gilt quasi als defakto
 * Standard im Unix- und Netzwerkbereich.
 */

/*!\var CCompression::Algorithm CCompression::Algo_BZIP2
 * bzip2 ist ein frei verfügbares Komprimierungsprogramm zur verlustfreien Kompression
 * von Dateien, entwickelt von Julian Seward. Es ist frei von jeglichen patentierten
 * Algorithmen und wird unter einer BSD-ähnlichen Lizenz vertrieben.
 * Die Kompression mit bzip2 ist oft effizienter, aber meist erheblich langsamer als
 * die Kompression mit Zlib.
 */

/*!\var CCompression::Algorithm CCompression::Unknown
 * Wird als Defaulteinstellung beim Dekomprimieren verwendet und hat keine eigentliche
 * Funktion.
 */

/*!\enum CCompression::Level
 * \brief Kompressionsrate
 *
 * Es werden verschiedene Einstellungen unterstützt, die Einfluß auf die Kompressionsrate
 * aber auch Speicherverbrauch und Geschwindigkeit haben:
 */

/*!\var CCompression::Level CCompression::Level_Fast
 * Niedrige Kompressionsrate, dafür aber in der Regel sehr schnell
 */

/*!\var CCompression::Level CCompression::Level_Normal
 * Ausgewogene Kompressionsrate und Geschwidigkeit.
 */

/*!\var CCompression::Level CCompression::Level_Default
 * \copydoc CCompression::Level_Normal
 */

/*!\var CCompression::Level CCompression::Level_High
 * Hohe Kompressionsrate, dafür aber auch langsamer als die anderen Einstellungen
 */

/*!\enum CCompression::Prefix
 * \brief Prefix voranstellen
 *
 * Verwendung eines Prefix, der den komprimierten Daten vorangestellt wird.
 * Siehe dazu auch \ref CCompression_Prefix
 */

/*!\var CCompression::Prefix CCompression::Prefix_None
 * Es wird kein Prefix vorangestellt. Die Anwendung muß sich selbst darum
 * kümmern, dass die Information über Größe der komprimierten und
 * unkomprimierten Daten erhalten bleibt.
 */

/*!\var CCompression::Prefix CCompression::Prefix_V1
 * Es wird ein 9-Byte langer Version 1 Prefix vorangestellt.
 */

/*!\var CCompression::Prefix CCompression::Prefix_V2
 * Es wird ein Version 2 Prefix mit variabler Länge vorangestellt.
 */


/*!\var CCompression::buffer
 * \brief Interner Speicher, der nach Aufruf von Compress die komprimierten Daten enthält
 *
 * Interner Speicher, der nach Aufruf von Compress die komprimierten Daten enthält
 */

/*!\var CCompression::uncbuffer
 * \brief Interner Speicher, der nach Aufruf von Uncompress die entpackten Daten enthält
 *
 * Interner Speicher, der nach Aufruf von Uncompress die entpackten Daten enthält
 */

/*!\var CCompression::aaa
 * \brief Enthält die durch Init oder den Konstruktor eingestellten Kompressionsmethode
 *
 * Enthält die durch Init oder den Konstruktor eingestellten Kompressionsmethode
 */

/*!\var CCompression::lll
 * \brief Enthält den durch Init oder den Konstruktor eingestellten Komprimierungslevel
 *
 * Enthält den durch Init oder den Konstruktor eingestellten Komprimierungslevel
 */

/*!\var CCompression::prefix
 * \brief Flag, ob und welcher Prefix beim Komprimieren vorangestellt wird
 *
 * Flag, ob und welcher Prefix beim Komprimieren vorangestellt wird
 */



CCompression::CCompression()
/*!\brief Konstruktor der Klasse
 *
 * \descr
 * Der parameterlose Konstruktor initialisiert die Klasse mit dem Zlib-Algorithmus und
 * dem Default-Level für die Komprimierungsrate.
 */
{
	buffer=NULL;
	uncbuffer=NULL;
	aaa=Algo_ZLIB;
	lll=Level_Default;
	prefix=Prefix_None;
}

CCompression::CCompression(Algorithm method, Level level)
/*!\brief Konstruktor mit Initialisierung der Komprimierungsmethode
 *
 * \descr
 * Mit diesem Konstruktor kann gleichzeitig bestimmt werden, welche Komprimierungsmethode verwendet werden soll,
 * und wie stark die Komprimierung sein soll. Hier gilt: je höher die Komprimierung, desto langsamer.
 *
 * @param method Komprimierungsmethode (siehe CCompression::Algorithm)
 * @param level Komprimierungslevel (siehe CCompression::Level)
 */
{
	buffer=NULL;
	uncbuffer=NULL;
	aaa=method;
	lll=level;
	prefix=Prefix_None;
}

CCompression::~CCompression()
/*!\brief Destruktor der Klasse
 *
 * \descr
 * Der Destruktor sorgt dafür, dass intern allokierter Speicher freigegeben wird.
 * Falls Ergebnisse aus Compress oder Uncompress Aufrufen in einem CBinary-Objekt
 * gespeichert wurden, ohne "copy"-Flag, so ist der darin enthaltene Speicher
 * ebenfalls ungültig und darf nicht mehr verwendet werden.
 */
{
	if(buffer) free(buffer);
	if(uncbuffer) free(uncbuffer);
}

void CCompression::UsePrefix(Prefix prefix)
/*!\brief Verwendung eines Prefix beim Komprimieren
 *
 * \descr
 * Durch Aufruf dieser Funktion kann festgelegt werden, ob beim Komprimieren
 * den komprimierten Daten ein Prefix vorangestellt wird.
 *
 * CCompression::Prefix
 *
 * @param prefix Der gewünschte Prefix
 *
 * \see CCompression_Prefix
 */
{
	this->prefix=prefix;
}

int CCompression::Init(Algorithm method, Level level)
/*!\brief Gewünschte Komprimierungsmethode einstellen
 *
 * \descr
 * Mit dieser Funktion wird eingestellt, welche Komprimierungsmethode verwendet werden soll,
 * und wie stark die Komprimierung sein soll. Hier gilt: je höher die Komprimierung, desto langsamer.
 *
 * @param method Komprimierungsmethode (siehe CCompression::Algorithm)
 * @param level Komprimierungslevel (siehe CCompression::Level)
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0
 */
{
#ifndef HAVE_LIBZ
	if (method==Algo_ZLIB) {
		SetError(237);
		return 0;
	}
#endif
#ifndef HAVE_BZIP2
	if (method==Algo_BZIP2) {
		SetError(237);
		return 0;
	}
#endif

	aaa=method;
	lll=level;
	return 1;
}

int CCompression::DoNone(void *dst, size_t *dstlen, const void *src, size_t size)
/*!\brief Keine Komprimierung verwenden
 *
 * \descr
 * Diese interne Funktion wird aufgerufen, wenn die Daten garnicht komprimiert werden sollen.
 * Sie ruft daher nun memcpy auf, um die Quelldaten von \p src nach \p dst zu kopieren.
 *
 * @param[in,out] dst Pointer auf den Speicherbereich, in dem die komprimierten Daten abgelegt werden sollen
 * @param[in,out] dstlen Pointer auf eine Variable, die bei Aufruf die Größe des Zielspeicherbereichs \p dst
 * enthält und nach erfolgreichem Aufruf Anzahl tatsächlich benötigter Bytes
 * @param[in] src Pointer auf den Speicherbereich, den komprimiert werden soll
 * @param[in] size Länge des zu komprimierenden Speicherbereichs
 * @return Bei Erfolg gibt die Funktion 1 zurück und die tatsächlich benötigten Bytes können
 * der Variablen \p dstlen entnommen werden. Im Fehlerfall gibt die Funktion 0 zurück.
 */
{
	if (*dstlen<size) {
		SetError(235);
		return 0;		// Buffer für Komprimierung zu klein
	}
	memcpy(dst,src,size);
	*dstlen=size;
	return 1;
}

int CCompression::DoZlib(void *dst, size_t *dstlen, const void *src, size_t size)
/*!\brief Zlib-Komprimierung verwenden
 *
 * \descr
 * Mit dieser internen Funktion werden die Quelldaten aus \p src mit Zlib komprimiert und
 * in \p dst abgelegt.
 *
 * @param[in,out] dst Pointer auf den Speicherbereich, in dem die komprimierten Daten abgelegt werden sollen
 * @param[in,out] dstlen Pointer auf eine Variable, die bei Aufruf die Größe des Zielspeicherbereichs \p dst
 * enthält und nach erfolgreichem Aufruf Anzahl tatsächlich benötigter Bytes
 * @param[in] src Pointer auf den Speicherbereich, den komprimiert werden soll
 * @param[in] size Länge des zu komprimierenden Speicherbereichs
 * @return Bei Erfolg gibt die Funktion 1 zurück und die tatsächlich benötigten Bytes können
 * der Variablen \p dstlen entnommen werden. Im Fehlerfall gibt die Funktion 0 zurück.
 */
{
#ifndef HAVE_LIBZ
	SetError(237);
	return 0;
#else
	uLongf dstlen_zlib;
	int zcomplevel;
	switch (lll) {			// Kompressionslevel festlegen
		case Level_Fast:
			zcomplevel=Z_BEST_SPEED;
			break;
		case Level_Normal:
			zcomplevel=5;
			break;
		case Level_High:
			zcomplevel=Z_BEST_COMPRESSION;
			break;
		default:
			zcomplevel=Z_DEFAULT_COMPRESSION;
			break;
	}
	dstlen_zlib=(uLongf)*dstlen;
	int res=::compress2((Bytef*)dst,(uLongf *)&dstlen_zlib,(const Bytef*)src,size,zcomplevel);
	if (res==Z_OK) {
		*dstlen=(ppluint32)dstlen_zlib;
		return 1;
	} else if (res==Z_MEM_ERROR) {
		SetError(2);
		return 0;
	} else if (res==Z_BUF_ERROR) {
		SetError(235);
		*dstlen=(ppluint32)dstlen_zlib;
		return 0;
	} else if (res==Z_STREAM_ERROR) {
		SetError(225);
		return 0;
	}
	SetError(225);
	return 0;
#endif
}

int CCompression::DoBzip2(void *dst, size_t *dstlen, const void *src, size_t size)
/*!\brief Bzip2-Komprimierung verwenden
 *
 * \descr
 * Mit dieser internen Funktion werden die Quelldaten aus \p src mit BZip2 komprimiert und
 * in \p dst abgelegt.
 *
 * @param[in,out] dst Pointer auf den Speicherbereich, in dem die komprimierten Daten abgelegt werden sollen
 * @param[in,out] dstlen Pointer auf eine Variable, die bei Aufruf die Größe des Zielspeicherbereichs \p dst
 * enthält und nach erfolgreichem Aufruf Anzahl tatsächlich benötigter Bytes
 * @param[in] src Pointer auf den Speicherbereich, den komprimiert werden soll
 * @param[in] size Länge des zu komprimierenden Speicherbereichs
 * @return Bei Erfolg gibt die Funktion 1 zurück und die tatsächlich benötigten Bytes können
 * der Variablen \p dstlen entnommen werden. Im Fehlerfall gibt die Funktion 0 zurück.
 */
{
#ifndef HAVE_BZIP2
	SetError(237);
	return 0;
#else
	int zcomplevel;
	switch (lll) {
		case Level_Fast:
			zcomplevel=1;
			break;
		case Level_Normal:
			zcomplevel=5;
			break;
		case Level_High:
			zcomplevel=9;
			break;
		default:
			zcomplevel=5;
			break;
	}
	int ret=BZ2_bzBuffToBuffCompress((char*)dst,(unsigned int *)dstlen,(char*)src,
		size,zcomplevel,0,30);
	if (ret==BZ_OK) {
		return 1;
	} else if (ret==BZ_MEM_ERROR) {
		SetError(2);
	} else if (ret==BZ_OUTBUFF_FULL) {
		SetError(236);
	} else {
		SetError(225);
	}
	return 0;
#endif
}

int CCompression::UnNone (void *dst, size_t *dstlen, const void *src, size_t srclen)
/*!\brief Speicherbereich ohne Dekompression kopieren
 *
 * \descr
 * Diese interne Funktion wird aufgerufen, wenn die zu dekomprimierenden Daten garnicht
 * komprimiert sind. Sie führt daher lediglich ein memcpy aus.
 *
 * @param[in,out] dst Pointer auf den Speicherbereich, in dem die dekomprimierten Daten abgelegt werden sollen
 * @param[in,out] dstlen Pointer auf eine Variable, die bei Aufruf die Größe des Zielspeicherbereichs \p dst
 * enthält und nach erfolgreichem Aufruf Anzahl tatsächlich benötigter Bytes
 * @param[in] src Pointer auf den Anfang des Speicherbereichs, der die komprimierten Daten enthält
 * @param[in] srclen Länge der komprimierten Daten
 * @return Bei Erfolg gibt die Funktion 1 zurück und die tatsächlich benötigten Bytes können
 * der Variablen \p dstlen entnommen werden. Im Fehlerfall gibt die Funktion 0 zurück.
 */
{
	if (*dstlen<srclen) {
		SetError(236);
		return 0;		// Buffer für Dekomprimierung zu klein
	}
	memcpy(dst,src,srclen);
	*dstlen=srclen;
	return 1;
}

int CCompression::UnZlib (void *dst, size_t *dstlen, const void *src, size_t srclen)
/*!\brief Zlib-Komprimierte Daten entpacken
 *
 * \descr
 * Diese interne Funktion wird aufgerufen, wenn die zu dekomprimierenden Daten mit Zlib
 * komprimiert sind.
 *
 * @param[in,out] dst Pointer auf den Speicherbereich, in dem die dekomprimierten Daten abgelegt werden sollen
 * @param[in,out] dstlen Pointer auf eine Variable, die bei Aufruf die Größe des Zielspeicherbereichs \p dst
 * enthält und nach erfolgreichem Aufruf Anzahl tatsächlich benötigter Bytes
 * @param[in] src Pointer auf den Anfang des Speicherbereichs, der die komprimierten Daten enthält
 * @param[in] srclen Länge der komprimierten Daten
 * @return Bei Erfolg gibt die Funktion 1 zurück und die tatsächlich benötigten Bytes können
 * der Variablen \p dstlen entnommen werden. Im Fehlerfall gibt die Funktion 0 zurück.
 */
{
#ifndef HAVE_LIBZ
	SetError(237);
	return 0;
#else
	ppluint32 d;
	uLongf dstlen_zlib;
	d=*dstlen;
	dstlen_zlib=(uLongf)d;
	int ret=::uncompress((Bytef*)dst,&dstlen_zlib,(const Bytef*) src,srclen);
	if (ret==Z_OK) {
		*dstlen=(ppluint32)dstlen_zlib;
		return 1;
	} else if (ret==Z_MEM_ERROR) {
		SetError(2);
	} else if (ret==Z_BUF_ERROR) {
		SetError(236);
	} else if (ret==Z_DATA_ERROR) {
		SetError(534);
	} else {
		SetError(216);
	}
	return 0;
#endif
}

int CCompression::UnBzip2 (void *dst, size_t *dstlen, const void *src, size_t srclen)
/*!\brief Bzip2-Komprimierte Daten entpacken
 *
 * \descr
 * Diese interne Funktion wird aufgerufen, wenn die zu dekomprimierenden Daten mit Bzip2
 * komprimiert sind.
 *
 * @param[in,out] dst Pointer auf den Speicherbereich, in dem die dekomprimierten Daten abgelegt werden sollen
 * @param[in,out] dstlen Pointer auf eine Variable, die bei Aufruf die Größe des Zielspeicherbereichs \p dst
 * enthält und nach erfolgreichem Aufruf Anzahl tatsächlich benötigter Bytes
 * @param[in] src Pointer auf den Anfang des Speicherbereichs, der die komprimierten Daten enthält
 * @param[in] srclen Länge der komprimierten Daten
 * @return Bei Erfolg gibt die Funktion 1 zurück und die tatsächlich benötigten Bytes können
 * der Variablen \p dstlen entnommen werden. Im Fehlerfall gibt die Funktion 0 zurück.
 */
{
#ifndef HAVE_BZIP2
	SetError(237);
	return 0;
#else
	int ret=BZ2_bzBuffToBuffDecompress((char*)dst,(unsigned int*)dstlen,(char*)src,srclen,0,0);
	if (ret==BZ_OK) {
		return 1;
	} else if (ret==BZ_MEM_ERROR) {
		SetError(2);
	} else if (ret==BZ_OUTBUFF_FULL) {
		SetError(236);
	} else if (ret==BZ_DATA_ERROR || ret==BZ_DATA_ERROR_MAGIC || ret==BZ_UNEXPECTED_EOF) {
		SetError(534);
	} else {
		SetError(216);
	}
	return 0;
#endif
}





int CCompression::Compress(void *dst, size_t *dstlen, const void *src, size_t srclen)
/*!\brief Komprimierung eines Speicherbereiches in einen anderen
 *
 * \descr
 * Mit dieser Version der Compress-Funktion wird ein Speicherbereich \p src mit einer Länge
 * von \p srclen Bytes komprimiert und das Ergebnis mit einer maximalen Länge von
 * \p dstlen Bytes ab der Speicherposition \p dst gespeichert. Der Zielspeicher \p dst
 * muss vorab allokiert worden sein und groß genug sein, um die komprimierten Daten aufzunehmen.
 * Wieviel Bytes tatsächlich verbraucht wurden, ist nach erfolgreichem Aufruf der Variablen
 * \p dstlen zu entnehmen.
 * \par
 * Diese Funktion führt nur die reine Komprimierung durch und unterstützt keinen Prefix.
 *
 * @param[in,out] dst Pointer auf den Speicherbereich, in dem die komprimierten Daten abgelegt werden sollen
 * @param[in,out] dstlen Pointer auf eine Variable, die bei Aufruf die Größe des Zielspeicherbereichs \p dst
 * enthält und nach erfolgreichem Aufruf Anzahl tatsächlich benötigter Bytes
 * @param[in] src Pointer auf den Speicherbereich, den komprimiert werden soll
 * @param[in] srclen Länge des zu komprimierenden Speicherbereichs
 * @return Bei Erfolg gibt die Funktion 1 zurück und die tatsächlich benötigten Bytes können
 * der Variablen \p dstlen entnommen werden. Im Fehlerfall gibt die Funktion 0 zurück.
 *
 * \note
 * Die Funktion prüft lediglich welche Komprimierungsmethode eingestellt wurde und ruft dann eine
 * der privaten Funktionen CCompression::DoNone, CCompression::DoZlib oder CCompression::DoBzip2 auf.
 */
{
	if ((!src) || (!dst)) {
		SetError(194);
		return 0;
	}
	switch (aaa) {
		case Algo_NONE: return DoNone(dst,dstlen,src,srclen);
		case Algo_ZLIB: return DoZlib(dst,dstlen,src,srclen);
		case Algo_BZIP2: return DoBzip2(dst,dstlen,src,srclen);
		default:
			SetError(67);
			return 0;
	}
}

int CCompression::Compress(CBinary &out, const void *ptr, size_t size, bool copy)
/*!\brief Komprimierung eines Speicherbereiches in ein CBinary Objekt
 *
 * \descr
 * Mit dieser Version der Compress-Funktion wird ein Speicherbereich \p ptr mit einer Länge
 * von \p size Bytes komprimiert und das Ergebnis im CBinary-Objekt \p out gespeichert.
 * Der optionale Parameter \p copy bestimmt, ob in CBinary eine Kopie der komprimierten
 * Daten abgelegt wird oder nur ein Pointer auf den internen Buffer der CCompression-Klasse.
 * \par
 * Diese Funktion unterstützt das Prefix-Flag (siehe CCompression::UsePrefix).
 *
 * @param[out] out CBinary-Objekt, in dem die komprimierten Daten gespeichert werden sollen
 * @param[in] ptr Pointer auf den Speicherbereich, den komprimiert werden soll
 * @param[in] size Länge des zu komprimierenden Speicherbereichs
 * @param[in] copy Zeigt an, ob die komprimierten Daten in das CBinary-Objekt \p out
 * kopiert (=true) werden sollen oder darin nur eine Referenz auf den Buffer der
 * CCompression-Klasse abgelegt wird (=false). Letzters ist schneller und spart Speicher,
 * es muss jedoch darauf geachtet werden, dass mit der Instanz von CCompression keine weiteren
 * Aktionen ausgeführt werden und sich auch nicht gelöscht wird, solange das \p out
 * noch verwendet wird!
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Die Länge der
 * komprimierten Daten kann \p out entnommen werden.
 *
 */
{
	if (buffer) free(buffer);
	size_t dstlen=size+64;
	buffer=malloc(dstlen+9);
	char *tgt=(char*)buffer+9;
	if (!buffer) {
		SetError(2);
		return 0;
	}
	if (!Compress(tgt,&dstlen,ptr,size)) {
		return 0;
	}
	if (prefix==Prefix_None) {
		if (copy) return out.Copy(tgt,dstlen);
		return out.Set(tgt,dstlen);
	} else if (prefix==Prefix_V1) {
		char *prefix=(char*)buffer;
		Poke8(prefix,(aaa&7));	// Nur die unteren 3 Bits sind gültig, Rest 0
		Poke32(prefix+1,size);	// Größe Unkomprimiert
		Poke32(prefix+5,dstlen);// Größe Komprimiert
		if (copy) return out.Copy(prefix,dstlen+9);
		return out.Set(prefix,dstlen+9);
	} else if (prefix==Prefix_V2) {
		// Zuerst prüfen wir, wieviel Bytes wir für die jeweiligen Blöcke brauchen
		int b_unc=4, b_comp=4;
		int flag=aaa&7;					// Nur die unteren 3 Bits sind gültig, Rest 0
		flag|=8;						// Version 2-Bit setzen

		if (size<=0xff) b_unc=1;
		else if (size<=0xffff) b_unc=2;
		else if (size<=0xffffff) b_unc=3;
		if (dstlen<=0xff) b_comp=1;
		else if (dstlen<=0xffff) b_comp=2;
		else if (dstlen<=0xffffff) b_comp=3;
		int bytes=1+b_unc+b_comp;
		char *prefix=tgt-bytes;
		char *p2=prefix+1;

		// Daten unkomprimiert
		if (b_unc==1) {
			ppl6::Poke8(prefix+1,size);
			p2=prefix+2;
		} else if (b_unc==2) {
			ppl6::Poke16(prefix+1,size);
			p2=prefix+3;
			flag|=16;
		} else if (b_unc==3) {
			ppl6::Poke24(prefix+1,size);
			p2=prefix+4;
			flag|=32;
		} else {
			ppl6::Poke32(prefix+1,size);
			p2=prefix+5;
			flag|=(16+32);
		}

		// Daten komprimiert
		if (b_comp==1) {
			ppl6::Poke8(p2,size);
		} else if (b_unc==2) {
			ppl6::Poke16(p2,size);
			flag|=64;
		} else if (b_unc==3) {
			ppl6::Poke24(p2,size);
			flag|=128;
		} else {
			ppl6::Poke32(p2,size);
			flag|=(128+64);
		}
		ppl6::Poke8(prefix,flag);
		if (copy) return out.Copy(prefix,dstlen+bytes);
		return out.Set(prefix,dstlen+bytes);
	}
	// Bis hierhin sollte es nicht kommen
	SetError(533);
	return 0;
}

int CCompression::Compress(CBinary &out, const CVar &object, bool copy)
/*!\brief Komprimierung eines von CVar abgeleiteten Objektes in ein CBinary Objekt
 *
 * \descr
 * Mit dieser Version der Compress-Funktion wird Speicher des Objektes \p object
 * komprimiert und das Ergebnis im CBinary-Objekt \p out gespeichert.
 * Der optionale Parameter \p copy bestimmt, ob in CBinary eine Kopie der komprimierten
 * Daten abgelegt wird oder nur ein Pointer auf den internen Buffer der CCompression-Klasse.
 * \p object kann ein CString, CWString oder CBinary sein.
 * \par
 * Diese Funktion unterstützt das Prefix-Flag (siehe CCompression::UsePrefix).
 *
 * @param[out] out CBinary-Objekt, in dem die komprimierten Daten gespeichert werden sollen
 * @param[in] object Ein von CVar abgeleitetes Objekt mit den zu komprimierenden Daten.
 * Zur Zeit werden folgende Datentypen unterstützt: CString, CWString, CBinary
 * @param[in] copy Zeigt an, ob die komprimierten Daten in das CBinary-Objekt \p out
 * kopiert (=true) werden sollen oder darin nur eine Referenz auf den Buffer der
 * CCompression-Klasse abgelegt wird (=false). Letzters ist schneller und spart Speicher,
 * es muss jedoch darauf geachtet werden, dass mit der Instanz von CCompression keine weiteren
 * Aktionen ausgeführt werden und sich auch nicht gelöscht wird, solange das \p out
 * noch verwendet wird!
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Die Länge der
 * komprimierten Daten kann \p out entnommen werden.
 *
 */
{
	int type=object.DataType();
	if (type==CVar::CBINARY) {
		const CBinary &bin=static_cast<const CBinary&>(object);
		return Compress(out,bin.GetPtr(),bin.Size(),copy);
	} else if (type==CVar::CSTRING) {
		const CString &str=static_cast<const CString&>(object);
		return Compress(out,str.GetPtr(),str.Size(),copy);
	} else if (type==CVar::CWSTRING) {
		const CWString &wstr=static_cast<const CWString&>(object);
		return Compress(out,wstr.GetBuffer(),wstr.Size(),copy);
	}
	SetError(337);
	return 0;
}

int CCompression::Compress(CMemory &out, const CMemoryReference &in)
/*!\brief Komprimierung eines Speicherbereichs in ein CMemory-Objekt
 *
 * \descr
 * Mit dieser Version der Compress-Funktion wird der durch \p in referenzierte
 * Speicher komprimiert und das Ergebnis in \p out gespeichert.
 * \par
 * Diese Funktion unterstützt das Prefix-Flag (siehe CCompression::UsePrefix).
 *
 * @param[out] out CMemory-Objekt, in dem die komprimierten Daten gespeichert werden sollen
 * @param[in] in Ein von CMemoryReferemce abgeleitetes Objekt, das den zu
 * komprimierenden Speicherbereich repräsentiert.
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Die Länge der
 * komprimierten Daten kann \p out entnommen werden.
 *
 */
{
	CBinary o;
	if (!Compress(o,in.adr(),in.size())) return 0;
	out.copy(o);
	return 1;
}

int CCompression::Uncompress(void *dst, size_t *dstlen, const void *src, size_t srclen, Algorithm method)
/*!\brief Dekomprimierung eines Speicherbereiches in einen anderen
 *
 * \descr
 * Mit dieser Version der Compress-Funktion wird ein komprimierter Speicherbereich \p src mit einer Länge
 * von \p srclen Bytes dekomprimiert und das entpackte Ergebnis mit einer maximalen Länge von
 * \p dstlen Bytes ab der Speicherposition \p dst gespeichert. Der Zielspeicher \p dst
 * muss vorab allokiert worden sein und groß genug sein, um die unkomprimierten Daten aufzunehmen.
 * Wieviel Bytes tatsächlich verbraucht wurden, ist nach erfolgreichem Aufruf der Variablen
 * \p dstlen zu entnehmen.
 * \par
 * Diese Funktion führt nur die reine Dekomprimierung durch und unterstützt keinen Prefix.
 *
 * @param[in,out] dst Pointer auf den Speicherbereich, in dem die dekomprimierten Daten abgelegt werden sollen
 * @param[in,out] dstlen Pointer auf eine Variable, die bei Aufruf die Größe des Zielspeicherbereichs \p dst
 * enthält und nach erfolgreichem Aufruf Anzahl tatsächlich benötigter Bytes
 * @param[in] src Pointer auf den Anfang des Speicherbereichs, der die komprimierten Daten enthält
 * @param[in] srclen Länge der komprimierten Daten
 * @param[in] method Optionaler Parameter, der die Kompressionsmethode angibt. Wird er nicht
 * angegeben, werden die Einstellung verwendet, die beim Aufruf von CCompression::Init bzw. über
 * den Konstruktor CCompression::CCompression(Algorithm method, Level level) angegeben wurde.
 * @return Bei Erfolg gibt die Funktion 1 zurück und die tatsächlich benötigten Bytes können
 * der Variablen \p dstlen entnommen werden. Im Fehlerfall gibt die Funktion 0 zurück.
 *
 * \note
 * Die Funktion prüft lediglich welche Komprimierungsmethode eingestellt wurde und ruft dann eine
 * der privaten Funktionen CCompression::UnNone, CCompression::UnZlib oder CCompression::UnBzip2 auf.
 */
{
	if ((!src) || (!dst)) {
		SetError(194);
		return 0;
	}
	if (method==Unknown) method=aaa;
	switch (method) {
		case Algo_NONE: return UnNone(dst,dstlen,src,srclen);
		case Algo_ZLIB: return UnZlib(dst,dstlen,src,srclen);
		case Algo_BZIP2: return UnBzip2(dst,dstlen,src,srclen);
		default:
			SetError(67);
			return 0;
	}
}
int CCompression::Uncompress(CBinary &out, const CBinary &object, bool copy)
/*!\brief Dekomprimierung eines CBinary Objektes
 *
 * \descr
 * Mit dieser Version der Compress-Funktion wird Speicher des Objektes \p object
 * entpackt und das Ergebnis im CBinary-Objekt \p out gespeichert.
 * Der optionale Parameter \p copy bestimmt, ob in CBinary eine Kopie der komprimierten
 * Daten abgelegt wird oder nur ein Pointer auf den internen Buffer der CCompression-Klasse.
 * \par
 * Diese Funktion unterstützt das Prefix-Flag (siehe CCompression::UsePrefix).
 *
 * @param[out] out CBinary-Objekt, in dem die entpackten Daten gespeichert werden sollen
 * @param[in] object CBinary-Objekt, das die komprimierten Daten enthält.
 * @param[in] copy Zeigt an, ob die komprimierten Daten in das CBinary-Objekt \p out
 * kopiert (=true) werden sollen oder darin nur eine Referenz auf den Buffer der
 * CCompression-Klasse abgelegt wird (=false). Letzters ist schneller und spart Speicher,
 * es muss jedoch darauf geachtet werden, dass mit der Instanz von CCompression keine weiteren
 * Aktionen ausgeführt werden und sich auch nicht gelöscht wird, solange das \p out
 * noch verwendet wird!
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Die Länge der
 * unkomprimierten Daten kann \p out entnommen werden.
 */
{
	return Uncompress(out,object.GetPtr(),object.Size(),copy);
}

int CCompression::Uncompress(CMemory &out, const CMemoryReference &in)
/*!\brief Dekomprimierung eines Speicherbereichs in ein CMemory-Objekt
 *
 * \descr
 * Mit dieser Version der Compress-Funktion wird der durch \p in referenzierte
 * Speicherbereich dekomprimiert und das Ergebnis im CMemory-Objekt
 * \p out gespeichert.
 * \par
 * Diese Funktion unterstützt das Prefix-Flag (siehe CCompression::UsePrefix).
 *
 * @param[out] out CMemory-Objekt, in dem die entpackten Daten gespeichert werden sollen
 * @param[in] in CMemoryReferemce-Objekt, das die komprimierten Daten enthält.
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Die Länge der
 * unkomprimierten Daten kann \p out entnommen werden.
 */
{
	CBinary o;
	if (!Uncompress(o,in.adr(),in.size())) return 0;
	out.copy(o);
	return 1;
}




int CCompression::Uncompress(CBinary &out, const void *ptr, size_t size, bool copy)
/*!\brief Dekomprimierung eines Speicherbereichs in ein CBinary Objekt
 *
 * \descr
 * Mit dieser Version der Compress-Funktion wird der durch \p ptr angegebene
 * Speicherbereich mit einer Länge von \p size Bytes dekomprimiert und die entpackten
 * Daten im CBinary-Objekt \p out gespeichert.
 * Der optionale Parameter \p copy bestimmt, ob in CBinary eine Kopie der komprimierten
 * Daten abgelegt wird oder nur ein Pointer auf den internen Buffer der CCompression-Klasse.
 * \par
 * Diese Funktion unterstützt das Prefix-Flag (siehe CCompression::UsePrefix).
 *
 * @param[out] out CBinary-Objekt, in dem die entpackten Daten gespeichert werden sollen
 * @param[in] ptr Pointer auf den Beginn des zu entpackenden Speicherbereichs
 * @param[in] size Größe des komprimierten Speicherbereichs
 * @param[in] copy Zeigt an, ob die komprimierten Daten in das CBinary-Objekt \p out
 * kopiert (=true) werden sollen oder darin nur eine Referenz auf den Buffer der
 * CCompression-Klasse abgelegt wird (=false). Letzters ist schneller und spart Speicher,
 * es muss jedoch darauf geachtet werden, dass mit der Instanz von CCompression keine weiteren
 * Aktionen ausgeführt werden und sich auch nicht gelöscht wird, solange das \p out
 * noch verwendet wird!
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Die Länge der
 * unkomprimierten Daten kann \p out entnommen werden.
 */
{
	if (uncbuffer) free(uncbuffer);
	uncbuffer=NULL;
	if (prefix==Prefix_None) {
		size_t bsize=size*3;
		while (1) {
			if (uncbuffer) free(uncbuffer);
			uncbuffer=malloc(bsize);
			if (!uncbuffer) {
				SetError(2);
				return 0;
			}
			// Wir prüfen, ob das Ergebnis in den Buffer passt
			size_t dstlen=bsize;
			if (!Uncompress(uncbuffer,&dstlen,ptr,size,aaa)) {
				if (GetErrorCode()!=236) {
					free(uncbuffer);
					uncbuffer=NULL;
					return 0;
				}
				// Der Buffer war nicht gross genug, wir vergrößern ihn
				bsize+=size;
			} else {	// Dekomprimierung erfolgreich
				if (copy) return out.Copy(uncbuffer,dstlen);
				else return out.Set(uncbuffer,dstlen);
			}
		}
	} else if (prefix==Prefix_V1) {
		char *buffer=(char*)ptr;
		int flag=Peek8(buffer);
		size_t size_unc=Peek32(buffer+1);
		size_t size_comp=Peek32(buffer+5);
		//printf ("Flag: %i, unc: %u, comp: %u\n",flag,size_unc, size_comp);
		if (uncbuffer) free(uncbuffer);
		uncbuffer=malloc(size_unc);
		if (!uncbuffer) {
			SetError(2);
			return 0;
		}
		size_t dstlen=size_unc;
		if (!Uncompress(uncbuffer,&dstlen,buffer+9,size_comp,(Algorithm)(flag&7))) {
			free(uncbuffer);
			uncbuffer=NULL;
			return 0;
		}
		// Dekomprimierung erfolgreich
		if (copy) return out.Copy(uncbuffer,dstlen);
		else return out.Set(uncbuffer,dstlen);
	} else if (prefix==Prefix_V2) {
		char *buffer=(char*)ptr;
		int flag=Peek8(buffer);
		Algorithm a=(Algorithm)(flag&7);
		if ((flag&8)==0) {	// Bit 3 muss aber gesetzt sein
			SetError(535);
			return 0;
		}
		int b_unc=4, b_comp=4;
		if ((flag&48)==0) b_unc=1;
		else if ((flag&48)==16) b_unc=2;
		else if ((flag&48)==32) b_unc=3;
		else b_unc=4;
		if ((flag&192)==0) b_comp=1;
		else if ((flag&192)==64) b_comp=2;
		else if ((flag&192)==128) b_comp=3;
		else b_comp=4;

		size_t size_unc=0;
		if (b_unc==1) size_unc=Peek8(buffer+1);
		else if (b_unc==2) size_unc=Peek16(buffer+1);
		else if (b_unc==3) size_unc=Peek24(buffer+1);
		else size_unc=Peek32(buffer+1);

		if (uncbuffer) free(uncbuffer);
		uncbuffer=malloc(size_unc);
		if (!uncbuffer) {
			SetError(2);
			return 0;
		}
		size_t dstlen=size_unc;
		size_t bytes=1+b_unc+b_comp;
		if (!Uncompress(uncbuffer,&dstlen,buffer+bytes,size-bytes,a)) {
			free(uncbuffer);
			uncbuffer=NULL;
			return 0;
		}
		// Dekomprimierung erfolgreich
		if (copy) return out.Copy(uncbuffer,dstlen);
		else return out.Set(uncbuffer,dstlen);
	}
	SetError(533);
	return 0;
}


int Compress(CBinary &out, const CVar &in, CCompression::Algorithm method, CCompression::Level level)
/*!\ingroup PPL6_COMPRESSION
 * \relatesalso CCompression
 * \brief Daten komprimieren
 *
 * \descr
 * Mit dieser Funktion wird der Speicher des von CVar abgeleiteten Objektes \p in
 * mit der Komprimierungsmethode \p method und dem Komprimierungslevel \p level komprimiert
 * und das Ergebnis im CBinary-Objekt \p out gespeichert.
 * \p in kann ein CString, CWString oder CBinary sein.
 * \par
 * Die Funktion stellt den komprimierten Daten automatisch einen Version 2 Prefix voran (siehe
 * \ref CCompression_Prefix), so dass die komprimierten Daten durch Aufruf der Funktion
 * Uncompress ohne Angabe der Kompressionsmethod wieder entpackt werden kann.
 *
 * @param[out] out CBinary-Objekt, in dem die komprimierten Daten gespeichert werden sollen
 * @param[in] in Ein von CVar abgeleitetes Objekt mit den zu komprimierenden Daten.
 * Zur Zeit werden folgende Datentypen unterstützt: CString, CWString, CBinary
 * @param[in] method Die gewünschte Komprimierungsmethode (siehe CCompression::Algorithm)
 * @param[in] level Der gewünschte Komprimierungslevel (siehe CCompression::Level)
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Die Länge der
 * komprimierten Daten kann \p out entnommen werden.
 *
 * \see CCompression
 */
{
	CCompression comp;
	if (!comp.Init(method,level)) return 0;
	comp.UsePrefix(CCompression::Prefix_V2);
	if (!comp.Compress(out,in)) return 0;
	return 1;
}

/*!\ingroup PPL6_COMPRESSION
 * \brief Speicherbereich komprimieren
 *
 * \desc
 * Mit dieser Funktion wird der durch \p in referenzierte Speicher
 * mit der Komprimierungsmethode \p method und dem Komprimierungslevel \p level komprimiert
 * und das Ergebnis im CMemory-Objekt \p out gespeichert.
 *
 * Speicherbereich komprimieren
 *
 * @param[out] out CMemory-Objekt, in dem die komprimierten Daten gespeichert werden sollen
 * @param[in] in Ein von CMemoryReference abgeleitetes Objekt mit den zu komprimierenden Daten
 * @param[in] method Die gewünschte Komprimierungsmethode (siehe CCompression::Algorithm)
 * @param[in] level Der gewünschte Komprimierungslevel (siehe CCompression::Level)
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Die Länge der
 * komprimierten Daten kann \p out entnommen werden.
 *
 * \see CCompression
 * @return
 */
int Compress(CMemory &out, const CMemoryReference &in, CCompression::Algorithm method, CCompression::Level level)
{
	CCompression comp;
	if (!comp.Init(method,level)) return 0;
	comp.UsePrefix(CCompression::Prefix_V2);
	if (!comp.Compress(out,in)) return 0;
	return 1;
}

int Compress(CBinary &out, const void *buffer, size_t size, CCompression::Algorithm method, CCompression::Level level)
/*!\ingroup PPL6_COMPRESSION
 * \relatesalso CCompression
 * \brief Daten komprimieren
 *
 * \descr
 * Mit dieser Funktion wird der durch \p buffer angegebene Speicherbereich mit einer Länge von
 * \p size Bytes mit der Komprimierungsmethode \p method und dem Komprimierungslevel \p level komprimiert
 * und das Ergebnis im CBinary-Objekt \p out gespeichert.
 * \par
 * Die Funktion stellt den komprimierten Daten automatisch einen Version 2 Prefix voran (siehe
 * \ref CCompression_Prefix), so dass die komprimierten Daten durch Aufruf der Funktion
 * Uncompress ohne Angabe der Kompressionsmethod wieder entpackt werden kann.
 *
 * @param[out] out CBinary-Objekt, in dem die komprimierten Daten gespeichert werden sollen
 * @param[in] buffer Pointer auf den zu komprimierenden Speicherbereich
 * @param[in] size Größe des zu komprimierenden Speicherbereichs
 * @param[in] method Die gewünschte Komprimierungsmethode (siehe CCompression::Algorithm)
 * @param[in] level Der gewünschte Komprimierungslevel (siehe CCompression::Level)
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Die Länge der
 * komprimierten Daten kann \p out entnommen werden.
 *
 * \see CCompression
 */
{
	CCompression comp;
	if (!comp.Init(method,level)) return 0;
	comp.UsePrefix(CCompression::Prefix_V2);
	if (!comp.Compress(out,buffer,size)) return 0;
	return 1;
}

/*!\ingroup PPL6_COMPRESSION
 * \relatesalso CCompression
 * \brief Daten dekomprimieren
 *
 * \descr
 * Mit dieser Funktion werden die in \p in enthaltenen komprimierten Daten
 * entpackt und das Ergebnis im CBinary-Objekt \p out gespeichert.
 * \par
 * Die Funktion geht davon aus, dass die komprimierten Daten mit einem
 * Version 2 Prefix beginnen (siehe \ref CCompression_Prefix). Ist dies nicht der
 * Fall, sollte statt dieser Funktion die Klasse CCompression verwendet werden,
 * deren CCompression::Uncompress-Funktionen auch Dekomprimierung ohne Prefix
 * unterstützen.
 *
 * @param[out] out CBinary-Objekt, in dem die entpackten Daten gespeichert werden sollen
 * @param[in] in Das CBinary-Objekt, das die komprimierten Daten enthält
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0
 *
 * \see CCompression
 */
int Uncompress(CBinary &out, const CBinary &in)
{
	CCompression comp;
	comp.UsePrefix(CCompression::Prefix_V2);
	if (!comp.Uncompress(out,in)) return 0;
	return 1;
}

/*!\ingroup PPL6_COMPRESSION
 * \relatesalso CCompression
 * \brief Daten dekomprimieren
 *
 * \descr
 * Mit dieser Funktion werden die in \p in enthaltenen komprimierten Daten
 * entpackt und das Ergebnis im CMemory-Objekt \p out gespeichert.
 * \par
 * Die Funktion geht davon aus, dass die komprimierten Daten mit einem
 * Version 2 Prefix beginnen (siehe \ref CCompression_Prefix). Ist dies nicht der
 * Fall, sollte statt dieser Funktion die Klasse CCompression verwendet werden,
 * deren CCompression::Uncompress-Funktionen auch Dekomprimierung ohne Prefix
 * unterstützen.
 *
 * @param[out] out CMemory-Objekt, in dem die entpackten Daten gespeichert werden sollen
 * @param[in] in Ein CMemoryReference-Objekt, das die komprimierten Daten enthält
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0
 *
 * \see CCompression
 */
int Uncompress(CMemory &out, const CMemoryReference &in)
{
	CCompression comp;
	comp.UsePrefix(CCompression::Prefix_V2);
	if (!comp.Uncompress(out,in)) return 0;
	return 1;

}


int Uncompress(CBinary &out, const void *buffer, size_t size)
/*!\ingroup PPL6_COMPRESSION
 * \relatesalso CCompression
 * \brief Daten dekomprimieren
 *
 * \descr
 * Mit dieser Funktion werden die komprimierten Daten in \p buffer mit einer Länge
 * von \p size Bytes entpackt und das Ergebnis im CBinary-Objekt \p out gespeichert.
 * \par
 * Die Funktion geht davon aus, dass die komprimierten Daten mit einem
 * Version 2 Prefix beginnen (siehe \ref CCompression_Prefix). Ist dies nicht der
 * Fall, sollte statt dieser Funktion die Klasse CCompression verwendet werden,
 * deren CCompression::Uncompress-Funktionen auch Dekomprimierung ohne Prefix
 * unterstützen.
 *
 * @param[out] out CBinary-Objekt, in dem die entpackten Daten gespeichert werden sollen
 * @param[in] buffer Pointer auf den Beginn des komprimierten Speicherbereichs
 * @param[in] size Länge des komprimierten Speicherbnereichs in Byte
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0
 *
 * \see CCompression
 *
 */
{
	CCompression comp;
	comp.UsePrefix(CCompression::Prefix_V2);
	if (!comp.Uncompress(out,buffer,size)) return 0;
	return 1;
}

int CompressZlib(CBinary &out, const CVar &in, CCompression::Level level)
/*!\ingroup PPL6_COMPRESSION
 * \relatesalso CCompression
 * \brief Daten mit ZLib komprimieren
 *
 * \descr
 * Mit dieser Funktion wird der Speicher des von CVar abgeleiteten Objektes \p in
 * mit der Komprimierungsmethode ZLib und dem Komprimierungslevel \p level komprimiert
 * und das Ergebnis im CBinary-Objekt \p out gespeichert.
 * \p in kann ein CString, CWString oder CBinary sein.
 * \par
 * Die Funktion stellt den komprimierten Daten automatisch einen Version 2 Prefix voran (siehe
 * \ref CCompression_Prefix), so dass die komprimierten Daten durch Aufruf der Funktion
 * Uncompress ohne Angabe der Kompressionsmethod wieder entpackt werden kann.
 *
 * @param[out] out CBinary-Objekt, in dem die komprimierten Daten gespeichert werden sollen
 * @param[in] in Ein von CVar abgeleitetes Objekt mit den zu komprimierenden Daten.
 * Zur Zeit werden folgende Datentypen unterstützt: CString, CWString, CBinary
 * @param[in] level Der gewünschte Komprimierungslevel (siehe CCompression::Level). Der Default ist
 * CCompression::Level_High
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Die Länge der
 * komprimierten Daten kann \p out entnommen werden.
 *
 * \see CCompression
 */
{
	return Compress(out,in,CCompression::Algo_ZLIB,level);
}

int CompressZlib(CMemory &out, const CMemoryReference &in, CCompression::Level level)
/*!\ingroup PPL6_COMPRESSION
 * \relatesalso CCompression
 * \brief Daten mit ZLib komprimieren
 *
 * \descr
 * Mit dieser Funktion wird der durch \p in referenzierte Speicherbereich
 * mit der Komprimierungsmethode ZLib und dem Komprimierungslevel \p level komprimiert
 * und das Ergebnis im CMemory-Objekt \p out gespeichert.
 * \par
 * Die Funktion stellt den komprimierten Daten automatisch einen Version 2 Prefix voran (siehe
 * \ref CCompression_Prefix), so dass die komprimierten Daten durch Aufruf der Funktion
 * Uncompress ohne Angabe der Kompressionsmethod wieder entpackt werden kann.
 *
 * @param[out] out CMemory-Objekt, in dem die komprimierten Daten gespeichert werden sollen
 * @param[in] in Ein CMemoryReference-Objekt mit den zu komprimierenden Daten.
 * @param[in] level Der gewünschte Komprimierungslevel (siehe CCompression::Level). Der Default ist
 * CCompression::Level_High
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Die Länge der
 * komprimierten Daten kann \p out entnommen werden.
 *
 * \see CCompression
 */
{
	return Compress(out,in,CCompression::Algo_ZLIB,level);
}


int CompressZlib(CBinary &out, const void *buffer, size_t size, CCompression::Level level)
/*!\ingroup PPL6_COMPRESSION
 * \relatesalso CCompression
 * \brief Daten mit ZLib komprimieren
 *
 * \descr
 * Mit dieser Funktion wird der durch \p buffer angegebene Speicherbereich mit einer Länge von
 * \p size Bytes mit der Komprimierungsmethode ZLib und dem Komprimierungslevel \p level komprimiert
 * und das Ergebnis im CBinary-Objekt \p out gespeichert.
 * \par
 * Die Funktion stellt den komprimierten Daten automatisch einen Version 2 Prefix voran (siehe
 * \ref CCompression_Prefix), so dass die komprimierten Daten durch Aufruf der Funktion
 * Uncompress ohne Angabe der Kompressionsmethod wieder entpackt werden kann.
 *
 * @param[out] out CBinary-Objekt, in dem die komprimierten Daten gespeichert werden sollen
 * @param[in] buffer Pointer auf den zu komprimierenden Speicherbereich
 * @param[in] size Größe des zu komprimierenden Speicherbereichs
 * @param[in] level Der gewünschte Komprimierungslevel (siehe CCompression::Level). Der Default ist
 * CCompression::Level_High
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Die Länge der
 * komprimierten Daten kann \p out entnommen werden.
 *
 * \see CCompression
 */
{
	return Compress(out, buffer, size, CCompression::Algo_ZLIB, level);
}

int CompressBZip2(CBinary &out, const CVar &in, CCompression::Level level)
/*!\ingroup PPL6_COMPRESSION
 * \relatesalso CCompression
 * \brief Daten mit BZip2 komprimieren
 *
 * \descr
 * Mit dieser Funktion wird der Speicher des von CVar abgeleiteten Objektes \p in
 * mit der Komprimierungsmethode BZip2 und dem Komprimierungslevel \p level komprimiert
 * und das Ergebnis im CBinary-Objekt \p out gespeichert.
 * \p in kann ein CString, CWString oder CBinary sein.
 * \par
 * Die Funktion stellt den komprimierten Daten automatisch einen Version 2 Prefix voran (siehe
 * \ref CCompression_Prefix), so dass die komprimierten Daten durch Aufruf der Funktion
 * Uncompress ohne Angabe der Kompressionsmethod wieder entpackt werden kann.
 *
 * @param[out] out CBinary-Objekt, in dem die komprimierten Daten gespeichert werden sollen
 * @param[in] in Ein von CVar abgeleitetes Objekt mit den zu komprimierenden Daten.
 * Zur Zeit werden folgende Datentypen unterstützt: CString, CWString, CBinary
 * @param[in] level Der gewünschte Komprimierungslevel (siehe CCompression::Level). Der Default ist
 * CCompression::Level_High
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Die Länge der
 * komprimierten Daten kann \p out entnommen werden.
 *
 * \see CCompression
 */
{
	return Compress(out,in,CCompression::Algo_BZIP2,level);
}

int CompressBZip2(CBinary &out, const void *buffer, size_t size, CCompression::Level level)
/*!\ingroup PPL6_COMPRESSION
 * \relatesalso CCompression
 * \brief Daten mit BZip2 komprimieren
 *
 * \descr
 * Mit dieser Funktion wird der durch \p buffer angegebene Speicherbereich mit einer Länge von
 * \p size Bytes mit der Komprimierungsmethode BZip2 und dem Komprimierungslevel \p level komprimiert
 * und das Ergebnis im CBinary-Objekt \p out gespeichert.
 * \par
 * Die Funktion stellt den komprimierten Daten automatisch einen Version 2 Prefix voran (siehe
 * \ref CCompression_Prefix), so dass die komprimierten Daten durch Aufruf der Funktion
 * Uncompress ohne Angabe der Kompressionsmethod wieder entpackt werden kann.
 *
 * @param[out] out CBinary-Objekt, in dem die komprimierten Daten gespeichert werden sollen
 * @param[in] buffer Pointer auf den zu komprimierenden Speicherbereich
 * @param[in] size Größe des zu komprimierenden Speicherbereichs
 * @param[in] level Der gewünschte Komprimierungslevel (siehe CCompression::Level). Der Default ist
 * CCompression::Level_High
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Die Länge der
 * komprimierten Daten kann \p out entnommen werden.
 *
 * \see CCompression
 */
{
	return Compress(out, buffer, size, CCompression::Algo_BZIP2, level);
}

int CompressBZip2(CMemory &out, const CMemoryReference &in, CCompression::Level level)
/*!\ingroup PPL6_COMPRESSION
 * \relatesalso CCompression
 * \brief Daten mit BZip2 komprimieren
 *
 * \descr
 * Mit dieser Funktion wird der durch \p in referenzierte Speicherbereich
 * mit der Komprimierungsmethode BZip2 und dem Komprimierungslevel \p level komprimiert
 * und das Ergebnis im CMemory-Objekt \p out gespeichert.
 * \par
 * Die Funktion stellt den komprimierten Daten automatisch einen Version 2 Prefix voran (siehe
 * \ref CCompression_Prefix), so dass die komprimierten Daten durch Aufruf der Funktion
 * Uncompress ohne Angabe der Kompressionsmethod wieder entpackt werden kann.
 *
 * @param[out] out CMemory-Objekt, in dem die komprimierten Daten gespeichert werden sollen
 * @param[in] in Ein CMemoryReference-Objekt mit den zu komprimierenden Daten.
 * @param[in] level Der gewünschte Komprimierungslevel (siehe CCompression::Level). Der Default ist
 * CCompression::Level_High
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Die Länge der
 * komprimierten Daten kann \p out entnommen werden.
 *
 * \see CCompression
 */
{
	return Compress(out,in,CCompression::Algo_BZIP2,level);
}


} // EOF namespace ppl
