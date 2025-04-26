cmake_minimum_required(VERSION 3.22.1)

project(project)

set(CMAKE_EXPORT_COMPILE_COMMANDS On)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_STANDARD 20)

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(SY_EXTRA_APP_LIBRARIES)
find_package(Vulkan REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/debug)
  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/debug)
  set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/debug)
  set(CMAKE_CXX_FLAGS "-g3 -Wall")
elseif(CMAKE_BUILD_TYPE STREQUAL "DebugSanitize")
  file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/debug)
  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/debug)
  set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/debug)
  set(CMAKE_CXX_FLAGS "-g3 -Wall -fsanitize=address -DNDEBUG")
  set(SY_EXTRA_APP_LIBRARIES Vulkan::Vulkan)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/release)
  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/release)
  set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/release)
  set(CMAKE_CXX_FLAGS "-O2 -DNDEBUG")
  set(SY_EXTRA_APP_LIBRARIES Vulkan::Vulkan)
endif()

add_subdirectory(app)
add_subdirectory(engine)

