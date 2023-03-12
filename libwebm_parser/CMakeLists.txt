# Copyright (c) 2015 The WebM project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license that can be found
# in the LICENSE file in the root of the source tree. An additional intellectual
# property rights grant can be found in the file PATENTS.  All contributing
# project authors may be found in the AUTHORS file in the root of the source
# tree.
cmake_minimum_required(VERSION 3.2)
project(LIBWEBMPARSER CXX)

# Build/test configuration flags.
option(LIBWEBM_PARSER_ENABLE_TESTS "Enables tests." OFF)

set(webm_parser_sources
    "webm_parser/src/ancestory.cc"
    "webm_parser/src/bit_utils.cc"
    "webm_parser/src/block_header_parser.cc"
    "webm_parser/src/block_parser.cc"
    "webm_parser/src/buffer_reader.cc"
    "webm_parser/src/callback.cc"
    "webm_parser/src/date_parser.cc"
    "webm_parser/src/file_reader.cc"
    "webm_parser/src/float_parser.cc"
    "webm_parser/src/id_element_parser.cc"
    "webm_parser/src/id_parser.cc"
    "webm_parser/src/istream_reader.cc"
    "webm_parser/src/master_parser.cc"
    "webm_parser/src/parser_utils.cc"
    "webm_parser/src/segment_parser.cc"
    "webm_parser/src/size_parser.cc"
    "webm_parser/src/skip_parser.cc"
    "webm_parser/src/unknown_parser.cc"
    "webm_parser/src/var_int_parser.cc"
    "webm_parser/src/virtual_block_parser.cc"
    "webm_parser/src/void_parser.cc"
    "webm_parser/src/webm_parser.cc")

set(webm_parser_demo_sources "webm_parser/demo/demo.cc")
set(webm_parser_tests_sources
    "webm_parser/tests/audio_parser_test.cc"
    "webm_parser/tests/bit_utils_test.cc"
    "webm_parser/tests/block_additions_parser_test.cc"
    "webm_parser/tests/block_group_parser_test.cc"
    "webm_parser/tests/block_header_parser_test.cc"
    "webm_parser/tests/block_more_parser_test.cc"
    "webm_parser/tests/block_parser_test.cc"
    "webm_parser/tests/bool_parser_test.cc"
    "webm_parser/tests/buffer_reader_test.cc"
    "webm_parser/tests/byte_parser_test.cc"
    "webm_parser/tests/callback_test.cc"
    "webm_parser/tests/chapter_atom_parser_test.cc"
    "webm_parser/tests/chapter_display_parser_test.cc"
    "webm_parser/tests/chapters_parser_test.cc"
    "webm_parser/tests/cluster_parser_test.cc"
    "webm_parser/tests/colour_parser_test.cc"
    "webm_parser/tests/content_enc_aes_settings_parser_test.cc"
    "webm_parser/tests/content_encoding_parser_test.cc"
    "webm_parser/tests/content_encodings_parser_test.cc"
    "webm_parser/tests/content_encryption_parser_test.cc"
    "webm_parser/tests/cue_point_parser_test.cc"
    "webm_parser/tests/cue_track_positions_parser_test.cc"
    "webm_parser/tests/cues_parser_test.cc"
    "webm_parser/tests/date_parser_test.cc"
    "webm_parser/tests/ebml_parser_test.cc"
    "webm_parser/tests/edition_entry_parser_test.cc"
    "webm_parser/tests/element_test.cc"
    "webm_parser/tests/float_parser_test.cc"
    "webm_parser/tests/id_element_parser_test.cc"
    "webm_parser/tests/id_parser_test.cc"
    "webm_parser/tests/info_parser_test.cc"
    "webm_parser/tests/int_parser_test.cc"
    "webm_parser/tests/istream_reader_test.cc"
    "webm_parser/tests/limited_reader_test.cc"
    "webm_parser/tests/master_parser_test.cc"
    "webm_parser/tests/master_value_parser_test.cc"
    "webm_parser/tests/mastering_metadata_parser_test.cc"
    "webm_parser/tests/parser_utils_test.cc"
    "webm_parser/tests/projection_parser_test.cc"
    "webm_parser/tests/recursive_parser_test.cc"
    "webm_parser/tests/seek_head_parser_test.cc"
    "webm_parser/tests/seek_parser_test.cc"
    "webm_parser/tests/segment_parser_test.cc"
    "webm_parser/tests/simple_tag_parser_test.cc"
    "webm_parser/tests/size_parser_test.cc"
    "webm_parser/tests/skip_parser_test.cc"
    "webm_parser/tests/slices_parser_test.cc"
    "webm_parser/tests/tag_parser_test.cc"
    "webm_parser/tests/tags_parser_test.cc"
    "webm_parser/tests/targets_parser_test.cc"
    "webm_parser/tests/time_slice_parser_test.cc"
    "webm_parser/tests/track_entry_parser_test.cc"
    "webm_parser/tests/tracks_parser_test.cc"
    "webm_parser/tests/unknown_parser_test.cc"
    "webm_parser/tests/var_int_parser_test.cc"
    "webm_parser/tests/video_parser_test.cc"
    "webm_parser/tests/virtual_block_parser_test.cc"
    "webm_parser/tests/void_parser_test.cc"
    "webm_parser/tests/webm_parser_test.cc"
    "webm_parser/test_utils/limited_reader.cc"
    "webm_parser/tests/webm_parser_tests.cc")

add_library(webm_parser ${webm_parser_sources})
target_include_directories(
  webm_parser
  PUBLIC webm_parser/include
  PUBLIC webm_parser)
set_target_properties(webm_parser PROPERTIES CXX_STANDARD 17
                                             CXX_STANDARD_REQUIRED ON)

add_executable(webm_parser_demo ${webm_parser_demo_sources})
target_link_libraries(webm_parser_demo webm_parser)

install(
  TARGETS webm_parser
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/webm)

if(LIBWEBM_PARSER_ENABLE_TESTS)
  find_package(GTest CONFIG REQUIRED)
  add_executable(webm_parser_tests ${webm_parser_tests_sources})
  target_link_libraries(
    webm_parser_tests PRIVATE GTest::gtest GTest::gtest_main GTest::gmock
                              GTest::gmock_main webm_parser)
endif()
