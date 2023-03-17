#include "SDL3/SDL.h"
#include "ashura/app.h"
#include "ashura/widget.h"
#include "ashura/widgets/image.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/dict.h"
#include "libavutil/imgutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/timestamp.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
}

using namespace ash;

#define LP                                                        \
  do                                                              \
  {                                                               \
    /* spdlog::info("reached log point on line #{}", __LINE__);*/ \
  } while (0)

struct VideoStream
{
  // TODO(lamarrr): handle endianness used in SDL and ffmpeg
  stx::Allocator   allocator       = stx::os_allocator;
  AVPacket        *packet          = nullptr;
  AVFrame         *frame           = nullptr;
  AVFormatContext *format_ctx      = nullptr;
  AVCodecContext  *video_codec_ctx = nullptr;
  AVCodecContext  *audio_codec_ctx = nullptr;
  SwsContext      *scale_ctx       = nullptr;
  SwrContext      *resample_ctx    = nullptr;
  u8              *image_frame     = nullptr;
  extent           image_extent;
  ImageFormat      image_fmt     = ImageFormat::Rgba;
  u8              *audio_samples = nullptr;               // interleaved channels i.e. [L, R, L, R, ...]
  u32              nchannels     = 0;                     // set to output device config
  u32              nsamples      = 0;                     // set to output device config
  u32              sample_rate   = 48000;                 // set to output device config
  PcmFormat        audio_fmt     = PcmFormat::S16;        // set to output device config

  VideoStream(char const *path)
  {
    packet = av_packet_alloc();
    ASH_CHECK(packet != nullptr);

    frame = av_frame_alloc();
    ASH_CHECK(frame != nullptr);
  }

  ~VideoStream()
  {
    if (packet != nullptr)
    {
      av_packet_free(&packet);
    }

    if (frame != nullptr)
    {
      av_frame_free(&frame);
    }

    if (format_ctx != nullptr)
    {
      avformat_close_input(&format_ctx);
    }

    if (video_codec_ctx != nullptr)
    {
      avcodec_free_context(&video_codec_ctx);
    }

    if (audio_codec_ctx != nullptr)
    {
      avcodec_free_context(&audio_codec_ctx);
    }

    if (scale_ctx != nullptr)
    {
      sws_freeContext(scale_ctx);
    }

    if (resample_ctx != nullptr)
    {
      swr_free(&resample_ctx);
    }

    if (image_frame != nullptr)
    {
      allocator.handle->deallocate(image_frame);
    }

    if (audio_samples != nullptr)
    {
      allocator.handle->deallocate(audio_samples);
    }
  }

  void dump_info()
  {
    void          *iter  = nullptr;
    AVCodec const *codec = nullptr;

    uint version = avformat_version();
    spdlog::info("ffmpeg avformat version: {}.{}.{}", AV_VERSION_MAJOR(version), AV_VERSION_MINOR(version),
                 AV_VERSION_MICRO(version));

    do
    {
      codec = av_codec_iterate(&iter);
      if (codec != nullptr)
      {
        spdlog::info("name: {}, long name: {}", codec->name, codec->long_name);
      }
    } while (codec != nullptr);
  }
};

struct AudioDevice
{};

int main(int argc, char **argv)
{
  ASH_CHECK(argc == 3);
  // TODO(lamarrr): refactor this out of window api
  ASH_CHECK(SDL_Init(SDL_INIT_EVERYTHING) == 0);

  // std::ifstream stream{argv[1], std::ios::binary | std::ios::ate};

  // ASH_CHECK(stream.is_open());

  // stx::Vec<char> bytes{stx::os_allocator};
  // bytes.resize(stream.tellg()).unwrap();
  // stream.seekg(0);
  // stream.read(bytes.data(), bytes.size());
  // stream.close();

  spdlog::info("System theme: {}", (int) SDL_GetSystemTheme());

  // might return -1
  for (int i = 0; i < SDL_GetNumAudioDrivers(); i++)
  {
    spdlog::info("audio driver: {}", SDL_GetAudioDriver(i));
  }

  for (int i = 0; i < SDL_GetNumAudioDevices(0); i++)
  {
    char const *dev_name = SDL_GetAudioDeviceName(i, 0);
    spdlog::info("audio device: {}", dev_name);

    SDL_AudioSpec     spec;
    SDL_AudioDeviceID id;

    ASH_CHECK(SDL_GetAudioDeviceSpec(i, 0, &spec) == 0);

    spec.callback;
    spdlog::info("channels: {}, format: {}, samplerate: {}, nsamples: {}", spec.channels, spec.format, spec.freq, spec.samples);

    spec.padding;
    spec.silence;
    spec.size;
    spec.userdata;
  }
  // SDL_PlayAudioDevice();
  // SDL_CloseAudioDevice();
  char         *name = nullptr;
  SDL_AudioSpec spec;
  // might be different from getaudiodevices
  ASH_CHECK(SDL_GetDefaultAudioInfo(&name, &spec, 0) == 0);
  spdlog::info("default device: {}, channels: {}, format: {}, samplerate: {}, nsamples: {}", name, spec.channels, spec.format,
               spec.freq, spec.samples);
  SDL_free(name);

  auto start = std::chrono::steady_clock::now();

  LP;
  AVFormatContext *fmt_ctx = nullptr;
  // fmt_ctx->ctx_flags       = AVFMT_FLAG_CUSTOM_IO;
  // fmt_ctx->duration;
  // fmt_ctx->chapters;
  // fmt_ctx->metadata;
  ASH_CHECK(avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr) >= 0);
  LP;
  ASH_CHECK(avformat_find_stream_info(fmt_ctx, nullptr) >= 0);
  LP;

  // av_seek_frame()

  int video_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
  ASH_CHECK(video_stream_index >= 0);
  LP;
  AVStream *video_stream = fmt_ctx->streams[video_stream_index];
  LP;

  int audio_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
  ASH_CHECK(audio_stream_index >= 0);
  LP;
  AVStream *audio_stream = fmt_ctx->streams[audio_stream_index];
  LP;

  void          *iter  = nullptr;
  const AVCodec *codec = nullptr;

  auto ver = avformat_version();
  spdlog::info("version: {}.{}.{}", AV_VERSION_MAJOR(ver), AV_VERSION_MINOR(ver), AV_VERSION_MICRO(ver));

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

  auto *audio_codec = avcodec_find_decoder(audio_stream->codecpar->codec_id);
  ASH_CHECK(audio_codec != nullptr);
  spdlog::info("audio codec name: {}, long name: {}", audio_codec->name, audio_codec->long_name);

  AVCodecContext *video_codec_ctx = avcodec_alloc_context3(video_codec);
  ASH_CHECK(video_codec_ctx != nullptr);
  LP;
  AVCodecContext *audio_codec_ctx = avcodec_alloc_context3(audio_codec);
  ASH_CHECK(audio_codec_ctx != nullptr);
  LP;

  ASH_CHECK(avcodec_parameters_to_context(video_codec_ctx, video_stream->codecpar) >= 0);
  LP;
  ASH_CHECK(avcodec_parameters_to_context(audio_codec_ctx, audio_stream->codecpar) >= 0);
  LP;
  ASH_CHECK(avcodec_open2(video_codec_ctx, video_codec, nullptr) >= 0);
  LP;
  ASH_CHECK(avcodec_open2(audio_codec_ctx, audio_codec, nullptr) >= 0);
  LP;

  AVFrame *frame = av_frame_alloc();
  ASH_CHECK(frame != nullptr);

  AVPacket *packet = av_packet_alloc();
  ASH_CHECK(packet != nullptr);

  bool done = false;

  while (av_read_frame(fmt_ctx, packet) == 0)
  {
    if (packet->stream_index == video_stream_index)
    {
      // spdlog::info("got video frame, sending packet");
      int err = avcodec_send_packet(video_codec_ctx, packet);
      // spdlog::info("sent packet");

      if (err < 0)
      {
        // spdlog::error("error: {}", err);
        break;
      }

      while (err >= 0)
      {
        // spdlog::info("requesting image frame from video frame");
        err = avcodec_receive_frame(video_codec_ctx, frame);
        // spdlog::info("got image frame");

        if (err < 0)
        {
          if (err == AVERROR_EOF)
          {
            // spdlog::info("eof");
            done = true;
            break;
          }
          else if (err == AVERROR(EAGAIN))
          {
            // spdlog::info("finished image frames from video frame");
            break;
          }
          else
          {
            // spdlog::error("got error: {}", (int) err);
            return -1;
          }
        }

        // spdlog::info("decoded image frame with format: {}",
        // av_get_pix_fmt_name((AVPixelFormat) frame->format));

        SwsContext *context   = sws_getContext(frame->width, frame->height, (AVPixelFormat) frame->format, frame->width,
                                               frame->height, AV_PIX_FMT_RGB24, 0, 0, 0, 0);
        u8         *rgb       = new u8[frame->width * frame->height * 3];
        u8         *planes[]  = {rgb};
        int         strides[] = {3 * frame->width};
        sws_scale(context, frame->data, frame->linesize, 0, frame->height, planes, strides);

        sws_freeContext(context);

        // TODO(lamarrr): does this mean it could be multi-threaded? and another
        // thread could be using it?
        av_frame_unref(frame);

        delete[] rgb;
      }

      if (done)
      {
        // spdlog::info("finished decoding");
        break;
      }
    }
    else if (packet->stream_index == audio_stream_index)
    {
      // spdlog::info("got audio frame, sending packet");
      int err = avcodec_send_packet(audio_codec_ctx, packet);
      // spdlog::info("sent packet");

      if (err < 0)
      {
        // spdlog::error("error: {}", err);
        break;
      }

      while (err >= 0)
      {
        // spdlog::info("requesting audio samples from audio frame");
        err = avcodec_receive_frame(audio_codec_ctx, frame);
        // spdlog::info("got audio samples");

        if (err < 0)
        {
          if (err == AVERROR_EOF)
          {
            // spdlog::info("eof");
            done = true;
            break;
          }
          else if (err == AVERROR(EAGAIN))
          {
            // spdlog::info("finished audio samples from audio frame");
            break;
          }
          else
          {
            // spdlog::error("got error: {}", (int) err);
            return -1;
          }
        }
        // NOTE(lamarrr): SDL stereo is interleaved
        // TODO(lamarrr) frame->format might be -1 meaning unknown AvPixelFormat
        // for images and AvSampleFormat for audio spdlog::info("decoded audio
        // samples with format: {}, nsamples: {}, sample rate: {}, nchannels:
        // {}",
        //              av_get_sample_fmt_name((AVSampleFormat) frame->format),
        //              frame->nb_samples, frame->sample_rate,
        //              frame->ch_layout.nb_channels);

        // AV_SAMPLE_FMT_*_P are not interleaved but planar
        // AV_SAMPLE_FMT_* are interleaved

        AVSampleFormat target_format = AV_SAMPLE_FMT_S16;

        // TODO(lamarrr) Check SDL for supported audio formats
        SwrContext *sw_context = nullptr;
        ASH_CHECK(swr_alloc_set_opts2(&sw_context, &frame->ch_layout, target_format, frame->sample_rate, &frame->ch_layout,
                                      (AVSampleFormat) frame->format, frame->sample_rate, 0, nullptr) == 0);
        ASH_CHECK(swr_init(sw_context) == 0);
        LP;
        // NOTE: interleaved single plane
        usize samples_buffer_size = av_samples_get_buffer_size(nullptr, frame->ch_layout.nb_channels, frame->nb_samples,
                                                               (AVSampleFormat) frame->format, 1);
        u8   *samples             = (u8 *) malloc(samples_buffer_size);
        ASH_CHECK(samples != nullptr);
        LP;
        ASH_CHECK(swr_convert(sw_context, &samples, frame->nb_samples, const_cast<u8 const **>(frame->data),
                              frame->nb_samples) == frame->nb_samples);
        LP;
        free(samples);
        LP;
        // spdlog::info("converted audio from: {}, to: {}",
        // av_get_sample_fmt_name((AVSampleFormat) frame->format),
        // av_get_sample_fmt_name(target_format));
        LP;
        swr_free(&sw_context);
        LP;
        // TODO(lamarrr): does this mean it could be multi-threaded? and another
        // thread could be using it?
        av_frame_unref(frame);
        LP;
      }

      if (done)
      {
        // spdlog::info("finished decoding");
        break;
      }
    }
  }

  int           width  = video_codec_ctx->width;
  int           height = video_codec_ctx->height;
  AVPixelFormat fmt    = video_codec_ctx->pix_fmt;
  ASH_CHECK(fmt != AV_PIX_FMT_NONE);

  av_packet_free(&packet);
  av_frame_free(&frame);
  avcodec_free_context(&video_codec_ctx);
  avcodec_free_context(&audio_codec_ctx);
  avformat_close_input(&fmt_ctx);

  spdlog::info("finished in {} seconds",
               std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count());

  AppConfig cfg{.enable_validation_layers = false};
  cfg.window_config.borderless = false;
  App                                   app{std::move(cfg), new Image{ImageProps{.source         = FileImageSource{.path = stx::string::make_static(argv[2])},
                                                                                 .border_radius  = vec4{200, 200, 200, 200},
                                                                                 .resize_on_load = true}}};
  std::chrono::steady_clock::time_point last_tick = std::chrono::steady_clock::now();
  while (true)
  {
    auto present = std::chrono::steady_clock::now();
    app.tick(present - last_tick);
    last_tick = present;
  }

  SDL_Quit();
  return 0;
}
