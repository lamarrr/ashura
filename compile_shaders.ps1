$OUT_DIR = "assets/shaders"
$IN_DIR = "ashura/shaders"


New-Item -ItemType Directory -Force -Path $OUT_DIR

function compile_shader {
    param(
        $out,
        $in
    )

    slangc "$IN_DIR/$in" -o "$OUT_DIR/$out" -I "$IN"
}


compile_shader -out blur.spv -in blur.slang 
compile_shader -out ngon.spv -in ngon.slang  
compile_shader -out pbr.spv -in pbr.slang  
compile_shader -out rrect.spv -in rrect.slang  

