add_project_library(vlk asset)
target_link_libraries(
  vlk_asset
  stx::struct
  stx::mem
  fmt::fmt
  stx::enum
  stx::async
  stx::scheduler
  vlk::subsystem
  vlk::primitives
  vlk_external_skia
vlk_external_stb
  vlk::utils)
