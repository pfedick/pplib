###############################################################################
# Eigenstaendiges CMake-Projekt fuer die Grafik-Referenztests.
# Bewusst NICHT ueber das Root-CMakeLists eingebunden:
#   - keine GoogleTest-Abhaengigkeit, keine CMake/CTest-Verzahnung mit den Unittests
#   - baut gegen die fertige Release-Bibliothek aus build/release
#   - Aufruf über das Root-Makefile: make gfxreftest (baut zuerst make release)
#
# Manuelle Konfiguration (ohne Makefile):
#   cmake -B build/gfxref -S tests/gfxreftest -DCMAKE_BUILD_TYPE=Release
#   cmake --build build/gfxref -j
###############################################################################

cmake_minimum_required(VERSION 3.20)
project(pplib8_gfxreftest LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Wurzel des Quellbaums und Ort des Release-Builds der Bibliothek
get_filename_component(PPLIB_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(PPLIB_RELEASE_DIR "${PPLIB_ROOT}/build/release" CACHE PATH
    "Bauverzeichnis der pplib8-Release-Bibliotheken (make release)")

set(PPLIB_INCLUDE_DIR "${PPLIB_ROOT}/include")

find_library(PPLIB_CORE_LIB NAMES pplib8-core
    PATHS "${PPLIB_RELEASE_DIR}" NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
find_library(PPLIB_GRAFIX_LIB NAMES pplib8-grafix
    PATHS "${PPLIB_RELEASE_DIR}" NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)

if(NOT PPLIB_CORE_LIB OR NOT PPLIB_GRAFIX_LIB)
    message(FATAL_ERROR
        "pplib8-core / pplib8-grafix nicht gefunden unter: ${PPLIB_RELEASE_DIR}\n"
        "Bitte zuerst im Quellbaum 'make release' ausfuehren (oder "
        "-DPPLIB_RELEASE_DIR=<pfad> setzen).\n"
        "Hinweis: Archive aus aelteren Staenden heissen libpplib_core.a statt "
        "libpplib8-core.a - in dem Fall build/release einmal neu konfigurieren.")
endif()

# ------------------------------------------------------------------------------
# Die Library wird statisch gebunden => die externen Bibliotheken, mit denen die
# fertige Library gebaut wurde, übernimmt man aus den pkg-config-Fragmenten des
# Release-Builds. So muss diese Suite die Abhängigkeiten (jpeg, png, tiff,
# freetype, pcre2, zstd, ...) nicht ein zweites Mal erfassen. Die Einträge, die
# die pplib-Module selbst betreffen (-lpplib8-*, -L${libdir}), werden entfernt;
# die Archives binden wir direkt über ihren vollen Pfad ein.
# ------------------------------------------------------------------------------
function(pplib_pc_external_libs module out_var)
    set(_pc "${PPLIB_RELEASE_DIR}/pplib8-${module}.pc")
    if(NOT EXISTS "${_pc}")
        set(${out_var} "" PARENT_SCOPE)
        return()
    endif()
    file(READ "${_pc}" _pc_content)
    string(REGEX MATCH "Libs:[^\n]*" _pc_libs "${_pc_content}")
    string(REGEX REPLACE "^Libs:" "" _pc_libs "${_pc_libs}")
    string(REGEX REPLACE "-lpplib8?[A-Za-z0-9_-]*" "" _pc_libs "${_pc_libs}")
    string(REGEX REPLACE "-L\\$\\{[A-Za-z_]+\\}" "" _pc_libs "${_pc_libs}")
    string(STRIP "${_pc_libs}" _pc_libs)
    separate_arguments(_pc_libs UNIX_COMMAND "${_pc_libs}")
    set(${out_var} ${_pc_libs} PARENT_SCOPE)
endfunction()

pplib_pc_external_libs(core EXTERNAL_LIBS_CORE)
pplib_pc_external_libs(grafix EXTERNAL_LIBS_GRAFIX)

# Threads ist eine öffentliche Abhängigkeit von pplib_core (pthread / winthreads).
find_package(Threads REQUIRED)

# ------------------------------------------------------------------------------
# Quellcode der Suite: neue Dateien nach src/ legen.
# ------------------------------------------------------------------------------
file(GLOB GFXREFTEST_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
if(NOT GFXREFTEST_SOURCES)
    message(FATAL_ERROR "Keine Quelldateien in ${CMAKE_CURRENT_LIST_DIR}/src gefunden.")
endif()

# ------------------------------------------------------------------------------
# Assembler-Kernel, die spaeter in die Library gehoeren, koennen hier als
# Prototype entwickelt und gemessen werden, bevor sie nach src/asm der
# Bibliothek wandern (dort gelten die Flags aus dem Root-CMakeLists).
# ------------------------------------------------------------------------------
set(GFXREFTEST_ASM_SOURCES "")
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|i[3-6]86)$")
    enable_language(ASM_NASM OPTIONAL)
    # Ohne installiertes nasm ist der Compilerpfad "-NOTFOUND" und damit falsch.
    if(CMAKE_ASM_NASM_COMPILER)
        file(GLOB GFXREFTEST_ASM_SOURCES CONFIGURE_DEPENDS "asm/*.asm")
        if(GFXREFTEST_ASM_SOURCES)
            set_source_files_properties(${GFXREFTEST_ASM_SOURCES}
                PROPERTIES LANGUAGE ASM_NASM)
            # Wie in der Library: includes aus dem Quellbaum der Bibliothek
            set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -i ${PPLIB_ROOT}/")
        endif()
    endif()
endif()

add_executable(gfxreftest ${GFXREFTEST_SOURCES} ${GFXREFTEST_ASM_SOURCES})

target_include_directories(gfxreftest PRIVATE "${PPLIB_INCLUDE_DIR}")

# Das zur Release-Bibliothek generierte config_pplib.h liegt im Buildverzeichnis.
# Nur noetig, falls Tests selbst auf HAVE_*- Makros zugreifen wollen.
if(EXISTS "${PPLIB_RELEASE_DIR}/include/config_pplib.h")
    target_include_directories(gfxreftest PRIVATE "${PPLIB_RELEASE_DIR}/include")
endif()

target_link_libraries(gfxreftest PRIVATE
    "${PPLIB_GRAFIX_LIB}"
    "${PPLIB_CORE_LIB}"
    ${EXTERNAL_LIBS_GRAFIX}
    ${EXTERNAL_LIBS_CORE}
    Threads::Threads
)
