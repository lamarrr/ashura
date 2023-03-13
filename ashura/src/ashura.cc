#include "SDL3/SDL.h"
#include "aom/aom_decoder.h"
#include "aom/aomdx.h"
#include "aom/internal/aom_codec_internal.h"
#include "aom_ports/mem_ops.h"
#include "ashura/app.h"
#include "ashura/widget.h"
#include "ashura/widgets/image.h"
#include "common/ivfdec.h"
#include "common/obudec.h"
#include "common/tools_common.h"
#include "common/video_reader.h"
#include "common/webmdec.h"
#include "webm/buffer_reader.h"
#include "webm/callback.h"
#include "webm/file_reader.h"
#include "webm/reader.h"
#include "webm/webm_parser.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>

#include "libavformat/avformat.h"
}

using namespace ash;

#define LP                                                   \
  do {                                                       \
    spdlog::info("reached log point on line #{}", __LINE__); \
  } while (0)

int main(int argc, char **argv) {
  ASH_CHECK(argc == 2);

  std::ifstream stream{argv[1], std::ios::binary | std::ios::ate};

  ASH_CHECK(stream.is_open());

  stx::Vec<char> bytes{stx::os_allocator};
  bytes.resize(stream.tellg()).unwrap();
  stream.seekg(0);
  stream.read(bytes.data(), bytes.size());

  stream.close();

  LP;
  AVFormatContext *ctx = nullptr;
  ASH_CHECK(avformat_open_input(&ctx, argv[1], nullptr, nullptr) >= 0);
  LP;
  ASH_CHECK(avformat_find_stream_info(ctx, nullptr) >= 0);
  LP;
  AVStream *st;

  int stream_idx =
      av_find_best_stream(ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
  ASH_CHECK(stream_idx >= 0);
  LP;
  st = ctx->streams[stream_idx];
  LP;

  auto *co = avcodec_find_decoder(st->codecpar->codec_id);
  ASH_CHECK(co != nullptr);
  spdlog::info("codec name: {}, long name: {}", co->name, co->long_name);

  AVCodecContext *coctx = avcodec_alloc_context3(co);
  ASH_CHECK(coctx != nullptr);
  LP;

  ASH_CHECK(avcodec_parameters_to_context(coctx, st->codecpar) >= 0);
  LP;
  ASH_CHECK(avcodec_open2(coctx, co, nullptr) >= 0);
  LP;

  AppConfig cfg{.enable_validation_layers = false};
  cfg.window_config.borderless = false;
  App app{
      std::move(cfg),
      new Image{ImageProps{
          .source = FileImageSource{.path = stx::string::make_static(argv[1])},
          .border_radius = vec4{200, 200, 200, 200},
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
