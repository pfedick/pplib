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
			--with-libiconv-prefix=/usr/local --with-nasm   \
			--with-jpeg --with-png --with-mpg123=/usr/local \
			--with-libgcrypt-prefix \
			--with-libgnutls-prefix \
			--with-ogg=/usr/local --with-libmicrohttpd=/usr/local
		;;
	MINGW32*:1.0.12*)
		export CPPFLAGS="-DCURL_STATICLIB -I/usr/local/include -I/sdk/WindowsSDK/include"
		export LDLAGS="-DCURL_STATICLIB -L/usr/local/lib -L/sdk/WindowsSDK/lib"
		export CFLAGS="-DCURL_STATICLIB"
		./configure --with-pcre=/mingw --with-bzip2=/mingw --with-zlib=/mingw \
			--with-nasm --with-libiconv-prefix=/mingw --with-libmad=/mingw \
			--with-lame=/mingw --with-jpeg=/mingw --with-png=/mingw \
			--with-libmcrypt-prefix=/mingw --with-libmhash=/mingw \
			--with-openssl=/mingw --with-libcurl \
			--with-ogg=/usr/local
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
		./configure --with-libmad --with-lame --with-pcre=/usr --with-x --with-openssl \
			--with-mysql=/usr  --with-libiconv-prefix --with-nasm \
			--with-libmhash --with-ogg=/usr \
			--with-postgresql=/usr/local/pgsql/bin/pg_config \
			--with-libgcrypt-prefix \
			--with-libgnutls-prefix \
			--with-sybase=/usr --with-jpeg --with-png -with-mpg123 --with-libmicrohttpd=/usr/local
		;;	
	*:Linux:*:*Ubuntu*)
		./configure --with-libmad --with-lame --with-pcre=/usr --with-x \
			--with-openssl --with-mysql=/usr  --with-libiconv-prefix --with-nasm \
			--with-sybase=/usr/local --with-postgresql=/usr/local/pgsql/bin/pg_config \
			--with-libgcrypt-prefix \
			--with-libgnutls-prefix \
			--with-jpeg --with-png --with-libmicrohttpd=/usr/local \
			--with-libmhash --with-mpg123 --with-ogg=/usr
		;;
	*)
		echo "Unknown host"
		;;
esac

