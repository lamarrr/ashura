add_project_library(stx flex)
target_link_libraries(stx_flex stx::allocator stx stx::void stx::struct)
