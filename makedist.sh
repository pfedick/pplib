#!/bin/sh
###############################################################################
###
### Build artefact script
###
###############################################################################
# This file is part of "Patrick's Programming Library", Version 7 (PPL7).
# Web: http://www.pfp.de/ppl/
#
# $Author: pafe $
# $Revision: 588 $
# $Date: 2013-04-26 20:47:43 +0200 (Fr, 26 Apr 2013) $
# $Id: Makefile.in 588 2013-04-26 18:47:43Z pafe $
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
    | cpio -pdmv tmp/$NAME-$VERSION > /dev/null 2>&1

if [ $? -ne 0 ] ; then
    echo "ERROR: filecopy failed "
    exit 1
fi
rm -rf tmp/$NAME-$VERSION/include/*config.h
   
##################################################################
# Tests
cd tests
find  src testdata gcovr Makefile.in ppl7-tests.h create_postgres_db.sh \
    | cpio -pdmv ../tmp/$NAME-$VERSION/tests > /dev/null 2>&1

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