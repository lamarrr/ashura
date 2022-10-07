add_library(asura STATIC src/app.cc src/window_api.cc src/window.cc)

target_include_directories(
  asura
  PUBLIC include
  PRIVATE src)

target_link_libraries(asura VulkanMemoryAllocator stx::stx SDL2::SDL2 fmt::fmt
                      ${Vulkan_LIBRARIES})
