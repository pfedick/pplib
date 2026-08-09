cmake_minimum_required(VERSION 3.20)
project(PPLIB VERSION 8.0.0 LANGUAGES C CXX)

include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckStructHasMember)

# C++ Standard festlegen
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------------------------
# Build-Optionen (Module & Features)
# ------------------------------------------------------------------------------
option(PPLIB_ENABLE_CORE     "Build Core module" ON)
option(PPLIB_ENABLE_AUDIO    "Build Audio module" ON)
option(PPLIB_ENABLE_CRYPTO   "Build Crypto module" ON)
option(PPLIB_ENABLE_GRAFIX   "Build Grafix module" ON)
option(PPLIB_ENABLE_INTERNET "Build Internet module" ON)
option(PPLIB_ENABLE_DATABASE "Build Database module" ON)

option(PPLIB_BUILD_TESTS     "Build Unit Tests" ON)
option(PPLIB_ENABLE_COVERAGE "Enable Code Coverage" OFF)

# Coverage flags
if(PPLIB_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(--coverage -O0)
        add_link_options(--coverage)
    endif()
endif()

# Syslog
check_include_file("syslog.h" HAVE_SYSLOG_H)

# Falls syslog.h existiert, prüfen wir, ob openlog und closelog zur Verfügung stehen
if(HAVE_SYSLOG_H)
    # Beachte: Manche Systeme verlangen die Angabe von syslog.h im Check-Kontext
    check_symbol_exists(openlog "syslog.h" HAVE_OPENLOG)
    check_symbol_exists(closelog "syslog.h" HAVE_CLOSELOG)
endif()

# Optionale Abhängigkeiten & Features
option(PPLIB_WITH_ASM        "Enable NASM Assembly optimizations" ON)
option(PPLIB_WITH_OPENSSL    "Enable OpenSSL support" ON)
option(PPLIB_WITH_CURL       "Enable cURL support" ON)
option(PPLIB_WITH_JPEG       "Enable JPEG support" ON)
option(PPLIB_WITH_PNG        "Enable PNG support" ON)
option(PPLIB_WITH_TIFF       "Enable TIFF support" ON)
option(PPLIB_WITH_FREETYPE   "Enable FreeType support" ON)
option(PPLIB_WITH_SQLITE3    "Enable SQLite3 support" ON)
option(PPLIB_WITH_MYSQL      "Enable MySQL support" ON)
option(PPLIB_WITH_POSTGRES   "Enable PostgreSQL support" ON)
option(PPLIB_WITH_MPG123     "Enable MPG123 audio decoder" ON)
option(PPLIB_WITH_LAME       "Enable LAME MP3 encoder" ON)
option(PPLIB_WITH_VORBIS     "Enable Ogg/Vorbis audio" ON)
option(PPLIB_WITH_MICROHTTPD "Enable libmicrohttpd" ON)

include(GNUInstallDirs)
find_package(PkgConfig QUIET)

# Gemeinsame Include-Verzeichnisse für alle PPLIB-Module
set(PPLIB_INCLUDE_DIRS
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)


include(TestBigEndian)
TEST_BIG_ENDIAN(PPLIB_IS_BIG_ENDIAN)
if(PPLIB_IS_BIG_ENDIAN)
    set(__BIG_ENDIAN__ 1)
else()
    set(__LITTLE_ENDIAN__ 1)
endif()

# ==============================================================================
# 1. CORE (Basis-Modul: core, types, math)
# ==============================================================================
if(PPLIB_ENABLE_CORE)
    file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
        "src/core/*.cpp"
        "src/types/*.cpp"
        "src/math/*.cpp"
    )

    add_library(pplib_core STATIC ${CORE_SOURCES})
    add_library(pplib::core ALIAS pplib_core)

    target_include_directories(pplib_core PUBLIC ${PPLIB_INCLUDE_DIRS})
    target_compile_definitions(pplib_core PUBLIC PPLIBLIB=1 HAVE_CONFIG_H=1)


    check_struct_has_member("struct tm" tm_gmtoff "time.h" STRUCT_TM_HAS_GMTOFF)

    # Standard System-Bibliotheken (z.B. pthreads, zlib, bzip2, pcre2)
    
    find_package(Threads REQUIRED)
    if(CMAKE_USE_PTHREADS_INIT AND NOT WIN32)
        set(HAVE_PTHREADS 1)
    endif()

    check_include_file("sched.h" HAVE_SCHED_H)
    if(HAVE_SCHED_H)
        check_symbol_exists(sched_yield "sched.h" HAVE_SCHED_YIELD)
    endif()
    if(NOT WIN32)
        check_include_file("pthread_np.h" HAVE_PTHREAD_NP_H)
        
        # Prüfen, welche Variante von pthread_setname_np / pthread_set_name_np existiert
        set(CMAKE_REQUIRED_LIBRARIES Threads::Threads)
        
        check_symbol_exists(pthread_setname_np "pthread.h" HAVE_PTHREAD_SETNAME_NP)
        
        if(HAVE_PTHREAD_NP_H)
            check_symbol_exists(pthread_set_name_np "pthread.h;pthread_np.h" HAVE_PTHREAD_SET_NAME_NP)
        else()
            check_symbol_exists(pthread_set_name_np "pthread.h" HAVE_PTHREAD_SET_NAME_NP)
        endif()
    endif()

    find_package(ZLIB QUIET)
    if(ZLIB_FOUND)
        set(HAVE_LIBZ 1)
        set(HAVE_ZLIB 1)
        target_link_libraries(pplib_core PRIVATE ZLIB::ZLIB)
        target_compile_definitions(pplib_core PRIVATE HAVE_ZLIB=1 HAVE_LIBZ=1)
    endif()

    find_package(BZip2 QUIET)
    if(BZip2_FOUND)
        set(HAVE_BZIP2 1)
        target_link_libraries(pplib_core PRIVATE BZip2::BZip2)
        target_compile_definitions(pplib_core PRIVATE HAVE_BZIP2=1)
    endif()

    find_package(Iconv QUIET)
    if(Iconv_FOUND)
        set(HAVE_ICONV 1)
        target_link_libraries(pplib_core PRIVATE Iconv::Iconv)
        target_compile_definitions(pplib_core PRIVATE HAVE_ICONV=1)
    endif()

    if(PKG_CONFIG_FOUND)
        pkg_check_modules(PCRE2_8 QUIET libpcre2-8)
        if(PCRE2_8_FOUND)
            set(HAVE_PCRE2 1)
            set(HAVE_PCRE2_BITS_8 1)
            target_include_directories(pplib_core PRIVATE ${PCRE2_8_INCLUDE_DIRS})
            target_link_libraries(pplib_core PRIVATE ${PCRE2_8_LIBRARIES})
        endif()

        pkg_check_modules(PCRE2_16 QUIET libpcre2-16)
        if(PCRE2_16_FOUND)
            set(HAVE_PCRE2 1)
            set(HAVE_PCRE2_BITS_16 1)
            target_include_directories(pplib_core PRIVATE ${PCRE2_16_INCLUDE_DIRS})
            target_link_libraries(pplib_core PRIVATE ${PCRE2_16_LIBRARIES})
        endif()

        pkg_check_modules(PCRE2_32 QUIET libpcre2-32)
        if(PCRE2_32_FOUND)
            set(HAVE_PCRE2 1)
            set(HAVE_PCRE2_BITS_32 1)
            target_include_directories(pplib_core PRIVATE ${PCRE2_32_INCLUDE_DIRS})
            target_link_libraries(pplib_core PRIVATE ${PCRE2_32_LIBRARIES})
        endif()
    endif()


    find_package(Threads REQUIRED)
    target_link_libraries(pplib_core PUBLIC Threads::Threads)
endif()

# ==============================================================================
# 2. CRYPTO (src/crypto)
# ==============================================================================
if(PPLIB_ENABLE_CRYPTO)
    file(GLOB CRYPTO_SOURCES CONFIGURE_DEPENDS "src/crypto/*.cpp")

    add_library(pplib_crypto STATIC ${CRYPTO_SOURCES})
    add_library(pplib::crypto ALIAS pplib_crypto)

    target_include_directories(pplib_crypto PUBLIC ${PPLIB_INCLUDE_DIRS})
    target_link_libraries(pplib_crypto PUBLIC pplib_core)

    if(PPLIB_WITH_OPENSSL)
        find_package(OpenSSL QUIET)
        if(OpenSSL_FOUND)
            set(HAVE_OPENSSL 1)
            target_link_libraries(pplib_crypto PRIVATE OpenSSL::Crypto)
            target_compile_definitions(pplib_crypto PRIVATE HAVE_OPENSSL=1)
        endif()
    endif()
endif()

# ==============================================================================
# 3. GRAFIX (src/grafix, src/asm)
# ==============================================================================
if(PPLIB_ENABLE_GRAFIX)
    file(GLOB GFX_SOURCES CONFIGURE_DEPENDS "src/grafix/*.cpp")

    # Assembler (NASM) Unterstützung
    set(ASM_SOURCES)
    if(PPLIB_WITH_ASM)
        enable_language(ASM_NASM OPTIONAL)
        if(CMAKE_ASM_NASM_COMPILER_LOADED)
            set(HAVE_NASM 1)
            file(GLOB ASM_SOURCES CONFIGURE_DEPENDS "src/asm/*.asm")
            set_source_files_properties(${ASM_SOURCES} PROPERTIES LANGUAGE ASM_NASM)
            set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -i ${CMAKE_CURRENT_SOURCE_DIR}/")
        endif()
    endif()

    add_library(pplib_grafix STATIC ${GFX_SOURCES} ${ASM_SOURCES})
    add_library(pplib::grafix ALIAS pplib_grafix)

    target_include_directories(pplib_grafix PUBLIC ${PPLIB_INCLUDE_DIRS})
    target_link_libraries(pplib_grafix PUBLIC pplib_core)

    if(CMAKE_ASM_NASM_COMPILER_LOADED)
        target_compile_definitions(pplib_grafix PRIVATE HAVE_X86_ASSEMBLER=1)
        set (HAVE_X86_ASSEMBLER 1)
    endif()

    # Formate & Bildbibliotheken
    if(PPLIB_WITH_JPEG)
        find_package(JPEG QUIET)
        if(JPEG_FOUND)
            set(HAVE_JPEG 1)
            target_link_libraries(pplib_grafix PRIVATE JPEG::JPEG)
            target_compile_definitions(pplib_grafix PRIVATE HAVE_JPEG=1)
        endif()
    endif()

    if(PPLIB_WITH_PNG)
        find_package(PNG QUIET)
        if(PNG_FOUND)
            set(HAVE_PNG 1)
            target_link_libraries(pplib_grafix PRIVATE PNG::PNG)
            target_compile_definitions(pplib_grafix PRIVATE HAVE_PNG=1)
        endif()
    endif()

    if(PPLIB_WITH_TIFF)
        find_package(TIFF QUIET)
        if(TIFF_FOUND)
            set(HAVE_TIFF 1)
            target_link_libraries(pplib_grafix PRIVATE TIFF::TIFF)
            target_compile_definitions(pplib_grafix PRIVATE HAVE_TIFF=1)
        endif()
    endif()

    if(PPLIB_WITH_FREETYPE)
        find_package(Freetype QUIET)
        if(Freetype_FOUND)
            set(HAVE_FREETYPE2 1)
            target_link_libraries(pplib_grafix PRIVATE Freetype::Freetype)
            target_compile_definitions(pplib_grafix PRIVATE HAVE_FREETYPE2=1)
        endif()
    endif()
endif()

# ==============================================================================
# 4. INTERNET (src/internet)
# ==============================================================================
if(PPLIB_ENABLE_INTERNET)
    file(GLOB INET_SOURCES CONFIGURE_DEPENDS "src/internet/*.cpp")

    add_library(pplib_internet STATIC ${INET_SOURCES})
    add_library(pplib::internet ALIAS pplib_internet)

    target_include_directories(pplib_internet PUBLIC ${PPLIB_INCLUDE_DIRS})
    target_link_libraries(pplib_internet PUBLIC pplib_core)

    if(PPLIB_WITH_CURL)
        find_package(CURL QUIET)
        if(CURL_FOUND)
            set(HAVE_LIBCURL 1)
            target_link_libraries(pplib_internet PRIVATE CURL::libcurl)
            target_compile_definitions(pplib_internet PRIVATE HAVE_CURL=1)
        endif()
    endif()

    if(PPLIB_WITH_OPENSSL)
        find_package(OpenSSL QUIET)
        if(OpenSSL_FOUND)
            set(HAVE_OPENSSL 1)
            target_link_libraries(pplib_internet PRIVATE OpenSSL::SSL OpenSSL::Crypto)
            target_compile_definitions(pplib_internet PRIVATE HAVE_OPENSSL=1)
        endif()
    endif()

    if(PPLIB_WITH_MICROHTTPD AND PKG_CONFIG_FOUND)
        pkg_check_modules(MICROHTTPD QUIET libmicrohttpd)
        if(MICROHTTPD_FOUND)
            set(HAVE_LIBMICROHTTPD 1)
            target_include_directories(pplib_internet PRIVATE ${MICROHTTPD_INCLUDE_DIRS})
            target_link_libraries(pplib_internet PRIVATE ${MICROHTTPD_LIBRARIES})
            target_compile_definitions(pplib_internet PRIVATE HAVE_LIBMICROHTTPD=1)
        endif()
    endif()
endif()

# ==============================================================================
# 5. DATABASE (src/database)
# ==============================================================================
if(PPLIB_ENABLE_DATABASE)
    file(GLOB DB_SOURCES CONFIGURE_DEPENDS "src/database/*.cpp")

    add_library(pplib_database STATIC ${DB_SOURCES})
    add_library(pplib::db ALIAS pplib_database)

    target_include_directories(pplib_database PUBLIC ${PPLIB_INCLUDE_DIRS})
    target_link_libraries(pplib_database PUBLIC pplib_core)

    if(PPLIB_WITH_SQLITE3)
        find_package(SQLite3 QUIET)
        if(SQLite3_FOUND)
            set(HAVE_SQLITE3 1)
            target_link_libraries(pplib_database PRIVATE SQLite3::SQLite3)
            target_compile_definitions(pplib_database PRIVATE HAVE_SQLITE3=1)
        endif()
    endif()

    if(PPLIB_WITH_MYSQL AND PKG_CONFIG_FOUND)
        pkg_check_modules(MYSQL QUIET mysqlclient)
        if(MYSQL_FOUND)
            set(HAVE_MYSQL 1)
            target_include_directories(pplib_database PRIVATE ${MYSQL_INCLUDE_DIRS})
            target_link_libraries(pplib_database PRIVATE ${MYSQL_LIBRARIES})
            target_compile_definitions(pplib_database PRIVATE HAVE_MYSQL=1)
        endif()
    endif()

    if(PPLIB_WITH_POSTGRES AND PKG_CONFIG_FOUND)
        pkg_check_modules(POSTGRES QUIET libpq)
        if(POSTGRES_FOUND)
            set(HAVE_POSTGRESQL 1)
            target_include_directories(pplib_database PRIVATE ${POSTGRES_INCLUDE_DIRS})
            target_link_libraries(pplib_database PRIVATE ${POSTGRES_LIBRARIES})
            target_compile_definitions(pplib_database PRIVATE HAVE_POSTGRESQL=1)
        endif()
    endif()
endif()

# ==============================================================================
# 6. AUDIO (src/audio)
# ==============================================================================
if(PPLIB_ENABLE_AUDIO)
    file(GLOB AUDIO_SOURCES CONFIGURE_DEPENDS "src/audio/*.cpp")

    add_library(pplib_audio STATIC ${AUDIO_SOURCES})
    add_library(pplib::audio ALIAS pplib_audio)

    target_include_directories(pplib_audio PUBLIC ${PPLIB_INCLUDE_DIRS})
    target_link_libraries(pplib_audio PUBLIC pplib_core)

    if(PKG_CONFIG_FOUND)
        if(PPLIB_WITH_MPG123)
            pkg_check_modules(MPG123 QUIET libmpg123)
            if(MPG123_FOUND)
                set(HAVE_MPG123 1)
                target_include_directories(pplib_audio PRIVATE ${MPG123_INCLUDE_DIRS})
                target_link_libraries(pplib_audio PRIVATE ${MPG123_LIBRARIES})
                target_compile_definitions(pplib_audio PRIVATE HAVE_MPG123=1)
            endif()
        endif()

        if(PPLIB_WITH_VORBIS)
            pkg_check_modules(VORBIS QUIET vorbis vorbisfile)
            if(VORBIS_FOUND)
                set(HAVE_LIBVORBIS 1)
                target_include_directories(pplib_audio PRIVATE ${VORBIS_INCLUDE_DIRS})
                target_link_libraries(pplib_audio PRIVATE ${VORBIS_LIBRARIES})
                target_compile_definitions(pplib_audio PRIVATE HAVE_VORBIS=1)
            endif()
        endif()
        if(PPLIB_WITH_LAME)
            pkg_check_modules(LAME QUIET lame)
            if(LAME_FOUND)
                set(HAVE_LAME 1)
                target_include_directories(pplib_audio PRIVATE ${LAME_INCLUDE_DIRS})
                target_link_libraries(pplib_audio PRIVATE ${LAME_LIBRARIES})
                target_compile_definitions(pplib_audio PRIVATE HAVE_LAME=1)
            endif()
        endif()
    endif()
endif()


# das landet in ${CMAKE_CURRENT_BINARY_DIR}/include
configure_file(include/config_pplib.h.in include/config_pplib.h)


# ==============================================================================
# Feature & Dependency Summary
# ==============================================================================
function(pplib_bool_str var result_var)
    if(${var})
        set(${result_var} "YES" PARENT_SCOPE)
    else()
        set(${result_var} "NO" PARENT_SCOPE)
    endif()
endfunction()

pplib_bool_str(PPLIB_ENABLE_CORE STR_CORE)
pplib_bool_str(PPLIB_ENABLE_AUDIO STR_AUDIO)
pplib_bool_str(PPLIB_ENABLE_CRYPTO STR_CRYPTO)
pplib_bool_str(PPLIB_ENABLE_GRAFIX STR_GRAFIX)
pplib_bool_str(PPLIB_ENABLE_INTERNET STR_INET)
pplib_bool_str(PPLIB_ENABLE_DATABASE STR_DB)

pplib_bool_str(CMAKE_ASM_NASM_COMPILER_LOADED STR_ASM)
pplib_bool_str(ZLIB_FOUND STR_ZLIB)
pplib_bool_str(BZip2_FOUND STR_BZIP2)
pplib_bool_str(PCRE2_FOUND STR_PCRE2)
pplib_bool_str(ICONV_FOUND STR_ICONV)
pplib_bool_str(OpenSSL_FOUND STR_OPENSSL)
pplib_bool_str(CURL_FOUND STR_CURL)
pplib_bool_str(JPEG_FOUND STR_JPEG)
pplib_bool_str(PNG_FOUND STR_PNG)
pplib_bool_str(TIFF_FOUND STR_TIFF)
pplib_bool_str(Freetype_FOUND STR_FREETYPE)
pplib_bool_str(SQLite3_FOUND STR_SQLITE3)
pplib_bool_str(MYSQL_FOUND STR_MYSQL)
pplib_bool_str(POSTGRES_FOUND STR_POSTGRES)
pplib_bool_str(MPG123_FOUND STR_MPG123)
pplib_bool_str(VORBIS_FOUND STR_VORBIS)
pplib_bool_str(MICROHTTPD_FOUND STR_MICROHTTPD)

pplib_bool_str(PPLIB_BUILD_TESTS STR_TESTS)
pplib_bool_str(PPLIB_ENABLE_COVERAGE STR_COVERAGE)

# ==============================================================================
# Tests
# ==============================================================================
if(PPLIB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests EXCLUDE_FROM_ALL)
endif()

message(STATUS "")
message(STATUS "------------------------------------------------------------------")
message(STATUS " PPLIB Build Configuration Summary (${PROJECT_VERSION})")
message(STATUS "------------------------------------------------------------------")
message(STATUS " Build Type        : ${CMAKE_BUILD_TYPE}")
message(STATUS " C++ Standard      : C++${CMAKE_CXX_STANDARD}")
message(STATUS " Compiler          : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS " Build Tests       : ${STR_TESTS}")
message(STATUS " Enable Coverage   : ${STR_COVERAGE}")
message(STATUS "")
message(STATUS " Enabled Modules:")
message(STATUS "   - core          : ${STR_CORE}")
message(STATUS "   - audio         : ${STR_AUDIO}")
message(STATUS "   - crypto        : ${STR_CRYPTO}")
message(STATUS "   - grafix        : ${STR_GRAFIX}")
message(STATUS "   - internet      : ${STR_INET}")
message(STATUS "   - database      : ${STR_DB}")
message(STATUS "")
message(STATUS " Third-Party Libraries & Features:")
message(STATUS "   - NASM (x86 asm): ${STR_ASM}")
message(STATUS "   - ZLIB          : ${STR_ZLIB}")
message(STATUS "   - BZip2         : ${STR_BZIP2}")
message(STATUS "   - Pcre2         : ${STR_PCRE2}")
message(STATUS "   - Iconv         : ${STR_ICONV}")
message(STATUS "   - OpenSSL       : ${STR_OPENSSL}")
message(STATUS "   - cURL          : ${STR_CURL}")
message(STATUS "   - JPEG          : ${STR_JPEG}")
message(STATUS "   - PNG           : ${STR_PNG}")
message(STATUS "   - TIFF          : ${STR_TIFF}")
message(STATUS "   - FreeType2     : ${STR_FREETYPE}")
message(STATUS "   - SQLite3       : ${STR_SQLITE3}")
message(STATUS "   - MySQL         : ${STR_MYSQL}")
message(STATUS "   - PostgreSQL    : ${STR_POSTGRES}")
message(STATUS "   - MPG123        : ${STR_MPG123}")
message(STATUS "   - Ogg/Vorbis    : ${STR_VORBIS}")
message(STATUS "   - microhttpd    : ${STR_MICROHTTPD}")
message(STATUS "------------------------------------------------------------------")
message(STATUS "")