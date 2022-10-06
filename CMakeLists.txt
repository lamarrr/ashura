cmake_minimum_required(VERSION 3.1)

project(Valkyrie)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

execute_process(
  COMMAND git clone -b main https://github.com/lamarrr/STX.git third_party/STX
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

execute_process(
  COMMAND git clone -b master https://github.com/nothings/stb.git
          third_party/stb WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

execute_process(
  COMMAND
    git clone -b v3.0.1
    https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    third_party/VulkanMemoryAllocator
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

execute_process(
  COMMAND git clone -b main https://github.com/google/skia.git third_party/skia
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

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
  COMMAND git clone -b curl-7_80_0 https://github.com/curl/curl.git
          third_party/curl WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

execute_process(
  COMMAND git clone -b chrome/m106 https://github.com/google/skia.git
          third_party/skia WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(add_project_library)

include(ExternalProject)

find_package(Vulkan REQUIRED)
find_package(Fontconfig REQUIRED)

add_subdirectory(third_party/STX)
add_subdirectory(third_party/VulkanMemoryAllocator)
add_subdirectory(third_party/googletest)
add_subdirectory(third_party/fmt)
add_subdirectory(third_party/spdlog)
add_subdirectory(third_party/SDL)
add_subdirectory(third_party/curl)

# TODO(lamarrr): check if skia binaries and header folder doesn't exists and
# pull in

set(SKIA_LIBRARIES_DIR ${CMAKE_CURRENT_LIST_DIR}/skia/libs)

add_library(vlk_external_stb INTERFACE)
target_include_directories(vlk_external_stb
                           INTERFACE ${CMAKE_CURRENT_LIST_DIR}/third_party/stb)

file(GLOB VLK_EXTERNAL_SKIA_LIBRARIES ${SKIA_LIBRARIES_DIR}/*.a)
add_library(vlk_external_skia STATIC ${CMAKE_CURRENT_LIST_DIR}/skia/src/skia.cc)
target_include_directories(vlk_external_skia
                           PUBLIC ${CMAKE_CURRENT_LIST_DIR}/third_party/skia)
target_link_libraries(vlk_external_skia ${VLK_EXTERNAL_SKIA_LIBRARIES}
                      ${Fontconfig_LIBRARIES})

# TODO(lamarrr): automate this
target_compile_definitions(
  vlk_external_skia
  PUBLIC SK_ENABLE_SPIRV_VALIDATION
         SK_R32_SHIFT=16
         SK_GAMMA_APPLY_TO_A8
         GR_OP_ALLOCATE_USE_NEW
         SK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1
         GR_TEST_UTILS=1
         SK_USE_VMA
         SKIA_IMPLEMENTATION=1
         SK_VULKAN
         SK_ENABLE_VK_LAYERS
         SK_ENABLE_DUMP_GPU)

# -fno-exceptions -fno-unwind-tables

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  add_compile_options(-fdiagnostics-color=always -Wall -Wextra -Wpedantic)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  add_compile_options(-fcolor-diagnostics -Wall -Wextra -Wpedantic)
endif()

add_subdirectory(palettes)
add_subdirectory(subsystem)
add_subdirectory(subsystems)
add_subdirectory(http)
add_subdirectory(asset)
add_subdirectory(primitives)
add_subdirectory(utils)
add_subdirectory(ui)
