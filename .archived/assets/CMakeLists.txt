add_library(vlk_assets src/image.cc)
target_include_directories(
  vlk_assets
  PUBLIC include
  PRIVATE src)
target_link_libraries(vlk_assets stx vlk_utils vlk_external_stb)
