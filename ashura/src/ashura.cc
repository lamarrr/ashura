#include "SDL3/SDL.h"
#include "ashura/app.h"
#include "ashura/widget.h"
#include "ashura/widgets/image.h"


#include "aom/aom_decoder.h"
#include "aom/aomdx.h"
#include "common/obudec.h"
#include "common/tools_common.h"
#include "common/video_reader.h"

int main(int argc, char** argv) {
  ASH_CHECK(argc == 2);

  AvxInputContext aom_input_ctx;
    ObuDecInputContext obu_ctx = { &aom_input_ctx, NULL, 0, 0, 0 };
  aom_codec_stream_info_t si;
  uint8_t tmpbuf[32];
  unsigned int i;

 // obu_ctx.avx_ctx->file = inputfile;
  // obu_ctx.avx_ctx->filename = argv[1];
  aom_codec_iface_t *decoder = get_aom_decoder_by_index(0);
  printf("Using %s\n", aom_codec_iface_name(decoder));



  ash::AppConfig cfg{.enable_validation_layers = false};
  cfg.window_config.borderless = false;
  ash::App app{
      std::move(cfg),
      new ash::Image{ash::ImageProps{
          .source =
              ash::FileImageSource{.path = stx::string::make_static(argv[1])},
          .border_radius = ash::vec4{200, 200, 200, 200},
          .aspect_ratio = stx::Some(1.0f),
          .resize_on_load = true}}};
  std::chrono::steady_clock::time_point last_tick =
      std::chrono::steady_clock::now();
  while (true) {
    auto present = std::chrono::steady_clock::now();
    app.tick(present - last_tick);
    last_tick = present;
  }

  return 0;
}
