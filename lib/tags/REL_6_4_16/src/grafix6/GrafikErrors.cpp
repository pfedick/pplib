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
#include "ppl6.h"
#include "ppl6-grafix.h"


namespace ppl6 {
namespace grafix {

using namespace ppl6;

static const char * errors[] = {
        "ppl6::grafix: Initialisierung der Engine fehlgeschlagen",					// 1000
        "ppl6::grafix: CGrafix ist bereits initialisiert",							// 1001
        "ppl6::grafix: Grafik-Engine muss zuerst initialisiert werden",				// 1002
        "ppl6::grafix: Filter konnte nicht hinzugefügt werden",						// 1003
        "ppl6::grafix: Filter konnte nicht gelöscht werden",						// 1004
        "ppl6::grafix: Basisklasse CFilter kann keine Grafikformate verarbeiten",	// 1005
        "ppl6::grafix: Filter wurde nicht gefunden",								// 1006
        "ppl6::grafix: Funktion wird von dieser Engine nicht unterstützt",			// 1007
        "ppl6::grafix: Fenster konnte nicht erstellt werden",						// 1008
        "ppl6::grafix: Surface konnte nicht gelocked werden",						// 1009
        "ppl6::grafix: Surface konnte nicht entsperrt werden",						// 1010
        "ppl6::grafix: Surface konnte nicht rotiert werden (flip)",					// 1011
        "ppl6::grafix: Grafikoperation wird nicht unterstützt",						// 1012
        "ppl6::grafix: Farbformat wird nicht unterstützt",							// 1013
        "ppl6::grafix: Grafikoperation fehlgeschlagen",								// 1014
        "ppl6::grafix: Quellsurface nicht angegeben",								// 1015
        "ppl6::grafix: Blitting liegt ausserhalb des Clipping-Bereichs",			// 1016
        "ppl6::grafix: Kein Filter für dieses Format gefunden",						// 1017
        "ppl6::grafix: Format wird von diesem Filter nicht unterstützt",			// 1018
        "ppl6::grafix: Farbformat des Quellsurface wird nicht unterstützt",			// 1019
        "ppl6::grafix: Filter unterstützt kein Speichern",							// 1020
        "ppl6::grafix: Fehler beim lesen der Grafikdatei",							// 1021
        "ppl6::grafix: FontEngine konnte nicht hinzugefügt werden",					// 1022
        "ppl6::grafix: Font konnte nicht geladen werden",							// 1023
        "ppl6::grafix: Basisklasse CFontEngine kann keine Fonts verarbeiten",		// 1024
        "ppl6::grafix: Keine passende Engine für diesen Font gefunden",				// 1025
        "ppl6::grafix: Fontengine wird bereits von einer anderen GrafikEngine verwendet",	// 1026
        "ppl6::grafix: Format wird von dieser FontEngine nicht unterstützt",		// 1027
        "ppl6::grafix: Fontdatei enthält keinen Namen",								// 1028
        "ppl6::grafix: Font wurde nicht von dieser Engine geladen",					// 1029
        "ppl6::grafix: Kein Font ausgewählt",										// 1030
        "ppl6::grafix: Fonts können zur Zeit nicht gelöscht werden",				// 1031
        "ppl6::grafix: Unbekannter Font",											// 1032
        "ppl6::grafix: Font ist nicht initialisiert",								// 1033
        "ppl6::grafix: Fonteigenschaften nicht gefunden",							// 1034
        "ppl6::grafix: FreeType-Library ist nicht eingebunden",						// 1035
        "ppl6::grafix: FontEngine ist bereits Initialisiert",						// 1036
        "ppl6::grafix: FontEngine ist nicht initialisiert",							// 1037
        "ppl6::grafix: FontEngine konnte nicht initialisiert werden",				// 1038
        "ppl6::grafix: Es darf nur eine Instanz von CGrafix initialisiert werden",	// 1039
        "ppl6::grafix: Es ist noch keine Instanz von CGrafix initialisiert",		// 1040
        "ppl6::grafix: Widget hat kein Surface",									// 1041
        "ppl6::grafix: Unbekannte DrawMethod",										// 1042
        "ppl6::grafix: ImageList nicht geladen",									// 1043
        "ppl6::grafix: Image nicht in ImageList",									// 1044
        "ppl6::grafix: Keine Images in der ImageList geladen",						// 1045
        "ppl6::grafix: Kein Image geladen",											// 1046
		"ppl6::grafix: Grafik-Engine ist bereits initialisiert",					// 1047
		"ppl6::grafix: Fenster wurde bereits erstellt",								// 1048
		"ppl6::grafix: Engine nicht verfügbar",										// 1049
		"ppl6::grafix: Surface konnte nicht erstellt werden",						// 1050
		"ppl6::grafix: Farbformat wurde nicht angegeben",							// 1051
		"ppl6::grafix: Kein Primary Surface vorhanden, bitte zuerst ein Fenster erstellen",	// 1052
		"ppl6::grafix: Surface hat keinen Backbuffer zum locken",					// 1053
		"ppl6::grafix: Scene konnte nicht gestartet werden",						// 1054
		"ppl6::grafix: Kein Sprite-Interface vorhanden",							// 1055
		"ppl6::grafix: Fehler im Sprite-Interface",									// 1056
		"ppl6::grafix: Textur ist zu gross",										// 1057
		"ppl6::grafix: Keine gültige Sprite-Datei",									// 1058
		"ppl6::grafix: Sprite-Datei fehlerhaft, Laden nicht möglich",				// 1059
		"ppl6::grafix: SpriteId nicht in Spriteklasse vorhanden",					// 1060
		"ppl6::grafix: Surface wurde bereits erstellt",								// 1061
		"ppl6::grafix: Es gibt bereits eine andere Instanz dieser Grafik-Engine",	// 1062
		"ppl6::grafix: Keine Grafik-Adapter installiert",							// 1063
		"ppl6::grafix: Ungültiger Grafik-Adapter",									// 1064
		"ppl6::grafix: Unbekanntes Farbformat",										// 1065
		"ppl6::grafix: Grafik-Modus ist ungültig oder steht nicht zur Verfügung",	// 1066
		"ppl6::grafix: Quellgrafik ist ungültig",									// 1067
		"ppl6::grafix: Keine GFXEngine angegeben",									// 1068
		"ppl6::grafix: Grafik enthält keine Daten",									// 1069
		"ppl6::grafix: Koordinaten sind ausserhalb des Zeichenbereichs",			// 1070
		"ppl6::grafix: Keine Grafik-Engine verfügbar",								// 1071
		"ppl6::grafix: Surface/Widget ist nicht mit einer GFXEngine verbunden",		// 1072
		"ppl6::grafix: Surface/Widget wurde nicht mit dieser GFXEngine erstellt",	// 1073
		"ppl6::grafix: Ungültiges Surface, keine Engine-spezifischen Daten",		// 1074
		"ppl6::grafix: Unbekannter Surface-Type",									// 1075
		"ppl6::grafik: Ungültige Breite oder Höhe der Grafik",						// 1076
		"unknown"											//
};

static const char *gfxerrors(int code)
{
	int max=sizeof(errors) / sizeof(char *)-2;
	if (code<1000 || code>1000+max) return "Unknown Error";
	code-=1000;
	return (errors[code]);
}


void CGrafix::InitErrors()
{
	ppl6::AttachErrorHandler(gfxerrors,1000,1999);
}


} // EOF namespace grafix
} // EOF namespace ppl6
