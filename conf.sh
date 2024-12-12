#!/bin/sh
###############################################################################
# This file is part of "Patrick's Programming Library", Version 7 (PPL7).
# Web: https://github.com/pfedick/pplib
###############################################################################
# Copyright (c) 2024, Patrick Fedick <patrick@pfp.de>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#    1. Redistributions of source code must retain the above copyright notice,
#       this list of conditions and the following disclaimer.
#    2. Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGE.
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
			--with-pcre2=/usr/local --with-x --with-mysql \
			--with-openssl=/usr \
			--with-libiconv-prefix=/usr/local --with-nasm  \
			--with-libjpegturbo=/usr/local --with-libpng --with-libtiff=/usr/local \
			--with-postgresql \
			--with-libidn=/usr/local \
			--with-ogg=/usr/local \
			--with-libmicrohttpd=/usr/local \
			--enable-gtest=/usr/local/googletest-release-1.10.0

		;;
	MINGW64*)
		export CPPFLAGS="-DCURL_STATICLIB -I/usr/local/include -I/sdk/WindowsSDK/include"
		export LDLAGS="-DCURL_STATICLIB -L/usr/local/lib -L/sdk/WindowsSDK/lib"
		export CFLAGS="-DCURL_STATICLIB"
		./configure --prefix=$PREFIX \
		    --without-postgresql --without-mysql \
		    --enable-gtest \
			--with-freetype2=/ucrt64 \
			--without-sdl-prefix

		;;

	Linux:*)

	    # export CFLAGS="-no-pie "
		echo "configuring for generic Linux"
		./configure --prefix=$PREFIX \
			--with-lame --with-pcre2 --with-x --with-openssl=/usr \
			--with-mysql  --with-libiconv-prefix --without-nasm \
			--with-gcrypt=auto --with-ogg=/usr \
			--with-postgresql \
			--with-libmicrohttpd=/usr/local \
			--with-jpeg --with-libpng=$PREFIX --with-libtiff \
			--with-libtiff=/usr --with-mpg123 \
            --with-libidn --with-libidn2 --with-libldns=/usr \
			--enable-gtest=/usr/local/googletest-release-1.8.1
		;;
	*)
		echo "Unknown host"
		;;
esac
