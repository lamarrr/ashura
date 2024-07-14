# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.6)

project(Ashura)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

set(VMA_DYNAMIC_VULKAN_FUNCTIONS
    ON
    CACHE BOOL "")
set(ENABLE_OPT
    OFF
    CACHE BOOL "")
set(ENABLE_HLSL
    OFF
    CACHE BOOL "")
set(ENABLE_GLSLANG_BINARIES
    OFF
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

include(FetchContent)

FetchContent_Declare(
  VulkanHeaders
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
  GIT_TAG vulkan-sdk-1.3.283.0)

FetchContent_Declare(
  VulkanGLSL
  GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
  GIT_TAG vulkan-sdk-1.3.283.0)

FetchContent_Declare(
  SpirvReflect
  GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Reflect.git
  GIT_TAG vulkan-sdk-1.3.283.0)

FetchContent_Declare(
  VulkanUtilityLibraries
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Utility-Libraries.git
  GIT_TAG vulkan-sdk-1.3.283.0)

FetchContent_Declare(
  VulkanMemoryAllocator
  GIT_REPOSITORY
    https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG v3.1.0)

FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG 98499d6)

FetchContent_Declare(
  harfbuzz
  GIT_REPOSITORY https://github.com/harfbuzz/harfbuzz.git
  GIT_TAG 8.5.0)

FetchContent_Declare(
  Freetype
  GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
  GIT_TAG VER-2-13-2)

FetchContent_Declare(
  SheenBidi
  GIT_REPOSITORY https://github.com/Tehreer/SheenBidi.git
  GIT_TAG v2.7
  CONFIGURE_COMMAND "" BUILD_COMMAND "")

FetchContent_Declare(
  WebP
  GIT_REPOSITORY https://chromium.googlesource.com/webm/libwebp
  GIT_TAG 1.4.0)

FetchContent_Declare(
  CURL
  GIT_REPOSITORY https://github.com/curl/curl.git
  GIT_TAG curl-8_8_0)

find_package(simdjson CONFIG REQUIRED)
find_package(GTest REQUIRED)
find_package(libjpeg-turbo REQUIRED)
find_package(PNG REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(xxHash CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)
find_package(Threads REQUIRED)

FetchContent_MakeAvailable(VulkanHeaders)
FetchContent_MakeAvailable(VulkanUtilityLibraries)
FetchContent_MakeAvailable(VulkanMemoryAllocator)
FetchContent_MakeAvailable(VulkanGLSL)
FetchContent_MakeAvailable(harfbuzz)
FetchContent_MakeAvailable(Freetype)
FetchContent_MakeAvailable(SheenBidi)
FetchContent_MakeAvailable(SpirvReflect)
FetchContent_MakeAvailable(SDL3)
FetchContent_MakeAvailable(WebP)
FetchContent_MakeAvailable(CURL)

find_package(Vulkan REQUIRED)

file(GLOB SheenBidi_SOURCES ${sheenbidi_SOURCE_DIR}/Source/*.c)

add_library(SheenBidi ${SheenBidi_SOURCES})
target_include_directories(
  SheenBidi
  PUBLIC ${sheenbidi_SOURCE_DIR}/Headers
  PRIVATE ${sheenbidi_SOURCE_DIR}/Source)
add_library(SheenBidi::SheenBidi ALIAS SheenBidi)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  add_compile_options(-Wall -Wextra -Wpedantic)
else()
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wno-nullability-completeness
                        -Wno-nullability-extension)
  else()
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
      add_compile_options(/W3 /Zc:preprocessor)
      add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    endif()
  endif()
endif()

add_library(
  ashura_std
  ashura/std/semaphore.cc
  ashura/std/async.cc
  ashura/std/io.cc
  ashura/std/allocator.cc
  ashura/std/hash.cc
  ashura/std/arena_allocator.cc
  ashura/std/format.cc
  ashura/std/log.cc
  ashura/std/runtime.cc)
target_include_directories(ashura_std PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ashura_std PUBLIC xxHash::xxhash Threads::Threads)

add_executable(
  ashura_std_tests
  ashura/std/tests/tests.cc
  ashura/std/tests/option.cc
  ashura/std/tests/async.cc
  ashura/std/tests/list.cc
  ashura/std/tests/hash_map.cc
  ashura/std/tests/result.cc
  ashura/std/tests/bit_span.cc
  ashura/std/tests/sparse_set.cc)
add_executable(ashura_std_bench ashura/std/bench/hash_map.cc)

target_link_libraries(ashura_std_tests ashura_std GTest::gtest
                      GTest::gtest_main)
target_link_libraries(ashura_std_bench benchmark::benchmark
                      benchmark::benchmark_main ashura_std)

add_library(ashura_gfx ashura/gfx/gfx.cc ashura/gfx/vma_def.cc
                       ashura/gfx/vulkan.cc)
target_link_libraries(
  ashura_gfx ashura_std Vulkan-Headers Vulkan::UtilityHeaders Vulkan::Vulkan
  GPUOpen::VulkanMemoryAllocator)
target_include_directories(ashura_gfx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(
  ashura_engine
  ashura/engine/font.cc
  ashura/engine/font_impl.cc
  ashura/engine/text.cc
  ashura/engine/canvas.cc
  ashura/engine/image_decoder.cc
  ashura/engine/shader.cc
  ashura/engine/text_compositor.cc
  ashura/engine/window.cc
  ashura/engine/widget.cc
  ashura/engine/widget_system.cc
  ashura/engine/widgets/button.cc
  ashura/engine/widgets/check_box.cc
  ashura/engine/widgets/collapsable.cc
  ashura/engine/widgets/color_picker.cc
  ashura/engine/widgets/combo_box.cc
  ashura/engine/widgets/demo.cc
  ashura/engine/widgets/flex_box.cc
  ashura/engine/widgets/input_box.cc
  ashura/engine/widgets/plot.cc
  ashura/engine/widgets/progress_bar.cc
  ashura/engine/widgets/radio_box.cc
  ashura/engine/widgets/scroll_box.cc
  ashura/engine/widgets/slider.cc
  ashura/engine/widgets/stack_box.cc
  ashura/engine/widgets/switch.cc
  ashura/engine/widgets/tab.cc
  ashura/engine/widgets/text_box.cc
  ashura/engine/scene.cc
  ashura/engine/renderer.cc
  ashura/engine/render_context.cc
  ashura/engine/passes/bloom.cc
  ashura/engine/passes/blur.cc
  ashura/engine/passes/ngon.cc
  ashura/engine/passes/pbr.cc
  ashura/engine/passes/poly.cc
  ashura/engine/passes/rrect.cc)
target_link_libraries(
  ashura_engine
  ashura_std
  ashura_gfx
  SPIRV
  glslang
  spirv-reflect-static
  SDL3::SDL3-static
  harfbuzz
  CURL::libcurl
  freetype
  simdjson::simdjson
  webp
  webpdecoder
  $<IF:$<TARGET_EXISTS:libjpeg-turbo::turbojpeg>,libjpeg-turbo::turbojpeg,libjpeg-turbo::turbojpeg-static>
  PNG::PNG
  ${FFMPEG_LIBRARIES}
  SheenBidi::SheenBidi)

target_include_directories(
  ashura_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${JPEG_INCLUDE_DIR}
                       ${FFMPEG_INCLUDE_DIRS} ${FFMPEG_LIBRARY_DIRS})

add_executable(ashura_engine_test ashura/engine/tests/test.cc)
target_link_libraries(ashura_engine_test ashura_engine)
