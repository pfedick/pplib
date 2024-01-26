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

SYS=`uname -s`
REL=`uname -r`
KERNEL=`uname -v`

PREFIX=$1
PREFIX=${PREFIX:=$HOME}
echo "PREFIX=$PREFIX, $SYS:$REL:$KERNEL"
echo ""

case "$SYS:$REL:$KERNEL" in
	FreeBSD:1[12].*)
		CLANGVERSION=
		export CC=clang$CLANGVERSION
		export CXX=clang++$CLANGVERSION
		export CPPFLAGS=-I/usr/local/include
		export LDLAGS=-L/usr/local/lib
		export GCOV_CFLAGS="-fprofile-arcs -ftest-coverage"
		export GCOV_LDFLAGS="-lgcov"
		./configure --prefix=$PREFIX \
			--with-lame=/usr/local --with-mpg123=/usr/local \
			--with-pcre=/usr/local --with-x --with-mysql \
			--with-openssl=/usr \
			--with-libiconv-prefix=/usr/local --with-nasm  \
			--with-libjpegturbo=/usr/local --with-libpng --with-libtiff=/usr/local \
			--with-postgresql \
			--with-libidn=/usr/local \
			--with-ogg=/usr/local \
			--with-libmicrohttpd=/usr/local \
			--enable-gtest=/usr/local/googletest-release-1.10.0

		;;
	MINGW32*:1.0.16*)
		export CPPFLAGS="-DCURL_STATICLIB -I/usr/local/include -I/sdk/WindowsSDK/include"
		export LDLAGS="-DCURL_STATICLIB -L/usr/local/lib -L/sdk/WindowsSDK/lib"
		export CFLAGS="-DCURL_STATICLIB"
		echo "Building for MinGW 1.0.16 - 32 Bit"

			#		preprocessor definitions will be in effect:
			# * #define _WIN64 1 (also WIN64, __WIN64, and __WIN64__)
			# * #define __MINGW64__ 1
			# * #define __x86_64 1 (also __x86_64__)
			# * #define __amd64 1 (also __amd64__)
			#Be sure to use "-m32" or "-m64" at both the compile stage and the link stage.

		./configure --prefix=$PREFIX \
			--build=i686-w64-mingw32 \
			--with-pcre=/mingw --with-bzip2=/mingw --with-zlib=/mingw \
			--with-nasm --with-libiconv-prefix=/mingw \
			--with-lame=/mingw --with-mpg123=/mingw \
			--with-jpeg=/mingw --with-png=/mingw \
			--with-gcrypt=auto \
			--with-openssl=/mingw --with-libcurl \
			--with-ogg=/mingw
		;;
	MINGW32*)
		export CPPFLAGS="-DCURL_STATICLIB -I/usr/local/include -I/sdk/WindowsSDK/include"
		export LDLAGS="-DCURL_STATICLIB -L/usr/local/lib -L/sdk/WindowsSDK/lib"
		export CFLAGS="-DCURL_STATICLIB"
		./configure --prefix=$PREFIX \
			--with-pcre=/usr/local --with-bzip2=/usr/local --with-zlib=/usr/local \
			--with-nasm --with-libiconv-prefix=/usr/local \
			--with-lame=/usr/local --with-mpg123=/usr/local \
			--with-jpeg=/usr/local --with-libpng=/usr/local \
			--with-gcrypt=auto \
			--with-openssl=/usr/local --with-libcurl \
			--with-ogg=/usr/local
		;;
	MINGW64*)
		export CPPFLAGS="-DCURL_STATICLIB -I/usr/local/include -I/sdk/WindowsSDK/include"
		export LDLAGS="-DCURL_STATICLIB -L/usr/local/lib -L/sdk/WindowsSDK/lib"
		export CFLAGS="-DCURL_STATICLIB"
		./configure --prefix=$PREFIX \
		    --without-postgresql --without-mysql \
		    --enable-gtest=/usr/src/gtest-1.7.0 \
		;;
	MSYS_NT-10*)
		if [ -d /jenkins/local/bin ] ; then
			PREFIX=/jenkins/local
		fi
		echo "configuring for MSYS2 MINGW64, PREFIX=$PREFIX"
		MINGW=/mingw64
		#export CPPFLAGS="-DCURL_STATICLIB -DPCRE_STATIC -I$PREFIX/include -I/usr/local/include -I/sdk/WindowsSDK/include"
		#export LDLAGS="-DCURL_STATICLIB -L$PREFIX/lib -L/usr/local/lib -L/sdk/WindowsSDK/lib"
		#export CFLAGS="-DCURL_STATICLIB -DPCRE_STATIC"
		#export SQLITE_CFLAGS="-I$PREFIX/include -I/usr/local/include"
		#export SQLITE_LIBS="-L$PREFIX/include -L/usr/local/lib -lsqlite3"
		export CFLAGS="-I/mingw64/include/freetype2 "
		export CPPFLAGS="-I/mingw64/include/freetype2 "
		export LDFLAGS="-L/mingw64/lib "
		export LIBS="-lfreetype"
		./configure --prefix=$PREFIX \
			--with-pcre=$MINGW --with-bzip2=$MINGW --with-zlib=$MINGW \
			--with-nasm --with-libiconv-prefix=$MINGW \
			--with-lame --with-mpg123 --with-ogg \
			--with-ft-prefix=$MINGW \
			--with-libtiff=$MINGW \
			--with-libjpegturbo=$MINGW --with-libpng=$MINGW \
			--with-openssl=$MINGW \
			--with-libldns=$MINGW --with-libidn=$MINGW \
		    --without-postgresql --without-mysql \
		    --with-sdl-prefix=no \
		    --enable-gtest=/usr/local/gtest-1.7.0 \

		;;
	Linux:*)

	    # export CFLAGS="-no-pie "
		echo "configuring for generic Linux"
		./configure --prefix=$PREFIX \
			--with-lame --with-pcre=/usr --with-x --with-openssl=/usr \
			--with-mysql  --with-libiconv-prefix --without-nasm \
			--with-gcrypt=auto --with-ogg=/usr \
			--with-postgresql \
			--with-libmicrohttpd=/usr/local \
			--with-jpeg --with-libpng=$PREFIX --with-libtiff \
			--with-libtiff=/usr --with-mpg123 \
            --with-libidn --with-libidn2 --with-libldns=/usr \
			--enable-gtest=/usr/local/googletest-release-1.7.0
		;;
	*)
		echo "Unknown host"
		;;
esac
