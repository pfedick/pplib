###############################################################################
# This file is part of "Patrick's Programming Library", Version 7 (PPL7).
# Web: https://github.com/pfedick/pplib
###############################################################################
# Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#    1. Redistributions of source code must retain the above copyright notice,
#       this list of conditions and the following disclaimer.
#    2. Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS AS IS
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGE.
#
###############################################################################



release:
	cmake -B build/release -DCMAKE_BUILD_TYPE=Release
	cmake --build build/release

debug:
	cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug
	cmake --build build/debug

configure:
	cmake -B build/release -DCMAKE_BUILD_TYPE=Release

all:	release debug

test: debug
	ctest --test-dir build/debug --output-on-failure

xml: debug
	-./build/debug/tests/test_core --gtest_output=xml:testresult_core.xml
	-./build/debug/tests/test_audio --gtest_output=xml:testresult_audio.xml
	-./build/debug/tests/test_crypto --gtest_output=xml:testresult_crypto.xml
	-./build/debug/tests/test_grafix --gtest_output=xml:testresult_grafix.xml
	-./test_database --gtest_output=xml:testresult_database.xml
	-./build/debug/tests/test_inet --gtest_output=xml:testresult_inet.xml

coverage:
	cmake -B build/coverage -DCMAKE_BUILD_TYPE=Debug -DPPL7_ENABLE_COVERAGE=ON
	cmake --build build/coverage
	-ctest --test-dir build/coverage
	gcovr --root . --build-dir build/coverage --xml-pretty -o coverage.xml --exclude 'tests/.*'

clean:
	-rm -rf build
	-rm -rf coverage
	-rm -rf coverage.xml testresult_*.xml
	-rm -rf documentation
	-cd tests; make clean

doku: html

html:
	mkdir -p documentation
	rm -rf documentation/html
	mkdir -p documentation/html
	doxygen Doxyfile
	cp docs/header-bg.png documentation/html

.PHONY: tests
tests:
	cd tests; make xml

memcheck:
	cd tests; make memcheck


