add_project_library(vlk scheduler)
target_link_libraries(
  vlk_scheduler
  stx::fn
  stx::mem
  stx
  stx::task
  vlk::subsystem
  stx::vec
  vlk::utils
  stx::str
  stx::void)
