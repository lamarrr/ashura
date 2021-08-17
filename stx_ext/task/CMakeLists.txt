add_project_library(stx task)
target_link_libraries(stx_task stx stx::async stx::mem stx::void stx::fn)
