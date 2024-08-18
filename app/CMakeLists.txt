
project(app)

set(CMAKE_DEBUG_POSTFIX "")

add_library(${PROJECT_NAME} MODULE
    ${CMAKE_CURRENT_SOURCE_DIR}/app.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/engine/include/
)

