#!/bin/sh
#
###############################################################################
# This file is part of "Patrick's Programming Library", Version 7 (PPL7).
# Web: http://www.pfp.de/ppl/
#
# $Author$
# $Revision$
# $Date$
# $Id$
#
###############################################################################
#
# Copyright (c) 2013, Patrick Fedick <patrick@pfp.de>
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright notice,
#      this list of conditions and the following disclaimer. 
#   2. Redistributions in binary form must reproduce the above copyright notice,
#      this list of conditions and the following disclaimer in the documentation
#      and/or other materials provided with the distribution. 
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
###############################################################################
SYSTEM=`uname -s`
FORMAT=-6
SUFFIX=fnt6
ANTIALIASING=--aa4

XORG_FONTPATH=/usr/local/lib/X11/fonts


genfontfile() {
	rm -f $FILE
	pplfontmaker -t $FILE -q $FONTPATH/$REGULAR -s $SIZE --mono1 -n $NAME --zlib $FORMAT
	if [ $? -ne 0 ] ;then exit; fi
	pplfontmaker -t $FILE -q $FONTPATH/$REGULAR -s $SIZE $ANTIALIASING -n $NAME --zlib $FORMAT
	if [ $? -ne 0 ] ;then exit; fi
	pplfontmaker -t $FILE -q $FONTPATH/$BOLD -s $SIZE --mono1 --isbold -n $NAME --zlib $FORMAT
	if [ $? -ne 0 ] ;then exit; fi
	pplfontmaker -t $FILE -q $FONTPATH/$BOLD -s $SIZE $ANTIALIASING --isbold -n $NAME --zlib $FORMAT
	if [ $? -ne 0 ] ;then exit; fi
	pplfontmaker -t $FILE -q $FONTPATH/$ITALIC -s $SIZE --mono1 --isitalic -n $NAME --zlib $FORMAT
	if [ $? -ne 0 ] ;then exit; fi
	pplfontmaker -t $FILE -q $FONTPATH/$ITALIC -s $SIZE $ANTIALIASING --isitalic -n $NAME --zlib $FORMAT
	if [ $? -ne 0 ] ;then exit; fi
	pplfontmaker -t $FILE -q $FONTPATH/$BOLDITALIC -s $SIZE --mono1 --isbold --isitalic -n $NAME --zlib $FORMAT
	if [ $? -ne 0 ] ;then exit; fi
	pplfontmaker -t $FILE -q $FONTPATH/$BOLDITALIC -s $SIZE $ANTIALIASING --isbold --isitalic -n $NAME --zlib $FORMAT
	if [ $? -ne 0 ] ;then exit; fi
}

genmonospace() {
	rm -f $FILE
	pplfontmaker -t $FILE -q $FONTPATH/$REGULAR -s $SIZE --mono1 -n $NAME --zlib $FORMAT
	if [ $? -ne 0 ] ;then exit; fi
	pplfontmaker -t $FILE -q $FONTPATH/$REGULAR -s $SIZE $ANTIALIASING -n $NAME --zlib $FORMAT
	if [ $? -ne 0 ] ;then exit; fi
	pplfontmaker -t $FILE -q $FONTPATH/$BOLD -s $SIZE --mono1 --isbold -n $NAME --zlib $FORMAT
	if [ $? -ne 0 ] ;then exit; fi
	pplfontmaker -t $FILE -q $FONTPATH/$BOLD -s $SIZE $ANTIALIASING --isbold -n $NAME --zlib $FORMAT
	if [ $? -ne 0 ] ;then exit; fi
}

###################################################################################################
# Liberation Sans
SIZE="8,10,12,14,16,18,20"
FONTPATH=/usr/share/fonts/truetype/ttf-liberation
if [ "$SYSTEM" = "FreeBSD" ] ; then FONTPATH="/usr/local/lib/X11/fonts/Liberation"; fi
REGULAR=LiberationSans-Regular.ttf
BOLD=LiberationSans-Bold.ttf
ITALIC=LiberationSans-Italic.ttf
BOLDITALIC=LiberationSans-BoldItalic.ttf
NAME="Liberation Sans"
FILE=liberationsans2.$SUFFIX
ANTIALIASING=--aa2
genfontfile

# Liberation Mono
# Monospace-Fonts, ohne Italic
SIZE="8,10,12,14"
FONTPATH=/usr/share/fonts/truetype/ttf-liberation
if [ "$SYSTEM" = "FreeBSD" ] ; then FONTPATH="/usr/local/lib/X11/fonts/Liberation"; fi
REGULAR=LiberationMono-Regular.ttf
BOLD=LiberationMono-Bold.ttf
ITALIC=LiberationMono-Italic.ttf
BOLDITALIC=LiberationMono-BoldItalic.ttf
NAME="Liberation Mono"
FILE=liberationmono2.$SUFFIX
ANTIALIASING=--aa2
genmonospace

