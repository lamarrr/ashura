add_library(vlk_utils src/utils.cc)
target_include_directories(
  vlk_utils
  PUBLIC include
  PRIVATE src)
target_link_libraries(vlk_utils spdlog::spdlog)
