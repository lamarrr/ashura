add_project_library(vlk asset)
target_link_libraries(
  vlk_asset
  stx::struct
  stx::mem
  fmt::fmt
  stx::enum
  vlk::primitives
  vlk_external_skia
  vlk::utils)
