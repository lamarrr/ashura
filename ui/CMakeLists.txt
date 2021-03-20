add_library(vlk_ui src/canvas.cc src/compositor.cc src/widgets/text.cc)
target_include_directories(
  vlk_ui
  PUBLIC include
  PRIVATE src)
target_link_libraries(vlk_ui stx vlk_utils vlk_assets vlk_external_skia)
target_compile_definitions(vlk_ui PUBLIC SK_VULKAN)

add_executable(
  vlk_ui_test
  tests/constraint_solver_test.cc tests/compositor_test.cc
  tests/snapshot_test.cc tests/widget_test.cc tests/layout_test.cc
  tests/raster_tiles_test.cc)

target_link_libraries(vlk_ui_test gtest gtest_main vlk_ui)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID}
                                                STREQUAL "GNU")
  target_compile_options(vlk_ui_test PRIVATE -Wall -Wextra -Wpedantic)
endif()
