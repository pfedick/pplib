#!/bin/sh
# Artefakt bauen
##################################################################
VERSION=$1
VERSION=${VERSION:=HEAD}

echo "INFO: create artefakt: dist/ppl-$VERSION.tar.bz2"
rm -rf dist tmp
mkdir -p dist tmp/ppl-$VERSION
if [ $? -ne 0 ] ; then
    echo "ERROR: could not create directory \"dist\" or \"tmp/ppl-$VERSION\" "
    exit 1
fi

##################################################################
# Library
find documentation include src *.m4 docs Doxyfile HISTORY.TXT README.TXT configure ppl6-config.in Makefile.in \
    | grep -v ".svn" | cpio -pdmv tmp/ppl-$VERSION > /dev/null 2>&1

if [ $? -ne 0 ] ; then
    echo "ERROR: filecopy failed "
    exit 1
fi
rm -rf tmp/ppl-$VERSION/include/*config.h
   
##################################################################
# Tests
cd tests
find  src testdata gcovr Makefile.in ppl6-tests.h \
    | grep -v ".svn" | cpio -pdmv ../tmp/ppl-$VERSION > /dev/null 2>&1

if [ $? -ne 0 ] ; then
    echo "ERROR: filecopy failed "
    exit 1
fi

##################################################################
# TAR-File bauen
cd ../tmp
tar -cjf ../dist/ppl-$VERSION.tar.bz2 ppl-$VERSION
if [ $? -ne 0 ] ; then
    echo "ERROR: failed to create tar-file"
    exit 1
fi
echo "INFO: artefakt done"
exit 0