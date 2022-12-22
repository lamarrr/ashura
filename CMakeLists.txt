cmake_minimum_required(VERSION 3.1)

project(Ashura)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

execute_process(
  COMMAND git clone -b main https://github.com/lamarrr/STX.git third_party/STX
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

execute_process(
  COMMAND git clone -b master https://github.com/nothings/stb.git
          third_party/stb WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

execute_process(
  COMMAND git clone -b v1.12.0 https://github.com/google/googletest.git
          third_party/googletest WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

execute_process(
  COMMAND git clone -b 9.0.0 https://github.com/fmtlib/fmt.git third_party/fmt
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

execute_process(
  COMMAND git clone -b v1.10.0 https://github.com/gabime/spdlog.git
          third_party/spdlog WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

execute_process(
  COMMAND git clone -b release-2.24.0 https://github.com/libsdl-org/SDL.git
          third_party/SDL WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

execute_process(
  COMMAND git clone -b 6.0.0 https://github.com/harfbuzz/harfbuzz.git
          third_party/harfbuzz WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
          
execute_process(
COMMAND git clone -b VER-2-12-1 https://github.com/freetype/freetype.git third_party/freetype WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
          

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(add_project_library)

include(ExternalProject)

find_package(Vulkan REQUIRED)

add_subdirectory(third_party/STX)
add_subdirectory(third_party/googletest)
add_subdirectory(third_party/fmt)
add_subdirectory(third_party/spdlog)
add_subdirectory(third_party/SDL)
add_subdirectory(third_party/harfbuzz)
add_subdirectory(third_party/freetype)

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