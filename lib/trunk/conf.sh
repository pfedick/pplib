#!/bin/sh

SYS=`uname -s`
REL=`uname -r`
KERNEL=`uname -v`

case "$SYS:$REL:$KERNEL" in
	FreeBSD:*)
        export CPPFLAGS=-I/usr/local/include
		export LDLAGS=-L/usr/local/lib
		./configure --with-libmad=/usr/local --with-lame=/usr/local \
			--with-pcre=/usr/local --with-x --with-openssl=/usr --with-mysql \
			--with-libmhash=/usr/local --with-sybase=/usr/local \
			--with-libiconv-prefix=/usr/local --with-nasm  \
			--with-jpeg --with-png --with-libtiff=/usr/local \
			--with-mpg123=/usr/local \
			--with-ogg=/usr/local \
			--with-libmicrohttpd=/usr/local
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
		
		./configure \
			--prefix=/mingw \
			--build=i686-w64-mingw32 \
			--with-pcre=/mingw --with-bzip2=/mingw --with-zlib=/mingw \
			--with-nasm --with-libiconv-prefix=/mingw --with-libmad=/mingw \
			--with-lame=/mingw --with-mpg123=/mingw \
			--with-jpeg=/mingw --with-png=/mingw --with-libmhash=/mingw \
			--with-libmcrypt-prefix=/mingw \
			--with-openssl=/mingw --with-libcurl \
			--with-ogg=/mingw
		;;
	MINGW32*)
		export CPPFLAGS="-DCURL_STATICLIB -I/usr/local/include -I/sdk/WindowsSDK/include"
		export LDLAGS="-DCURL_STATICLIB -L/usr/local/lib -L/sdk/WindowsSDK/lib"
		export CFLAGS="-DCURL_STATICLIB"
		./configure --with-pcre=/usr/local --with-bzip2=/usr/local --with-zlib=/usr/local \
			--with-nasm --with-libiconv-prefix=/usr/local --with-libmad=/usr/local \
			--with-lame=/usr/local --with-mpg123=/usr/local \
			--with-jpeg=/usr/local --with-png=/usr/local --with-libmhash=/usr/local \
			--with-libmcrypt-prefix=/usr/local \
			--with-openssl=/mingw --with-libcurl \
			--with-ogg=/usr/local
		;;
	Linux:*generic*)
		./configure --with-libmad --with-lame --with-pcre=/usr --with-x --with-openssl=/usr \
			--with-mysql=/usr  --with-libiconv-prefix --with-nasm \
			--with-libmhash --with-ogg=/usr \
			--with-sybase=/usr --with-jpeg --with-png \
			--with-libtiff=/usr --with-mpg123
		;;	
	*:Linux:*:*Ubuntu*)
		./configure --with-libmad --with-lame --with-pcre=/usr --with-x \
			--with-openssl=/usr --with-mysql=/usr  --with-libiconv-prefix --with-nasm \
			--with-sybase=/usr/local --with-jpeg --with-png \
			--with-libmhash --with-mpg123 --with-ogg=/usr
		;;
	*)
		echo "Unknown host"
		;;
esac

