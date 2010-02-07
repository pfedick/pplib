#!/bin/sh

genfontfile() {
	rm -f $FILE
	font5maker -t $FILE -q $FONTPATH/$REGULAR -s $SIZE --mono1 -n $NAME --zlib
	font5maker -t $FILE -q $FONTPATH/$REGULAR -s $SIZE --aa2 -n $NAME --zlib
	font5maker -t $FILE -q $FONTPATH/$BOLD -s $SIZE --mono1 --isbold -n $NAME --zlib
	font5maker -t $FILE -q $FONTPATH/$BOLD -s $SIZE --aa2 --isbold -n $NAME --zlib
	font5maker -t $FILE -q $FONTPATH/$ITALIC -s $SIZE --mono1 --isitalic -n $NAME --zlib
	font5maker -t $FILE -q $FONTPATH/$ITALIC -s $SIZE --aa2 --isitalic -n $NAME --zlib
	font5maker -t $FILE -q $FONTPATH/$BOLDITALIC -s $SIZE --mono1 --isbold --isitalic -n $NAME --zlib
	font5maker -t $FILE -q $FONTPATH/$BOLDITALIC -s $SIZE --aa2 --isbold --isitalic -n $NAME --zlib
}


SIZE="8,10,12,14,16,18,20"
FONTPATH=/usr/share/fonts/truetype/ttf-liberation
REGULAR=LiberationSans-Regular.ttf
BOLD=LiberationSans-Bold.ttf
ITALIC=LiberationSans-Italic.ttf
BOLDITALIC=LiberationSans-BoldItalic.ttf
NAME="PPL Liberation Sans"
FILE=liberationsans2.fnt5

genfontfile

# Monospace-Fonts, ohne Italic
SIZE="8,10,12,14"
FONTPATH=/usr/share/fonts/truetype/ttf-liberation
REGULAR=LiberationMono-Regular.ttf
BOLD=LiberationMono-Bold.ttf
ITALIC=LiberationMono-Italic.ttf
BOLDITALIC=LiberationMono-BoldItalic.ttf
NAME="PPL Liberation Mono"
FILE=liberationmono2.fnt5

font5maker -t $FILE -q $FONTPATH/$REGULAR -s $SIZE --mono1 -n $NAME --zlib
font5maker -t $FILE -q $FONTPATH/$REGULAR -s $SIZE --aa2 -n $NAME --zlib
font5maker -t $FILE -q $FONTPATH/$BOLD -s $SIZE --mono1 --isbold -n $NAME --zlib
font5maker -t $FILE -q $FONTPATH/$BOLD -s $SIZE --aa2 --isbold -n $NAME --zlib


