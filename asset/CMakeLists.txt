add_project_library(vlk asset)
target_link_libraries(
  vlk_asset
  stx::stx
  fmt::fmt
  vlk::subsystem
  vlk::primitives
  vlk_external_skia
vlk_external_stb
  vlk::utils)
