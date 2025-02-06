
project(app)

# set(GCC_COVERAGE_LINK_FLAGS "--no-gnu-unique")

add_library(${PROJECT_NAME} MODULE
    ${CMAKE_CURRENT_SOURCE_DIR}/app.cpp
    ${CMAKE_SOURCE_DIR}/engine/src/sy_arena.cpp
    ${CMAKE_SOURCE_DIR}/engine/src/render/sy_render.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/engine/src/
)

