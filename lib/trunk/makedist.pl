#!/usr/bin/perl
# $Id$

#  
# CONTACT     : fedick@denic.de 
# AUTHOR      : Patrick Fedick <fedick@denic.de> 
# 
# CREATED     : 2006/01/03 11:30:00 
# REVISION    : $Revision$  
# DATE        : $Date$ 
# 

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
