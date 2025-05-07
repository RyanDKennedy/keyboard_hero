cmake_minimum_required(VERSION 3.22.1)

project(app)

set(APP_SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/app.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/util.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/db.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/menu.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/picker.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/edit.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/create.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/play.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/deletion.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/FT232H_device.cpp
  ${CMAKE_SOURCE_DIR}/engine/src/sy_arena.cpp
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_library(${PROJECT_NAME} MODULE ${APP_SOURCE_FILES})
elseif(CMAKE_BUILD_TYPE STREQUAL "DebugSanitize")
  add_library(${PROJECT_NAME} STATIC ${APP_SOURCE_FILES})
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_library(${PROJECT_NAME} STATIC ${APP_SOURCE_FILES})
endif()

find_package(SQLite3 REQUIRED)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/
  ${CMAKE_SOURCE_DIR}/engine/src/
  ${CMAKE_SOURCE_DIR}/engine/external/
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  SQLite::SQLite3
  ftd2xx
  ${SY_EXTRA_APP_LIBRARIES}
)

set_target_properties(${PROJECT_NAME}
       PROPERTIES
       LINK_FLAGS "-Wl,-rpath,/usr/local/lib"
)
