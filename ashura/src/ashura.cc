#include "SDL3/SDL.h"
#include "ashura/app.h"
#include "ashura/widget.h"
#include "ashura/widgets/image.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/timestamp.h"
#include "libswscale/swscale.h"
}

using namespace ash;

#define LP                                                   \
  do                                                         \
  {                                                          \
    spdlog::info("reached log point on line #{}", __LINE__); \
  } while (0)

int main(int argc, char **argv)
{
  ASH_CHECK(argc == 3);

  // std::ifstream stream{argv[1], std::ios::binary | std::ios::ate};

  // ASH_CHECK(stream.is_open());

  // stx::Vec<char> bytes{stx::os_allocator};
  // bytes.resize(stream.tellg()).unwrap();
  // stream.seekg(0);
  // stream.read(bytes.data(), bytes.size());

  // stream.close();

  LP;
  AVFormatContext *fmt_ctx = nullptr;
  ASH_CHECK(avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr) >= 0);
  LP;
  ASH_CHECK(avformat_find_stream_info(fmt_ctx, nullptr) >= 0);
  LP;

  int video_stream_index =
      av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
  ASH_CHECK(video_stream_index >= 0);
  LP;
  AVStream *video_stream = fmt_ctx->streams[video_stream_index];
  LP;

  // int audio_stream_idx =
  //     av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
  // ASH_CHECK(audio_stream_idx >= 0);
  // LP;
  // AVStream *audio_stream = fmt_ctx->streams[audio_stream_idx];
  // LP;

  void          *iter  = nullptr;
  const AVCodec *codec = nullptr;

  auto ver = avformat_version();
  spdlog::info("version: {}.{}.{}", AV_VERSION_MAJOR(ver),
               AV_VERSION_MINOR(ver), AV_VERSION_MICRO(ver));

  do
  {
    codec = av_codec_iterate(&iter);
    if (codec != nullptr)
    {
      spdlog::info("name: {}, long name: {}", codec->name, codec->long_name);
    }
  } while (codec != nullptr);

  auto *video_codec = avcodec_find_decoder(video_stream->codecpar->codec_id);
  ASH_CHECK(video_codec != nullptr);
  spdlog::info("video codec name: {}, long name: {}", video_codec->name, video_codec->long_name);

  // auto *audio_codec = avcodec_find_decoder(audio_stream->codecpar->codec_id);
  // ASH_CHECK(audio_codec != nullptr);
  // spdlog::info("audio codec name: {}, long name: {}", audio_codec->name, audio_codec->long_name);

  AVCodecContext *video_codec_ctx = avcodec_alloc_context3(video_codec);
  ASH_CHECK(video_codec_ctx != nullptr);
  LP;
  // AVCodecContext *audio_codec_ctx = avcodec_alloc_context3(audio_codec);
  // ASH_CHECK(audio_codec_ctx != nullptr);
  // LP;

  ASH_CHECK(avcodec_parameters_to_context(video_codec_ctx, video_stream->codecpar) >= 0);
  LP;
  // ASH_CHECK(avcodec_parameters_to_context(audio_codec_ctx, audio_stream->codecpar) >= 0);
  // LP;
  ASH_CHECK(avcodec_open2(video_codec_ctx, video_codec, nullptr) >= 0);
  LP;
  // ASH_CHECK(avcodec_open2(audio_codec_ctx, audio_codec, nullptr) >= 0);
  // LP;

  AVFrame *frame = av_frame_alloc();
  ASH_CHECK(frame != nullptr);

  AVPacket *packet = av_packet_alloc();
  ASH_CHECK(packet != nullptr);

  bool done = false;

  while (av_read_frame(fmt_ctx, packet) == 0)
  {
    spdlog::info("got frame, sending packet");
    int err = avcodec_send_packet(video_codec_ctx, packet);
    spdlog::info("sent packet");

    if (err < 0)
    {
      spdlog::error("error: {}", err);
      break;
    }

    while (err >= 0)
    {
      spdlog::info("decoding frame");
      err = avcodec_receive_frame(video_codec_ctx, frame);
      if (err < 0)
      {
        if (err == AVERROR_EOF)
        {
          spdlog::info("eof");
          done = true;
          break;
        }
        else if (err == AVERROR(EAGAIN))
        {
          spdlog::info("finished images from frame");
          break;
        }
        else
        {
          spdlog::error("got error: {}", (int) err);
          return -1;
        }
      }

      spdlog::info("decoded frame with format: {}",
                   av_get_pix_fmt_name((AVPixelFormat) frame->format));

      SwsContext *context = sws_getContext(
          frame->width, frame->height, (AVPixelFormat) frame->format,
          frame->width, frame->height, AV_PIX_FMT_RGB24, 0, 0, 0, 0);
      u8 *rgb       = new u8[frame->width * frame->height * 3];
      u8 *planes[]  = {rgb};
      int strides[] = {3 * frame->width};
      sws_scale(context, frame->data, frame->linesize, 0, frame->height, planes,
                strides);

      // TODO(lamarrr): is this necessary?
      av_frame_unref(frame);

      delete[] rgb;
    }

    if (done)
    {
      spdlog::info("finished decoding");
      break;
    }
  }

  int           width  = video_codec_ctx->width;
  int           height = video_codec_ctx->height;
  AVPixelFormat fmt    = video_codec_ctx->pix_fmt;
  ASH_CHECK(fmt != AV_PIX_FMT_NONE);

  av_packet_free(&packet);
  av_frame_free(&frame);
  avcodec_free_context(&video_codec_ctx);
  avformat_close_input(&fmt_ctx);

  AppConfig cfg{.enable_validation_layers = false};
  cfg.window_config.borderless = false;
  App app{
      std::move(cfg),
      new Image{ImageProps{
          .source         = FileImageSource{.path = stx::string::make_static(argv[2])},
          .border_radius  = vec4{200, 200, 200, 200},
          .resize_on_load = true}}};
  std::chrono::steady_clock::time_point last_tick =
      std::chrono::steady_clock::now();
  while (true)
  {
    auto present = std::chrono::steady_clock::now();
    app.tick(present - last_tick);
    last_tick = present;
  }

  return 0;
}
