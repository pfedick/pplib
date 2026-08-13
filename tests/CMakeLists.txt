cmake_minimum_required(VERSION 3.20)

# GoogleTest (im Unterordner) einbinden
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_subdirectory(googletest-1.18.0 EXCLUDE_FROM_ALL)

# Gemeinsame Test-Ressourcen/Klassen (main.cpp, textsnippets.cpp, wordlist.cpp)
add_library(pplib_test_common STATIC
    src/main.cpp
    src/textsnippets.cpp
    src/wordlist.cpp
)

target_include_directories(pplib_test_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(pplib_test_common PUBLIC gtest pplib::core)

include(GoogleTest)

# Hilfsmakro zum Erstellen einzelner Testsuites
macro(add_pplib_test_suite name source_glob link_lib)
    file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${source_glob})
    if(TEST_SOURCES)
        add_executable(${name} ${TEST_SOURCES})
        target_link_libraries(${name} PRIVATE pplib_test_common ${link_lib})
        gtest_discover_tests(${name}
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            EXTRA_ARGS "--gtest_output=xml:${CMAKE_BINARY_DIR}/testresult_${name}.xml"
        )
    endif()
endmacro()

if(PPLIB_ENABLE_CORE)
    add_pplib_test_suite(test_core "src/core/*.cpp" pplib::core)
    add_pplib_test_suite(test_core "src/types/*.cpp" pplib::core)
endif()

if(PPLIB_ENABLE_CRYPTO)
    add_pplib_test_suite(test_crypto "src/crypto/*.cpp" pplib::crypto)
endif()

if(PPLIB_ENABLE_DATABASE)
    add_pplib_test_suite(test_database "src/database/*.cpp" pplib::db)
endif()

if(PPLIB_ENABLE_GRAFIX)
    add_pplib_test_suite(test_grafix "src/grafix/*.cpp" pplib::grafix)
endif()

if(PPLIB_ENABLE_INTERNET)
    add_pplib_test_suite(test_inet "src/inet/*.cpp" pplib::internet)
endif()

if(PPLIB_ENABLE_AUDIO)
    add_pplib_test_suite(test_audio "src/audio/*.cpp" pplib::audio)
endif()
