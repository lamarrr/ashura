

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

int get_format_from_sample_fmt(const char **fmt,
                               enum AVSampleFormat sample_fmt) {
  int i;
  struct sample_fmt_entry {
    enum AVSampleFormat sample_fmt;
    const char *fmt_be, *fmt_le;
  } sample_fmt_entries[] = {
      {AV_SAMPLE_FMT_U8, "u8", "u8"},
      {AV_SAMPLE_FMT_S16, "s16be", "s16le"},
      {AV_SAMPLE_FMT_S32, "s32be", "s32le"},
      {AV_SAMPLE_FMT_FLT, "f32be", "f32le"},
      {AV_SAMPLE_FMT_DBL, "f64be", "f64le"},
  };
  *fmt = NULL;

  for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
    struct sample_fmt_entry *entry = &sample_fmt_entries[i];
    if (sample_fmt == entry->sample_fmt) {
      *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
      return 0;
    }
  }

  fprintf(stderr, "sample format %s is not supported as output format\n",
          av_get_sample_fmt_name(sample_fmt));
  return -1;
}

void decode(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame) {
  int i, ch;
  int ret, data_size;

  /* send the packet with the compressed data to the decoder */
  ret = avcodec_send_packet(dec_ctx, pkt);
  if (ret < 0) {
    char buf[64];

    int rec = av_strerror(ret, buf, 64);

    spdlog::info("recognized?: {}   error: {}", rec, buf);
    fprintf(stderr, "Error submitting the packet to the decoder\n");
    exit(1);
  }

  /* read all the output frames (in general there may be any number of them */
  while (ret >= 0) {
    ret = avcodec_receive_frame(dec_ctx, frame);
    spdlog::info("sample format: {}", (int)frame->format);

    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
      return;
    else if (ret < 0) {
      fprintf(stderr, "Error during decoding\n");
      exit(1);
    }
    data_size = av_get_bytes_per_sample(dec_ctx->sample_fmt);
    if (data_size < 0) {
      /* This should not occur, checking just for paranoia */
      fprintf(stderr, "Failed to calculate data size\n");
      exit(1);
    }
    for (i = 0; i < frame->nb_samples; i++)
      for (ch = 0; ch < dec_ctx->ch_layout.nb_channels; ch++) {
        // fwrite( 1, data_size, outfile);
        // fwrite(frame->data[ch] + data_size*i,  1,    data_size, )
      }
  }
}

int main(int argc, char *argv[]) {
  ASH_CHECK(argc == 3);

  std::ifstream file{argv[2], std::ios::binary | std::ios::ate};

  int mp3_size = file.tellg();

  spdlog::info("file size: {}", mp3_size);
  uint8_t *mp3 = (uint8_t *)malloc(mp3_size + AV_INPUT_BUFFER_PADDING_SIZE);

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

  if (avcodec_open2(context, codec, &dict) < 0) {
    fprintf(stderr, "could not open codec\n");
    exit(1);
  }

  uint8_t out[44];
  AVCodecParserContext *parser = av_parser_init(codec->id);

  AVFrame *decoded_frame = av_frame_alloc();
  ASH_CHECK(decoded_frame != nullptr);

  AVPacket *packet = av_packet_alloc();
  auto mp3_dec = mp3;
  auto mp3_size_dec = mp3_size;

  while (mp3_size_dec > 0) {
    int read =
        av_parser_parse2(parser, context, &packet->data, &packet->size, mp3_dec,
                         mp3_size_dec, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
    mp3_dec += read;
    mp3_size_dec -= read;

    if (packet->size != 0) {
      decode(context, packet, decoded_frame);
    }
  }

  AVFormatContext *fmt_ctx = avformat_alloc_context();
  ASH_CHECK(fmt_ctx != nullptr);
  AVDictionary *meta;

  unsigned char *iobuff = (unsigned char *)av_malloc(4096);

  buffer_data b;

  b.src = mp3;
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
  char *title;
  av_dict_get_string(fmt_ctx->metadata, &title, '=', ',');
  spdlog::info("title: {}", title);

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
