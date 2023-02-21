add_library(ashura STATIC src/app.cc src/engine.cc src/window_api.cc
                          src/widget.cc src/window.cc src/rengoku.cc)

target_include_directories(
  ashura
  PUBLIC include ${Stb_INCLUDE_DIR}
  PRIVATE src)

target_link_libraries(
  ashura
  stx::stx
  SDL2::SDL2
  SDL2::SDL2main
  fmt::fmt
  Vulkan::Vulkan
  spdlog::spdlog
  harfbuzz
  Freetype::Freetype
  simdjson::simdjson)

add_executable(ashura_main src/ashura.cc)
target_link_libraries(ashura_main ashura)

add_executable(ashura_test tests/layout.cc)
target_link_libraries(ashura_test ashura GTest::gtest GTest::gtest_main)
