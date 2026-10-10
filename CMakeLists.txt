cmake_minimum_required(VERSION 3.22)
project(BookMatch VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)

add_library(bookmatch_core
    src/core/document.cpp
    src/core/file_scanner.cpp
    src/text/text_normalizer.cpp
    src/similarity/similarity.cpp
)
target_include_directories(bookmatch_core PUBLIC include)
target_link_libraries(bookmatch_core PUBLIC OpenSSL::Crypto)
target_compile_features(bookmatch_core PUBLIC cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(bookmatch_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()

add_executable(bookmatch src/main.cpp)
target_link_libraries(bookmatch PRIVATE bookmatch_core)

include(CTest)
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    add_executable(bookmatch_tests
        tests/unit/text_normalizer_test.cpp
        tests/unit/similarity_test.cpp
        tests/unit/file_scanner_test.cpp
    )
    target_link_libraries(bookmatch_tests PRIVATE bookmatch_core GTest::gtest_main)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(bookmatch_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
    endif()
    include(GoogleTest)
    gtest_discover_tests(bookmatch_tests)
endif()
