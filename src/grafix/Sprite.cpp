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
#include "ppl7.h"
#include "ppl7-grafix.h"

namespace ppl7 {
namespace grafix {

/*!\page PFPSpriteTexture1 Format PFP Sprite-Textures, Version 1
 *
Eine Texture-Sprite-Datei wird zum Speichern von Sprites verwendet, die mit der Klasse ppl7::grafix::Sprite
geladen und dargestellt werden können. Dabei werden viele einzelne kleine Sprite-Grafiken auf einer
oder mehreren größeren Texturen zusammengefasst.
Die Datei verwendet als Basisformat das \ref PFPFileVersion3 "PFP-File-Format Version 3" mit seinen Chunks.
Das File trägt als ID "TEXS", Hauptversion 1, Unterversion 0. Eine Texturedatei enthält immer einen
Index-Chunk (INDX) und beliebig viele Surface-Chunks (SURF).

\par INDX: Index-Chunk

Der INDX-Chunk beginnt mit einem 4-Byte-Wert im Little Endian Format, der angibt, wieviele Sprites
im Index vorhanden sind.
\code
Byte 0:  Anzahl Sprites                               4 Byte
\endcode

Darauf folgt dann pro Sprite ein 22 Byte langer Block mit folgenden Daten:
\code
Byte 0:  ItemId                                       4 Byte
Byte 4:  TextureId                                    2 Byte
Byte 6:  TextureRect                                  8 Byte
         Byte 6:  left          (2 Byte)
         Byte 8:  top           (2 Byte)
         Byte 10: right         (2 Byte)
         Byte 12: bottom        (2 Byte)
Byte 14: Pivot-Punkt                                  4 Byte
         Byte 14: x-Koordinate  (2 Byte)
         Byte 16: y-Koordinate  (2 Byte)
Byte 18: Offset                                       4 Byte
         Byte 18: x-Koordinate  (2 Byte)
         Byte 20: y-Koordinate  (2 Byte)
\endcode
Sämtliche Werte müssen im Little-Endian-Format angegeben werden.

Beschreibung:
<ul>
<li><b>ItemId</b>\n
Jedes Sprite hat seine eigene ID. Diese muss nicht zwangsläufig ein fortlaufender Zähler sein, sondern
kann eine beliebige Ziffer sein. Sie muss nur eindeutig sein.
</li>
<li><b>TextureId</b>\n
Eine Sprite-Datei kann mehrere Texturen enthalten. Dieser Wert stellt eine Referenz auf die
ID der Textur dar.
</li>
<li><b>TextureRect</b>\n
Da pro Textur mehrere Sprites gespeichert werden, müssen dessen Koordinaten innerhalb der Textur
festgehalten werden. Diese geschieht innerhalb dieses Datenblocks, bei dem jeder Wert mit 2 Byte
im Little-Endian Format dargestellt wird.
</li>
<li><b>Pivot-Punkt</b>\n
Der Pivot-Punkt oder auch Ursprung ist die Position innerhalb des Sprites, aber der mit dem Zeichnen
begonnen wird. Bei einer Figur könnte dies z.B. der Mittelpunkt der Füße sein, bei einem Baum das
unterste Ende des Stamms.
</li>
<li><b>Offset</b>\n
Falls das Sprite in der Quellgrafik nicht bündig mit der ersten Pixelreihe bzw. Pixelspalte
beginnt, wird der sogenannte Offset gespeichert. Er gibt ganz einfach Spalte und Zeile an, ab der
das Sprite beginnt.
</li>
</ul>
\par SURF: Surface-Chunk
Der Surface-Chunk setzt sich aus einem 8 Byte langem Header, gefolgt von den Zlib-komprimierten
Pixel-Daten zusammen:

\code
Byte 0:  TextureId                                    2 Byte
Byte 2:  RGB-Format                                   1 Byte
Byte 3:  Bittiefe                                     1 Byte
Byte 4:  Breite                                       2 Byte
Byte 6:  Höhe                                         2 Byte
\endcode
Beschreibung:
<ul>
<li><b>TextureId</b>\n
Eindeutige ID der Textur, in der Regel eine fortlaufende Nummer. Wird als Referenz im INDX-Chunk
verwendet.
</li>
<li><b>RGB-Format</b>\n
Eine eindeutige Nummer, die das Farbformat der Grafikdaten angibt. Folgende Formate sind definiert:
- 9: A8R8G8B8
</li>
<li><b>Bittiefe</b>\n
Die Bittiefe eines einzelnen Farbwertes. Ist abhängig vom RGB-Format und in der Regel 32 Bit.
</li>
<li><b>Breite/Höhe</b>\n
Gibt Breite und Höhe der Grafik in Pixel an. Texturen unterliegen gewissen Einschränkungen, daher
sollten Höhe und Breite möglichst identisch und ein Vielfaches von 2 sein, also beispielsweise 32, 64,
128, 256, 512, 1024, 2048. Die meisten Grafikkarten unterstützen Texturen bis zu einer Größe von 4096 x 4096
Pixel.
</li>
</ul>

Die Pixel-Daten werden mit der Funktion ppl6::CCompression::CompressZlib komprimiert. Sie beginnen daher
mit einem 9-Byte langen Header, gefolgt von den eigentlichen Zlib-komprimierten Daten:
\code
Byte 0:  Komprimierunsart, ist immer 1 für Zlib       1 Byte
Byte 1:  Anzahl Bytes unkomprimiert (litte endian)    4 Byte
Byte 5:  Anzahl Bytes komprimiert (litte endian)      4 Byte
Byte 9:  Beginn des komprimierten Datenstroms
\endcode

 */




/*!\class Sprite
 * \ingroup PPLGroupGrafik
 * \brief Klasse zum Anzeigen von Sprites
 *
 * \desc
 * Diese Klasse kann zum Laden und Anzeigen von Sprites verwendet werden, die im
 * \ref PFPSpriteTexture1 "PFP-Sprite-Texture-Format" vorliegen.
 *
 * \since
 * Die Klasse wurde mit Version 6.2.4 eingeführt.
 */

/*!\var Sprite::TextureList
 * \brief Liste aller Texturen
 */

/*!\var Sprite::SpriteList
 * \brief Liste aller Sprites
 */


Sprite::Sprite()
/*!\brief Konstruktor der Klasse
 *
 * \desc
 * Dies ist der Konstruktor der Klasse. Gegenwärtig werden hier jedoch keinerlei Daten initialisiert.
 */
{
}

Sprite::~Sprite()
/*!\brief Destruktor der Klasse
 *
 * \desc
 * Dies ist der Destruktor der Klasse. Er ruft die Clear-Funktion auf, um sicherzustellen, dass sämtliche
 * Texturen und Speicherbereiche der Klasse wieder freigegeben werden.
 */
{
	clear();
}

void Sprite::clear()
/*!\brief Speicherbereiche der Klasse wieder freigeben
 *
 * \desc
 * Diese Funktion sorgt dafür, dass sämtlicher durch die Klasse allokierter Speicher und Texturen
 * wieder freigegeben werden.
 */
{
	TextureList.clear();
	SpriteList.clear();
}

const Drawable *Sprite::findTexture(int id) const
/*!\brief Textur anhand ihrer ID finden
 *
 * \desc
 * Diese Funktion wird intern aufgerufen, um einen Surface-Pointer auf eine bereits geladene Textur zu erhalten.
 * \param[in] id ID der gewünschten Texture
 * \returns Ist die Textur-ID vorhanden und bereits geladen, wird ein Pointer auf deren CSurface-Klasse
 * zurückgeliefert, andernfalls NULL.
 */
{
	try {
		const SpriteTexture &tx=TextureList.find(id);
		return &tx.surface;
	} catch (...) {
		throw InvalidSpriteException();
	}
	return NULL;
}

void Sprite::loadIndex(PFPChunk *chunk)
/*!\brief Laden des Sprite-Index
 *
 * \desc
 * Diese Funktion wird intern verwendet, um den INDX-Chunk der Sprite-Datei einzulesen.
 * Sie darf erst nach Aufruf von Sprite::LoadTexture verwendet werden.
 *
 * \param[in] chunk Pointer auf den INDX-Chunk
 * \returns Konnte der Index erfolgreich eingelesen und alle Texturen zugeordnet werden,
 * liefert die Funktion true (1) zurück, andernfalls false (0).
 */
{
	char *buffer=(char*)chunk->data();
	int num=Peek32(buffer);		// Anzahl Einträge in der Tabelle
	char *p=buffer+4;
	SpriteIndexItem item;
	for (int i=0;i<num;i++) {
		item.id=Peek32(p+0);
		item.surface=findTexture(Peek16(p+4));
		item.r.x1=Peek16(p+6+0);
		item.r.y1=Peek16(p+6+2);
		item.r.x2=Peek16(p+6+4)+1;
		item.r.y2=Peek16(p+6+6)+1;
		item.Pivot.x=Peek16(p+14+0);
		item.Pivot.y=Peek16(p+14+2);
		item.Offset.x=Peek16(p+18+0);
		item.Offset.y=Peek16(p+18+2);
		/*
		PrintDebug("Id: %i, (%i/%i)-(%i/%i)\n",
			item->id,item->r.left,item->r.top,item->r.right,item->r.bottom);
			*/
		SpriteList.add(item.id,item);
		p+=22;
	}
}

void Sprite::loadTexture(PFPChunk *chunk)
/*!\brief Laden eines Texture-Chunks
 *
 * \desc
 * Diese Funktion wird intern verwendet, um einen SURF-Chunk mit einer
 * Textur aus der Sprite-Datei zu laden.
 *
 * \param[in] chunk Pointer auf den zu ladenden SURF-Chunk
 * \param[in] flags Flags, mit dem das Texture-Surfaces erstellt werden soll.
 * Dies ist eine Kombination der Werte innerhalb der Enumeration ppl6::grafix::Surface::Flags
 * \returns Konnte die Textur erfolgreich geladen werden,
 * liefert die Funktion true (1) zurück, andernfalls false (0).
 */
{
	Compression Comp;
	Comp.usePrefix(Compression::Prefix_V2);
	char *buffer=(char*)chunk->data();
	// Textur anlegen
	SpriteTexture tex;
	// Zunächst lesen wir dem Header
	tex.id=Peek16(buffer+0);
	int rgbformat=Peek8(buffer+2);
	switch (rgbformat) {
		case 9: tex.rgbformat=RGBFormat::A8R8G8B8;
				break;
		default:
			throw UnsupportedColorFormatException();
	}
	tex.bitdepth=Peek8(buffer+3);
	tex.width=Peek16(buffer+4);
	tex.height=Peek16(buffer+6);
	// Nutzdaten dekomprimieren
	ByteArray uncompressed;
	Comp.uncompress(uncompressed,buffer+8,chunk->size()-8);
	buffer=(char*)uncompressed.ptr();
	// Nun erstellen wir ein neues Surface
	tex.surface.create(tex.width,tex.height,tex.rgbformat);
	for (int y=0;y<tex.height;y++) {
		for (int x=0;x<tex.width;x++) {
			if (tex.rgbformat==RGBFormat::A8R8G8B8) {
				tex.surface.putPixel(x,y,Color(Peek8(buffer+2),Peek8(buffer+1),Peek8(buffer),Peek8(buffer+3)));
				buffer+=4;
			}
		}
	}
	TextureList.add(tex.id,tex);
}

void Sprite::load(const String &filename)
/*!\brief Laden einer Textur-Datei
 *
 * \desc
 * Mit dieser Funktion kann eine Textur-Sprite-Datei geladen werden.
 *
 * \param[in] filename Name der zu ladenden Datei
 * \param[in] flags Flags mit der die Textur-Surfaces erstellt werden sollen. Dies ist eine Kombination der Werte
 * innerhalb der Enumeration ppl6::grafix::Surface::Flags
 * \returns Konnte die Datei erfolgreich geladern werden, gibt die Funktion 1 (true) zurück,
 * im Fehlerfall 0 (false). Im Fehlerfall wird ein entsprechender Fehlercode gesetzt.
 */
{
	File ff;
	ff.open(filename);
	load(ff);
}

void Sprite::load(FileObject &ff)
/*!\brief Laden einer Textur-Datei
 *
 * \desc
 * Mit dieser Funktion kann eine bereits geöffnete Textur-Sprite-Datei geladen werden.
 *
 * \param[in] ff Pointer auf die CFileObject-Klasse, die die Daten enthält.
 * \param[in] flags Flags mit der die Textur-Surfaces erstellt werden sollen. Dies ist eine Kombination der Werte
 * innerhalb der Enumeration ppl6::grafix::Surface::Flags
 * \returns Konnte die Datei erfolgreich geladern werden, gibt die Funktion 1 (true) zurück,
 * im Fehlerfall 0 (false). Im Fehlerfall wird ein entsprechender Fehlercode gesetzt.
 */
{
	PFPFile File;
	clear();
	File.load(ff);
	int major, minor;
	File.getVersion(&major,&minor);
	if (File.getID()!="TEXS" || major!=1 || minor!=0) throw InvalidSpriteException();
	// Texture Chunks laden
	PFPChunk *chunk;
	PFPFile::Iterator it;
	File.reset(it);
	while ((chunk=File.findNextChunk(it,"SURF"))) {
		loadTexture(chunk);
	}
	// Index Chunks laden
	File.reset(it);
	while ((chunk=File.findNextChunk(it,"INDX"))) {
		loadIndex(chunk);
	}
}

void Sprite::draw(Drawable &target, int x, int y, int id) const
/*!\brief Sprite darstellen
 *
 * \desc
 * Mit dieser Funktion wird ein beliebiges Sprite auf dem Ziel-Surface angezeigt.
 *
 * \param[in] target Ziel-Surface
 * \param[in] x X-Koordinate, bei der das Sprite gezeichnet werden soll
 * \param[in] y Y-Koordinate, bei der das Sprite gezeichnet werden soll
 * \param[in] id ID des anzuzeigenden Sprites
 * \returns Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	// Sprite im Index finden
	const SpriteIndexItem &item=SpriteList.find(id);

	target.bltAlpha(*item.surface,item.r,x+item.Offset.x-item.Pivot.x, y+item.Offset.y-item.Pivot.y);
}

int Sprite::numTextures() const
/*!\brief Anzahl Texturen in der Klasse
 *
 * \desc
 * Diese Funktion liefert die Anzahl durch die Klasse geladener Texturen zurück.
 * \returns Anzahl Texturen oder 0, wenn keine geladen sind.
 */
{
	return (int)TextureList.num();
}

int Sprite::numSprites() const
/*!\brief Anzahl Sprites in der Klasse
 *
 * \desc
 * Diese Funktion liefert die Anzahl durch die Klasse geladener Sprites zurück.
 * \returns Anzahl Sprites oder 0, wenn keine geladen sind.
 */
{
	return (int)SpriteList.num();
}



} // EOF namespace grafix
} // EOF namespace ppl7
