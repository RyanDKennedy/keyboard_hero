cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS On)
set(CMAKE_DEBUG_POSTFIX DEBUG)

project(test)

add_subdirectory(engine)
add_subdirectory(app)

