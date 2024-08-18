cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS On)

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/debug)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/debug)
    set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/debug)
    set(GCC_COVERAGE_COMPILE_FLAGS "-g3")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/release)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/release)
    set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/release)
    set(GCC_COVERAGE_COMPILE_FLAGS "-o3")
endif()

project(test)

add_subdirectory(engine)
add_subdirectory(app)

