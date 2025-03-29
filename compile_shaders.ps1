$SRC_DIR = "ashura/engine/shaders"
$OUT_DIR = "assets/shaders"
$INCLUDE_DIR = $SRC_DIR


New-Item -ItemType Directory -Force -Path $OUT_DIR

$FEATURES = "SPV_EXT_descriptor_indexing+SPV_KHR_non_semantic_info+spvImageQuery+spvImageGatherExtended+spvShaderNonUniformEXT+spvShaderNonUniform+SPV_GOOGLE_user_type+spvDerivativeControl+spvSparseResidency+spvMinLod+spvFragmentFullyCoveredEXT"
$FLAGS = @("-profile", "spirv_1_3+$FEATURES", "-emit-spirv-directly", "-preserve-params" , "-matrix-layout-row-major" , "-fvk-use-gl-layout", "-fvk-use-entrypoint-name", "-O3")

function compile_shader {
    param(
        $in
    )

    slangc -target spirv @FLAGS "$SRC_DIR/$in" -o "$OUT_DIR/$in.spv" -I "$INCLUDE_DIR"
    slangc -target spirv-asm @FLAGS "$SRC_DIR/$in" -o "$OUT_DIR/$in.spv-asm" -I "$INCLUDE_DIR"
}

# [ ] make this part of cmake

compile_shader -in blur.slang 
compile_shader -in ngon.slang  
compile_shader -in pbr.slang  
compile_shader -in rrect.slang  

