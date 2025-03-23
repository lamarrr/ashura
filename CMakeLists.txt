# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.20)

project(Ashura LANGUAGES C CXX ASM)

option(ASH_WERROR "Make all ASH's compilation warnings manifest as errors" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

set(VMA_DYNAMIC_VULKAN_FUNCTIONS
    ON
    CACHE BOOL "")

set(ENABLE_SPVREMAPPER
    OFF
    CACHE BOOL "")
set(SPIRV_REFLECT_EXECUTABLE
    OFF
    CACHE BOOL "")
set(SPIRV_REFLECT_STATIC_LIB
    ON
    CACHE BOOL "")
set(SDL_STATIC
    ON
    CACHE BOOL "")
set(SDL_JOYSTICK
    OFF
    CACHE BOOL "")
set(SDL_HAPTIC
    OFF
    CACHE BOOL "")

set(PNG_SHARED
    OFF
    CACHE BOOL "")

set(SLANG_ENABLE_CUDA
    OFF
    CACHE BOOL "")
set(SLANG_ENABLE_OPTIX
    OFF
    CACHE BOOL "")
set(SLANG_ENABLE_NVAPI
    OFF
    CACHE BOOL "")
set(SLANG_ENABLE_XLIB
    OFF
    CACHE BOOL "")
set(SLANG_ENABLE_AFTERMATH
    OFF
    CACHE BOOL "")
set(SLANG_ENABLE_DX_ON_VK
    OFF
    CACHE BOOL "")
set(SLANG_EMBED_CORE_MODULE_SOURCE
    ON
    CACHE BOOL "")
set(SLANG_EMBED_CORE_MODULE
    OFF
    CACHE BOOL "")
set(SLANG_ENABLE_SLANG_RHI
    OFF
    CACHE BOOL "")
set(SLANG_ENABLE_DXIL
    OFF
    CACHE BOOL "")
set(SLANG_ENABLE_GFX
    OFF
    CACHE BOOL "")
set(SLANG_ENABLE_SLANGD
    OFF
    CACHE BOOL "")
set(SLANG_ENABLE_SLANGRT
    OFF
    CACHE BOOL "")
set(SLANG_ENABLE_SLANG_GLSLANG
    OFF
    CACHE BOOL "")
set(SLANG_ENABLE_TESTS
    OFF
    CACHE BOOL "")
set(SLANG_ENABLE_EXAMPLES
    OFF
    CACHE BOOL "")
set(SLANG_ENABLE_REPLAYER
    OFF
    CACHE BOOL "")
set(SLANG_ENABLE_SPLIT_DEBUG_INFO
    OFF
    CACHE BOOL "")
set(SLANG_USE_SYSTEM_VULKAN_HEADERS
    ON
    CACHE BOOL "")

set(ZLIB_BUILD_TESTING
    OFF
    CACHE BOOL "")
set(ZLIB_BUILD_SHARED
    OFF
    CACHE BOOL "")
set(ZLIB_BUILD_STATIC
    ON
    CACHE BOOL "")
set(ZLIB_BUILD_MINIZIP
    OFF
    CACHE BOOL "")
set(ZLIB_PREFIX
    OFF
    CACHE BOOL "")

set(BUILD_GMOCK
    OFF
    CACHE BOOL "")

set(SIMDJSON_BUILD_STATIC_LIB
    ON
    CACHE BOOL "")

include(FetchContent)
include(ExternalProject)

set(ASH_VULKAN_VERSION "1.4.309.0")
set(ASH_VULKAN_SDK_VERSION "vulkan-sdk-${ASH_VULKAN_VERSION}")

FetchContent_Declare(
  VulkanHeaders
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
  GIT_TAG ${ASH_VULKAN_SDK_VERSION}
  OVERRIDE_FIND_PACKAGE)

FetchContent_Declare(
  VulkanUtilityLibraries
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Utility-Libraries.git
  GIT_TAG ${ASH_VULKAN_SDK_VERSION}
  OVERRIDE_FIND_PACKAGE)

FetchContent_Declare(
  VulkanLoader
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader.git
  GIT_TAG ${ASH_VULKAN_SDK_VERSION}
  OVERRIDE_FIND_PACKAGE)

FetchContent_Declare(
  VulkanMemoryAllocator
  GIT_REPOSITORY
    https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG v3.2.0
  OVERRIDE_FIND_PACKAGE)

FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-3.2.8)

FetchContent_Declare(
  harfbuzz
  GIT_REPOSITORY https://github.com/harfbuzz/harfbuzz.git
  GIT_TAG 8.5.0)

FetchContent_Declare(
  Freetype
  GIT_REPOSITORY https://github.com/freetype/freetype.git
  GIT_TAG VER-2-13-3)

FetchContent_Declare(
  SheenBidi
  GIT_REPOSITORY https://github.com/Tehreer/SheenBidi.git
  GIT_TAG v2.8
  CONFIGURE_COMMAND "" BUILD_COMMAND "")

FetchContent_Declare(
  WebP
  GIT_REPOSITORY https://chromium.googlesource.com/webm/libwebp
  GIT_TAG 1.4.0)

FetchContent_Declare(
  CURL
  GIT_REPOSITORY https://github.com/curl/curl.git
  GIT_TAG curl-8_8_0)

FetchContent_Declare(
  xxHash
  GIT_REPOSITORY https://github.com/Cyan4973/xxHash.git
  GIT_TAG v0.8.3
  CONFIGURE_COMMAND "" BUILD_COMMAND "")

FetchContent_Declare(
  fast_float
  GIT_REPOSITORY https://github.com/fastfloat/fast_float.git
  GIT_TAG v7.0.0
  CONFIGURE_COMMAND "" BUILD_COMMAND "")

FetchContent_Declare(
  simdjson
  GIT_REPOSITORY https://github.com/simdjson/simdjson.git
  GIT_TAG dfa43f6)

FetchContent_Declare(
  libpng
  GIT_REPOSITORY https://git.code.sf.net/p/libpng/code
  GIT_TAG 44f97f)

FetchContent_Declare(
  gtest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG 0bdccf4)

FetchContent_Declare(
  slang
  GIT_REPOSITORY https://github.com/shader-slang/slang.git
  GIT_TAG ${ASH_VULKAN_SDK_VERSION})

find_package(FFMPEG REQUIRED)
find_package(JPEG REQUIRED)
find_package(Threads REQUIRED)

FetchContent_MakeAvailable(VulkanHeaders)
FetchContent_MakeAvailable(VulkanUtilityLibraries)
FetchContent_MakeAvailable(VulkanLoader)
FetchContent_MakeAvailable(VulkanMemoryAllocator)
FetchContent_MakeAvailable(harfbuzz)
FetchContent_MakeAvailable(Freetype)
FetchContent_MakeAvailable(SheenBidi)
FetchContent_MakeAvailable(SDL3)
FetchContent_MakeAvailable(WebP)
FetchContent_MakeAvailable(CURL)
FetchContent_MakeAvailable(xxHash)
FetchContent_MakeAvailable(fast_float)
FetchContent_MakeAvailable(simdjson)
FetchContent_MakeAvailable(libpng)
FetchContent_MakeAvailable(gtest)
FetchContent_MakeAvailable(slang)

set(xxHash_INCLUDE_DIR ${xxhash_SOURCE_DIR})
set(fast_float_INCLUDE_DIR ${fast_float_SOURCE_DIR}/include)

file(GLOB SHEENBIDI_SOURCES ${sheenbidi_SOURCE_DIR}/Source/*.c)

add_library(sheenbidi STATIC ${SHEENBIDI_SOURCES})
target_include_directories(
  sheenbidi
  PUBLIC ${sheenbidi_SOURCE_DIR}/Headers
  PRIVATE ${sheenbidi_SOURCE_DIR}/Source)
add_library(sheenbidi::sheenbidi ALIAS sheenbidi)

list(APPEND ASH_COMPILE_OPTIONS)
list(APPEND ASH_COMPILE_DEFINITIONS)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  list(APPEND ASH_COMPILE_OPTIONS -Wall -Wextra -Wpedantic)

  if(ASH_WERROR)
    list(APPEND ASH_COMPILE_OPTIONS -Werror)
  endif()

else()
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(
      APPEND
      ASH_COMPILE_OPTIONS
      -Wall
      -Wextra
      -Wpedantic
      -Wno-nullability-completeness
      -Wno-nullability-extension
      -Wno-bitwise-instead-of-logical)

    if(ASH_WERROR)
      list(APPEND ASH_COMPILE_OPTIONS -Werror)
    endif()

  else()
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
      list(APPEND ASH_COMPILE_OPTIONS /W3 /Zc:preprocessor /permissive)

      if(ASH_WERROR)
        list(APPEND ASH_COMPILE_OPTIONS /Wx)
      endif()

    endif()
  endif()
endif()

if(WIN32)
  list(APPEND ASH_COMPILE_DEFINITIONS _CRT_SECURE_NO_WARNINGS)
endif()

# ASHURA STD

add_library(
  ashura_std STATIC
  ashura/std/allocator.cc
  ashura/std/allocators.cc
  ashura/std/async.cc
  ashura/std/format.cc
  ashura/std/fs.cc
  ashura/std/hash.cc
  ashura/std/log.cc
  ashura/std/panic.cc
  ashura/std/trace.cc)
target_include_directories(
  ashura_std
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${xxHash_INCLUDE_DIR})
target_link_libraries(ashura_std PUBLIC Threads::Threads)
target_compile_options(ashura_std PUBLIC ${ASH_COMPILE_OPTIONS})
target_compile_definitions(ashura_std PUBLIC ${ASH_COMPILE_DEFINITIONS})

# ASHURA STD - TESTS

add_executable(
  ashura_std_tests
  ashura/std/tests/async.cc
  ashura/std/tests/enum.cc
  ashura/std/tests/hash_map.cc
  ashura/std/tests/list.cc
  ashura/std/tests/main.cc
  ashura/std/tests/option.cc
  ashura/std/tests/range.cc
  ashura/std/tests/result.cc
  ashura/std/tests/sparse_vec.cc)

target_link_libraries(ashura_std_tests ashura_std GTest::gtest
                      GTest::gtest_main)

# ASHURA STD - BENCHMARKS

# add_executable(ashura_std_bench ashura/std/bench/hash_map.cc)

# target_link_libraries(ashura_std_bench benchmark::benchmark
# benchmark::benchmark_main ashura_std)

# ASHURA GPU

add_library(ashura_gpu STATIC ashura/gpu/gpu.cc ashura/gpu/vma_def.cc
                              ashura/gpu/vulkan.cc)
target_link_libraries(
  ashura_gpu
  PUBLIC ashura_std
  PUBLIC Vulkan-Headers Vulkan::UtilityHeaders Vulkan::Loader
         GPUOpen::VulkanMemoryAllocator)

target_include_directories(ashura_gpu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(ashura_gpu PUBLIC ${ASH_COMPILE_OPTIONS})
target_compile_definitions(ashura_gpu PUBLIC ${ASH_COMPILE_DEFINITIONS})

# ASHURA ENGINE

add_library(
  ashura_engine STATIC
  ashura/engine/animation.cc
  ashura/engine/canvas.cc
  ashura/engine/engine.cc
  ashura/engine/font_impl.cc
  ashura/engine/font_system_impl.cc
  ashura/engine/font.cc
  ashura/engine/gpu_system.cc
  ashura/engine/image_decoder.cc
  ashura/engine/passes.cc
  ashura/engine/render_text.cc
  ashura/engine/renderer.cc
  ashura/engine/scene.cc
  ashura/engine/shader.cc
  ashura/engine/systems.cc
  ashura/engine/text_compositor.cc
  ashura/engine/text.cc
  ashura/engine/view_system.cc
  ashura/engine/view.cc
  ashura/engine/views.cc
  ashura/engine/window.cc)

target_link_libraries(
  ashura_engine
  PUBLIC ashura_std ashura_gpu
  PRIVATE slang
          SDL3::SDL3-static
          harfbuzz
          CURL::libcurl
          freetype
          simdjson::simdjson_static
          webp
          webpdecoder
          JPEG::JPEG
          png_static
          sheenbidi::sheenbidi
          ${FFMPEG_LIBRARIES})

target_include_directories(
  ashura_engine
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${FFMPEG_INCLUDE_DIRS} ${FFMPEG_LIBRARY_DIRS}
          ${fast_float_INCLUDE_DIR})
target_compile_options(ashura_engine PUBLIC ${ASH_COMPILE_OPTIONS})
target_compile_definitions(ashura_engine PUBLIC ${ASH_COMPILE_DEFINITIONS})

# ASHURA ENGINE - TESTS

add_executable(
  ashura_engine_tests
  ashura/engine/tests/main.cc ashura/engine/tests/animation.cc
  ashura/engine/tests/render_text.cc ashura/engine/tests/views.cc
  ashura/engine/tests/text_compositor.cc)
target_link_libraries(ashura_engine_tests ashura_std ashura_engine GTest::gtest)

# ASHURA EDITOR

add_executable(ashura_editor ashura/editor/editor.cc)
target_link_libraries(ashura_editor ashura_std ashura_engine)
