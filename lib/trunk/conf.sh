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
echo "PREFIX=$PREFIX"
echo ""

case "$SYS:$REL:$KERNEL" in
	FreeBSD:*)
		#export CC=gcc47
		#export CXX=g++47
        	export CPPFLAGS=-I/usr/local/include
		export LDLAGS=-L/usr/local/lib
		./configure --prefix=$PREFIX \
			--with-lame=/usr/local --with-mpg123=/usr/local \
			--with-pcre=/usr/local --with-x --with-openssl=/usr --with-mysql \
			--with-libmhash=/usr/local --with-gcrypt=auto \
			--with-libiconv-prefix=/usr/local --with-nasm  \
			--with-jpeg --with-libpng --with-libtiff=/usr/local \
			--with-postgresql \
			--with-libidn=/usr/local \
			--with-ogg=/usr/local \
			--with-libmicrohttpd=/usr/local \
			--enable-gtest=/usr/local/gtest-1.7.0
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
			--with-jpeg=/mingw --with-png=/mingw --with-libmhash=/mingw \
			--with-libmcrypt-prefix=/mingw --with-gcrypt=auto \
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
			--with-jpeg=/usr/local --with-libpng=/usr/local --with-libmhash=/usr/local \
			--with-libmcrypt-prefix=/usr/local --with-gcrypt=auto \
			--with-openssl=/usr/local --with-libcurl \
			--with-ogg=/usr/local
		;;
	Linux:*generic*)
		./configure --prefix=$PREFIX \
			--with-lame --with-pcre=/usr --with-x --with-openssl=/usr \
			--with-mysql=/usr  --with-libiconv-prefix --with-nasm \
			--with-libmhash --with-gcrypt=auto --with-ogg=/usr \
			--with-postgresql \
			--with-jpeg --with-libpng=$PREFIX \
			--with-libtiff=/usr --with-mpg123 \
			--enable-gtest=/usr/local/gtest-1.6.0
		;;	
	*:Linux:*:*Ubuntu*)
		./configure --prefix=$PREFIX \
			--with-lame --with-pcre=/usr --with-x \
			--with-openssl=/usr --with-mysql=/usr  --with-libiconv-prefix --with-nasm \
			--with-postgresql --with-gcrypt=auto \
			--with-jpeg --with-png \
			--with-libmhash --with-mpg123 --with-ogg=/usr
		;;
	*)
		echo "Unknown host"
		;;
esac

