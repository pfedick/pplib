#!/bin/sh

SYS=`uname -s`
REL=`uname -r`
KERNEL=`uname -v`
PREFIX=$1
PREFIX=${PREFIX:=$HOME}

echo "PREFIX=$PREFIX"
echo ""

case "$SYS:$REL:$KERNEL" in
	FreeBSD:*)
        export CPPFLAGS=-I/usr/local/include
		export LDLAGS=-L/usr/local/lib
		./configure --prefix=$PREFIX \
			--with-libmad=/usr/local --with-lame=/usr/local \
			--with-pcre=/usr/local --with-x --with-openssl=/usr --with-mysql \
			--with-libmhash=/usr/local --with-freetds=/usr/local \
			--with-libiconv-prefix=/usr/local --with-nasm   \
			--with-jpeg --with-png --with-mpg123=/usr/local \
			--with-libgcrypt-prefix \
			--with-libgnutls-prefix \
			--with-postgresql=/usr/local \
			--with-ogg=/usr/local --with-libmicrohttpd=/usr/local \
			--enable-gtest=/usr/local/gtest-1.6.0
		;;
	MINGW32*:1.0.12*)
		export CPPFLAGS="-DCURL_STATICLIB -I/usr/local/include -I/sdk/WindowsSDK/include"
		export LDLAGS="-DCURL_STATICLIB -L/usr/local/lib -L/sdk/WindowsSDK/lib"
		export CFLAGS="-DCURL_STATICLIB"
		./configure --prefix=$PREFIX \
			--with-pcre=/mingw --with-bzip2=/mingw --with-zlib=/mingw \
			--with-nasm --with-libiconv-prefix=/mingw --with-libmad=/mingw \
			--with-lame=/mingw --with-jpeg=/mingw --with-png=/mingw \
			--with-libmcrypt-prefix=/mingw --with-libmhash=/mingw \
			--with-openssl=/mingw --with-libcurl \
			--with-ogg=/usr/local
		;;
	MINGW32*)
		echo "MinGW Configuration"
		export CPPFLAGS="-DCURL_STATICLIB -I/usr/local/include -I/sdk/WindowsSDK/include"
		export LDLAGS="-DCURL_STATICLIB -L/usr/local/lib -L/sdk/WindowsSDK/lib"
		export CFLAGS="-DCURL_STATICLIB"
		export SQLITE_CFLAGS="-I/usr/local/include"
		export SQLITE_LIBS="-L/usr/local/lib -lsqlite3"
		./configure --prefix=$PREFIX \
			--with-pcre=/usr/local --with-bzip2=/usr/local --with-zlib=/usr/local \
			--with-nasm --with-libiconv-prefix=/usr/local --with-libmad=/usr/local \
			--with-postgresql=/usr/local/pgsql/bin/pg_config \
			--without-mysql \
			--with-sqlite=/usr/local \
			--with-lame=/usr/local --with-mpg123=/usr/local \
			--with-jpeg=/usr/local --with-png=/usr/local --with-libmhash=/usr/local \
			--with-libmcrypt-prefix=/usr/local \
			--with-openssl=/usr/local --with-libcurl \
			--with-ogg=/usr/local \
			--without-libmicrohttpd --without-libgnutls-prefix \
			--disable-sdltest --without-sdl-prefix --without-sdl-exec-prefix
		;;
	Linux:*generic*)
		./configure --prefix=$PREFIX \
			--with-libmad --with-lame --with-pcre=/usr --with-x \
			--with-mysql=/usr  --with-libiconv-prefix --with-nasm \
			--with-libmhash --with-ogg=/usr \
			--with-postgresql --with-libldns \
			--with-libgcrypt-prefix \
			--with-libgnutls-prefix=/usr \
			--enable-gtest=/usr/local/gtest-1.6.0 \
			--with-freetds --with-jpeg --with-png -with-mpg123 --with-libmicrohttpd=/home/patrickf
		;;	
	*:Linux:*:*Ubuntu*)
		./configure --prefix=$PREFIX \
			--with-libmad --with-lame --with-pcre=/usr --with-x \
			--with-mysql=/usr  --with-libiconv-prefix --with-nasm \
			--with-freetds=/usr/local --with-postgresql=/usr/local/pgsql/bin/pg_config \
			--with-libgcrypt-prefix \
			--with-libgnutls-prefix \
			--with-jpeg --with-png --with-libmicrohttpd=/usr/local \
			--with-libmhash --with-mpg123 --with-ogg=/usr
		;;
	*)
		echo "Unknown host"
		;;
esac

