#!/bin/sh
# Artefakt bauen
##################################################################
VERSION=${VERSION:=HEAD}
NAME=ppl7

echo "INFO: create artefakt: dist/$NAME-$VERSION.tar.bz2"
rm -rf dist tmp
mkdir -p dist tmp/$NAME-$VERSION tmp/$NAME-$VERSION/tests
if [ $? -ne 0 ] ; then
    echo "ERROR: could not create directory \"dist\" or \"tmp/$NAME-$VERSION\" "
    exit 1
fi

##################################################################
# Library
find documentation include autoconf configure.ac src resource *.m4 docs Doxyfile LICENSE.TXT HISTORY.TXT README.TXT configure ppl7-config.in Makefile.in \
    | grep -v ".svn" | cpio -pdmv tmp/$NAME-$VERSION > /dev/null 2>&1

if [ $? -ne 0 ] ; then
    echo "ERROR: filecopy failed "
    exit 1
fi
rm -rf tmp/$NAME-$VERSION/include/*config.h
   
##################################################################
# Tests
cd tests
find  src testdata gcovr Makefile.in ppl7-tests.h \
    | grep -v ".svn" | cpio -pdmv ../tmp/$NAME-$VERSION/tests > /dev/null 2>&1

if [ $? -ne 0 ] ; then
    echo "ERROR: filecopy failed "
    exit 1
fi

##################################################################
# TAR-File bauen
cd ../tmp
tar -cjf ../dist/$NAME-$VERSION.tar.bz2 $NAME-$VERSION
if [ $? -ne 0 ] ; then
    echo "ERROR: failed to create tar-file"
    exit 1
fi
echo "INFO: artefakt done"
exit 0