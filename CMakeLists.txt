cmake_minimum_required(VERSION 3.1)

project(Ashura)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

execute_process(
  COMMAND git clone -b main https://github.com/lamarrr/STX.git third_party/STX
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

execute_process(
  COMMAND git clone -b main https://github.com/libsdl-org/SDL.git
          third_party/SDL WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(add_project_library)

include(ExternalProject)

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

add_subdirectory(third_party/STX)
add_subdirectory(third_party/SDL)

set(SKIA_LIBRARIES_DIR ${CMAKE_CURRENT_LIST_DIR}/skia/libs)

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
