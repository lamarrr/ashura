cmake_minimum_required(VERSION 3.6)

project(Ashura)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

set(VMA_DYNAMIC_VULKAN_FUNCTIONS ON)

include(FetchContent)

FetchContent_Declare(
  VulkanHeaders
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
  GIT_TAG v1.3.276)

FetchContent_Declare(
  VulkanUtilityLibraries
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Utility-Libraries.git
  GIT_TAG v1.3.276)

FetchContent_Declare(
  VulkanMemoryAllocator
  GIT_REPOSITORY
    https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG 94b1aca)

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

FetchContent_MakeAvailable(VulkanHeaders)
FetchContent_MakeAvailable(VulkanUtilityLibraries)
FetchContent_MakeAvailable(VulkanMemoryAllocator)
FetchContent_MakeAvailable(SheenBidi)
find_package(Vulkan REQUIRED)

file(GLOB SheenBidi_SOURCES ${sheenbidi_SOURCE_DIR}/Source/*.c)

add_library(SheenBidi ${SheenBidi_SOURCES} "ashura/std/bit_span.h")
target_include_directories(
  SheenBidi
  PUBLIC ${sheenbidi_SOURCE_DIR}/Headers
  PRIVATE ${sheenbidi_SOURCE_DIR}/Source)
add_library(SheenBidi::SheenBidi ALIAS SheenBidi)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  add_compile_options(-Wall -Wextra -Wpedantic -fno-rtti -fno-exceptions)
else()
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -fno-rtti -fno-exceptions)
  else()
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
      add_compile_options(/W3)
    endif()
  endif()
endif()

add_library(
  ashura_std ashura/std/allocator.cc ashura/std/arena_allocator.cc
             ashura/std/format.cc ashura/std/log.cc ashura/std/runtime.cc)
target_include_directories(ashura_std PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(
  ashura_std_tests ashura/std/tests/tests.cc ashura/std/tests/option.cc
                   ashura/std/tests/result.cc 
                   ashura/std/tests/bit_span.cc
                   ashura/std/tests/sparse_set.cc)
target_link_libraries(ashura_std_tests ashura_std GTest::gtest
                      GTest::gtest_main)

add_library(ashura_gfx ashura/gfx/vma_def.cc ashura/gfx/vulkan.cc "ashura/std/bit_span.h")
target_link_libraries(
  ashura_gfx ashura_std Vulkan-Headers Vulkan::UtilityHeaders Vulkan::Vulkan
  GPUOpen::VulkanMemoryAllocator)
target_include_directories(ashura_gfx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(
  ashura_engine
  # ashura/engine/image_decoder.cc
  ashura/engine/renderer.cc
  ashura/engine/passes/bloom.cc
  ashura/engine/passes/blur.cc
  ashura/engine/passes/chromatic_aberration.cc
  ashura/engine/passes/fxaa.cc
  ashura/engine/passes/pbr.cc
  ashura/engine/passes/rrect.cc
  ashura/engine/passes/view.cc
  ashura/engine/widget.cc "ashura/std/bit_span.h")
target_link_libraries(ashura_engine ashura_std ashura_gfx)
target_include_directories(
  ashura_engine
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
         ${JPEG_INCLUDE_DIR} ${FFMPEG_INCLUDE_DIRS} ${FFMPEG_LIBRARY_DIRS})

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

add_executable(ashura_engine_tests ashura/engine/tests/renderer.cc "ashura/std/bit_span.h")
target_link_libraries(ashura_engine_tests ashura_std ashura_gfx ashura_engine
                      GTest::gtest GTest::gtest_main)
