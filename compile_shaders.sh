set -e

OUT=assets/shaders
IN=ashura/shaders

glslangValidator -V $IN/blur.frag -o $OUT/blur_downsample.frag.spv -DUPSAMPLE=0
glslangValidator -V $IN/blur.vert -o $OUT/blur_downsample.vert.spv -DUPSAMPLE=0
glslangValidator -V $IN/blur.frag -o $OUT/blur_upsample.frag.spv -DUPSAMPLE=1
glslangValidator -V $IN/blur.vert -o $OUT/blur_upsample.vert.spv -DUPSAMPLE=1
glslangValidator -V $IN/ngon.frag -o $OUT/ngon.frag.spv
glslangValidator -V $IN/ngon.vert -o $OUT/ngon.vert.spv
glslangValidator -V $IN/pbr.frag -o $OUT/pbr.frag.spv
glslangValidator -V $IN/pbr.vert -o $OUT/pbr.vert.spv
glslangValidator -V $IN/rrect.frag -o $OUT/rrect.frag.spv
glslangValidator -V $IN/rrect.vert -o $OUT/rrect.vert.spv
