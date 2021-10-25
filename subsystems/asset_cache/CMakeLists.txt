add_project_library(vlk_subsystems asset_cache)
target_link_libraries(
  vlk_subsystems_asset_cache
  vlk::asset
  stx::async
  stx::mem
  stx::option_result
  vlk::subsystem
  vlk::utils
  fmt::fmt)
