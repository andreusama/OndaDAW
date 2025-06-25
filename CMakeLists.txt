cmake_minimum_required(VERSION 3.15)
project(MiniDAW)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

# SDL3
FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-3.2.16   # valid tag
)

FetchContent_MakeAvailable(SDL3)

# ImGui
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.9b
)
FetchContent_MakeAvailable(imgui)

add_library(imgui_lib
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui_lib PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui_lib PUBLIC SDL3::SDL3)

# miniaudio (single header)
add_library(miniaudio_lib INTERFACE)
target_include_directories(miniaudio_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)

# Executable
add_executable(mydaw LearnC++/LearnC++.cpp)

target_link_libraries(mydaw
  PRIVATE SDL3::SDL3 imgui_lib miniaudio_lib
)

# Platform-specific libs
if (APPLE)
  find_library(COCOA Cocoa)
  find_library(IOKIT IOKit)
  find_library(COREVIDEO CoreVideo)
  target_link_libraries(mydaw PRIVATE ${COCOA} ${IOKIT} ${COREVIDEO})
elseif (UNIX)
  find_package(OpenGL REQUIRED)
  target_link_libraries(mydaw PRIVATE OpenGL::GL)
endif()

# Include directories
target_include_directories(mydaw PRIVATE
  ${SDL3_INCLUDE_DIRS}
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
  thirdparty
)
