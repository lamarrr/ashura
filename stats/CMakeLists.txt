add_project_library(vlk http)

target_link_libraries(vlk_http stx vlk_utils stx::async stx::task libcurl fmt::fmt)
