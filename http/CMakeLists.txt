add_project_library(vlk http)

target_link_libraries(
  vlk_http
  stx
  stx::stx
  vlk_utils
  libcurl
  fmt::fmt)
