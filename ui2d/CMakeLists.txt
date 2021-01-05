add_library(vlk_ui2d src/canvas.cc src/compositor.cc)
target_include_directories(
  vlk_ui2d
  PUBLIC include
  PRIVATE src)
target_link_libraries(vlk_ui2d stx vlk_utils vlk_assets vlk_external_skia)
target_compile_definitions(vlk_ui2d PUBLIC SK_VULKAN)

add_executable(vlk_ui2d_test tests/base_widget_test.cc tests/compositor_test.cc
                             tests/snapshot_test.cc tests/widget_test.cc)

target_link_libraries(vlk_ui2d_test gtest gtest_main vlk_ui2d)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID}
                                                STREQUAL "GNU")
  target_compile_options(vlk_ui2d_test PRIVATE -Wall -Wextra -Wpedantic)
endif()
