# defines can be found at: third_party/skia/out/Debug/obj/skia.ninja

args = [

    "is_debug=true",

    "paragraph_bench_enabled=false",
    "paragraph_gms_enabled=false",
    "paragraph_tests_enabled=false"


    "skia_build_fuzzers=false",

    "skia_compile_processors=false",
    "skia_compile_sksl_tests=false",


    "skia_enable_android_utils=false",
    "skia_enable_api_available_macro=false",
    "skia_enable_ccpr=true",
    "skia_enable_direct3d_debug_layer=false",
    "skia_enable_discrete_gpu=true",
    "skia_enable_flutter_defines=false",
    "skia_enable_fontmgr_FontConfigInterface=false",
    "skia_enable_fontmgr_android=false",
    "skia_enable_fontmgr_custom_directory=false",
    "skia_enable_fontmgr_custom_embedded=false",
    "skia_enable_fontmgr_custom_empty=false",
    "skia_enable_fontmgr_empty=false",
    "skia_enable_fontmgr_fontconfig=false",
    "skia_enable_fontmgr_fuchsia=false",
    "skia_enable_fontmgr_win=false",
    "skia_enable_fontmgr_win_gdi=false",
    "skia_enable_gpu=true",
    "skia_enable_gpu_debug_layers=true",  # debug builds
    "skia_enable_nvpr=true",
    "skia_enable_particles=false",
    "skia_enable_pdf=false",
    "skia_enable_skottie=false",
    "skia_enable_skparagraph=true",
    "skia_enable_skrive=false",
    "skia_enable_skshaper=true",
    "skia_enable_sksl_interpreter=false",
    "skia_enable_skvm_jit_when_possible=false",
    "skia_enable_spirv_validation=true",  # debug builds
    "skia_enable_svg=false",
    "skia_enable_tools=false",
    "skia_enable_vulkan_debug_layers=true",


    "skia_update_fuchsia_sdk=false",



    "skia_use_angle=false",
    "skia_use_dawn=false",
    "skia_use_direct3d=false",
    "skia_use_dng_sdk=false",
    "skia_use_egl=false",
    "skia_use_expat=false",
    "skia_use_experimental_xform=false",
    "skia_use_ffmpeg=false",
    "skia_use_fixed_gamma_text=false",
    "skia_use_fontconfig=true",
    "skia_use_fonthost_mac=false",
    "skia_use_freetype=true",
    "skia_use_freetype_woff2=false",
    "skia_use_gl=false",
    "skia_use_harfbuzz=true",
    "skia_use_icu=true",
    "skia_use_libfuzzer_defaults=true",
    "skia_use_libgifcodec=false",
    "skia_use_libheif=false",
    "skia_use_libjpeg_turbo_decode=false",
    "skia_use_libjpeg_turbo_encode=false",
    "skia_use_libpng_decode=false",
    "skia_use_libpng_encode=false",
    "skia_use_libwebp_decode=false",
    "skia_use_libwebp_encode=false",
    "skia_use_lua=false",
    "skia_use_metal=false",
    "skia_use_ndk_images=false",
    "skia_use_opencl=false",
    "skia_use_piex=false",
    "skia_use_sfml=false",
    "skia_use_sfntly=true",
    "skia_use_system_expat=false",
    "skia_use_system_freetype2=false",
    "skia_use_system_harfbuzz=false",
    "skia_use_system_icu=false",
    "skia_use_system_libjpeg_turbo=false",
    "skia_use_system_libpng=false",
    "skia_use_system_libwebp=false",
    "skia_use_system_zlib=false",
    "skia_use_vma=true",
    "skia_use_vulkan=true",
    "skia_use_webgl=false",
    "skia_use_wuffs=false",
    # skia_use_x11
    "skia_use_xps=false",
    "skia_use_zlib=false"
]


print("bin/gn gen out/Debug --args='" + "\n".join(args) + "'")
