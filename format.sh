#!/usr/bin/env bash

set -xe

CURRENT_SRC_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"


CLANG_FORMAT="clang-format"
CLANG_FORMAT="$HOME/LLVM-21.1.8-Linux-X64/bin/clang-format"

cpp_format() {
    local folder="$1"
    $CLANG_FORMAT -i --style=file "$folder"/*.h  
    $CLANG_FORMAT -i --style=file "$folder"/*.cc  
}

slang_format() {
    local folder="$1"
    $CLANG_FORMAT -i --style=file "$folder"/*.slang 
}

cmake_format() {
    local file="$1"
    cmake-format -c "$CURRENT_SRC_DIR/.cmake-format.json" -i "$file"
}

cpp_format "$CURRENT_SRC_DIR/ashura/std"
# cpp_format "$CURRENT_SRC_DIR/ashura/std/tests"
cpp_format "$CURRENT_SRC_DIR/ashura/gpu"
cpp_format "$CURRENT_SRC_DIR/ashura/engine"
# cpp_format "$CURRENT_SRC_DIR/ashura/engine/tests"
cpp_format "$CURRENT_SRC_DIR/ashura/engine/shaders"
cpp_format "$CURRENT_SRC_DIR/ashura/engine/pipelines"
cpp_format "$CURRENT_SRC_DIR/ashura/engine/views"
slang_format "$CURRENT_SRC_DIR/ashura/engine/shaders"
slang_format "$CURRENT_SRC_DIR/ashura/engine/shaders/modules"
slang_format "$CURRENT_SRC_DIR/ashura/engine/shaders/materials"
cmake_format "$CURRENT_SRC_DIR/CMakeLists.txt"