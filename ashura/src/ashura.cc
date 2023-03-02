

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavutil/dict.h"
#include "libavutil/file.h"
#include "libavutil/mathematics.h"

}

#include "SDL3/SDL.h"
#include "ashura/app.h"
#include "ashura/widget.h"
#include "ashura/widgets/image.h"

struct buffer_data {
  uint8_t const *src;
  int32_t left;
};

int main(int argc, char *argv[]) {
  ASH_CHECK(argc == 3);

  std::ifstream file{argv[2], std::ios::binary | std::ios::ate};

  size_t mp3_size = file.tellg();

  spdlog::info("file size: {}", mp3_size);
  void *mp3 = malloc(mp3_size + AV_INPUT_BUFFER_PADDING_SIZE);

  file.seekg(0);
  file.read((char *)mp3, mp3_size);
  file.close();

  AVCodec const *codec;
  AVCodecContext *context = NULL;

  printf("Audio encoding\n");

  AVDictionary *dict = NULL;
  codec = avcodec_find_decoder(AV_CODEC_ID_AAC);
  if (!codec) {
    fprintf(stderr, "codec not found\n");
    exit(1);
  }

  context = avcodec_alloc_context3(codec);

  // // put sample parameters
  // c->bit_rate = 64000;
  // c->sample_rate = 44100;
  // c->channels = 2;
  // c->sample_fmt = AV_SAMPLE_FMT_S16;

  if (avcodec_open2(context, codec, &dict) < 0) {
    fprintf(stderr, "could not open codec\n");
    exit(1);
  }

  AVPacket avpkt;

  av_init_packet(&avpkt);
  avpkt.data = (uint8_t *)mp3;
  avpkt.size = mp3_size;

  uint8_t out[44];
  ASH_CHECK(avcodec_send_packet(context, &avpkt) == 0);

  AVFrame *frame = av_frame_alloc();
  ASH_CHECK(frame != nullptr);

  ASH_CHECK(avcodec_receive_frame(context, frame) == 0);
  int frame_size = av_samples_get_buffer_size(
      nullptr, context->channels, frame->nb_samples, context->sample_fmt, 1);
  spdlog::info(
      "channels : {}, data: {}, nsamples per frame: {}, duration of this "
      "frame: {}, current frame size: {}",
      frame->channels, AS_I32(frame->data[0][0]), frame->nb_samples,
      frame->pkt_duration, frame_size);

      {
  AVFormatContext *fmt_ctx = avformat_alloc_context();
  ASH_CHECK(fmt_ctx != nullptr);
unsigned char *iobuff = (unsigned char *)av_malloc(4096);

  buffer_data b;

  b.src = (uint8_t *)mp3;
  b.left = mp3_size;

  // avformat_find_stream_info()
  AVIOContext *io = avio_alloc_context(
      iobuff, 4096, 0, &b,
      [](void *opaque, uint8_t *buff, int size) {
        buffer_data *bd = (buffer_data *)opaque;
        spdlog::info("ptr: {}  request size: {} left: {}", (void *)bd->src,
                     size, bd->left);
        if (bd->left == 0) return AVERROR_EOF;

        size = std::min(size, bd->left);

        /* copy internal buffer data to buf */
        memcpy(buff, bd->src, size);
        bd->src += size;
        bd->left -= size;

        return size;
      },
      nullptr, nullptr);
  ASH_CHECK(io != nullptr);
  fmt_ctx->pb = io;

  ASH_CHECK(avformat_open_input(&fmt_ctx, nullptr, nullptr, nullptr) == 0);

      }

  // AVFormatContext *fmt_ctx = avformat_alloc_context();
  // ASH_CHECK(fmt_ctx != nullptr);
  // AVDictionary *meta;

  // unsigned char *iobuff = (unsigned char *)av_malloc(4096);

  // buffer_data b;

  // b.src = (uint8_t *)mp3;
  // b.left = mp3_size;

  // // avformat_find_stream_info()
  // AVIOContext *io = avio_alloc_context(
  //     iobuff, 4096, 0, &b,
  //     [](void *opaque, uint8_t *buff, int size) {
  //       buffer_data *bd = (buffer_data *)opaque;
  //       spdlog::info("ptr: {}  request size: {} left: {}", (void *)bd->src,
  //                    size, bd->left);
  //       if (bd->left == 0) return AVERROR_EOF;

  //       size = std::min(size, bd->left);

  //       /* copy internal buffer data to buf */
  //       memcpy(buff, bd->src, size);
  //       bd->src += size;
  //       bd->left -= size;

  //       return size;
  //     },
  //     nullptr, nullptr);
  // ASH_CHECK(io != nullptr);
  // fmt_ctx->pb = io;

  // ASH_CHECK(avformat_open_input(&fmt_ctx, nullptr, nullptr, nullptr) == 0);
  // char *title;
  // av_dict_get_string(fmt_ctx->metadata, &title, '=', ',');
  // spdlog::info("title: {}", title);

  // avformat_find_stream_info(fmt_ctx, nullptr);
  // avpkt.data = buff;
  // ASH_CHECK(
  // avformat_find_stream_info(fmt_ctx, nullptr ) == 0);

  // int len = avcodec_decode_audio3(c, (short *)out, &out_size, &avpkt);
  // ASH_CHECK(len > 0);

  // /* the codec gives us the frame size, in samples */
  // frame_size = c->frame_size;
  // samples = malloc(frame_size * 2 * c->channels);
  // outbuf_size = 10000;
  // outbuf = malloc(outbuf_size);

  // SDL_SetMainReady();
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
