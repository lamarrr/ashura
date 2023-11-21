add_library(ashura_gfx STATIC src/gfx.cc src/vulkan_gfx.cc)

target_link_libraries(ashura_gfx stx::stx Vulkan::Headers
                      GPUOpen::VulkanMemoryAllocator)

target_include_directories(
  ashura_gfx
  PUBLIC include
  PRIVATE src)

add_library(
  ashura STATIC
  src/app.cc
  src/engine.cc
  src/widget.cc
  src/rengoku.cc
  src/loggers.cc
  src/subsystems/http_client.cc
  src/ecs.cc)

target_include_directories(
  ashura
  PUBLIC include ${JPEG_INCLUDE_DIR}
  PRIVATE src)

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
  avif
  ${FFMPEG_LIBRARIES}
  SheenBidi::SheenBidi)

target_include_directories(ashura PUBLIC ${FFMPEG_INCLUDE_DIRS})
target_link_directories(ashura PUBLIC ${FFMPEG_LIBRARY_DIRS})

set_target_properties(ashura PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED
                                                        ON)

add_executable(ashura_main src/ashura.cc)
target_link_libraries(ashura_main ashura)

add_executable(ashura_networking_test tests/http.cc)
target_link_libraries(ashura_networking_test ashura GTest::gtest
                      GTest::gtest_main)

add_executable(ashura_test tests/layout.cc tests/gfx.cc)
target_link_libraries(ashura_test ashura GTest::gtest GTest::gtest_main)
