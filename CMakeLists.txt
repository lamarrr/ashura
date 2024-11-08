# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.6)

if(APPLE)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for macOS" FORCE)
    set(CMAKE_APPLE_SILICON_PROCESSOR "arm64")
  endif()
endif()

project(Ashura)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(APPLE)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    set(HOMEBREW_PREFIX "/opt/homebrew")
  else()
    set(HOMEBREW_PREFIX "/usr/local")
  endif()

  set(VULKAN_SDK $ENV{VULKAN_SDK})
  set(VULKAN_VERSION "1.3.296.0")
  set(VK_HEADER_VERSION "1.3.296.0")

  if(NOT VULKAN_SDK)
    # Try to find the latest SDK version in the home directory
    file(GLOB VULKAN_SDK_VERSIONS RELATIVE "$ENV{HOME}/VulkanSDK" "$ENV{HOME}/VulkanSDK/*")
    list(SORT VULKAN_SDK_VERSIONS)
    list(GET VULKAN_SDK_VERSIONS -1 VULKAN_VERSION)
    set(VULKAN_SDK "$ENV{HOME}/VulkanSDK/${LATEST_VERSION}")
  endif()

  message(STATUS "Looking for Vulkan SDK in: ${VULKAN_SDK}")

  find_program(GLSLC_EXECUTABLE glslc
    HINTS
    "${VULKAN_SDK}/macOS/bin"
    "${VULKAN_SDK}/bin"
    REQUIRED
  )

  set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK}/macOS/include")
  set(Vulkan_LIBRARIES "${VULKAN_SDK}/macOS/lib")

  find_package(Vulkan REQUIRED)

  add_definitions(-DVULKAN_STRING_HELPER_NO_AMDX)

  if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc not found. Please install the Vulkan SDK from https://vulkan.lunarg.com/sdk/home#mac")
  endif()

  message(STATUS "GLSLC path: ${GLSLC_EXECUTABLE}")
  message(STATUS "Vulkan include dirs: ${Vulkan_INCLUDE_DIRS}")
  message(STATUS "Vulkan libraries: ${Vulkan_LIBRARIES}")

  set(ENV{VULKAN_SDK} "${VULKAN_SDK}/macOS")
  set(ENV{VK_ICD_FILENAMES} "${VULKAN_SDK}/macOS/share/vulkan/icd.d/MoltenVK_icd.json")
  set(ENV{VK_LAYER_PATH} "${VULKAN_SDK}/macOS/share/vulkan/explicit_layer.d")

  find_library(SPIRV_LIBRARY
    NAMES SPIRV
    PATHS
    "${HOMEBREW_PREFIX}/lib"
    REQUIRED
  )

  message(STATUS "SPIRV library: ${SPIRV_LIBRARY}")

  list(APPEND CMAKE_PREFIX_PATH
    "${HOMEBREW_PREFIX}/opt/vulkan-sdk"
    "${HOMEBREW_PREFIX}/opt/molten-vk"
  )

  include_directories(SYSTEM
    "${HOMEBREW_PREFIX}/include"
    "${VULKAN_SDK_PATH}/include"
    "${HOMEBREW_PREFIX}/include/spirv-tools"
  )
  link_directories(
    "${HOMEBREW_PREFIX}/lib"
    "${VULKAN_SDK_PATH}/lib"
  )

  set(ENV{PKG_CONFIG_PATH} "${HOMEBREW_PREFIX}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
endif()

set(VMA_DYNAMIC_VULKAN_FUNCTIONS ON CACHE BOOL "")
set(ENABLE_SPVREMAPPER OFF CACHE BOOL "")
set(SPIRV_REFLECT_EXECUTABLE OFF CACHE BOOL "")
set(SPIRV_REFLECT_STATIC_LIB ON CACHE BOOL "")
set(SDL_STATIC ON CACHE BOOL "")

include(FetchContent)

FetchContent_Declare(
  VulkanHeaders
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
  GIT_TAG vulkan-sdk-1.3.296.0
)

if(APPLE)
  FetchContent_GetProperties(VulkanHeaders)

  if(NOT vulkanheaders_POPULATED)
    FetchContent_MakeAvailable(VulkanHeaders)

    # Configure and install VulkanHeaders
    execute_process(
      COMMAND ${CMAKE_COMMAND}
      -S ${vulkanheaders_SOURCE_DIR}
      -B ${vulkanheaders_BINARY_DIR}
      -DCMAKE_INSTALL_PREFIX=${vulkanheaders_BINARY_DIR}/install
    )

    execute_process(
      COMMAND ${CMAKE_COMMAND} --build ${vulkanheaders_BINARY_DIR} --target install
    )
  endif()

  list(APPEND CMAKE_PREFIX_PATH ${vulkanheaders_BINARY_DIR}/install)

  set(VulkanHeaders_DIR ${VULKAN_HEADERS_INSTALL_DIR}/share/cmake/VulkanHeaders)

  set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "")
  set(ENABLE_GLSLANG_JS OFF CACHE BOOL "")
  set(ENABLE_HLSL OFF CACHE BOOL "")
  set(ENABLE_OPT OFF CACHE BOOL "")
  set(BUILD_TESTING OFF CACHE BOOL "")
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
  set(SKIP_GLSLANG_INSTALL ON CACHE BOOL "")
  set(ENABLE_PCH OFF CACHE BOOL "")
endif()

FetchContent_Declare(
  VulkanGLSL
  GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
  GIT_TAG vulkan-sdk-1.3.296.0
)

FetchContent_Declare(
  VulkanLoader
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader.git
  GIT_TAG be0e1c3
)

FetchContent_Declare(
  SpirvReflect
  GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Reflect.git
  GIT_TAG vulkan-sdk-1.3.296.0
)

FetchContent_Declare(
  VulkanUtilityLibraries
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Utility-Libraries.git
  GIT_TAG vulkan-sdk-1.3.296.0
)

FetchContent_Declare(
  VulkanMemoryAllocator
  GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG v3.1.0
)

FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG preview-3.1.3
)

FetchContent_Declare(
  harfbuzz
  GIT_REPOSITORY https://github.com/harfbuzz/harfbuzz.git
  GIT_TAG 8.5.0
)

FetchContent_Declare(
  Freetype
  GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
  GIT_TAG VER-2-13-2
)

FetchContent_Declare(
  SheenBidi
  GIT_REPOSITORY https://github.com/Tehreer/SheenBidi.git
  GIT_TAG v2.7
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
)

# WebP configuration
set(WEBP_BUILD_GIF2WEBP ON CACHE BOOL "Build gif2webp" FORCE)
set(WEBP_ENABLE_GIF OFF CACHE BOOL "Enable GIF support" FORCE)
set(WEBP_BUILD_DEPS ON CACHE BOOL "Build dependencies" FORCE)
set(WEBP_ENABLE_SWAP_16BIT_CSP ON CACHE BOOL "Enable byte swap for 16 bit colorspaces" FORCE)

FetchContent_Declare(
  WebP
  GIT_REPOSITORY https://chromium.googlesource.com/webm/libwebp
  GIT_TAG 1.4.0
)

FetchContent_Declare(
  CURL
  GIT_REPOSITORY https://github.com/curl/curl.git
  GIT_TAG curl-8_8_0
)

find_package(simdjson CONFIG REQUIRED)
find_package(GTest REQUIRED)
find_package(JPEG REQUIRED)
find_package(libjpeg-turbo CONFIG REQUIRED)
find_package(PNG REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(xxHash CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)
find_package(Threads REQUIRED)

FetchContent_MakeAvailable(VulkanHeaders)
FetchContent_MakeAvailable(VulkanLoader)
FetchContent_MakeAvailable(VulkanGLSL)
FetchContent_MakeAvailable(VulkanUtilityLibraries)
FetchContent_MakeAvailable(VulkanMemoryAllocator)
FetchContent_MakeAvailable(harfbuzz)
FetchContent_MakeAvailable(Freetype)
FetchContent_MakeAvailable(SheenBidi)
FetchContent_MakeAvailable(SpirvReflect)
FetchContent_MakeAvailable(SDL3)
FetchContent_MakeAvailable(WebP)
FetchContent_MakeAvailable(CURL)

if(APPLE)
  set_target_properties(SPIRV PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CMAKE_CXX_FLAGS "-fPIC"
  )

  set_target_properties(SPIRV PROPERTIES
    OUTPUT_NAME "SPIRV"
    PREFIX "lib"
    SUFFIX ".a"
  )

  if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    set_target_properties(SPIRV PROPERTIES
      OSX_ARCHITECTURES "arm64"
    )
  endif()
endif()

# GoogleTest configuration
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# FetchContent_MakeAvailable(googletest)
file(GLOB SheenBidi_SOURCES ${sheenbidi_SOURCE_DIR}/Source/*.c)
add_library(SheenBidi ${SheenBidi_SOURCES})
target_include_directories(
  SheenBidi
  PUBLIC ${sheenbidi_SOURCE_DIR}/Headers
  PRIVATE ${sheenbidi_SOURCE_DIR}/Source
)
add_library(SheenBidi::SheenBidi ALIAS SheenBidi)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  add_compile_options(-Wall -Wextra -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Wno-nullability-completeness
    -Wno-nullability-extension -std=c++20 -stdlib=libc++ -fexperimental-library)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  add_compile_options(/W3 /Zc:preprocessor)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_library(
  ashura_std
  ashura/std/allocator.cc
  ashura/std/allocators.cc
  ashura/std/async.cc
  ashura/std/format.cc
  ashura/std/fs.cc
  ashura/std/hash.cc
  ashura/std/log.cc
  ashura/std/panic.cc
)
target_include_directories(ashura_std PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ashura_std PUBLIC xxHash::xxhash Threads::Threads)

add_library(ashura_gpu
  ashura/gpu/gpu.cc
  ashura/gpu/vma_def.cc
  ashura/gpu/vulkan.cc
)

if(APPLE)
  target_include_directories(ashura_gpu
    PRIVATE
    ${Vulkan_INCLUDE_DIRS}
    "${HOMEBREW_PREFIX}/include"
  )

  target_link_directories(ashura_gpu
    PRIVATE
    "${HOMEBREW_PREFIX}/lib"
    "${VULKAN_SDK_PATH}/lib"
  )
endif()

target_link_libraries(
  ashura_gpu
  PUBLIC
  ashura_std
  $<IF:$<BOOL:${APPLE}>,Vulkan::Vulkan,Vulkan-Headers>
  Vulkan::UtilityHeaders
  $<IF:$<BOOL:${APPLE}>,${MOLTEN_VK_LIBRARY},Vulkan::Loader>
  GPUOpen::VulkanMemoryAllocator
)

add_library(
  ashura_engine
  ashura/engine/engine.cc
  ashura/engine/font.cc
  ashura/engine/font_impl.cc
  ashura/engine/text.cc
  ashura/engine/canvas.cc
  ashura/engine/image_decoder.cc
  ashura/engine/render_text.cc
  ashura/engine/shader.cc
  ashura/engine/text_compositor.cc
  ashura/engine/window.cc
  ashura/engine/view.cc
  ashura/engine/view_system.cc
  ashura/engine/views.cc
  ashura/engine/scene.cc
  ashura/engine/renderer.cc
  ashura/engine/render_context.cc
  ashura/engine/passes.cc
 )
target_link_libraries(
  ashura_engine
  ashura_std
  ashura_gpu
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
  SheenBidi::SheenBidi
)

if(APPLE)
  target_link_libraries(
    ashura_engine
    ${SPIRV_LIBRARY}
  )
else()
  target_link_libraries(
    ashura_engine
    SPIRV
  )
endif()

target_include_directories(
  ashura_engine
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${JPEG_INCLUDE_DIR}
  ${FFMPEG_INCLUDE_DIRS}
  ${FFMPEG_LIBRARY_DIRS}

  # PRIVATE
  # ${GIF_INCLUDE_DIR}
)

add_executable(
  ashura_std_tests
  ashura/std/tests/tests.cc
  ashura/std/tests/option.cc
  ashura/std/tests/async.cc
  ashura/std/tests/list.cc
  ashura/std/tests/hash_map.cc
  ashura/std/tests/result.cc
  ashura/std/tests/sparse_vec.cc
)

add_executable(ashura_std_bench
  ashura/std/bench/hash_map.cc
)

target_link_libraries(ashura_std_tests
  ashura_std
  GTest::gtest
  GTest::gtest_main
)

target_link_libraries(ashura_std_bench
  benchmark::benchmark
  benchmark::benchmark_main
  ashura_std
)

add_executable(
  ashura_engine_tests
  ashura/engine/tests/main.cc
  ashura/engine/tests/render_text.cc
  ashura/engine/tests/views.cc
  ashura/engine/tests/text_compositor.cc
)

target_link_libraries(ashura_engine_tests
  ashura_std
  ashura_engine
  GTest::gtest
)

add_executable(ashura_app_test
  ashura/engine/tests/app.cc
)

target_link_libraries(ashura_app_test ashura_engine)
