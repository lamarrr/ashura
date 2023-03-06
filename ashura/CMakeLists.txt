add_library(ashura STATIC src/app.cc src/engine.cc src/window_api.cc
                          src/widget.cc src/window.cc src/rengoku.cc)

target_include_directories(
  ashura
  PUBLIC include ${JPEG_INCLUDE_DIR} $<TARGET_PROPERTY:aom,INCLUDE_DIRECTORIES>
         $<TARGET_PROPERTY:webm,INCLUDE_DIRECTORIES>
  PRIVATE src)

target_link_libraries(
  ashura
  stx::stx
  SDL3::SDL3-static
  fmt::fmt
  Vulkan::Vulkan
  spdlog::spdlog
  harfbuzz
  Freetype::Freetype
  simdjson::simdjson
  WebP::webp
  WebP::webpdecoder
  ${JPEG_LIBRARIES}
  PNG::PNG
  aom
  webm)

target_link_directories(ashura PUBLIC ${FFMPEG_LIBRARY_DIRS})

set_target_properties(ashura PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED
                                                        ON)

add_executable(ashura_main src/ashura.cc)
target_link_libraries(ashura_main ashura)

add_executable(ashura_test tests/layout.cc)
target_link_libraries(ashura_test ashura GTest::gtest GTest::gtest_main)
