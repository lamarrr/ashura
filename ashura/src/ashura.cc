

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavutil/dict.h"
#include "libavutil/file.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
}

#include "SDL3/SDL.h"
#include "ashura/app.h"
#include "ashura/widget.h"
#include "ashura/widgets/image.h"

struct buffer_data {
  uint8_t const *src;
  int32_t left;
};

void __print_err(int err, char const *file, int line) {
  char buff[128];
  int e = av_strerror(err, buff, 128);
  spdlog::error("error: {} [{}:{}]", buff, file, line);
}

#define print_err(err) __print_err(err, __FILE__, __LINE__)

/**
 * Open an input file and the required decoder.
 * @param      filename             File to be opened
 * @param[out] input_fmt_ctx Format context of opened file
 * @param[out] input_codec_ctx  Codec context of opened file
 * @return Error code (0 if successful)
 */
int open_input_file(AVFormatContext **input_fmt_ctx,
                    AVCodecContext **decoder_ctx, AVIOContext *io) {
  AVCodecContext *avctx;
  const AVCodec *input_codec;
  const AVStream *stream;
  int error;

  /* Open the input file to read from it. */
  *input_fmt_ctx = avformat_alloc_context();
  (*input_fmt_ctx)->pb = io;

  if ((error = avformat_open_input(input_fmt_ctx, nullptr, NULL, NULL)) < 0) {
    print_err(error);
    *input_fmt_ctx = NULL;
    return error;
  }

  spdlog::info("opened formats");

  /* Get information on the input file (number of streams etc.). */
  if ((error = avformat_find_stream_info(*input_fmt_ctx, NULL)) < 0) {
    print_err(error);
    avformat_close_input(input_fmt_ctx);
    return error;
  }

  /* Make sure that there is only one stream in the input file. */
  if ((*input_fmt_ctx)->nb_streams != 1) {
    spdlog::warn("Expected one audio input stream, but found {}",
                 (*input_fmt_ctx)->nb_streams);
  }


  // av
  // for(int i = 0; i <((*input_fmt_ctx)->nb_streams; i++){
  // if(((*input_fmt_ctx)->streams[i]->)
  // }

// TODO(lamarrr): proper stream selection
  ASH_CHECK((*input_fmt_ctx)->nb_streams > 0);

  stream = (*input_fmt_ctx)->streams[0];

  /* Find a decoder for the audio stream. */
  if (!(input_codec = avcodec_find_decoder(stream->codecpar->codec_id))) {
    fprintf(stderr, "Could not find input codec\n");
    avformat_close_input(input_fmt_ctx);
    return AVERROR_EXIT;
  }

  /* Allocate a new decoding context. */
  avctx = avcodec_alloc_context3(input_codec);
  if (!avctx) {
    fprintf(stderr, "Could not allocate a decoding context\n");
    avformat_close_input(input_fmt_ctx);
    return AVERROR(ENOMEM);
  }

  /* Initialize the stream parameters with demuxer information. */
  error = avcodec_parameters_to_context(avctx, stream->codecpar);
  if (error < 0) {
    avformat_close_input(input_fmt_ctx);
    avcodec_free_context(&avctx);
    return error;
  }

  /* Open the decoder for the audio stream to use it later. */
  if ((error = avcodec_open2(avctx, input_codec, NULL)) < 0) {
    print_err(error);
    avcodec_free_context(&avctx);
    avformat_close_input(input_fmt_ctx);
    return error;
  }

  /* Set the packet timebase for the decoder. */
  avctx->pkt_timebase = stream->time_base;

  /* Save the decoder context for easier access later. */
  *decoder_ctx = avctx;

  return 0;
}

/**
 * Decode one audio frame from the input file.
 * @param      frame                Audio frame to be decoded
 * @param      input_fmt_ctx Format context of the input file
 * @param      input_codec_ctx  Codec context of the input file
 * @param[out] data_present         Indicates whether data has been decoded
 * @param[out] finished             Indicates whether the end of file has
 *                                  been reached and all data has been
 *                                  decoded. If this flag is false, there
 *                                  is more data to be decoded, i.e., this
 *                                  function has to be called again.
 * @return Error code (0 if successful)
 */
static int decode_audio_frame(AVFrame *frame, AVFormatContext *input_fmt_ctx,
                              AVCodecContext *input_codec_ctx,
                              int *data_present, int *finished) {
  /* Packet used for temporary storage. */
  AVPacket *input_packet = av_packet_alloc();
  ASH_CHECK(input_packet != nullptr);

  *data_present = 0;
  *finished = 0;
  int error;
  /* Read one audio frame from the input file into a temporary packet. */
  if ((error = av_read_frame(input_fmt_ctx, input_packet)) < 0) {
    /* If we are at the end of the file, flush the decoder below. */
    if (error == AVERROR_EOF)
      *finished = 1;
    else {
      print_err(error);
      av_packet_free(&input_packet);
      return error;
    }
  }

  /* Send the audio frame stored in the temporary packet to the decoder.
   * The input audio stream decoder is used to do this. */
  if ((error = avcodec_send_packet(input_codec_ctx, input_packet)) < 0) {
    print_err(error);
    av_packet_free(&input_packet);
    return error;
  }

  /* Receive one frame from the decoder. */
  error = avcodec_receive_frame(input_codec_ctx, frame);
  print_err(error);
  /* If the decoder asks for more data to be able to decode a frame,
   * return indicating that no data is present. */
  if (error == AVERROR(EAGAIN)) {
    error = 0;
    av_packet_free(&input_packet);
    return error;
    /* If the end of the input file is reached, stop decoding. */
  } else if (error == AVERROR_EOF) {
    *finished = 1;
    error = 0;
    av_packet_free(&input_packet);
    return error;
  } else if (error < 0) {
    print_err(error);
    av_packet_free(&input_packet);
    return error;
    /* Default case: Return decoded data. */
  } else {
    *data_present = 1;
    av_packet_free(&input_packet);
    return error;
  }
}

/**
 * Read one audio frame from the input file, decode, convert and store
 * it in the FIFO buffer.
 * @param      fifo                 Buffer used for temporary storage
 * @param      fmt_ctx Format context of the input file
 * @param      codec_ctx  Codec context of the input file
 * @param      output_codec_context Codec context of the output file
 * @param      resampler_context    Resample context for the conversion
 * @param[out] finished             Indicates whether the end of file has
 *                                  been reached and all data has been
 *                                  decoded. If this flag is false,
 *                                  there is more data to be decoded,
 *                                  i.e., this function has to be called
 *                                  again.
 * @return Error code (0 if successful)
 */
int read_decode(AVFormatContext *fmt_ctx, AVCodecContext *codec_ctx,
                int *finished) {
  /* Temporary storage of the input samples of the frame read from the file. */
  AVFrame *input_frame = av_frame_alloc();
  ASH_CHECK(input_frame != nullptr);

  /* Temporary storage for the converted input samples. */
  uint8_t **converted_input_samples = NULL;
  int data_present;
  int ret = AVERROR_EXIT;

  /* Decode one frame worth of audio samples. */
  if (decode_audio_frame(input_frame, fmt_ctx, codec_ctx, &data_present,
                         finished)) {
    av_frame_free(&input_frame);
    print_err(ret);
    return ret;
  }
  /* If we are at the end of the file and there are no more samples
   * in the decoder which are delayed, we are actually finished.
   * This must not be treated as an error. */
  if (*finished) {
    ret = 0;
    av_frame_free(&input_frame);
    return ret;
  }

  return 0;
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

  unsigned char *iobuff =
      (unsigned char *)av_malloc(mp3_size + AV_INPUT_BUFFER_PADDING_SIZE);

  buffer_data b{.src = mp3, .left = mp3_size};

  auto cb = [](void *opaque, uint8_t *buff, int size) {
    buffer_data *bd = (buffer_data *)opaque;
    spdlog::info("ptr: {}  request size: {} left: {}", (void *)bd->src, size,
                 bd->left);
    if (bd->left == 0) return AVERROR_EOF;

    size = std::min(size, bd->left);

    /* copy internal buffer data to buf */
    memcpy(buff, bd->src, size);
    bd->src += size;
    bd->left -= size;

    return size;
  };

  AVIOContext *io =
      avio_alloc_context(iobuff, mp3_size + AV_INPUT_BUFFER_PADDING_SIZE, 0, &b,
                         cb, nullptr, nullptr);
  ASH_CHECK(io != nullptr);

  AVFormatContext *fmt_ctx;
  AVCodecContext *codec_ctx;
  ASH_CHECK(open_input_file(&fmt_ctx, &codec_ctx, io) == 0);
  spdlog::info("read headers, decoding...");
  int done;
  int err  = read_decode(fmt_ctx, codec_ctx, &done);
  print_err(err);
  ASH_CHECK(err == 0);

  // AVCodec const *codec;
  // AVCodecContext *context = NULL;

  // printf("Audio encoding\n");

  // AVDictionary *dict = NULL;
  // codec = avcodec_find_decoder(AV_CODEC_ID_AAC);
  // if (!codec) {
  //   fprintf(stderr, "codec not found\n");
  //   exit(1);
  // }

  // context = avcodec_alloc_context3(codec);

  // if (avcodec_open2(context, codec, &dict) < 0) {
  //   fprintf(stderr, "could not open codec\n");
  //   exit(1);
  // }

  // uint8_t out[44];
  // AVCodecParserContext *parser = av_parser_init(codec->id);

  // AVFrame *decoded_frame = av_frame_alloc();
  // ASH_CHECK(decoded_frame != nullptr);

  // AVPacket *packet = av_packet_alloc();
  // auto mp3_dec = mp3;
  // auto mp3_size_dec = mp3_size;

  // while (mp3_size_dec > 0) {
  //   int read =
  //       av_parser_parse2(parser, context, &packet->data, &packet->size,
  //       mp3_dec,
  //                        mp3_size_dec, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
  //   mp3_dec += read;
  //   mp3_size_dec -= read;

  //   if (packet->size != 0) {
  //     decode(context, packet, decoded_frame);
  //   }
  // }

  // AVFormatContext *fmt_ctx = avformat_alloc_context();
  // ASH_CHECK(fmt_ctx != nullptr);
  // AVDictionary *meta;

  // // avformat_find_stream_info()

  // fmt_ctx->pb = io;

  // ASH_CHECK(avformat_open_input(&fmt_ctx, nullptr, nullptr, nullptr) == 0);
  // char *title;
  // av_dict_get_string(fmt_ctx->metadata, &title, '=', ',');
  // spdlog::info("title: {}", title);

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
