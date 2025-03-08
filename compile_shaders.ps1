$OUT = "assets/shaders"
$IN = "ashura/shaders"


New-Item -ItemType Directory -Force -Path $OUT

function compile_shader {
    param(
        $name,
        $file,
        $defines
    )

    slangc "$IN/$file" -o "$OUT/$name.spv" $defines -I "$IN"
}


compile_shader -name blur_upsample -file blur.slang -defines "-DUPSAMPLE=1"
compile_shader -name blur_downsample -file blur.slang -defines "-DUPSAMPLE=0"
compile_shader -name ngon -file ngon.slang -defines ""
compile_shader -name pbr -file pbr.slang -defines ""
compile_shader -name rrect -file rrect.slang -defines ""

