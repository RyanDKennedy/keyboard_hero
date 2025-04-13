cmake_minimum_required(VERSION 3.22.1)

project(app)

# set(GCC_COVERAGE_LINK_FLAGS "--no-gnu-unique")

set(APP_SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/app.cpp
  ${CMAKE_SOURCE_DIR}/engine/src/sy_arena.cpp
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_library(${PROJECT_NAME} MODULE ${APP_SOURCE_FILES})
elseif(CMAKE_BUILD_TYPE STREQUAL "DebugSanitize")
  add_library(${PROJECT_NAME} STATIC ${APP_SOURCE_FILES})
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_library(${PROJECT_NAME} STATIC ${APP_SOURCE_FILES})
endif()

target_include_directories(${PROJECT_NAME}
  PUBLIC
  ${CMAKE_SOURCE_DIR}/engine/src/
  ${CMAKE_SOURCE_DIR}/engine/external/
)

