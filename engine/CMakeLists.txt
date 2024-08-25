project(syengine)

# PLATFORM UNSPECIFIC FILES
set(SY_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sy_syengine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sy_arena.cpp
)

# PLATFORM UNSPECIFIC FILES
set(SY_LINK_LIBRARIES)

if (UNIX)
    list(APPEND SY_SOURCE_FILES 
      ${CMAKE_CURRENT_SOURCE_DIR}/src/sy_linux_entry.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/src/sy_linux_window.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/src/sy_linux_input.cpp
    )

    list(APPEND SY_LINK_LIBRARIES
      dl
      xcb
      xcb-xkb
      xkbcommon
      xkbcommon-x11
    )
endif (UNIX)

add_executable(${PROJECT_NAME} ${SY_SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/
)

target_link_libraries(${PROJECT_NAME} ${SY_LINK_LIBRARIES})

