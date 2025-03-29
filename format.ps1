

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

    cmake-format.exe -c ".cmake-format.json" -i "$file"
}

cpp_format -folder ashura/std
cpp_format -folder ashura/gpu
cpp_format -folder ashura/engine
slang_format -folder ashura/engine/shaders
slang_format -folder ashura/engine/shaders/modules
cmake_format -file ./CMakeLists.txt