#!/bin/sh
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

