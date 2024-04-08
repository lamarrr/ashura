cmake_minimum_required(VERSION 3.6)

project(Ashura)

set(CMAKE_CXX_STANDARD 20)
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

include(FetchContent)

FetchContent_Declare(
  VulkanHeaders
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
  GIT_TAG v1.3.280)

FetchContent_Declare(
  VulkanGLSL
  GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
  GIT_TAG vulkan-sdk-1.3.280.0)

FetchContent_Declare(
  SpirvReflect
  GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Reflect.git
  GIT_TAG vulkan-sdk-1.3.280.0)

FetchContent_Declare(
  VulkanUtilityLibraries
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Utility-Libraries.git
  GIT_TAG v1.3.280)

FetchContent_Declare(
  VulkanMemoryAllocator
  GIT_REPOSITORY
    https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG 19b940e)

FetchContent_Declare(
  SheenBidi
  GIT_REPOSITORY https://github.com/Tehreer/SheenBidi.git
  GIT_TAG v2.7
  CONFIGURE_COMMAND "" BUILD_COMMAND "")

find_package(Freetype REQUIRED)
find_package(harfbuzz REQUIRED)
find_package(simdjson CONFIG REQUIRED)
find_package(GTest REQUIRED)
find_package(WebP CONFIG REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(SDL3 REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(xxHash CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)

FetchContent_MakeAvailable(VulkanHeaders)
FetchContent_MakeAvailable(VulkanUtilityLibraries)
FetchContent_MakeAvailable(VulkanMemoryAllocator)
FetchContent_MakeAvailable(VulkanGLSL)
FetchContent_MakeAvailable(SheenBidi)
FetchContent_MakeAvailable(SpirvReflect)
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
    add_compile_options(
      -Wall
      -Wextra
      -Wpedantic
      -Wno-nullability-completeness
      -Wno-nullability-extension)
  else()
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
      add_compile_options(/W3)
      add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    endif()
  endif()
endif()

add_library(
  ashura_std
  ashura/std/async.cc
  ashura/std/io.cc
  ashura/std/allocator.cc
  ashura/std/hash.cc
  ashura/std/arena_allocator.cc
  ashura/std/format.cc
  ashura/std/log.cc
  ashura/std/runtime.cc)
target_include_directories(ashura_std PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ashura_std PRIVATE xxHash::xxhash)

add_executable(
  ashura_std_tests
  ashura/std/tests/tests.cc ashura/std/tests/option.cc
  ashura/std/tests/hash_map.cc ashura/std/tests/result.cc
  ashura/std/tests/bit_span.cc ashura/std/tests/sparse_set.cc)
add_executable(ashura_std_bench ashura/std/bench/hash_map.cc)

target_link_libraries(ashura_std_tests ashura_std GTest::gtest
                      GTest::gtest_main)
target_link_libraries(ashura_std_bench benchmark::benchmark
                      benchmark::benchmark_main ashura_std)

add_library(ashura_gfx ashura/gfx/vma_def.cc ashura/gfx/vulkan.cc)
target_link_libraries(
  ashura_gfx ashura_std Vulkan-Headers Vulkan::UtilityHeaders Vulkan::Vulkan
  GPUOpen::VulkanMemoryAllocator)
target_include_directories(ashura_gfx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(
  ashura_renderer
  ashura/renderer/scene.cc
  ashura/renderer/view.cc
  ashura/renderer/renderer.cc
  ashura/renderer/render_context.cc
  ashura/renderer/shader.cc
  ashura/renderer/passes/bloom.cc
  ashura/renderer/passes/blur.cc
  ashura/renderer/passes/custom.cc
  ashura/renderer/passes/fxaa.cc
  ashura/renderer/passes/msaa.cc
  ashura/renderer/passes/pbr.cc
  ashura/renderer/passes/rrect.cc)
target_link_libraries(ashura_renderer ashura_std ashura_gfx)
target_include_directories(ashura_renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(
  ashura_engine ashura/engine/image_decoder.cc ashura/engine/shader.cc
                ashura/engine/window.cc ashura/engine/widget.cc)
target_link_libraries(
  ashura_engine
  ashura_std
  ashura_gfx
  ashura_renderer
  SPIRV
  glslang
  spirv-reflect-static)
target_include_directories(
  ashura_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${JPEG_INCLUDE_DIR}
                       ${FFMPEG_INCLUDE_DIRS} ${FFMPEG_LIBRARY_DIRS})

target_link_libraries(
  ashura_engine
  ashura_std
  ashura_gfx
  SDL3::SDL3-static
  harfbuzz
  CURL::libcurl
  Freetype::Freetype
  simdjson::simdjson
  WebP::webp
  WebP::webpdecoder
  ${JPEG_LIBRARIES}
  PNG::PNG
  ${FFMPEG_LIBRARIES}
  SheenBidi::SheenBidi)

add_executable(ashura_engine_test ashura/engine/tests/test.cc)
target_link_libraries(ashura_engine_test ashura_engine)
