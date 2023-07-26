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

set(SheenBidi_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party/SheenBidi)

if(NOT EXISTS ${SheenBidi_DIR})
  execute_process(
    COMMAND git clone https://github.com/Tehreer/SheenBidi.git --depth=1 -bv2.6
            ${SheenBidi_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
endif()

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
find_package(libavif CONFIG REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(CURL CONFIG REQUIRED)

FetchContent_MakeAvailable(stx)

file(GLOB SheenBidi_SOURCES ${SheenBidi_DIR}/Source/*.c)

add_library(SheenBidi ${SheenBidi_SOURCES})
target_include_directories(
  SheenBidi
  PUBLIC ${SheenBidi_DIR}/Headers
  PRIVATE ${SheenBidi_DIR}/Source)
add_library(SheenBidi::SheenBidi ALIAS SheenBidi)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  add_compile_options(-fdiagnostics-color=always -Wall -Wextra -Wpedantic)
else()
  if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics -Wall -Wextra -Wpedantic)
  else()
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
      add_compile_options(
        /Wall
        /wd5045
        /wd5264
        /wd4514
        /wd4061
        /wd4583
        /wd4820
        /wd4426
        /wd4625
        /wd4623
        /wd4626
        /wd4868
        /wd4100
        /wd4458
        /wd4456
        /wd5039
        /wd5246
        /wd5026
        /wd5027
        /wd5204
        /wd4996
        /wd4710
        /wd4711)
    endif()
  endif()
endif()

# add_subdirectory(assets)
add_subdirectory(ashura)
