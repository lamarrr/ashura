#include "SDL3/SDL.h"
#include "ashura/app.h"
#include "ashura/widget.h"
#include "ashura/widgets/image.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/timestamp.h"
#include "libswscale/swscale.h"
}

using namespace ash;

#define LP                                                   \
  do {                                                       \
    spdlog::info("reached log point on line #{}", __LINE__); \
  } while (0)

int main(int argc, char **argv) {
  ASH_CHECK(argc == 3);

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

  void *iter = nullptr;
  const AVCodec *codec = nullptr;

  auto ver = avformat_version();
  spdlog::info("version: {}.{}.{}", AV_VERSION_MAJOR(ver),
               AV_VERSION_MINOR(ver), AV_VERSION_MICRO(ver));

  do {
    codec = av_codec_iterate(&iter);
    if (codec != nullptr) {
      spdlog::info("name: {}, long name: {}", codec->name, codec->long_name);
    }
  } while (codec != nullptr);

  auto *co = avcodec_find_decoder(st->codecpar->codec_id);
  ASH_CHECK(co != nullptr);
  spdlog::info("codec name: {}, long name: {}", co->name, co->long_name);

  AVCodecContext *cctx = avcodec_alloc_context3(co);
  ASH_CHECK(cctx != nullptr);
  LP;

  ASH_CHECK(avcodec_parameters_to_context(cctx, st->codecpar) >= 0);
  LP;
  ASH_CHECK(avcodec_open2(cctx, co, nullptr) >= 0);
  LP;

  AVFrame *frame = av_frame_alloc();
  ASH_CHECK(frame != nullptr);

  AVPacket *packet = av_packet_alloc();
  ASH_CHECK(packet != nullptr);

  bool done = false;
  while (av_read_frame(ctx, packet) >= 0) {
    int err = avcodec_send_packet(cctx, packet);

    if (err < 0) {
      spdlog::error("error: {}", err);
      break;
    }

    while (err >= 0) {
      err = avcodec_receive_frame(cctx, frame);
      if (err < 0) {
        if (err == AVERROR_EOF) {
          done = true;
          break;
        }
        if (err == AVERROR(EAGAIN)) break;
      }

      spdlog::info("decoded frame with format: {}",
                   av_get_pix_fmt_name((AVPixelFormat)frame->format));

      SwsContext *context = sws_getContext(
          frame->width, frame->height, (AVPixelFormat)frame->format,
          frame->width, frame->height, AV_PIX_FMT_RGB24, 0, 0, 0, 0);
      u8 *rgb = new u8[frame->width * frame->height * 3];
      u8 *planes[] = {rgb};
      int strides[] = {3 * frame->width};
      sws_scale(context, frame->data, frame->linesize, 0, frame->height, planes,
                strides);

      av_frame_unref(frame);
    }

    if (done) {
      spdlog::info("finished decoding");
      break;
    }
  }

  int width = cctx->width;
  int height = cctx->height;
  AVPixelFormat fmt = cctx->pix_fmt;
  ASH_CHECK(fmt != AV_PIX_FMT_NONE);

  u8 *dst_data[4] = {0, 0, 0, 0};
  int line_sizes[4] = {0, 0, 0, 0};
  int img_read = av_image_alloc(dst_data, line_sizes, width, height, fmt, 1);
  ASH_CHECK(img_read >= 0);

  AppConfig cfg{.enable_validation_layers = false};
  cfg.window_config.borderless = false;
  App app{
      std::move(cfg),
      new Image{ImageProps{
          .source = FileImageSource{.path = stx::string::make_static(argv[2])},
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
