add_library(
  ashura STATIC
  src/app.cc
  src/engine.cc
  src/stb_truetype.cc
  src/window_api.cc
  src/widget.cc
  src/window.cc
  src/rect_pack.cc)

target_include_directories(
  ashura
  PUBLIC include
  PRIVATE src)

target_link_libraries(
  ashura
  stx::stx
  SDL2::SDL2
  SDL2::SDL2main
  fmt::fmt
  Vulkan::Vulkan
  spdlog::spdlog
  ashura_external_stb
  harfbuzz
  freetype)

add_executable(ashura_test src/ashura.cc)
target_link_libraries(ashura_test ashura)
