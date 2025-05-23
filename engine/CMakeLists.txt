cmake_minimum_required(VERSION 3.22.1)

project(syengine)

# PLATFORM UNSPECIFIC FILES
set(SY_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sy_syengine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sy_arena.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sy_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/sy_render_info.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/sy_resources.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/sy_physical_device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/sy_logical_device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/sy_swapchain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/sy_pipeline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/sy_buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/sy_drawing.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/sy_descriptor_allocator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/sy_fonts.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/sy_image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sound/sy_sound.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sound/sy_play.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/asset_system/sy_obj_parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/asset_system/sy_asset_system.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/external/vma.cpp
)

# PLATFORM UNSPECIFIC FILES
set(SY_LINK_LIBRARIES
  Vulkan::Vulkan
  freetype
  openal
  alut
)

set(SY_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/
  ${CMAKE_CURRENT_SOURCE_DIR}/external/
  )

if(UNIX)
  list(APPEND SY_SOURCE_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sy_linux_entry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sy_linux_window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sy_linux_input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/sy_linux_init.cpp
  )

  list(APPEND SY_LINK_LIBRARIES
    dl
    xcb
    xcb-xkb
    xkbcommon
    xkbcommon-x11
    xcb-xinput
  )

  list(APPEND SY_INCLUDE_DIRECTORIES
    /usr/include/freetype2/  
  )
endif(UNIX)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  list(APPEND SY_LINK_LIBRARIES app)
elseif(CMAKE_BUILD_TYPE STREQUAL "DebugSanitize")
  list(APPEND SY_LINK_LIBRARIES app)
endif()

add_executable(${PROJECT_NAME} ${SY_SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
  PUBLIC
  ${SY_INCLUDE_DIRECTORIES}
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  ${SY_LINK_LIBRARIES}
)

