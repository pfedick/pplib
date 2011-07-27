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
} elsif ($TAG eq "HEAD") {
	$VERSION="HEAD";
	$TAG="branches/RELENG_6";
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
#print "ok, erstelle PDF...\n";
#print "Kopiere PDF nach documentation...\n";
#print "ok, loesche ueberfluessige Dokumentation...\n";


#print "Kopiere Doku...\n";
#`mkdir -p tmp/$CVSTREE/documentation; cp documentation/*.pdf tmp/$CVSTREE/documentation; cd documentation; find html | cpio -pdmv ../tmp/$CVSTREE/documentation`;
print " done\n";
print "Erstelle Distribution fuer Version: $VERSION\n";
print "PWD=$PWD\n";
print `mkdir -p dist
rm -rf dist/$DISTNAME*
mkdir -p dist/$DISTNAME
cd tmp/$CVSTREE
tar -cf $PWD/dist/tmp.tar --exclude *.core --exclude CVS --exclude config --exclude .cvsignore configure Makefile.in ppl6-config.in *.TXT VERSION src include Doxyfile docs documentation autoconf *.sln *.vcproj ppl6.ico resource/res.h  resource/ppl6-icon-256x256.png resource/ppl6-icon-32x32.png tools/pplgenresource
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
