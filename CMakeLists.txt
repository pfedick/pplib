cmake_minimum_required(VERSION 3.20)
project(PPL7 VERSION 7.1.0 LANGUAGES C CXX)

# C++ Standard festlegen
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------------------------
# Build-Optionen (Module & Features)
# ------------------------------------------------------------------------------
option(PPL7_ENABLE_CORE     "Build Core module" ON)
option(PPL7_ENABLE_AUDIO    "Build Audio module" ON)
option(PPL7_ENABLE_CRYPTO   "Build Crypto module" ON)
option(PPL7_ENABLE_GRAFIX   "Build Grafix module" ON)
option(PPL7_ENABLE_INTERNET "Build Internet module" ON)
option(PPL7_ENABLE_DATABASE "Build Database module" ON)

# Optionale Abhängigkeiten & Features
option(PPL7_WITH_ASM        "Enable NASM Assembly optimizations" ON)
option(PPL7_WITH_OPENSSL    "Enable OpenSSL support" ON)
option(PPL7_WITH_CURL       "Enable cURL support" ON)
option(PPL7_WITH_JPEG       "Enable JPEG support" ON)
option(PPL7_WITH_PNG        "Enable PNG support" ON)
option(PPL7_WITH_TIFF       "Enable TIFF support" ON)
option(PPL7_WITH_FREETYPE   "Enable FreeType support" ON)
option(PPL7_WITH_SQLITE3    "Enable SQLite3 support" ON)
option(PPL7_WITH_MYSQL      "Enable MySQL support" ON)
option(PPL7_WITH_POSTGRES   "Enable PostgreSQL support" ON)
option(PPL7_WITH_MPG123     "Enable MPG123 audio decoder" ON)
option(PPL7_WITH_LAME       "Enable LAME MP3 encoder" ON)
option(PPL7_WITH_VORBIS     "Enable Ogg/Vorbis audio" ON)
option(PPL7_WITH_MICROHTTPD "Enable libmicrohttpd" ON)

include(GNUInstallDirs)
find_package(PkgConfig QUIET)

# Gemeinsame Include-Verzeichnisse für alle PPL7-Module
set(PPL7_INCLUDE_DIRS
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# ==============================================================================
# 1. CORE (Basis-Modul: core, types, math)
# ==============================================================================
if(PPL7_ENABLE_CORE)
    file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
        "src/core/*.cpp"
        "src/types/*.cpp"
        "src/math/*.cpp"
    )

    add_library(ppl7_core STATIC ${CORE_SOURCES})
    add_library(ppl7::core ALIAS ppl7_core)

    target_include_directories(ppl7_core PUBLIC ${PPL7_INCLUDE_DIRS})
    target_compile_definitions(ppl7_core PUBLIC PPL7LIB=1 HAVE_CONFIG_H=1)

    # Standard System-Bibliotheken (z.B. pthreads, zlib, bzip2, pcre2)
    find_package(ZLIB QUIET)
    if(ZLIB_FOUND)
        target_link_libraries(ppl7_core PRIVATE ZLIB::ZLIB)
        target_compile_definitions(ppl7_core PRIVATE HAVE_ZLIB=1 HAVE_LIBZ=1)
    endif()

    find_package(BZip2 QUIET)
    if(BZip2_FOUND)
        target_link_libraries(ppl7_core PRIVATE BZip2::BZip2)
        target_compile_definitions(ppl7_core PRIVATE HAVE_BZIP2=1)
    endif()

    find_package(Threads REQUIRED)
    target_link_libraries(ppl7_core PUBLIC Threads::Threads)
endif()

# ==============================================================================
# 2. CRYPTO (src/crypto)
# ==============================================================================
if(PPL7_ENABLE_CRYPTO)
    file(GLOB CRYPTO_SOURCES CONFIGURE_DEPENDS "src/crypto/*.cpp")

    add_library(ppl7_crypto STATIC ${CRYPTO_SOURCES})
    add_library(ppl7::crypto ALIAS ppl7_crypto)

    target_include_directories(ppl7_crypto PUBLIC ${PPL7_INCLUDE_DIRS})
    target_link_libraries(ppl7_crypto PUBLIC ppl7_core)

    if(PPL7_WITH_OPENSSL)
        find_package(OpenSSL QUIET)
        if(OpenSSL_FOUND)
            target_link_libraries(ppl7_crypto PRIVATE OpenSSL::Crypto)
            target_compile_definitions(ppl7_crypto PRIVATE HAVE_OPENSSL=1)
        endif()
    endif()
endif()

# ==============================================================================
# 3. GRAFIX (src/grafix, src/asm)
# ==============================================================================
if(PPL7_ENABLE_GRAFIX)
    file(GLOB GFX_SOURCES CONFIGURE_DEPENDS "src/grafix/*.cpp")

    # Assembler (NASM) Unterstützung
    set(ASM_SOURCES)
    if(PPL7_WITH_ASM)
        enable_language(ASM_NASM OPTIONAL)
        if(CMAKE_ASM_NASM_COMPILER_LOADED)
            file(GLOB ASM_SOURCES CONFIGURE_DEPENDS "src/asm/*.asm")
            set_source_files_properties(${ASM_SOURCES} PROPERTIES LANGUAGE ASM_NASM)
            set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -i ${CMAKE_CURRENT_SOURCE_DIR}/")
        endif()
    endif()

    add_library(ppl7_grafix STATIC ${GFX_SOURCES} ${ASM_SOURCES})
    add_library(ppl7::grafix ALIAS ppl7_grafix)

    target_include_directories(ppl7_grafix PUBLIC ${PPL7_INCLUDE_DIRS})
    target_link_libraries(ppl7_grafix PUBLIC ppl7_core)

    if(CMAKE_ASM_NASM_COMPILER_LOADED)
        target_compile_definitions(ppl7_grafix PRIVATE HAVE_X86_ASSEMBLER=1)
    endif()

    # Formate & Bildbibliotheken
    if(PPL7_WITH_JPEG)
        find_package(JPEG QUIET)
        if(JPEG_FOUND)
            target_link_libraries(ppl7_grafix PRIVATE JPEG::JPEG)
            target_compile_definitions(ppl7_grafix PRIVATE HAVE_JPEG=1)
        endif()
    endif()

    if(PPL7_WITH_PNG)
        find_package(PNG QUIET)
        if(PNG_FOUND)
            target_link_libraries(ppl7_grafix PRIVATE PNG::PNG)
            target_compile_definitions(ppl7_grafix PRIVATE HAVE_PNG=1)
        endif()
    endif()

    if(PPL7_WITH_TIFF)
        find_package(TIFF QUIET)
        if(TIFF_FOUND)
            target_link_libraries(ppl7_grafix PRIVATE TIFF::TIFF)
            target_compile_definitions(ppl7_grafix PRIVATE HAVE_TIFF=1)
        endif()
    endif()

    if(PPL7_WITH_FREETYPE)
        find_package(Freetype QUIET)
        if(Freetype_FOUND)
            target_link_libraries(ppl7_grafix PRIVATE Freetype::Freetype)
            target_compile_definitions(ppl7_grafix PRIVATE HAVE_FREETYPE2=1)
        endif()
    endif()
endif()

# ==============================================================================
# 4. INTERNET (src/internet)
# ==============================================================================
if(PPL7_ENABLE_INTERNET)
    file(GLOB INET_SOURCES CONFIGURE_DEPENDS "src/internet/*.cpp")

    add_library(ppl7_internet STATIC ${INET_SOURCES})
    add_library(ppl7::internet ALIAS ppl7_internet)

    target_include_directories(ppl7_internet PUBLIC ${PPL7_INCLUDE_DIRS})
    target_link_libraries(ppl7_internet PUBLIC ppl7_core)

    if(PPL7_WITH_CURL)
        find_package(CURL QUIET)
        if(CURL_FOUND)
            target_link_libraries(ppl7_internet PRIVATE CURL::libcurl)
            target_compile_definitions(ppl7_internet PRIVATE HAVE_CURL=1)
        endif()
    endif()

    if(PPL7_WITH_OPENSSL)
        find_package(OpenSSL QUIET)
        if(OpenSSL_FOUND)
            target_link_libraries(ppl7_internet PRIVATE OpenSSL::SSL OpenSSL::Crypto)
            target_compile_definitions(ppl7_internet PRIVATE HAVE_OPENSSL=1)
        endif()
    endif()

    if(PPL7_WITH_MICROHTTPD AND PKG_CONFIG_FOUND)
        pkg_check_modules(MICROHTTPD QUIET libmicrohttpd)
        if(MICROHTTPD_FOUND)
            target_include_directories(ppl7_internet PRIVATE ${MICROHTTPD_INCLUDE_DIRS})
            target_link_libraries(ppl7_internet PRIVATE ${MICROHTTPD_LIBRARIES})
            target_compile_definitions(ppl7_internet PRIVATE HAVE_LIBMICROHTTPD=1)
        endif()
    endif()
endif()

# ==============================================================================
# 5. DATABASE (src/database)
# ==============================================================================
if(PPL7_ENABLE_DATABASE)
    file(GLOB DB_SOURCES CONFIGURE_DEPENDS "src/database/*.cpp")

    add_library(ppl7_database STATIC ${DB_SOURCES})
    add_library(ppl7::db ALIAS ppl7_database)

    target_include_directories(ppl7_database PUBLIC ${PPL7_INCLUDE_DIRS})
    target_link_libraries(ppl7_database PUBLIC ppl7_core)

    if(PPL7_WITH_SQLITE3)
        find_package(SQLite3 QUIET)
        if(SQLite3_FOUND)
            target_link_libraries(ppl7_database PRIVATE SQLite3::SQLite3)
            target_compile_definitions(ppl7_database PRIVATE HAVE_SQLITE3=1)
        endif()
    endif()

    if(PPL7_WITH_MYSQL AND PKG_CONFIG_FOUND)
        pkg_check_modules(MYSQL QUIET mysqlclient)
        if(MYSQL_FOUND)
            target_include_directories(ppl7_database PRIVATE ${MYSQL_INCLUDE_DIRS})
            target_link_libraries(ppl7_database PRIVATE ${MYSQL_LIBRARIES})
            target_compile_definitions(ppl7_database PRIVATE HAVE_MYSQL=1)
        endif()
    endif()

    if(PPL7_WITH_POSTGRES AND PKG_CONFIG_FOUND)
        pkg_check_modules(POSTGRES QUIET libpq)
        if(POSTGRES_FOUND)
            target_include_directories(ppl7_database PRIVATE ${POSTGRES_INCLUDE_DIRS})
            target_link_libraries(ppl7_database PRIVATE ${POSTGRES_LIBRARIES})
            target_compile_definitions(ppl7_database PRIVATE HAVE_POSTGRESQL=1)
        endif()
    endif()
endif()

# ==============================================================================
# 6. AUDIO (src/audio)
# ==============================================================================
if(PPL7_ENABLE_AUDIO)
    file(GLOB AUDIO_SOURCES CONFIGURE_DEPENDS "src/audio/*.cpp")

    add_library(ppl7_audio STATIC ${AUDIO_SOURCES})
    add_library(ppl7::audio ALIAS ppl7_audio)

    target_include_directories(ppl7_audio PUBLIC ${PPL7_INCLUDE_DIRS})
    target_link_libraries(ppl7_audio PUBLIC ppl7_core)

    if(PKG_CONFIG_FOUND)
        if(PPL7_WITH_MPG123)
            pkg_check_modules(MPG123 QUIET libmpg123)
            if(MPG123_FOUND)
                target_include_directories(ppl7_audio PRIVATE ${MPG123_INCLUDE_DIRS})
                target_link_libraries(ppl7_audio PRIVATE ${MPG123_LIBRARIES})
                target_compile_definitions(ppl7_audio PRIVATE HAVE_MPG123=1)
            endif()
        endif()

        if(PPL7_WITH_VORBIS)
            pkg_check_modules(VORBIS QUIET vorbis vorbisfile)
            if(VORBIS_FOUND)
                target_include_directories(ppl7_audio PRIVATE ${VORBIS_INCLUDE_DIRS})
                target_link_libraries(ppl7_audio PRIVATE ${VORBIS_LIBRARIES})
                target_compile_definitions(ppl7_audio PRIVATE HAVE_VORBIS=1)
            endif()
        endif()
    endif()
endif()