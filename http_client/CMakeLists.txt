add_library(vlk_http_client STATIC src/http.cc)

target_include_directories(vlk_http_client PUBLIC include)

target_link_libraries(vlk_http stx vlk_utils stx::async stx::task libcurl fmt::fmt)
