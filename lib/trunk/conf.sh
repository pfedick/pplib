#!/bin/sh

SYS=`uname -s`
REL=`uname -r`
KERNEL=`uname -v`

case "$SYS:$REL:$KERNEL" in
	FreeBSD:*)
        export CPPFLAGS=-I/usr/local/include
		export LDLAGS=-L/usr/local/lib
		./configure --with-libmad=/usr/local --with-lame=/usr/local \
			--with-pcre=/usr/local --with-x --with-openssl --with-mysql \
			--with-libmhash=/usr/local --with-sybase=/usr/local \
			--with-libiconv-prefix=/usr/local --with-nasm --with-pgsql \
			--with-jpeg --with-png --with-mpg123=/usr/local
		;;
	MINGW32*:1.0.12*)
		export CPPFLAGS="-DCURL_STATICLIB -I/usr/local/include -I/sdk/WindowsSDK/include"
		export LDLAGS="-DCURL_STATICLIB -L/usr/local/lib -L/sdk/WindowsSDK/lib"
		export CFLAGS="-DCURL_STATICLIB"
		./configure --with-pcre=/mingw --with-bzip2=/mingw --with-zlib=/mingw \
			--with-nasm --with-libiconv-prefix=/mingw --with-libmad=/mingw \
			--with-lame=/mingw --with-jpeg=/mingw --with-png=/mingw \
			--with-libmcrypt-prefix=/mingw --with-libmhash=/mingw \
			--with-openssl=/mingw --with-libcurl
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
			--with-openssl=/mingw --with-libcurl
		;;
	Linux:*generic*)
		./configure --with-libmad --with-lame --with-pcre=/usr --with-x --with-openssl \
			--with-mysql=/usr  --with-libiconv-prefix --with-nasm \
			--with-libmhash \
			--with-sybase=/usr --with-jpeg --with-png -with-mpg123
		;;	
	*:Linux:*:*Ubuntu*)
		./configure --with-libmad --with-lame --with-pcre=/usr --with-x \
			--with-openssl --with-mysql=/usr  --with-libiconv-prefix --with-nasm \
			--with-sybase=/usr/local --with-jpeg --with-png \
			--with-libmhash --with-mpg123
		;;
	*)
		echo "Unknown host"
		;;
esac

