
$CURRENT_SRC_DIR = "$PSScriptRoot"


function cpp_format {

    param(
        $folder
    )

    clang-format.exe -i --style=file "$folder/*.h"
    clang-format.exe -i --style=file "$folder/*.cc"
}


function slang_format {

    param( $folder)

    clang-format.exe -i --style=file "$folder/*.slang"
}


function cmake_format {

    param($file)

    cmake-format.exe -c "$CURRENT_SRC_DIR/.cmake-format.json" -i "$file"
}

cpp_format -folder $CURRENT_SRC_DIR/ashura/std
cpp_format -folder $CURRENT_SRC_DIR/ashura/gpu
cpp_format -folder $CURRENT_SRC_DIR/ashura/engine
cpp_format -folder $CURRENT_SRC_DIR/ashura/engine/pipelines
cpp_format -folder $CURRENT_SRC_DIR/ashura/engine/views
slang_format -folder $CURRENT_SRC_DIR/ashura/engine/shaders
slang_format -folder $CURRENT_SRC_DIR/ashura/engine/shaders/modules
slang_format -folder $CURRENT_SRC_DIR/ashura/engine/shaders/materials
cmake_format -file $CURRENT_SRC_DIR/CMakeLists.txt
