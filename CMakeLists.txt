cmake_minimum_required(VERSION 3.1)

project(Ashura)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(add_project_library)
include(FetchContent)

FetchContent_Declare(
  stx
  GIT_REPOSITORY https://github.com/lamarrr/STX.git
  GIT_TAG main)

FetchContent_Declare(
  VulkanHeaders
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
  GIT_TAG v1.3.269)

FetchContent_Declare(
  VulkanMemoryAllocator
  GIT_REPOSITORY
    https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG master)

set(SheenBidi_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party/SheenBidi)

if(NOT EXISTS ${SheenBidi_DIR})
  execute_process(
    COMMAND git clone https://github.com/Tehreer/SheenBidi.git --depth=1 -bv2.6
            ${SheenBidi_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
endif()

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
find_package(FFMPEG REQUIRED)
find_package(CURL CONFIG REQUIRED)

FetchContent_MakeAvailable(stx)
FetchContent_MakeAvailable(VulkanHeaders)
FetchContent_MakeAvailable(VulkanMemoryAllocator)

file(GLOB SheenBidi_SOURCES ${SheenBidi_DIR}/Source/*.c)

add_library(SheenBidi ${SheenBidi_SOURCES})
target_include_directories(
  SheenBidi
  PUBLIC ${SheenBidi_DIR}/Headers
  PRIVATE ${SheenBidi_DIR}/Source)
add_library(SheenBidi::SheenBidi ALIAS SheenBidi)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  add_compile_options(-Wall -Wextra -Wpedantic -fno-rtti -fno-exceptions)
else()
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -fno-rtti -fno-exceptions)
  else()
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
      add_compile_options(/W4)
    endif()
  endif()
endif()

add_library(ashura_allocator STATIC ashura/allocator.cc
                                    ashura/arena_allocator.cc)
add_library(ashura_gfx STATIC ashura/gfx.cc ashura/vulkan_gfx.cc)
add_library(
  ashura STATIC
  ashura/app.cc
  ashura/engine.cc
  ashura/widget.cc
  ashura/rengoku.cc
  ashura/loggers.cc
  ashura/subsystems/http_client.cc
  ashura/ecs.cc)
add_executable(ashura_main ashura/ashura.cc)
add_executable(ashura_networking_test ashura/tests/http.cc)
add_executable(ashura_test ashura/tests/layout.cc)

target_include_directories(ashura_allocator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ashura_gfx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(
  ashura PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${JPEG_INCLUDE_DIR}
                ${FFMPEG_INCLUDE_DIRS} ${FFMPEG_LIBRARY_DIRS})

target_link_libraries(ashura_gfx stx::stx Vulkan::Headers
                      GPUOpen::VulkanMemoryAllocator ashura_allocator)
target_link_libraries(
  ashura
  ashura_gfx
  stx::stx
  SDL3::SDL3-static
  fmt::fmt
  spdlog::spdlog
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
target_link_libraries(ashura_main ashura)
target_link_libraries(ashura_networking_test ashura GTest::gtest
                      GTest::gtest_main)
target_link_libraries(ashura_test ashura GTest::gtest GTest::gtest_main)
