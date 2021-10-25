add_project_library(vlk scheduler)
target_link_libraries(
  vlk_scheduler
  stx::fn
  stx::mem
  stx::option_result
  stx::span
  stx::task
  vlk::subsystem
  stx::flex
  vlk::utils
  "stx::string"
  stx::void)
