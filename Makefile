###############################################################################
# This file is part of "Patrick's Programming Library", Version 8 (PPLIB).
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



ifeq ($(OS),Windows_NT)
    EXE = .exe
else
    EXE =
endif

release:
	cmake -B build/release -DCMAKE_BUILD_TYPE=Release
	cmake --build build/release
	ln -sf build/release/compile_commands.json compile_commands.json

debug:
	cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug
	cmake --build build/debug -j1
	ln -sf build/debug/compile_commands.json compile_commands.json

configure:
	cmake -B build/release -DCMAKE_BUILD_TYPE=Release

all:	release debug

test_core:
	cmake --build build/debug --target test_core
	cp build/debug/tests/test_core$(EXE) tests
	#cd tests && ./test_core$(EXE) --gtest_color=yes --gtest_filter=WideStringTest*
	cd tests && ./test_core$(EXE) --gtest_color=yes
	
test: debug
	cmake --build build/debug --target test_core test_audio test_crypto test_grafix test_database test_inet
	cd tests && ../build/debug/tests/test_core$(EXE) --gtest_color=yes

xml: debug
	cmake --build build/debug --target test_core test_audio test_crypto test_grafix test_database test_inet
	cd tests && ../build/debug/tests/test_core$(EXE) --gtest_output=xml:testresult_core.xml
	-cd tests && ../build/debug/tests/test_audio$(EXE) --gtest_output=xml:testresult_audio.xml
	-cd tests && ../build/debug/tests/test_crypto$(EXE) --gtest_output=xml:testresult_crypto.xml
	-cd tests && ../build/debug/tests/test_grafix$(EXE) --gtest_output=xml:testresult_grafix.xml
	-cd tests && ../build/debug/tests/test_database$(EXE) --gtest_output=xml:testresult_database.xml
	-cd tests && ../build/debug/tests/test_inet$(EXE) --gtest_output=xml:testresult_inet.xml

coverage:
	cmake -B build/coverage -DCMAKE_BUILD_TYPE=Debug -DPPLIB_ENABLE_COVERAGE=ON
	cmake --build build/coverage --target test_core
	ln -sf build/coverage/compile_commands.json compile_commands.json
	-cd tests && ../build/coverage/tests/test_core$(EXE)
	mkdir -p coverage_html
	gcovr --root . build/coverage --medium-threshold 70 --source-encoding UTF-8 --exclude-throw-branches --html-details coverage_html/index.html --exclude 'tests/.*'
	@echo "Report: coverage_html/index.html"
	gcovr --root . build/coverage --medium-threshold 70 --source-encoding UTF-8 --exclude-throw-branches --xml-pretty -o coverage.xml --exclude 'tests/.*'

assocarray:
	cmake -B build/coverage -DCMAKE_BUILD_TYPE=Debug -DPPLIB_ENABLE_COVERAGE=ON
	cmake --build build/coverage --target test_core
	ln -sf build/coverage/compile_commands.json compile_commands.json
	-cd tests && ../build/coverage/tests/test_core$(EXE) --gtest_filter=AssocArrayTest*
	mkdir -p coverage_html
	gcovr --root . build/coverage --medium-threshold 70 --source-encoding UTF-8 --exclude-throw-branches --html-details coverage_html/index.html --exclude 'tests/.*'

clean:
	-rm -rf build
	-rm -rf coverage coverage_html
	-rm -rf coverage.xml tests/testresult_*.xml compile_commands.json
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


