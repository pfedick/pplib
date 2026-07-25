cmake_minimum_required(VERSION 3.20)

# GoogleTest (im Unterordner) einbinden
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(googletest-release-1.12.1 EXCLUDE_FROM_ALL)

# Gemeinsame Test-Ressourcen/Klassen (main.cpp, textsnippets.cpp, wordlist.cpp)
add_library(ppl7_test_common STATIC
    src/main.cpp
    src/textsnippets.cpp
    src/wordlist.cpp
)

target_include_directories(ppl7_test_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ppl7_test_common PUBLIC gtest ppl7::core)

include(GoogleTest)

# Hilfsmakro zum Erstellen einzelner Testsuites
macro(add_ppl7_test_suite name source_glob link_lib)
    file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${source_glob})
    if(TEST_SOURCES)
        add_executable(${name} ${TEST_SOURCES})
        target_link_libraries(${name} PRIVATE ppl7_test_common ${link_lib})
        gtest_discover_tests(${name}
            EXTRA_ARGS "--gtest_output=xml:${CMAKE_BINARY_DIR}/testresult_${name}.xml"
        )
    endif()
endmacro()

if(PPL7_ENABLE_CORE)
    add_ppl7_test_suite(test_core "src/core/*.cpp" ppl7::core)
endif()

if(PPL7_ENABLE_CRYPTO)
    add_ppl7_test_suite(test_crypto "src/crypto/*.cpp" ppl7::crypto)
endif()

if(PPL7_ENABLE_DATABASE)
    add_ppl7_test_suite(test_database "src/database/*.cpp" ppl7::db)
endif()

if(PPL7_ENABLE_GRAFIX)
    add_ppl7_test_suite(test_grafix "src/grafix/*.cpp" ppl7::grafix)
endif()

if(PPL7_ENABLE_INTERNET)
    add_ppl7_test_suite(test_inet "src/inet/*.cpp" ppl7::internet)
endif()

if(PPL7_ENABLE_AUDIO)
    add_ppl7_test_suite(test_audio "src/audio/*.cpp" ppl7::audio)
endif()
