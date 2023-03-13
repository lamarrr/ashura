cmake_minimum_required(VERSION 3.1)

project(Ashura)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(add_project_library)
include(FetchContent)

FetchContent_Declare(
  stx
  GIT_REPOSITORY https://github.com/lamarrr/STX.git
  GIT_TAG main)

FetchContent_Declare(
  aom
  GIT_REPOSITORY https://aomedia.googlesource.com/aom
  GIT_TAG v3.6.0)

find_program(NASM nasm REQUIRED)
find_package(Vulkan REQUIRED)
find_package(Freetype REQUIRED)
find_package(harfbuzz REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(simdjson CONFIG REQUIRED)
find_package(GTest REQUIRED)
find_package(WebP CONFIG REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(SDL3 REQUIRED)
find_package(Ogg CONFIG REQUIRED)
find_package(Vorbis CONFIG REQUIRED)
find_package(Opus CONFIG REQUIRED)
find_package(libavif CONFIG REQUIRED)
find_package(FFMPEG REQUIRED)

FetchContent_MakeAvailable(stx)
FetchContent_MakeAvailable(aom)
add_subdirectory(libwebm_parser)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  add_compile_options(-fdiagnostics-color=always -Wall -Wextra -Wpedantic)
else()
  if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics -Wall -Wextra -Wpedantic)
  else()
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
      # add_compile_options(/Wall)
    endif()
  endif()
endif()

# add_subdirectory(assets)
add_subdirectory(ashura)
