$OUT_DIR = "assets/shaders"
$INCLUDE_DIR = "ashura/shaders"
$SRC_DIR = "ashura/shaders"


New-Item -ItemType Directory -Force -Path $OUT_DIR

function compile_shader {
    param(
        $in
    )

    slangc "$SRC_DIR/$in" -o "$OUT_DIR/$in.spv" -I "$INCLUDE_DIR"
    slangc "$SRC_DIR/$in" -o "$OUT_DIR/$in.spv-asm" -I "$INCLUDE_DIR"
}

# [ ] make this part of cmake

compile_shader -in blur.slang 
compile_shader -in ngon.slang  
compile_shader -in pbr.slang  
compile_shader -in rrect.slang  

