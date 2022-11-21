add_library(ashura STATIC src/app.cc src/engine.cc src/window_api.cc
                          src/window.cc)

target_include_directories(
  ashura
  PUBLIC include
  PRIVATE src)

target_link_libraries(ashura stx::stx SDL2::SDL2 fmt::fmt ashura::utils
                      Vulkan::Vulkan)
