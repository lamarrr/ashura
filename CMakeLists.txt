cmake_minimum_required(VERSION 3.1)

project(Valkyrie)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

execute_process(
  COMMAND git clone https://github.com/lamarrr/STX.git third_party/STX
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
execute_process(
  COMMAND git clone -b v1.8.2 https://github.com/gabime/spdlog.git
          third_party/spdlog WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
# execute_process( COMMAND git clone -b 0.9.9.8
# https://github.com/g-truc/glm.git third_party/glm WORKING_DIRECTORY
# ${CMAKE_CURRENT_LIST_DIR})
execute_process(
  COMMAND git clone -b master https://github.com/nothings/stb.git
          third_party/stb WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
execute_process(
  COMMAND git clone -b release-1.10.0 https://github.com/google/googletest.git
          third_party/googletest WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
execute_process(
  COMMAND git clone -b 7.1.3 https://github.com/fmtlib/fmt.git third_party/fmt
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
execute_process(
  COMMAND git clone -b release-2.0.14 https://github.com/libsdl-org/SDL.git
          third_party/SDL WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
execute_process(
  COMMAND git clone -b curl-7_77_0 https://github.com/curl/curl.git
          third_party/curl WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

# TODO(lamarrr): use a tagged version with CMakeLists.txt
execute_process(
  COMMAND
    git clone -b master
    https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    third_party/VulkanMemoryAllocator
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

# TODO(lamarrr): check if skia binaries and header folder doesn't exists and
# pull in

if(NOT CMAKE_BUILD_TYPE)
  set(SKIA_LIBRARIES_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party/skia/out/Debug)
else()
  if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(SKIA_LIBRARIES_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party/skia/out/Debug)
  else()
    set(SKIA_LIBRARIES_DIR
        ${CMAKE_CURRENT_LIST_DIR}/third_party/skia/out/Release)
  endif()
endif()

find_package(Vulkan REQUIRED)
find_package(Fontconfig REQUIRED)

set(GLFW_BUILD_EXAMPLES
    OFF
    CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS
    OFF
    CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS
    OFF
    CACHE BOOL "" FORCE)
set(GLFW_INSTALL
    OFF
    CACHE BOOL "" FORCE)
set(STX_ENABLE_BACKTRACE
    ON
    CACHE BOOL "" FORCE)
set(BUILD_GMOCK
    OFF
    CACHE BOOL "" FORCE)

set(SDL_ATOMIC_ENABLED_BY_DEFAULT
    OFF
    CACHE BOOL "" FORCE)
set(SDL_AUDIO_ENABLED_BY_DEFAULT
    OFF
    CACHE BOOL "" FORCE)
set(SDL_VIDEO_ENABLED_BY_DEFAULT
    ON
    CACHE BOOL "" FORCE)
set(SDL_RENDER_ENABLED_BY_DEFAULT
    OFF
    CACHE BOOL "" FORCE)
set(SDL_EVENTS_ENABLED_BY_DEFAULT
    ON
    CACHE BOOL "" FORCE)
# set(SDL_JOYSTICK_ENABLED_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(SDL_HAPTIC_ENABLED_BY_DEFAULT
    ON
    CACHE BOOL "" FORCE)
set(SDL_POWER_ENABLED_BY_DEFAULT
    OFF
    CACHE BOOL "" FORCE)
set(SDL_THREADS_ENABLED_BY_DEFAULT
    OFF
    CACHE BOOL "" FORCE)
set(SDL_TIMERS_ENABLED_BY_DEFAULT
    OFF
    CACHE BOOL "" FORCE)
set(SDL_FILE_ENABLED_BY_DEFAULT
    OFF
    CACHE BOOL "" FORCE)
set(SDL_LOADSO_ENABLED_BY_DEFAULT
    ON
    CACHE BOOL "" FORCE)
set(SDL_CPUINFO_ENABLED_BY_DEFAULT
    OFF
    CACHE BOOL "" FORCE)
set(SDL_FILESYSTEM_ENABLED_BY_DEFAULT
    OFF
    CACHE BOOL "" FORCE)
set(SDL_DLOPEN_ENABLED_BY_DEFAULT
    ON
    CACHE BOOL "" FORCE)
set(SDL_SENSOR_ENABLED_BY_DEFAULT
    OFF
    CACHE BOOL "" FORCE)
set(SDL_LOCALE_ENABLED_BY_DEFAULT
    OFF
    CACHE BOOL "" FORCE)

add_subdirectory(third_party/STX EXCLUDE_FROM_ALL)
add_subdirectory(third_party/spdlog EXCLUDE_FROM_ALL)
add_subdirectory(third_party/googletest EXCLUDE_FROM_ALL)
add_subdirectory(third_party/fmt EXCLUDE_FROM_ALL)
add_subdirectory(third_party/VulkanMemoryAllocator EXCLUDE_FROM_ALL)
add_subdirectory(third_party/SDL EXCLUDE_FROM_ALL)
add_subdirectory(third_party/curl EXCLUDE_FROM_ALL)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(add_project_library)

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

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  add_compile_options(-fdiagnostics-color=always -Wall -Wextra -Wpedantic -fno-exceptions -fno-unwind-tables)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  add_compile_options(-fcolor-diagnostics -Wall -Wextra -Wpedantic -fno-exceptions -fno-unwind-tables)
endif()

# TODO(lamarrr): we need extensive unit testing even for dependencies

add_subdirectory(stx_ext)
add_subdirectory(palettes)
add_subdirectory(subsystem)
add_subdirectory(subsystems)
add_subdirectory(http)
add_subdirectory(asset)
add_subdirectory(primitives)
add_subdirectory(utils)
add_subdirectory(ui)
add_subdirectory(scheduler)
