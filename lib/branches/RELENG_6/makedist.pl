#!/usr/bin/perl
# $Id: makedist.pl,v 1.5 2010/10/16 14:09:17 pafe Exp $

#  
# CONTACT     : fedick@denic.de 
# AUTHOR      : Patrick Fedick <fedick@denic.de> 
# 
# CREATED     : 2006/01/03 11:30:00 
# REVISION    : $Revision: 1.5 $  
# DATE        : $Date: 2010/10/16 14:09:17 $ 
# 

my $PACKAGE="PPL6";
my $CVSTREE="lib";
my $TAG=$ARGV[0];

if (!$TAG) {
	print "Aufruf: makedist.pl CVSTAG\n";
	exit;
}

my $VERSION;

if (($TAG =~ /^SNAP_(.*)$/)) {
	my $DATE=$1;
	$VERSION=$ARGV[1];
	if (!$VERSION) {
		print "Aufruf: makedist.pl SNAP_yyyymmdd VERSION\n";
		print ("Bei einem Snapshot muss als zweiter Parameter die Version angegegen werden.");
		exit;		
	}
	$VERSION.="-SNAP";
	$VERSION.=$DATE;
	
} else {
	$TAG =~ /^REL_(.*)$/;
	$VERSION=lc($1);
	$VERSION =~ s/_/\./g;
}

if ($TAG eq "HEAD") {
	$VERSION="HEAD";
}
my $DISTNAME="ppl-$VERSION";

print "Packe $PACKAGE mit dem Tag \"$TAG\" aus dem CVS aus...\n";
print "Version:  $VERSION\n";
print "Distname: $DISTNAME\n";
my $PWD=`pwd`;
chomp($PWD);
my $err=`mkdir -p tmp
cd tmp
rm -rf $CVSTREE
cvs -z3 -d:pserver:anonymous\@pplib.cvs.sourceforge.net:/cvsroot/pplib co -P -r $TAG $CVSTREE 2>&1`;

if ($? != 0 ) {
	print "ERROR: Version konnte nicht aus dem CVS geholt werden!\n";
	print $err;
	print "\n";
	exit;
}

print "Erstelle Dokumentation...\n";
print "Doxygen...\n";
`mkdir -p tmp/$CVSTREE/documentation`;
print `cd tmp/$CVSTREE; doxygen Doxyfile; cp docs/header-bg.png documentation/html`;
#print "ok, erstelle PDF...\n";
#print `cd tmp/$CVSTREE/documentation/latex; make`;
#print "Kopiere PDF nach documentation...\n";
#`mv tmp/$CVSTREE/documentation/latex/refman.pdf tmp/$CVSTREE/documentation/ppl6-manual.pdf`;
#print "ok, loesche ueberfluessige Dokumentation...\n";
#`cd tmp/$CVSTREE/documentation; rm -rf *.tmp latex man`;

#print "Kopiere Doku...\n";
#`mkdir -p tmp/$CVSTREE/documentation; cp documentation/*.pdf tmp/$CVSTREE/documentation; cd documentation; find html | cpio -pdmv ../tmp/$CVSTREE/documentation`;
print " done\n";
print "Erstelle Distribution fuer Version: $VERSION\n";
print "PWD=$PWD\n";
print `mkdir -p dist
rm -rf dist/$DISTNAME*
mkdir -p dist/$DISTNAME
cd tmp/$CVSTREE
tar -cf $PWD/dist/tmp.tar --exclude *.core --exclude CVS --exclude config --exclude .cvsignore configure Makefile.in ppl6-config.in *.TXT VERSION src include Doxyfile Doxyfile.latex docs documentation autoconf *.sln *.vcproj ppl6.ico resource/res.h  resource/ppl6-icon-256x256.png resource/ppl6-icon-32x32.png tools/pplgenresource
cd $PWD
cd dist/$DISTNAME
tar -xf ../tmp.tar
cd ..
tar -cjf $DISTNAME-src.tar.bz2 --exclude documentation  $DISTNAME
tar -cjf $DISTNAME.tar.bz2 $DISTNAME
tar -cjf $DISTNAME-html.tar.bz2 $DISTNAME/documentation/html $DISTNAME/documentation/ppl6.tagfile
#tar -cjf $DISTNAME-pdf.tar.bz2 $DISTNAME/documentation/ppl6-manual.pdf
rm -rf tmp.tar $DISTNAME
cd $PWD
rm -rf tmp
`;
print "\n";
