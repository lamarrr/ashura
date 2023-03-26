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
using Clock        = std::chrono::steady_clock;
using timepoint    = Clock::time_point;
using nanoseconds  = std::chrono::nanoseconds;
using milliseconds = std::chrono::milliseconds;
using seconds      = std::chrono::seconds;

constexpr nanoseconds SYNC_THRESHOLD   = milliseconds{10};
constexpr nanoseconds NOSYNC_THRESHOLD = seconds{10};
constexpr nanoseconds MAX_FRAME_DELAY  = seconds{1};

enum class Error : i64
{
  Ok           = 0,
  Eof          = AVERROR(EOF),
  NeedsPackets = AVERROR(EAGAIN),
  Invalid      = AVERROR(EINVAL)
};

struct AudioDeviceInfo
{
  stx::String   name;
  SDL_AudioSpec spec{.freq = 48000, .format = AUDIO_S16SYS, .channels = 2, .samples = 4800, .size = 4800 * 2, .callback = nullptr, .userdata = nullptr};

  static stx::Vec<AudioDeviceInfo> enumerate()
  {
    stx::Vec<AudioDeviceInfo> devices{stx::os_allocator};

    for (int i = 0; i < SDL_GetNumAudioDevices(false); i++)
    {
      char const   *device_name = SDL_GetAudioDeviceName(i, false);
      SDL_AudioSpec spec;
      int           err = SDL_GetAudioDeviceSpec(i, false, &spec);

      if (err != 0)
      {
        continue;
      }

      devices.push(AudioDeviceInfo{
                       .name = stx::string::make(stx::os_allocator, device_name).unwrap(),
                       .spec = spec})
          .unwrap();
    }

    return devices;
  }

  static stx::Option<AudioDeviceInfo> get_default()
  {
    char         *device_name = nullptr;
    SDL_AudioSpec spec;
    int           err = SDL_GetDefaultAudioInfo(&device_name, &spec, false);

    if (err != 0)
    {
      return stx::None;
    }

    AudioDeviceInfo info{
        .name = device_name == nullptr ? stx::string::make_static("") : stx::string::make(stx::os_allocator, device_name).unwrap(),
        .spec = spec};

    SDL_free(device_name);

    return stx::Some(std::move(info));
  }
};

struct VideoFrame
{
  stx::Option<ImageBuffer> image;
  nanoseconds              pts{0};

  void fit(int width, int height)
  {
    extent target_extent{width, height};
    if (image.is_none())
    {
      image = stx::Some(ImageBuffer{
          .memory = stx::mem::allocate(stx::os_allocator, target_extent.area() * 3).unwrap(),
          .extent = target_extent,
          .format = ImageFormat::Rgb});
    }
    else
    {
      if (image.value().extent != target_extent)
      {
        stx::mem::reallocate(image.value().memory, target_extent.area() * 3).unwrap();
        image.value().extent = target_extent;
      }
    }
  }
};

// Demuxer runs on main thread, fetches raw streams/packets from the files and dispatches them to the decoder threads
//
// the audio/video decode thread decodes audio/video frames, performs conversions/resampling and sends them to the renderer/audio device
//
struct Decoder
{
  stx::SpinLock        lock;
  AVCodecContext      *ctx    = nullptr;
  AVStream            *stream = nullptr;
  AVFrame             *frame  = nullptr;
  stx::Vec<AVPacket *> packets{stx::os_allocator};

  void push_packet(AVPacket *packet)
  {
    AVPacket *dst = av_packet_alloc();
    ASH_CHECK(dst != nullptr);
    av_packet_move_ref(dst, packet);
    {
      stx::LockGuard guard{lock};
      packets.push_inplace(dst).unwrap();
    }
  }

  // bool /*Error*/ try_send_packet()
  // {
  //   AVPacket *packet;
  //   {
  //     stx::LockGuard guard{lock};
  //     if (!packets.is_empty())
  //     {
  //       packet = packets[0];
  //       packets.erase(packets.span().slice(0, 1));
  //     }
  //     else
  //       return;
  //   }

  //   int err = avcodec_send_packet(ctx, packet);
  //   av_packet_unref(packet);
  //   av_packet_free(&packet);

  //   if (err < 0)
  //   {
  //   }
  // }

  void destroy()
  {
    avcodec_free_context(&ctx);
    av_frame_free(&frame);
    for (AVPacket *packet : packets)
    {
      av_packet_free(&packet);
    }
  }
};

struct VideoDemuxer
{
  AVFormatContext *ctx    = nullptr;
  AVPacket        *packet = nullptr;

  void open(char const *path)
  {
    ASH_CHECK(avformat_open_input(&ctx, path, nullptr, nullptr) >= 0);
    // checks if codec or file format is supported
    ASH_CHECK(avformat_find_stream_info(ctx, nullptr) >= 0);
  }

  void destroy()
  {
    avformat_close_input(&ctx);
    av_packet_free(&packet);
  }

  stx::Option<Decoder> make_decoder(AVMediaType media_type) const
  {
    int stream_index = av_find_best_stream(ctx, media_type, -1, -1, nullptr, 0);

    if (stream_index < 0)
    {
      return stx::None;
    }

    AVStream *stream = ctx->streams[stream_index];

    if (stream == nullptr)
    {
      return stx::None;
    }

    AVCodec const *codec = avcodec_find_decoder(stream->codecpar->codec_id);

    if (codec == nullptr)
    {
      return stx::None;
    }

    AVCodecContext *codec_context = avcodec_alloc_context3(codec);

    ASH_CHECK(codec_context != nullptr);
    ASH_CHECK(avcodec_parameters_to_context(codec_context, stream->codecpar) >= 0);
    ASH_CHECK(avcodec_open2(codec_context, codec, nullptr) >= 0);

    AVFrame *frame = av_frame_alloc();
    ASH_CHECK(frame != nullptr);

    return stx::Some(Decoder{.ctx = codec_context, .stream = stream, .frame = frame});
  }

  stx::Option<Decoder> make_video_decoder() const
  {
    return make_decoder(AVMEDIA_TYPE_VIDEO);
  }

  stx::Option<Decoder> make_audio_decoder() const
  {
    return make_decoder(AVMEDIA_TYPE_AUDIO);
  }

  stx::Option<Decoder> make_subtitle_decoder() const
  {
    return make_decoder(AVMEDIA_TYPE_SUBTITLE);
  }

  int read_frame(stx::Span<Decoder *const> decoders)
  {
    int err = av_read_frame(ctx, packet);
    if (err < 0)
    {
      return err;
    }

    for (Decoder *decoder : decoders)
    {
      if (packet->stream_index == decoder->stream->index)
      {
        decoder->push_packet(packet);
        break;
      }
    }

    return 0;
  }
};

struct ResamplerConfig
{
  AVSampleFormat  src_fmt         = AV_SAMPLE_FMT_NONE;
  AVSampleFormat  dst_fmt         = AV_SAMPLE_FMT_NONE;
  int             src_sample_rate = 0;
  int             dst_sample_rate = 0;
  AVChannelLayout channel_layout  = AV_CHANNEL_LAYOUT_MONO;

  bool operator==(ResamplerConfig const &other) const
  {
    return src_fmt == other.src_fmt && dst_fmt == other.dst_fmt && src_sample_rate == other.src_sample_rate && dst_sample_rate == other.dst_sample_rate && (av_channel_layout_compare(&channel_layout, &other.channel_layout) == 0);
  }

  bool operator!=(ResamplerConfig const &other) const
  {
    return !(*this == other);
  }
};

struct VideoDecoder
{
  Decoder     decoder;
  nanoseconds last_frame_pts{0};
  nanoseconds last_frame_delay{0};
  nanoseconds total_delays{0};
  timepoint   begin_timepoint;

  nanoseconds get_frame_delay() const
  {
    f32 time_base   = ratio{.numerator = decoder.stream->time_base.num, .denominator = decoder.stream->time_base.den}.as_f32();
    f32 extra_delay = decoder.frame->repeat_pict * time_base / 2;
    f32 delay       = time_base + extra_delay;
    return nanoseconds{AS(nanoseconds::rep, nanoseconds{seconds{1}}.count() * delay)};
  }

  Error decode_frame()
  {
    // stx::LockGuard guard{lock};
    int err = avcodec_receive_frame(decoder.ctx, decoder.frame);
    if (err < 0)
    {
      return AS(Error, err);
    }
    // av_frame_unref(frame);
    // frame->width;
    return Error::Ok;
  }

  template <typename... T>
  void schedule_refresh(T...);

  // frame is a newly decoded frame
  void refresh(/*Ctx &ctx,*/ VideoFrame &frame, std::atomic<nanoseconds::rep> const &audio_clock)
  {
    nanoseconds delay = frame.pts - ctx.last_frame_pts;

    if (delay <= nanoseconds{0} || delay >= MAX_FRAME_DELAY)
    {
      // means delay is incorrect
      delay = ctx.last_frame_delay;
    }

    ctx.last_frame_pts         = frame.pts;
    ctx.last_frame_delay       = delay;
    nanoseconds diff           = frame.pts - nanoseconds{audio_clock.load(std::memory_order_relaxed)};
    nanoseconds sync_threshold = delay > SYNC_THRESHOLD ? delay : SYNC_THRESHOLD;

    if (std::chrono::abs(diff) < NOSYNC_THRESHOLD)
    {
      if (std::chrono::abs(diff) <= -sync_threshold)
      {
        delay = nanoseconds{0};
      }
      else if (diff >= sync_threshold)
      {
        delay *= 2;
      }
    }

    ctx.total_delays += delay;
    nanoseconds time_passed  = std::chrono::steady_clock::now() - ctx.begin_timepoint;
    nanoseconds actual_delay = ctx.total_delays - time_passed;

    if (actual_delay.count() < 0)
    {
      actual_delay = milliseconds{10};
    }

    schedule_refresh(frame, actual_delay);
    // video_display()
  }
};

struct AudioDecoder
{
  Decoder                       decoder;
  std::atomic<nanoseconds::rep> clock;

  void decode_frame()
  {
    AVPacket *packet;
    f32       clock_f = 0;
    // TODO: WE CAN'T SET TO ZERO
    if (packet->pts != AV_NOPTS_VALUE)
    {
      clock_f = ratio{.numerator = decoder.stream->time_base.num, .denominator = decoder.stream->time_base.den}.as_f32() * packet->pts;
    }

    avcodec_send_packet(decoder.ctx, packet);
    avcodec_receive_frame(decoder.ctx, decoder.frame);
    int n   = 2 * decoder.ctx->channels;        // why 2?
    f32 pts = clock_f;
    decoder.frame->clock_f += decoder.frame->pkt_duration;
    // f32 pts =
  }
};

// TODO(lamarrr): we are not using timepoints, why?
struct AudioDevice
{
  SDL_AudioDeviceID id = 0;
  AudioDeviceInfo   info;
  SwrContext       *resampler = nullptr;
  ResamplerConfig   resampler_cfg;
  AudioDecoder     *decoder = nullptr;
  std::atomic<f32>  volume  = 1;

  // will be called on a different thread, use SDL_LockAudioDevice and SDL_UnlockAudioDevice to prevent it from running. i.e syncing
  static void audio_callback(void *userdata, u8 *stream, int len)
  {
    AudioDevice *This   = AS(AudioDevice *, userdata);
    f32          volume = This->volume.load(std::memory_order_relaxed);

    This->decoder->decode_frame();

    ResamplerConfig target_cfg{
        .src_fmt         = AS(AVSampleFormat, This->decoder->decoder.frame->format),
        .dst_fmt         = AV_SAMPLE_FMT_NONE,
        .src_sample_rate = This->decoder->decoder.frame->sample_rate,
        .dst_sample_rate = This->info.spec.freq,
        .channel_layout  = This->decoder->decoder.frame->ch_layout};

    switch (This->info.spec.format)
    {
      case AUDIO_U8:
      {
        target_cfg.dst_fmt = AV_SAMPLE_FMT_U8;
      }
      break;
      case AUDIO_S16SYS:
      {
        target_cfg.dst_fmt = AV_SAMPLE_FMT_S16;
      }
      break;
      case AUDIO_S32SYS:
      {
        target_cfg.dst_fmt = AV_SAMPLE_FMT_S32;
      }
      break;
      case AUDIO_F32SYS:
      {
        target_cfg.dst_fmt = AV_SAMPLE_FMT_FLT;
      }
      break;
      default:
        break;
    }

    switch (This->info.spec.channels)
    {
      case 1:
      {
        target_cfg.channel_layout = AV_CHANNEL_LAYOUT_MONO;
      }
      break;
      case 2:
      {
        target_cfg.channel_layout = AV_CHANNEL_LAYOUT_STEREO;
      }
      break;
      case 4:
      {
        target_cfg.channel_layout = AV_CHANNEL_LAYOUT_QUAD;
      }
      break;
      case 6:
      {
        target_cfg.channel_layout = AV_CHANNEL_LAYOUT_HEXAGONAL;
      }
      break;
      case 8:
      {
        target_cfg.channel_layout = AV_CHANNEL_LAYOUT_OCTAGONAL;
      }
      break;
      case 16:
      {
        target_cfg.channel_layout = AV_CHANNEL_LAYOUT_HEXADECAGONAL;
      }
      break;
      default:
      {
        spdlog::warn("Unsupported number of channels encountered");
        // TODO(lamarrr): fill with silence and return?
      }
      break;
    }

    // TODO(lamarrr): vital bug: the frame size will not be equal to the buffer size
    if (This->resampler_cfg != target_cfg)
    {
      if (This->resampler != nullptr)
      {
        swr_free(&This->resampler);
        This->resampler = nullptr;
      }

      ASH_CHECK(swr_alloc_set_opts2(&This->resampler, &target_cfg.channel_layout,
                                    target_cfg.dst_fmt,
                                    target_cfg.dst_sample_rate, &This->decoder->decoder.frame->ch_layout,
                                    target_cfg.src_fmt, target_cfg.src_sample_rate, 0, nullptr) == 0);
      ASH_CHECK(swr_init(This->resampler) == 0);
      // av_get_sample_fmt_name((AVSampleFormat) frame->format),
      // av_get_sample_fmt_name(target_format));
    }

    // av_samples_get_buffer_size

    ASH_CHECK(swr_convert(This->resampler, &stream, This->info.spec.samples, &This->decoder->decoder.frame->data[0], This->decoder->decoder.frame->nb_samples) == 0);

    av_frame_unref(This->decoder->decoder.frame);

    // if (volume == 0)
    // {
    //   std::fill(stream, stream + len, 0);
    //   return;
    // }
  }

  void play()
  {
    int err = SDL_PlayAudioDevice(id);
    ASH_CHECK(err == 0);
  }

  void pause()
  {
    int err = SDL_PauseAudioDevice(id);
    ASH_CHECK(err == 0);
  }

  static stx::Option<stx::Rc<AudioDevice *>> open(AudioDeviceInfo const &info, u8 nchannels)
  {
    stx::Rc       dev = stx::rc::make_inplace<AudioDevice>(stx::os_allocator).unwrap();
    SDL_AudioSpec desired_spec{info.spec};
    desired_spec.userdata = dev.handle;
    desired_spec.callback = audio_callback;
    desired_spec.channels = nchannels;

    switch (desired_spec.format)
    {
      case AUDIO_U8:
      case AUDIO_S16SYS:
      case AUDIO_S32SYS:
      case AUDIO_F32SYS:
        break;
      default:
      {
        desired_spec.format = AUDIO_S16SYS;
      }
      break;
    }

    // desired_spec.size is modified to the hardware buffer size
    u32 id = SDL_OpenAudioDevice(dev->info.name.c_str(), 0, &desired_spec, &dev->info.spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_SAMPLES_CHANGE);
    if (id == 0)
    {
      return stx::None;
    }
    dev->info.name = info.name.copy(stx::os_allocator).unwrap();
    dev->id        = id;
    // TODO(lamarrr): dev->decoder;
    return stx::Some(std::move(dev));
  }

  void close()
  {
    SDL_CloseAudioDevice(id);
  }
};

//   void dump_info()
//   {
//     void          *iter  = nullptr;
//     AVCodec const *codec = nullptr;
//     uint version = avformat_version();
//     spdlog::info("ffmpeg avformat version: {}.{}.{}", AV_VERSION_MAJOR(version), AV_VERSION_MINOR(version),
//                  AV_VERSION_MICRO(version));
//     do
//     {
//       codec = av_codec_iterate(&iter);
//       if (codec != nullptr)
//       {
//         spdlog::info("name: {}, long name: {}", codec->name, codec->long_name);
//       }
//     } while (codec != nullptr);
//   }
/*
Design of Video Widget and System



Video Widget
gets: video source
performs:
  - widget resizing
  - seeking, playing, pausing, volume adjustment


Video Stream
performs demuxing, stream selection, and decoding
all this is performed on a separate dedicated worker thread
streams audio to AudioDevice
streams pictures to Vulkan Backend
it will have as many images as the number of frames so we don't block the vulkan backend or cause data races whilst writing to the images ???? might not be possible?
sdl audio device requires a callback so it can request for audio frames whenever and that would mean we'd have to use the silence value of the sdl spec when we don't have audio samples available

subtitle
*/
int main(int argc, char **argv)
{
  ASH_CHECK(argc == 3);
  // TODO(lamarrr): refactor this out of window api
  ASH_CHECK(SDL_Init(SDL_INIT_EVERYTHING) == 0);

  spdlog::info("System theme: {}", (int) SDL_GetSystemTheme());

  stx::Vec devices = AudioDeviceInfo::enumerate();
  for (AudioDeviceInfo const &dev : devices)
  {
    spdlog::info("name: {}, channels: {}, format: {}, samplerate: {}, nsamples: {}",
                 dev.name.c_str(),
                 dev.spec.channels, dev.spec.format, dev.spec.freq, dev.spec.samples);
  }

  AudioDeviceInfo dev = AudioDeviceInfo::get_default().unwrap();
  spdlog::info("default device: {}, channels: {}, format: {}, samplerate: {}, nsamples: {}", dev.name.c_str(), dev.spec.channels, dev.spec.format,
               dev.spec.freq, dev.spec.samples);

  // fmt_ctx->ctx_flags       = AVFMT_FLAG_CUSTOM_IO;
  // fmt_ctx->chapters;
  // fmt_ctx->metadata;
  // spdlog::info("video codec name: {}, long name: {}", video_codec->name, video_codec->long_name);
  // av_dump_format(fmt_ctx, video_stream_index,)
  // AV_DISPOSITION_ATTACHED_PIC contains album art
  //

  AVFrame *frame = av_frame_alloc();
  ASH_CHECK(frame != nullptr);

  AVPacket *packet = av_packet_alloc();
  ASH_CHECK(packet != nullptr);

  bool done = false;

  SwsContext *context = sws_getContext(frame->width, frame->height, (AVPixelFormat) frame->format, frame->width,
                                       frame->height, AV_PIX_FMT_RGB24, 0, 0, 0, 0);
  // sws_getCachedContext() - instead of reallocating, check if it can be reused
  u8 *rgb       = new u8[frame->width * frame->height * 3];
  u8 *planes[]  = {rgb};
  int strides[] = {3 * frame->width};
  sws_scale(context, frame->data, frame->linesize, 0, frame->height, planes, strides);

  sws_freeContext(context);

  // TODO(lamarrr): does this mean it could be multi-threaded? and another
  // thread could be using it?
  av_frame_unref(frame);

  //   // spdlog::info("requesting audio samples from audio frame");
  //   err = avcodec_receive_frame(audio_codec_ctx, frame);
  //   // spdlog::info("got audio samples");

  //   if (err < 0)
  //   {
  //     if (err == AVERROR_EOF)
  //     {
  //       // spdlog::info("eof");
  //       done = true;
  //       break;
  //     }
  //     else if (err == AVERROR(EAGAIN))
  //     {
  //       // spdlog::info("finished audio samples from audio frame");
  //       break;
  //     }
  //     else
  //     {
  //       // spdlog::error("got error: {}", (int) err);
  //       return -1;
  //     }
  //   }
  // }

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
