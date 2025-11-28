cmake_minimum_required(VERSION 3.28)
project(positionless_cpp)

set(CMAKE_CXX_STANDARD 23)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-stdlib=libc++)
    add_link_options(-stdlib=libc++)
endif()

# Setup CPM
file(DOWNLOAD
    https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.2/CPM.cmake
    ${CMAKE_BINARY_DIR}/cmake/CPM.cmake
    EXPECTED_HASH SHA256=c8cdc32c03816538ce22781ed72964dc864b2a34a310d3b7104812a5ca2d835d
)
include(${CMAKE_BINARY_DIR}/cmake/CPM.cmake)

# Add doctest
CPMAddPackage(
    NAME doctest
    GITHUB_REPOSITORY doctest/doctest
    VERSION 2.4.12
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(positionless INTERFACE)
target_include_directories(positionless INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(positionless INTERFACE
    $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

add_executable(unit_tests test/partitioning_tests.cpp)
target_link_libraries(unit_tests PRIVATE positionless doctest::doctest)
target_compile_options(unit_tests PRIVATE
    $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

enable_testing()
add_test(NAME unit_tests COMMAND unit_tests)
