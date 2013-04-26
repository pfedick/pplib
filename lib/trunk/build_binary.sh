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


MYPWD=`pwd`
WORK=$MYPWD/tmp
ROOT=$MYPWD/tmp/build
VERSION="7.0.0"
NAME="PPLib"
HOMEPAGE="http://www.pfp.de/ppl"
MAINTAINER="Patrick Fedick <patrick@pfp.de>"
DESCRIPTION="C++ programming library"
COMMENT="C++ programming library"
DISTFILES=$MYPWD/distfiles
MAKE=make

if [ -f /etc/lsb-release ] ; then
	. /etc/lsb-release
elif [ -f /etc/system-release ] ; then
	DISTRIB_ID=`cat /etc/system-release | awk '{print $1}'`
	DISTRIB_RELEASE=`cat /etc/system-release | awk '{print $3}'`
else
	DISTRIB_ID=`uname -s`
	DISTRIB_RELEASE=`uname -r`

	case "$DISTRIB_ID:$DISTRIB_RELEASE" in
		MINGW32*:1.0.11*)
			DISTRIB_ID="MINGW32";
			DISTRIB_RELEASE="1.0.11";
			TESTGUI=TestGUI.exe
			;;
	esac
fi

# Wir haben nun unter anderem:
#    DISTRIB_ID=Ubuntu
#    DISTRIB_RELEASE=9.10

echo "Baue $NAME $VERSION für: $DISTRIB_ID, $DISTRIB_RELEASE..."
echo ""
rm -rf tmp



build_sources() {
mkdir -p tmp/build
if [ ! -d tmp/build ] ; then
	echo "Could not create temporary directory tmp/build"
	exit 1
fi

cd tmp/build
#rm -rf *
mkdir -p include lib bin src
cd src
mkdir -p ppl6 libdenictest testgui

echo "Kopiere PPL6 in temporäres Build-Verzeichnis"
cd $PPL6SOURCE
if [ $? -ne 0 ] ; then
	echo "PPL6 not found in $PPL6SOURCE"
	exit 1
fi

find *.m4 conf.sh configure Doxyfile *.TXT *.in *.ico VERSION autoconf config docs include resource src tools tests | cpio -pdmv $ROOT/src/ppl6

echo "Kopiere libdenictest in temporäres Build-Verzeichnis"
cd $LIBDENICTESTSOURCE
if [ $? -ne 0 ] ; then
	echo "libdenictest not found in $LIBDENICTESTSOURCEt"
	exit 1
fi
find autoconf docs include src *.m4 configure *.in Doxyfile HISTORY mingw.sh | cpio -pdmv $ROOT/src/libdenictest


echo "Kopiere testgui in temporäres Build-Verzeichnis"
cd $TESTGUISOURCE
if [ $? -ne 0 ] ; then
	echo "testgui not found in $TESTGUISOURCE"
	exit 1
fi
find setup.iss forms help include resources src HISTORY *.rc *.qrc | cpio -pdmv $ROOT/src/testgui
cat TestGUI.pro | sed -e "s/--libs/--archive/" > $ROOT/src/testgui/TestGUI.pro

cd $ROOT
mkdir -p DENIC-ITStt-TestGUI-$VERSION-src-complete/build
cd DENIC-ITStt-TestGUI-$VERSION-src-complete/build
mkdir -p include lib bin src
cd $ROOT

cp $TESTGUISOURCE/build_binary.sh DENIC-ITStt-TestGUI-$VERSION-src-complete

mv src DENIC-ITStt-TestGUI-$VERSION-src-complete/build
tar -czf $MYPWD/distfiles/DENIC-ITStt-TestGUI-$VERSION-src-complete.tar.gz --exclude .svn DENIC-ITStt-TestGUI-$VERSION-src-complete
mv DENIC-ITStt-TestGUI-$VERSION-src-complete/build/src ./
rm -rf DENIC-ITStt-TestGUI-$VERSION-src-complete
cp $MYPWD/distfiles/DENIC-ITStt-TestGUI-$VERSION-src-complete.tar.gz $TARGETDIR

}


build_binary() {

case "$DISTRIB_ID:$DISTRIB_RELEASE" in
	MINGW32*:1.0.11*)
		MAKE=gmake
		QMAKE=qmake
		;;
esac


cd $ROOT/src/ppl6

if [ ! -f Makefile ]
then
	echo "Konfiguriere PPL6 fuer $DISTRIB_ID:$DISTRIB_RELEASE..."

	case "$DISTRIB_ID:$DISTRIB_RELEASE" in
	FreeBSD:*)
        export CPPFLAGS=-I/usr/local/include
		export LDLAGS=-L/usr/local/lib
		./configure --prefix=$ROOT \
			--with-pcre=/usr/local  --with-libmhash=/usr/local --with-libiconv-prefix=/usr/local --with-nasm \
			--with-libmcrypt-prefix=/usr/local --with-libiconv-prefix=/usr/local \
			--without-pgsql --without-png --without-lame --without-ft-prefix \
			--without-libmad --without-lame --without-x --without-mysql --without-sybase \
			--without-libcurl --with-sdl-prefix=$ROOT
		;;
	Ubuntu:*)
		./configure --prefix=$ROOT \
			--with-pcre=/usr  --with-libiconv-prefix --with-nasm \
			--with-libmcrypt-prefix=/usr --with-libmhash=/usr \
			--without-pgsql --without-png --without-lame --without-ft-prefix \
			--without-libmad --without-lame --without-x --without-mysql --without-sybase \
			--without-libcurl --with-sdl-prefix=$ROOT
		;;	
	Fedora:*)
		./configure --prefix=$ROOT \
			--with-pcre=/usr  --with-libiconv-prefix --with-nasm \
			--with-libmcrypt-prefix=/usr --with-libmhash=/usr \
			--without-pgsql --without-png --without-lame --without-ft-prefix \
			--without-libmad --without-lame --without-x --without-mysql --without-sybase \
			--without-libcurl --with-sdl-prefix=$ROOT
		;;
	MINGW32*:1.0.11*)
                export CPPFLAGS="-DCURL_STATICLIB -I/usr/local/include -I/sdk/WindowsSDK/include"
                export LDLAGS="-DCURL_STATICLIB -L/usr/local/lib -L/sdk/WindowsSDK/lib"
                export CFLAGS="-DCURL_STATICLIB"
                ./configure --prefix=$ROOT \
			--with-pcre=/usr/local --with-bzip2=/usr/local --with-zlib=/usr/local \
                        --with-nasm --with-libiconv-prefix=/usr/local \
                        --without-pgsql --without-png --without-lame --without-ft-prefix \
						--without-libmad --without-lame --without-x --without-mysql --without-sybase \
						--without-libcurl --with-sdl-prefix=$ROOT \
                        --with-libmhash=/usr/local \
                        --with-libmcrypt-prefix=/usr/local \
                        --with-openssl=/mingw --with-libcurl
		;;
	*)
		./configure --prefix=$ROOT \
			--with-pcre=/usr --with-libiconv-prefix --with-nasm \
			--with-libmcrypt-prefix=/usr --with-libmhash=/usr \
			--without-pgsql --without-png --without-lame --without-ft-prefix \
			--without-libmad --without-lame --without-x --without-mysql --without-sybase \
			--without-libcurl --with-sdl-prefix=$ROOT
		;;
	esac
	if [ $? -ne 0 ] ; then
		exit 1
	fi
fi
PATH=$ROOT/bin:$PATH

$MAKE release
if [ $? -ne 0 ] ; then
	exit 1
fi
$MAKE install
if [ $? -ne 0 ] ; then
	exit 1
fi


echo -n "ppl6-config liegt in "
which ppl6-config



cd $ROOT/src/libdenictest

if [ ! -f Makefile ]
then
	echo "Konfiguriere libdenictest..."
	case "$DISTRIB_ID:$DISTRIB_RELEASE" in
	MINGW32*:1.0.11*)
		./configure --prefix=$ROOT --with-xml-prefix=/usr/local --with-libppl6=$ROOT --with-dchk=/usr/local --with-libidn=/usr/local
		;;
	*)
		./configure --prefix=$ROOT --with-libppl6=$ROOT
		;;
	esac

	if [ $? -ne 0 ] ; then
		exit 1
	fi
fi
$MAKE release
if [ $? -ne 0 ] ; then
	exit 1
fi
$MAKE install
if [ $? -ne 0 ] ; then
	exit 1
fi

echo -n "denictest-config liegt in "
which denictest-config



cd $ROOT/src/testgui

echo "INCLUDEPATH += $ROOT/include" >> TestGUI.pro
echo "INCLUDEPATH += c:/MinGW/msys/1.0/$ROOT/include" >> TestGUI.pro
echo "LIBPATH += $ROOT/lib" >> TestGUI.pro
echo "LIBPATH += C:/MinGW/msys/1.0/$ROOT/lib" >> TestGUI.pro
$QMAKE
if [ $? -ne 0 ] ; then
	exit 1
fi
make release
if [ $? -ne 0 ] ; then
	exit 1
fi
cp release/$TESTGUI $ROOT/bin
strip $ROOT/bin/$TESTGUI


}

ubuntu_write_control() {
	(
		echo "Source: $NAME"
     	echo "Section: misc"
     	echo "Priority: optional"
     	echo "Maintainer: $MAINTAINER"
		echo "Package: $NAME"
		echo "Version: $VERSION"
		echo "Homepage: $HOMEPAGE"
		echo "Architecture: $PLATFORM"
		echo "Depends: $DEPENDS"
		echo "Installed-Size: 5500"
		echo "Description: $DESCRIPTION"
		echo " $COMMENT"
	) > debian/control

}

#################################################################################
# UBUNTU-Paket bauen
#################################################################################
build_ubuntu() {
	echo "Baue Ubuntu-Paket für $DISTRIB_CODENAME"
	DISTNAME="$NAME-$VERSION";
	cd $WORK
	if [ $? -ne 0 ] ; then
		echo "Konnte nicht nach $WORK wechseln"
		exit
	fi
	rm -rf debian
	mkdir -p debian
	cd debian
	if [ $? -ne 0 ] ; then
		echo "Konnte nicht nach $WORK/debian wechseln"
		exit
	fi
	mkdir -p DEBIAN usr/bin usr/share/applications usr/share/pixmaps
	cp $ROOT/bin/TestGUI usr/bin
	cp $ROOT/src/testgui/resources/icon64.png usr/share/pixmaps/DENICTestGUI.png

	cd $WORK
	DEPENDS=""
	ubuntu_write_control
	(
		echo "[Desktop Entry]"
		echo "Encoding=UTF-8"
		echo "Name=$NAME"
		echo "Comment=$COMMENT"
		echo "Exec=TestGUI"
		echo "Terminal=false"
		echo "Type=Application"
		echo "Categories=Office;"
		echo "Icon=/usr/share/pixmaps/DENICTestGUI.png"
	) > debian/usr/share/applications/$NAME.desktop
	(
		echo "testgui ($VERSION) unstable; urgency=low"
		echo ""
		echo " * No upstream changes."
		echo ""
		echo -n "-- $USERNAME   "
		date 
		echo ""
		echo "Local variables:"
     	echo "mode: debian-changelog"
     	echo "End:"
	) > debian/changelog
	
	dpkg-shlibdeps debian/usr/bin/TestGUI
	
	DEPENDS=`grep "shlibs:Depends" debian/substvars | sed -e "s/shlibs:Depends=//"`
	ubuntu_write_control
	
	cp debian/control debian/DEBIAN
	rm debian/control
	rm debian/changelog
	rm debian/substvars
	
	mkdir -p $DISTFILES
	dpkg -b debian $DISTFILES/$DISTNAME-Ubuntu-$DISTRIB_RELEASE-$PLATFORM.deb
	if [ $? -ne 0 ] ; then
		echo "ERROR: Fehler beim Erstellen des Pakets"
		exit
	fi
	cp $DISTFILES/$DISTNAME-Ubuntu-$DISTRIB_RELEASE-$PLATFORM.deb $TARGETDIR
	
}


build_specfile() {
	cd $WORK
	(
	echo "Name:		$NAME"
	echo "Version:	$VERSION"
	echo "Release:	1%{?dist}"
	echo "Summary:	$COMMENT"
	echo ""
	echo "Group:		Applications/Office"
	echo "License:	GPL"
	echo "URL:		$HOMEPAGE"
	#echo "Source:	$MYPWD/distfiles/DENIC-ITStt-TestGUI-$VERSION-src-complete.tar.gz"
	echo "Source:	DENIC-ITStt-TestGUI-%{version}-src-complete.tar.gz"
	echo "BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)"
	echo ""
	echo "BuildRequires:	gcc, gcc-c++, libgcc, bzip2-devel, zlib-devel, libstdc++-devel, openssl-devel, mhash-devel, qt-devel, libidn-devel, libmcrypt-devel, glibc-devel, libxml2-devel, pcre-devel"
	echo "#Requires:"
	echo ""	
	echo "%description"
	echo "$COMMENT"
	echo ""
	echo "%prep"
	echo "%setup -q -n %{name}-%{version}-src-complete"
	echo ""
	echo "%build"
	echo "./build_binary.sh"
	echo ""
	echo "%install"
	echo "rm -rf \$RPM_BUILD_ROOT"
	echo "mkdir -p \$RPM_BUILD_ROOT/usr/bin"
	echo "mkdir -p \$RPM_BUILD_ROOT/usr/share/icons"
	echo "mkdir -p \$RPM_BUILD_ROOT/usr/share/applications"
	echo "cp build/bin/TestGUI \$RPM_BUILD_ROOT/usr/bin"
	echo "cp build/src/testgui/resources/icon64.png \$RPM_BUILD_ROOT/usr/share/icons/DENICTestGUI.png"
	echo ""
	echo "# Desktop menu entry"
	echo "cat > %{name}.desktop << EOF"
	echo "[Desktop Entry]"
	echo "Name=$NAME"
	echo "Comment=$COMMENT"
	echo "Exec=$TESTGUI"
	echo "Icon=DENICTestGUI.png"
	echo "Terminal=0"
	echo "Type=Application"
	echo "EOF"
	echo ""
	echo "desktop-file-install --vendor DENIC \\"
  	echo "  --dir \$RPM_BUILD_ROOT/usr/share/applications \\"
  	echo "  --add-category Application \\"
  	echo "  --add-category Office \\"
  	echo "%{name}.desktop"
  	echo ""
	echo "%clean"
	echo "rm -rf \$RPM_BUILD_ROOT"
	echo ""
	echo "%files"
	echo "%defattr(-,root,root,-)"
	echo "/usr/bin/*"
	echo "/usr/share/icons/*"
	echo "/usr/share/applications/*"
	echo "%doc"
	echo ""
	echo "%changelog"
	echo ""
	) > testgui.spec
}

build_srpm() {
	if [ ! -d ~.rpmmacros ] ; then
		echo "Bereite RPM-Buildsystem vor..."
		mkdir -p ~/rpmbuild/{BUILD,RPMS,S{OURCE,PEC,RPM}S}
		if [ $? -ne 0 ] ; then
			echo "Konnte RPM-Verzeichnisse nicht anlegen: ~/rpmbuild/{BUILD,RPMS,S{OURCE,PEC,RPM}S}"
			exit 1
		fi
		echo "%_topdir $HOME/rpmbuild" > ~/.rpmmacros
	fi
	TOPDIR=`cat ~/.rpmmacros | grep "%_topdir" | grep -v grep | awk {'print $2'}`
	if [ -z "$TOPDIR" ] ; then
		echo "%_topdir ist nicht in ~/.rpmmacros vorhanden"
		exit 1;
	fi
	if [ ! -d "$TOPDIR" ] ; then
		echo "%_topdir ist nicht vorhanden: $TOPDIR"
		exit 1
	fi
	if [ ! -d "$TOPDIR/SOURCES" ] ; then
		echo "%_topdir/SOURCES ist nicht vorhanden: $TOPDIR/SOURCES"
		exit 1
	fi
	cp $MYPWD/distfiles/DENIC-ITStt-TestGUI-$VERSION-src-complete.tar.gz $TOPDIR/SOURCES
	cd $WORK
	rpmbuild -bs testgui.spec
	if [ $? -ne 0 ] ; then
		echo "Build fehlgeschlagen"
		exit 1
	fi
	cp $TOPDIR/SRPMS/DENIC-ITStt-TestGUI-$VERSION-1.src.rpm $DISTFILES
	cp $TOPDIR/SRPMS/DENIC-ITStt-TestGUI-$VERSION-1.src.rpm $TARGETDIR
}

build_mingw32()
{
	echo "*******************************************************"
	echo "Erstelle Windows-Setup Programm"
	cd $ROOT/src/testgui
	cat setup.iss | sed -e "s/OutputBaseFilename=.*/OutputBaseFilename=$NAME-$VERSION-Win32Setup/" \
		| sed -e "s/AppVerName=.*/AppVerName=ITStt-TestGUI $VERSION/" \
		| sed -e "s/AppVersion=.*/AppVersion=$VERSION/" \
		| sed -e "s/VersionInfoVersion=.*/VersionInfoVersion=$VERSION/" \
		| sed -e "s/test2.opsblau.de/www.denic-itstt.de/" \
		> setup2.iss

	"$INNOSETUP" setup2.iss
	if [ $? -ne 0 ] ; then
		echo "Inno-Setup fehlgeschlagen"
		exit 1
	fi
	cd $ROOT/src/testgui
	cp distfiles/$NAME-$VERSION-Win32Setup.exe ../../../
	cp distfiles/$NAME-$VERSION-Win32Setup.exe $TARGETDIR
	
}

##############################################################################################

ARCH=`uname -m`;
PLATFORM="i386";

if [ "$ARCH" = "x86_64" ] ; then
	PLATFORM="amd64"
fi


##############################################################################################
# Sourcen zusammenfassen, sofern wir im Sourceverzeichnis der GUI sind
if [ -f TestGUI.pro ] ; then
	echo "Erstelle Source-Distribution für GUI..."
	build_sources
	echo "Fertig"
	echo "========================================================================================"
else
	# Sind wir nicht, wir müssen ein paar Pfade anpassen
	WORK=$MYPWD
	ROOT=$MYPWD/build
fi

##############################################################################################
# Libs und Binary bauen
if [ "$1" != "nobin" ] ; then 
	build_binary
fi

##############################################################################################
# Pakete bauen
if [ "$DISTRIB_ID" = "Ubuntu" ] ; then
	if [ "$1" != "nobin" ] ; then
		build_ubuntu
	fi
	build_specfile
	build_srpm
elif [ "$DISTRIB_ID" = "MINGW32" ] ; then
	build_mingw32
fi


##############################################################################################
# Aufräumen
# rm -rf $WORK

