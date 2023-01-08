cmake_minimum_required(VERSION 3.1)

project(Ashura)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

execute_process(
  COMMAND git clone -b main https://github.com/lamarrr/STX.git third_party/STX
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

# execute_process( COMMAND git clone -b master
# https://github.com/nothings/stb.git third_party/stb WORKING_DIRECTORY
# ${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(add_project_library)

include(ExternalProject)

find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)
find_package(Freetype REQUIRED)
find_package(spdlog REQUIRED)
find_package(harfbuzz REQUIRED)
find_package(fmt REQUIRED)
find_package(
  ICU
  COMPONENTS uc
  REQUIRED)

add_subdirectory(third_party/STX)

set(SKIA_LIBRARIES_DIR ${CMAKE_CURRENT_LIST_DIR}/skia/libs)

add_library(ashura_external_stb INTERFACE)
target_include_directories(ashura_external_stb
                           INTERFACE ${CMAKE_CURRENT_LIST_DIR}/third_party/stb)

# -fno-exceptions -fno-unwind-tables

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  add_compile_options(-fdiagnostics-color=always -Wall -Wextra -Wpedantic)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  add_compile_options(-fcolor-diagnostics -Wall -Wextra -Wpedantic)
endif()

# add_subdirectory(asset)
add_subdirectory(ashura)
