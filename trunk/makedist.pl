#!/usr/bin/perl
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

my $PACKAGE="PPL7";
my $CVSTREE="lib";
my $TAG=$ARGV[0];

if (!$TAG) {
	print "Aufruf: makedist.pl CVSTAG\n";
	exit;
}

my $VERSION;


if ($TAG eq "HEAD") {
	$VERSION="HEAD";
	$TAG="trunk";
} else {
	$TAG =~ /^REL_(.*)$/;
	$VERSION=lc($1);
	$VERSION =~ s/_/\./g;
	$TAG="tags/$TAG";
}

my $DISTNAME="ppl-$VERSION";

print "Packe $PACKAGE mit dem Tag \"$TAG\" aus dem SVN aus...\n";
print "Version:  $VERSION\n";
print "Distname: $DISTNAME\n";
my $PWD=`pwd`;
chomp($PWD);
my $err=`mkdir -p tmp
cd tmp
rm -rf $CVSTREE
svn co https://pplib.svn.sourceforge.net/svnroot/pplib/lib/$TAG lib 2>&1`;

if ($? != 0 ) {
	print "ERROR: Version konnte nicht aus dem SVN geholt werden!\n";
	print $err;
	print "\n";
	exit;
}

print "Erstelle Dokumentation...\n";
print "Doxygen...\n";
`mkdir -p tmp/$CVSTREE/documentation`;
print `cd tmp/$CVSTREE; doxygen Doxyfile; cp docs/header-bg.png documentation/html`;

#print "Kopiere Doku...\n";
#`mkdir -p tmp/$CVSTREE/documentation; cp documentation/*.pdf tmp/$CVSTREE/documentation; cd documentation; find html | cpio -pdmv ../tmp/$CVSTREE/documentation`;
print " done\n";
print "Erstelle Distribution fuer Version: $VERSION\n";
print "PWD=$PWD\n";
print `mkdir -p dist
rm -rf dist/$DISTNAME*
mkdir -p dist/$DISTNAME
cd tmp/$CVSTREE
tar -cf $PWD/dist/tmp.tar --exclude *.core --exclude CVS --exclude config --exclude .cvsignore configure Makefile.in ppl6-config.in *.TXT src include Doxyfile docs documentation autoconf resource/ppl7.ico resource/res.h  resource/ppl7-icon-256x256.png resource/ppl7-icon-32x32.png
cd $PWD
cd dist/$DISTNAME
tar -xf ../tmp.tar
cd ..
tar -cjf $DISTNAME-src.tar.bz2 --exclude documentation  $DISTNAME
tar -cjf $DISTNAME.tar.bz2 $DISTNAME
tar -cjf $DISTNAME-html.tar.bz2 $DISTNAME/documentation/html $DISTNAME/documentation/ppl7.tagfile
rm -rf tmp.tar $DISTNAME
cd $PWD
rm -rf tmp
`;
print "\n";
