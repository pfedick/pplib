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

PREFIX ?= /usr/local

ifeq ($(OS),Windows_NT)
    EXE = .exe
    ASAN_LEAKS ?= 0    # LeakSanitizer existiert nur fuer Linux/Android
else
    EXE =
    ASAN_LEAKS ?= 1
endif

release:
	cmake -B build/release -DCMAKE_BUILD_TYPE=Release
	cmake --build build/release -j
	ln -sf build/release/compile_commands.json compile_commands.json

debug:
	cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug
	cmake --build build/debug -j
	ln -sf build/debug/compile_commands.json compile_commands.json

configure:
	cmake -B build/release -DCMAKE_BUILD_TYPE=Release

all:	release debug

install:
	cmake -B build/release -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$(PREFIX)
	cmake --build build/release -j
	cmake --install build/release

install_home:
	cmake -B build/release -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$(HOME)
	cmake --build build/release -j
	cmake --install build/release


build_tests:
	cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug
	cmake --build build/debug -j --target test_core test_crypto test_all

test: build_tests
	cd tests && ../build/debug/tests/test_all$(EXE) --gtest_color=yes

# Build & Lauf der Tests mit Address- und Undefined-Behaviour-Sanitizer.
# Eigener Build-Ordner build/asan, damit der normale Debug-Build unangetastet bleibt.
# TEST_ARGS erlaubt Selektion, z.B.: make asan TEST_ARGS="--gtest_filter=WideStringTest*"
# Ohne installierte libubsan (Fedora: sudo dnf install libubsan) nur ASan verwenden:
#   make asan ASAN_SANITIZERS=address
ASAN_SANITIZERS ?= address,undefined
ASAN_FLAGS ?= -fsanitize=$(ASAN_SANITIZERS) -fno-omit-frame-pointer -g -O1
ASAN_OPTIONS ?= detect_leaks=$(ASAN_LEAKS):halt_on_error=0:max_errors=30:allocator_may_return_null=1
UBSAN_OPTIONS ?= print_stacktrace=1:halt_on_error=0
ASAN_LOG ?= build/asan/sanitizer.log

# SHELL=bash, damit PIPESTATUS den Exit-Code des Tests hinter 'tee' liefert
asan: SHELL := /bin/bash
asan:
	cmake -B build/asan -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="$(ASAN_FLAGS)" -DCMAKE_GTEST_DISCOVER_TESTS_DISCOVERY_MODE=PRE_TEST
	cmake --build build/asan -j --target test_all
	# Mitschnitt: Sanitizer-Meldungen gehen auf stderr und liegen zwischen den
	# gtest-Zeilen. Komplette Ausgabe bleibt im Log erhalten, am Ende folgt die Zusammenfassung.
	cd tests && ASAN_OPTIONS=$(ASAN_OPTIONS) UBSAN_OPTIONS=$(UBSAN_OPTIONS) ../build/asan/tests/test_all$(EXE) --gtest_color=yes $(TEST_ARGS) 2>&1 | tee ../$(ASAN_LOG); \
	rc=$${PIPESTATUS[0]}; \
	sed -i 's/\x1b\[[0-9;]*[A-Za-z]//g' ../$(ASAN_LOG); \
	found=$$(grep -cE 'ERROR: (Address|Leak)Sanitizer|runtime error:' ../$(ASAN_LOG) || true); \
	echo ""; \
	echo "=== Sanitizer-Report ($(ASAN_SANITIZERS), Log: $(ASAN_LOG)) ==="; \
	echo "Gefundene Meldungen: $$found    Exit-Code der Tests: $$rc"; \
	grep -oE '^[^ ]+\.(c|cpp|h):[0-9]+:[0-9]+: runtime error: .*' ../$(ASAN_LOG) | sort -u | sed 's|^$(CURDIR)/||; s|^|  UBSan  |'; \
	grep -E 'ERROR: (Address|Leak)Sanitizer|SUMMARY: (Address|Leak)Sanitizer|detected memory leaks' ../$(ASAN_LOG) | sort -u | sed 's|^|  ASan   |'; \
	[ $$rc -eq 0 ] && [ $$found -eq 0 ] || exit 1

#xml: debug
#	cmake --build build/debug --target test_core test_audio test_crypto test_grafix test_database test_inet
#	cd tests && ../build/debug/tests/test_core$(EXE) --gtest_output=xml:testresult_core.xml
#	-cd tests && ../build/debug/tests/test_audio$(EXE) --gtest_output=xml:testresult_audio.xml
#	-cd tests && ../build/debug/tests/test_crypto$(EXE) --gtest_output=xml:testresult_crypto.xml
#	-cd tests && ../build/debug/tests/test_grafix$(EXE) --gtest_output=xml:testresult_grafix.xml
#	-cd tests && ../build/debug/tests/test_database$(EXE) --gtest_output=xml:testresult_database.xml
#	-cd tests && ../build/debug/tests/test_inet$(EXE) --gtest_output=xml:testresult_inet.xml

coverage:
	cmake -B build/coverage -DCMAKE_BUILD_TYPE=Debug -DPPLIB_ENABLE_COVERAGE=ON
	#cmake --build build/coverage -j --target test_core test_crypto test_audio
	cmake --build build/coverage -j --target test_all
	ln -sf build/coverage/compile_commands.json compile_commands.json
	cd tests && ../build/coverage/tests/test_all$(EXE)
	mkdir -p coverage_html
	gcovr --root . build/coverage --medium-threshold 70 --source-encoding UTF-8 --exclude-throw-branches --html-details coverage_html/index.html --xml-pretty -o coverage.xml --exclude 'tests/.*' --exclude 'build/coverage/_deps'
	@echo "Report: coverage_html/index.html"

wip:
	cmake -B build/coverage -DCMAKE_BUILD_TYPE=Debug -DPPLIB_ENABLE_COVERAGE=ON
	cmake --build build/coverage -j --target test_all
	ln -sf build/coverage/compile_commands.json compile_commands.json
	-cd tests && ../build/coverage/tests/test_all$(EXE) --gtest_filter=DateTimeTest*
	mkdir -p coverage_html
	gcovr --root . build/coverage --medium-threshold 70 --source-encoding UTF-8 --exclude-throw-branches --html-details coverage_html/index.html --xml-pretty -o coverage.xml --exclude 'tests/.*' --exclude 'build/coverage/_deps'

clean:
	-rm -rf build
	-rm -rf coverage coverage_html
	-rm -rf coverage.xml tests/testresult_*.xml compile_commands.json
	-rm -rf documentation tests/tmp
	-cd tests; make clean

doku: html

html:
	mkdir -p documentation
	rm -rf documentation/html
	mkdir -p documentation/html
	doxygen Doxyfile
	cp docs/header-bg.png documentation/html
	cp docs/pplib-icon-48x48.png documentation/html
	echo "Documentation generated at documentation/html/index.html"

.PHONY: all build_tests test asan coverage wip clean doku html
