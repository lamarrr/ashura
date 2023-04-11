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

constexpr u8          MIN_VOLUME      = 0;
constexpr u8          MAX_VOLUME      = 255;
constexpr nanoseconds SYNC_THRESHOLD  = milliseconds{16};
constexpr nanoseconds MAX_FRAME_DELAY = seconds{1};

#define ASH_LOG_FFMPEG_ERR(err_exp)                                                                                           \
  do                                                                                                                          \
  {                                                                                                                           \
    char ASH_LOG_FFMPEG_ERR_error_buffer[256];                                                                                \
    int  ASH_LOG_FFMPEG_ERR_error = (err_exp);                                                                                \
    if (av_strerror(ASH_LOG_FFMPEG_ERR_error, ASH_LOG_FFMPEG_ERR_error_buffer, sizeof(ASH_LOG_FFMPEG_ERR_error_buffer)) == 0) \
    {                                                                                                                         \
      ASH_LOG_ERR(MediaPlayer, "FFMPEG returned error: {}={}", ASH_LOG_FFMPEG_ERR_error, ASH_LOG_FFMPEG_ERR_error_buffer);    \
    }                                                                                                                         \
    else                                                                                                                      \
    {                                                                                                                         \
      ASH_LOG_ERR(MediaPlayer, "FFMPEG returned error: {}", ASH_LOG_FFMPEG_ERR_error);                                        \
    }                                                                                                                         \
  } while (0)

constexpr void fill_silence(stx::Span<u8> samples, SDL_AudioFormat format)
{
  switch (format)
  {
    case AUDIO_U8:
      samples.fill(128);
      break;

    case AUDIO_S8:
    case AUDIO_S16LSB:
    case AUDIO_S16MSB:
    case AUDIO_S32LSB:
    case AUDIO_S32MSB:
    case AUDIO_F32LSB:
    case AUDIO_F32MSB:
      samples.fill(0);
      break;

    default:
      // guessing
      samples.fill(0);
      break;
  }
}

constexpr nanoseconds timebase_to_ns(AVRational timebase)
{
  return nanoseconds{AS(nanoseconds::rep, 1'000'000'000LL * AS(f32, timebase.num) / AS(f32, timebase.den))};
}

// Demuxer runs on main thread, fetches raw streams/packets from the files and dispatches them to the decoder threads
//
// the audio/video decode thread decodes audio/video frames, performs conversions/resampling and sends them to the renderer/audio device
//
struct DecodeContext
{
  stx::SpinLock        lock;
  AVCodecContext      *ctx    = nullptr;
  AVStream            *stream = nullptr;
  AVFrame             *frame  = nullptr;
  stx::Vec<AVPacket *> packets{stx::os_allocator};
  // TODO(lamarrr): add a flush packet and/or is_completed marker

  DecodeContext(AVCodecContext *ictx, AVStream *istream, AVFrame *iframe) :
      ctx{ictx}, stream{istream}, frame{iframe}
  {}

  STX_MAKE_PINNED(DecodeContext)

  ~DecodeContext()
  {
    avcodec_free_context(&ctx);
    av_frame_free(&frame);
    for (AVPacket *packet : packets)
    {
      av_packet_free(&packet);
    }
  }
};

enum class DemuxError : u8
{
  InvalidPath,
  StreamNotFound,
  CodecNotSupported
};

struct VideoDemuxer
{
  static constexpr int AVIO_BUFFER_SIZE = 4096;
  AVIOContext         *io_ctx           = nullptr;
  AVFormatContext     *fmt_ctx          = nullptr;
  AVPacket            *packet           = nullptr;
  FILE                *file             = nullptr;

  VideoDemuxer(AVIOContext *iio_ctx, AVFormatContext *ictx, AVPacket *ipacket, FILE *ifile) :
      io_ctx{iio_ctx}, fmt_ctx{ictx}, packet{ipacket}, file{ifile}
  {}

  STX_MAKE_PINNED(VideoDemuxer)

  ~VideoDemuxer()
  {
    fclose(file);
    avformat_close_input(&fmt_ctx);
    avio_context_free(&io_ctx);
    av_packet_free(&packet);
  }

  static int packet_file_read_callback(void *opaque, u8 *buffer, int buffer_size)
  {
    // AVERROR(EAGAIN);
    std::FILE *file = AS(VideoDemuxer *, opaque)->file;

    usize read = std::fread(buffer, 1, buffer_size, file);

    if (std::ferror(file) != 0)
    {
      return AVERROR_UNKNOWN;
    }

    if (read == 0)
    {
      ASH_CHECK(std::feof(file));
      return AVERROR_EOF;
    }

    return AS(int, read);
  }

  static i64 packet_file_seek_callback(void *opaque, i64 offset, int whence)
  {
    std::FILE *file = AS(VideoDemuxer *, opaque)->file;
    if (whence == SEEK_SET || whence == SEEK_CUR || whence == SEEK_END)
    {
      int error = std::fseek(file, offset, whence);
      if (error == 0)
      {
        return std::ftell(file);
      }
      else
      {
        return -1;
      }
    }
    else if (whence == AVSEEK_SIZE)
    {
      // return -1 if not supported or info can't be determined
      long pos   = std::ftell(file);
      int  error = std::fseek(file, 0, SEEK_END);
      if (error != 0)
      {
        return -1;
      }
      long size = std::ftell(file);
      error     = std::fseek(file, pos, SEEK_SET);
      if (error != 0)
      {
        return -1;
      }
      return size;
    }
    else
    {
      return -1;
    }
  }

  static stx::Result<stx::Rc<VideoDemuxer *>, DemuxError> from_file(stx::CStringView path)
  {
    if (!std::filesystem::exists(path.c_str()))
    {
      return stx::Err(DemuxError::InvalidPath);
    }

    std::FILE *file = std::fopen(path.c_str(), "rb");
    ASH_CHECK(file != nullptr);

    void *avio_buffer = av_malloc(AVIO_BUFFER_SIZE);        // TODO(lamarrr): this memory is presently being leaked
    ASH_CHECK(avio_buffer != nullptr);

    stx::Rc demuxer = stx::rc::make_inplace<VideoDemuxer>(stx::os_allocator, nullptr, nullptr, nullptr, nullptr).unwrap();

    AVIOContext *io_ctx = avio_alloc_context(AS(uchar *, avio_buffer), AVIO_BUFFER_SIZE, 0, demuxer.handle, packet_file_read_callback, nullptr, packet_file_seek_callback /* can be null if seeking not supported, TODO(lamarrr): check if av_seek_frame requires this*/);
    ASH_CHECK(io_ctx != nullptr);

    AVFormatContext *fmt_ctx = avformat_alloc_context();
    ASH_CHECK(fmt_ctx != nullptr);
    fmt_ctx->pb = io_ctx;

    AVPacket *packet = av_packet_alloc();
    ASH_CHECK(packet != nullptr);

    demuxer->file    = file;
    demuxer->io_ctx  = io_ctx;
    demuxer->fmt_ctx = fmt_ctx;
    demuxer->packet  = packet;

    ASH_CHECK(avformat_open_input(&fmt_ctx, nullptr, nullptr, nullptr) >= 0);

    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0)
    {
      return stx::Err(DemuxError::StreamNotFound);
    }

    AVDictionaryEntry *prev = nullptr;
    do
    {
      prev = av_dict_get(fmt_ctx->metadata, "", prev, AV_DICT_IGNORE_SUFFIX);
      if (prev != nullptr)
      {
        spdlog::info("metadata: {}={}", prev->key, prev->value);
      }
    } while (prev != nullptr);

    return stx::Ok(std::move(demuxer));
  }

  stx::Result<stx::Rc<DecodeContext *>, DemuxError> make_decoder(AVMediaType media_type) const
  {
    int stream_index = av_find_best_stream(fmt_ctx, media_type, -1, -1, nullptr, 0);

    if (stream_index < 0)
    {
      return stx::Err(DemuxError::StreamNotFound);
    }

    AVStream *stream = fmt_ctx->streams[stream_index];

    if (stream == nullptr)
    {
      return stx::Err(DemuxError::StreamNotFound);
    }

    AVCodec const *codec = avcodec_find_decoder(stream->codecpar->codec_id);

    if (codec == nullptr)
    {
      return stx::Err(DemuxError::CodecNotSupported);
    }

    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);

    ASH_CHECK(codec_ctx != nullptr);
    ASH_CHECK(avcodec_parameters_to_context(codec_ctx, stream->codecpar) >= 0);
    ASH_CHECK(avcodec_open2(codec_ctx, codec, nullptr) >= 0);

    AVFrame *frame = av_frame_alloc();
    ASH_CHECK(frame != nullptr);

    AVDictionaryEntry *prev = nullptr;
    do
    {
      prev = av_dict_get(stream->metadata, "", prev, AV_DICT_IGNORE_SUFFIX);
      if (prev != nullptr)
      {
        spdlog::info("metadata: {}={}", prev->key, prev->value);
      }
    } while (prev != nullptr);

    return stx::Ok(stx::rc::make_inplace<DecodeContext>(stx::os_allocator, codec_ctx, stream, frame).unwrap());
  }

  stx::Result<stx::Rc<DecodeContext *>, DemuxError> make_video_decoder() const
  {
    return make_decoder(AVMEDIA_TYPE_VIDEO);
  }

  stx::Result<stx::Rc<DecodeContext *>, DemuxError> make_audio_decoder() const
  {
    return make_decoder(AVMEDIA_TYPE_AUDIO);
  }

  stx::Result<stx::Rc<DecodeContext *>, DemuxError> make_subtitle_decoder() const
  {
    return make_decoder(AVMEDIA_TYPE_SUBTITLE);
  }
};

struct VideoFrame
{
  ash::extent extent;
  u8         *pixels = nullptr;
  nanoseconds pts{0};

  VideoFrame()
  {}

  STX_MAKE_PINNED(VideoFrame)

  ~VideoFrame()
  {
    av_freep(pixels);
  }

  void fit(ash::extent new_extent)
  {
    if (new_extent != extent)
    {
      if (pixels != nullptr)
      {
        av_freep(pixels);
        pixels = nullptr;
      }
      int linesizes[4] = {AS(int, new_extent.area()) * 3, 0, 0, 0};
      u8 *planes[4]    = {nullptr, nullptr, nullptr, nullptr};

      int nbytes = av_image_alloc(planes, linesizes, new_extent.width, new_extent.height, AV_PIX_FMT_RGB24, 1);
      ASH_CHECK(nbytes >= 0);
      pixels = planes[0];
      extent = new_extent;
    }
  }
};

struct VideoDecodeContext
{
  VideoFrame    frame;
  stx::SpinLock lock;        // locks the frame
  SwsContext   *rescaler = nullptr;
  nanoseconds   timebase{0};
  nanoseconds   last_frame_pts{0};
  nanoseconds   last_frame_pts_interval{0};
  nanoseconds   frame_timer{0};
  timepoint     begin_timepoint;

  VideoDecodeContext(timepoint ibegin_timepoint, nanoseconds itimebase) :
      begin_timepoint{ibegin_timepoint}, timebase{itimebase}
  {}

  STX_MAKE_PINNED(VideoDecodeContext)

  ~VideoDecodeContext()
  {
    sws_freeContext(rescaler);
  }

  void store_frame(AVFrame const *src)
  {
    ASH_CHECK(src->pts != AV_NOPTS_VALUE);
    frame.pts = timebase * src->pts;
    rescaler  = sws_getCachedContext(rescaler, src->width, src->height, AS(AVPixelFormat, src->format), src->width, src->height, AV_PIX_FMT_RGB24, 0, nullptr, nullptr, nullptr);
    ASH_CHECK(rescaler != nullptr);

    lock.lock();

    frame.fit(extent{AS(u32, src->width), AS(u32, src->height)});

    u8 *planes[4]  = {frame.pixels, nullptr, nullptr, nullptr};
    int strides[4] = {src->width * 3, 0, 0, 0};

    sws_scale(rescaler, src->data, src->linesize, 0, src->height, planes, strides);

    lock.unlock();
  }

  nanoseconds refresh(nanoseconds audio_pts, timepoint current_timepoint)
  {
    nanoseconds pts_interval = frame.pts - last_frame_pts;

    if (pts_interval <= nanoseconds{0} || pts_interval >= MAX_FRAME_DELAY)
    {
      // means delay is incorrect, we thus guess the frame delay by using the previous one
      pts_interval = last_frame_pts_interval;
    }

    last_frame_pts          = frame.pts;
    last_frame_pts_interval = pts_interval;

    nanoseconds diff           = frame.pts - audio_pts;                                                // time difference between present audio and video frames
    nanoseconds sync_threshold = pts_interval > SYNC_THRESHOLD ? pts_interval : SYNC_THRESHOLD;        // skip or repeat the frame. Take delay into account we still doesn't "know if this is the best guess."
    nanoseconds delay          = pts_interval;

    if (std::chrono::abs(diff) < seconds{10})
    {
      if (diff <= -sync_threshold)        // video frame is lagging behind audio frame, speed up
      {
        delay = nanoseconds{0};
      }
      else if (diff >= sync_threshold)        // audio frame is lagging behind video frame, slow down
      {
        delay = 2 * delay;
      }
    }

    frame_timer += delay;

    // now sync to actual clock
    nanoseconds time_passed  = current_timepoint - begin_timepoint;
    nanoseconds actual_delay = frame_timer - time_passed;        // time remaining

    // really skip instead
    if (actual_delay < SYNC_THRESHOLD)
    {
      actual_delay = SYNC_THRESHOLD;
    }

    return actual_delay;
  }
};

struct ResamplerConfig
{
  AVSampleFormat  src_fmt            = AV_SAMPLE_FMT_NONE;
  AVSampleFormat  dst_fmt            = AV_SAMPLE_FMT_NONE;
  int             src_sample_rate    = 0;
  int             dst_sample_rate    = 0;
  AVChannelLayout src_channel_layout = AV_CHANNEL_LAYOUT_MONO;
  AVChannelLayout dst_channel_layout = AV_CHANNEL_LAYOUT_MONO;

  bool operator==(ResamplerConfig const &other) const
  {
    return src_fmt == other.src_fmt && dst_fmt == other.dst_fmt && src_sample_rate == other.src_sample_rate && dst_sample_rate == other.dst_sample_rate &&
           (av_channel_layout_compare(&src_channel_layout, &other.src_channel_layout) == 0) && (av_channel_layout_compare(&dst_channel_layout, &other.dst_channel_layout) == 0);
  }

  bool operator!=(ResamplerConfig const &other) const
  {
    return !(*this == other);
  }
};

struct AudioDecodeContext
{
  std::atomic<nanoseconds::rep> clock{0};
  stx::Vec<u8>                  samples{stx::os_allocator};
  usize                         bytes_consumed = 0;        // portion of samples consumed
  SwrContext                   *resampler      = nullptr;
  ResamplerConfig               resampler_cfg;

  AudioDecodeContext(SwrContext *iresampler, ResamplerConfig iresampler_cfg) :
      resampler{iresampler}, resampler_cfg{iresampler_cfg}
  {}

  STX_MAKE_PINNED(AudioDecodeContext)

  ~AudioDecodeContext()
  {
    swr_free(&resampler);
  }
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

      devices.push(AudioDeviceInfo{.name = stx::string::make(stx::os_allocator, device_name).unwrap(), .spec = spec}).unwrap();
    }

    return devices;
  }

  static stx::Option<AudioDeviceInfo> get_default()
  {
    char         *name = nullptr;
    SDL_AudioSpec spec;
    int           err = SDL_GetDefaultAudioInfo(&name, &spec, false);

    if (err != 0)
    {
      return stx::None;
    }

    AudioDeviceInfo info{.name = name == nullptr ? stx::string::make_static("") : stx::string::make(stx::os_allocator, name).unwrap(), .spec = spec};

    SDL_free(name);

    return stx::Some(std::move(info));
  }
};

struct AudioSource
{
  stx::Unique<stx::Fn<bool(stx::Span<u8>, SDL_AudioSpec)>> callback;
  bool                                                     is_open = true;
};

enum class AudioDeviceError : u8
{
  UnsupportedChannelCount,
  UnsupporedSpec
};

struct AudioDevice
{
  SDL_AudioDeviceID     id = 0;
  AudioDeviceInfo       info;
  stx::SpinLock         lock;
  stx::Vec<AudioSource> audio_sources{stx::os_allocator};

  AudioDevice(SDL_AudioDeviceID iid, AudioDeviceInfo iinfo) :
      id{iid}, info{std::move(iinfo)}
  {}

  STX_MAKE_PINNED(AudioDevice)

  ~AudioDevice()
  {
    SDL_CloseAudioDevice(id);
  }

  void add_source(stx::Unique<stx::Fn<bool(stx::Span<u8>, SDL_AudioSpec)>> callback)
  {
    lock.lock();
    audio_sources.push(AudioSource{.callback = std::move(callback), .is_open = true}).unwrap();
    lock.unlock();
  }

  // will be called on a different thread, use SDL_LockAudioDevice and SDL_UnlockAudioDevice to prevent this callback from running
  static void audio_callback(void *userdata, u8 *pstream, int len)
  {
    AudioDevice *This = AS(AudioDevice *, userdata);
    stx::Span    stream{pstream, AS(usize, len)};

    fill_silence(stream, This->info.spec.format);

    This->lock.lock();
    for (AudioSource &src : This->audio_sources)
    {
      src.is_open = src.callback.handle(stream, This->info.spec);
    }

    auto [open, closed] = This->audio_sources.span().partition([](AudioSource const &src) { return src.is_open; });
    (void) open;
    if (!closed.is_empty())
    {
      ASH_LOG_INFO(MediaPlayer, "Closing {} audio sources", closed.size());
    }
    This->audio_sources.erase(closed);

    This->lock.unlock();
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
    stx::Rc dev = stx::rc::make_inplace<AudioDevice>(stx::os_allocator, AS(SDL_AudioDeviceID, 0), AudioDeviceInfo{}).unwrap();

    SDL_AudioSpec desired_spec;
    desired_spec.freq     = info.spec.freq;
    desired_spec.format   = info.spec.format;
    desired_spec.channels = nchannels;
    desired_spec.samples  = info.spec.samples;
    desired_spec.size     = 0;
    desired_spec.callback = audio_callback;
    desired_spec.userdata = dev.handle;

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

    // TODO(lamarrr): remove this
    // switch (desired_spec.channels)
    // {
    //   case 1:
    //   case 2:
    //   case 4:
    //   case 6:
    //   case 8:
    //   case 16:
    //     break;
    //   default:
    //   {
    //     desired_spec.channels = 2;
    //   }
    //   break;
    // }

    // desired_spec.size is modified to the hardware buffer size
    u32 id = SDL_OpenAudioDevice(info.name.c_str(), 0, &desired_spec, &dev->info.spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_SAMPLES_CHANGE);

    if (id == 0)
    {
      return stx::None;
    }

    dev->info.name = info.name.copy(stx::os_allocator).unwrap();
    dev->id        = id;

    return stx::Some(std::move(dev));
  }
};

struct MediaPlayerAudioContext
{
  stx::Promise<void>       promise;
  stx::Rc<DecodeContext *> ctx;
  AudioDecodeContext       decode_ctx;
  std::atomic<u8>          volume = MAX_VOLUME;

  MediaPlayerAudioContext(stx::Promise<void> ipromise, stx::Rc<DecodeContext *> ictx) :
      promise{std::move(ipromise)}, ctx{std::move(ictx)}, decode_ctx{nullptr, {}}
  {}

  bool write_samples(stx::Span<u8> stream, SDL_AudioSpec spec)
  {
    int volume                     = AS(int, this->volume.load(std::memory_order_relaxed));
    volume                         = (volume * 128) / 255;
    usize           bytes_written  = 0;
    AVSampleFormat  sample_fmt     = AV_SAMPLE_FMT_NONE;
    AVChannelLayout channel_layout = AV_CHANNEL_LAYOUT_MONO;
    bool            is_open        = true;
    int             error          = 0;
    nanoseconds     clock{0};

    switch (spec.format)
    {
      case AUDIO_U8:
      {
        sample_fmt = AV_SAMPLE_FMT_U8;
      }
      break;
      case AUDIO_S16SYS:
      {
        sample_fmt = AV_SAMPLE_FMT_S16;
      }
      break;
      case AUDIO_S32SYS:
      {
        sample_fmt = AV_SAMPLE_FMT_S32;
      }
      break;
      case AUDIO_F32SYS:
      {
        sample_fmt = AV_SAMPLE_FMT_FLT;
      }
      break;
      default:
      {
        ASH_LOG_ERR(MediaPlayer, "encountered unsupported SDL Device audio format: {}", AS(int, spec.format));
        is_open = false;
      }
      break;
    }

    switch (spec.channels)
    {
      case 1:
      {
        channel_layout = AV_CHANNEL_LAYOUT_MONO;
      }
      break;
      case 2:
      {
        channel_layout = AV_CHANNEL_LAYOUT_STEREO;
      }
      break;
      case 4:
      {
        channel_layout = AV_CHANNEL_LAYOUT_QUAD;
      }
      break;
      case 6:
      {
        channel_layout = AV_CHANNEL_LAYOUT_HEXAGONAL;
      }
      break;
      case 8:
      {
        channel_layout = AV_CHANNEL_LAYOUT_OCTAGONAL;
      }
      break;
      case 16:
      {
        channel_layout = AV_CHANNEL_LAYOUT_HEXADECAGONAL;
      }
      break;
      default:
      {
        ASH_LOG_ERR(MediaPlayer, "encountered unsupported number of channels: {}", spec.channels);
        is_open = false;
      }
      break;
    }

    while (is_open && bytes_written < stream.size() && promise.fetch_cancel_request() != stx::CancelState::Canceled)
    {
      if (decode_ctx.bytes_consumed != decode_ctx.samples.size())
      {
        usize bytes_left     = stream.size() - bytes_written;
        usize bytes_to_write = std::min(bytes_left, AS(usize, decode_ctx.samples.size() - decode_ctx.bytes_consumed));

        ASH_SDL_CHECK(SDL_MixAudioFormat(stream.data() + bytes_written, decode_ctx.samples.data() + decode_ctx.bytes_consumed, spec.format, bytes_to_write, volume) == 0);
        bytes_written += bytes_to_write;
        decode_ctx.bytes_consumed += bytes_to_write;

        usize nsamples_written = (bytes_to_write / spec.channels) / av_get_bytes_per_sample(sample_fmt);

        clock = nanoseconds{decode_ctx.clock.load(std::memory_order_relaxed)};
        clock += nanoseconds{AS(nanoseconds::rep, 1'000'000'000LL * AS(f32, nsamples_written) / AS(f32, spec.freq))};
      }
      else
      {
        ctx->lock.lock();
        if (ctx->packets.is_empty())
        {
          ctx->lock.unlock();
          break;
        }

        AVPacket *packet = ctx->packets[0];
        ctx->packets.erase(ctx->packets.span().slice(0, 1));
        ctx->lock.unlock();

        if (packet->pts != AV_NOPTS_VALUE)
        {
          clock = timebase_to_ns(ctx->stream->time_base) * packet->pts;
        }

        error = avcodec_send_packet(ctx->ctx, packet);

        av_packet_free(&packet);

        if (error != 0)
        {
          is_open = false;
          if (error != AVERROR(EOF))
          {
            ASH_LOG_FFMPEG_ERR(error);
          }
          break;
        }

        error = avcodec_receive_frame(ctx->ctx, ctx->frame);

        if (error != 0)
        {
          if (error == AVERROR(EAGAIN))
          {
            continue;
          }
          else
          {
            is_open = false;
            ASH_LOG_FFMPEG_ERR(error);
            break;
          }
        }

        ResamplerConfig target_cfg{
            .src_fmt            = AS(AVSampleFormat, ctx->frame->format),
            .dst_fmt            = sample_fmt,
            .src_sample_rate    = ctx->frame->sample_rate,
            .dst_sample_rate    = spec.freq,
            .src_channel_layout = ctx->frame->ch_layout,
            .dst_channel_layout = channel_layout};

        if (decode_ctx.resampler_cfg != target_cfg || decode_ctx.resampler == nullptr)
        {
          if (decode_ctx.resampler != nullptr)
          {
            swr_free(&decode_ctx.resampler);
            decode_ctx.resampler = nullptr;
          }

          error = swr_alloc_set_opts2(&decode_ctx.resampler, &target_cfg.dst_channel_layout, target_cfg.dst_fmt, target_cfg.dst_sample_rate,
                                      &ctx->frame->ch_layout, target_cfg.src_fmt, target_cfg.src_sample_rate, 0, nullptr);

          if (error != 0)
          {
            ASH_LOG_FFMPEG_ERR(error);
            break;
          }

          decode_ctx.resampler_cfg = target_cfg;

          error = swr_init(decode_ctx.resampler);

          if (error != 0)
          {
            ASH_LOG_FFMPEG_ERR(error);
            break;
          }
        }

        int max_nsamples = swr_get_out_samples(decode_ctx.resampler, ctx->frame->nb_samples);

        if (max_nsamples < 0)
        {
          error = max_nsamples;
          ASH_LOG_FFMPEG_ERR(error);
          break;
        }

        int max_buffer_size = av_samples_get_buffer_size(nullptr, spec.channels, max_nsamples, target_cfg.dst_fmt, 1);

        if (max_buffer_size < 0)
        {
          error = max_buffer_size;
          ASH_LOG_FFMPEG_ERR(error);
          break;
        }

        decode_ctx.samples.resize(max_buffer_size).unwrap();

        u8 *out = decode_ctx.samples.data();

        int nsamples = swr_convert(decode_ctx.resampler, &out, max_nsamples, (u8 const **) ctx->frame->data, ctx->frame->nb_samples);

        av_frame_unref(ctx->frame);

        if (nsamples < 0)
        {
          error = nsamples;
          ASH_LOG_FFMPEG_ERR(error);
          break;
        }

        int buffer_size = av_samples_get_buffer_size(nullptr, spec.channels, nsamples, target_cfg.dst_fmt, 1);

        if (buffer_size < 0)
        {
          error = buffer_size;
          ASH_LOG_FFMPEG_ERR(error);
          break;
        }

        decode_ctx.samples.resize(buffer_size).unwrap();

        usize bytes_left     = stream.size() - bytes_written;
        usize bytes_to_write = std::min(bytes_left, AS(usize, buffer_size));

        ASH_SDL_CHECK(SDL_MixAudioFormat(stream.data() + bytes_written, decode_ctx.samples.data(), spec.format, bytes_to_write, volume) == 0);

        decode_ctx.bytes_consumed = bytes_to_write;

        bytes_written += bytes_to_write;

        usize nsamples_written = (bytes_to_write / spec.channels) / av_get_bytes_per_sample(target_cfg.dst_fmt);

        clock += nanoseconds{AS(nanoseconds::rep, 1'000'000'000LL * AS(f32, nsamples_written) / AS(f32, spec.freq))};
      }
    }

    decode_ctx.clock.store(clock.count(), std::memory_order_relaxed);

    if (promise.fetch_cancel_request() == stx::CancelState::Canceled)
    {
      promise.notify_canceled();
      is_open = false;
    }
    else if (!is_open)
    {
      // TODO(lamarrr): use frame seeking and packets to check if completed actually
      promise.notify_completed();
    }

    return is_open;
  }
};

//
// Design of Video Widget and System
//
// Video Widget
// gets: video source
// performs:
//   - widget resizing
//   - seeking, playing, pausing, volume adjustment
//
// Video Stream
// performs demuxing, stream selection, and decoding
// all this is performed on a separate dedicated worker thread
// streams audio to AudioDevice
// streams pictures to Vulkan Backend
// it will have as many images as the number of frames so we don't block the vulkan backend or cause data races whilst writing to the images ???? might not be possible?
// sdl audio device requires a callback so it can request for audio frames whenever and that would mean we'd have to use the silence value of the sdl spec when we don't have audio samples available
//
// subtitle
//

enum class MediaSessionError : u8
{
  Loading
};

enum class SeekType : u8
{
  Exact,
  Forward,
  Backward
};

enum class MediaType : u8
{
  Audio,
  Video
};

using media_session = u64;

// TODO(lamarrr): make loading audio/video optional
struct MediaSession
{
  stx::Rc<VideoDemuxer *>                    demuxer;
  stx::Option<stx::Rc<DecodeContext *>>      audio_decode_ctx;
  stx::Option<stx::Rc<DecodeContext *>>      video_decode_ctx;
  stx::Option<stx::Rc<VideoDecodeContext *>> video_decode_ctx2;
  gfx::image                                 image = 0;
};

struct Lyrics
{
  stx::Vec<std::string_view> content{stx::os_allocator};
  stx::String                data;
};

struct AudioMetaData
{
  stx::String title;
  stx::String artist;
  stx::String album;
  stx::String date;
  Lyrics      lyrics;
};

struct MediaVideoFrame
{
  gfx::image  image = gfx::WHITE_IMAGE;
  ash::extent extent;
};

// TODO(lamarrr): handle audio device defaulting and updating
//
// REQUIRED FEATURES:
//
// - Play, Pause, Seek, Stop, Volume Setting
// - Subtitles with selection (based on language)
// - Timelines/Video Previews
// - Audio with selection (based on language)
// - Timestamps
// - Album Art
// - Lyrics
// - ID3 tag extraction
//
// TODO(lamarrr): main thread should be used for demuxing and another thread for decoding
//
struct MediaPlayer : public Plugin
{
  template <typename T>
  using Result = stx::Result<T, MediaSessionError>;

  static void dump_supported_codecs()
  {
    ASH_LOG_INFO(MediaPlayer, "Querying Available Codecs");
    void          *iter  = nullptr;
    AVCodec const *codec = nullptr;
    do
    {
      codec = av_codec_iterate(&iter);
      if (codec != nullptr)
      {
        ASH_LOG_INFO(MediaPlayer, "codec -> name: {}, long name: {}, media type: {}", codec->name, codec->long_name, codec->type);
      }
    } while (codec != nullptr);
  }

  virtual void on_startup(Context &context)
  {
    task_scheduler = context.task_scheduler;
    // open the audio device, handle audio device changing
    image_manager = context.get_plugin<ImageManager>("ImageManager").unwrap();
    dump_supported_codecs();
  }

  virtual void tick(Context &context, std::chrono::nanoseconds interval)
  {}

  virtual void on_exit(Context &context)
  {}

  virtual std::string_view get_name()
  {
    return "MediaPlayer";
  }

  stx::Result<media_session, DemuxError> create_session(stx::CStringView source)
  {
    // TODO(lamarrr): do not do demuxing or stream seeking on main thread, it is not clear how long that will take
    media_session session_id = next_session_id;
    TRY_OK(demuxer, VideoDemuxer::from_file(source));
    next_session_id++;
    // sessions.emplace(session_id, MediaSession{});
    // preferably don't use locks on the data? at leas the video session
    return stx::Ok(AS(media_session, session_id));
  }

  Result<bool> is_playing(media_session session);

  Result<bool> is_play_ended(media_session session);

  Result<bool> has_audio(media_session session);

  Result<bool> has_video(media_session session);

  // TODO(lamarrr): we might not be able to tell yet as this is async
  Result<stx::Vec<usize>> get_audio_streams(media_session session)
  {
    auto pos = sessions.find(session);
    ASH_CHECK(pos != sessions.end());

    pos->second.demuxer->fmt_ctx->streams[0]->metadata;
    pos->second.demuxer->fmt_ctx->nb_streams;
  }

  Result<stx::Vec<usize>> get_subtitles(media_session session);

  Result<stx::Vec<usize>> get_chapters(media_session session);

  Result<nanoseconds> get_duration(media_session session);

  Result<nanoseconds> get_current_time(media_session session);

  Result<usize> get_current_frame(media_session session);

  Result<MediaVideoFrame> get_image(media_session session);

  Result<MediaVideoFrame> get_preview_image(media_session session);

  Result<MediaVideoFrame> get_subtitle_image(media_session session);

  Result<MediaVideoFrame> get_album_art(media_session session);

  Result<Lyrics> get_lyrics(media_session session);

  Result<stx::Void> play(media_session session, usize video_stream, usize audio_stream);

  Result<stx::Void> pause(media_session session);

  Result<stx::Void> stop(media_session session);

  Result<stx::Void> seek_time(media_session session, nanoseconds timepoint, SeekType seek = SeekType::Exact);

  Result<stx::Void> seek_frame(media_session session, usize frame, SeekType seek = SeekType::Exact);

  Result<stx::Void> seek_preview_at_time(media_session session, nanoseconds timepoint);

  Result<stx::Void> seek_preview_at_frame(media_session session, usize frame);

  Result<stx::Void> set_volume(media_session session, u8 volume);

  Result<stx::Void> set_autoplay(media_session session, bool autoplay);

  Result<stx::Void> change_subtitle(media_session session, usize);

  Result<stx::Void> change_audio(media_session session, usize);

  u64                                   next_session_id = 0;
  std::map<media_session, MediaSession> sessions;
  stx::Option<stx::Future<void>>        demuxer_promise;
  stx::Option<stx::Future<void>>        video_decode_promise;
  stx::TaskScheduler                   *task_scheduler = nullptr;
  ImageManager                         *image_manager  = nullptr;
};

struct Video : public Widget
{
  Video()
  {}

  explicit Video(std::string_view source)
  {}

  virtual ~Video() override
  {}

  virtual WidgetInfo get_info() override
  {
    return WidgetInfo{.type = "Video"};
  }

  virtual Layout layout(rect area) override
  {
  }

  virtual void tick(Context &context, std::chrono::nanoseconds interval) override
  {
  }

  void fullscreen();

  void exclusive_fullscreen();

  void non_fullscreen();

  bool show_controls = true;
};

int main(int argc, char **argv)
{
  ASH_CHECK(argc == 3);
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

  MediaPlayer::dump_supported_codecs();

  stx::Rc      demuxer          = VideoDemuxer::from_file(argv[1]).unwrap();
  stx::Rc      audio_decode_ctx = demuxer->make_audio_decoder().unwrap();
  stx::Rc      video_decode_ctx = demuxer->make_video_decoder().unwrap();
  stx::Promise promise          = stx::make_promise<void>(stx::os_allocator).unwrap();
  stx::Rc      audio_device     = AudioDevice::open(dev, 2).unwrap();

  spdlog::info("opened device: {}, channels: {}, format: {}, samplerate: {}, nsamples: {}, size: {}, silence: {}", dev.name.c_str(), dev.spec.channels,
               dev.spec.format, dev.spec.freq, dev.spec.samples, audio_device->info.spec.size, (int) audio_device->info.spec.silence);

  auto media_ctx = stx::rc::make_inplace<MediaPlayerAudioContext>(stx::os_allocator, stx::make_promise<void>(stx::os_allocator).unwrap(),
                                                                  audio_decode_ctx.share())
                       .unwrap();
  audio_device->audio_sources.push(AudioSource{
      stx::fn::rc::make_unique_functor(stx::os_allocator, [ctx = media_ctx.share()](stx::Span<u8> stream, SDL_AudioSpec spec) {
        return ctx->write_samples(stream, spec);
      }).unwrap()});

  audio_device->play();
  media_ctx->volume.store(25, std::memory_order_relaxed);

  std::thread demuxer_thread{[demuxer = demuxer.share(), promise = promise.share(), video_decode_ctx = video_decode_ctx.share(), audio_decode_ctx = audio_decode_ctx.share()]() {
    spdlog::info("demuxer thread running");

    int error = 0;

    while (error >= 0 && promise.fetch_cancel_request() == stx::CancelState::Executing)
    {
      error = av_read_frame(demuxer->fmt_ctx, demuxer->packet);
      if (error >= 0)
      {
        AVPacket *packet = av_packet_alloc();
        ASH_CHECK(packet != nullptr);
        av_packet_move_ref(packet, demuxer->packet);
        if (packet->stream_index == video_decode_ctx->stream->index)
        {
          video_decode_ctx->lock.lock();
          video_decode_ctx->packets.push_inplace(packet).unwrap();
          video_decode_ctx->lock.unlock();
        }
        else if (packet->stream_index == audio_decode_ctx->stream->index)
        {
          audio_decode_ctx->lock.lock();
          audio_decode_ctx->packets.push_inplace(packet).unwrap();
          audio_decode_ctx->lock.unlock();
        }
        else
        {
        }
      }
    }

    if (promise.fetch_cancel_request() == stx::CancelState::Canceled)
    {
      promise.notify_canceled();
      spdlog::info("demuxer thread canceled");
    }
    else
    {
      promise.notify_completed();
      spdlog::info("demuxer thread completed");
    }
  }};

  std::thread video_decode_thread{
      [video_decode_ctx = video_decode_ctx.share(),
       audio_ctx        = media_ctx.share(),
       promise = promise.share(), ctx = stx::rc::make_inplace<VideoDecodeContext>(stx::os_allocator, Clock::now(), timebase_to_ns(video_decode_ctx->stream->time_base)).unwrap()]() {
        int error = 0;

        while (error >= 0 && promise.fetch_cancel_request() == stx::CancelState::Executing)
        {
          video_decode_ctx->lock.lock();
          if (video_decode_ctx->packets.is_empty())
          {
            video_decode_ctx->lock.unlock();
            continue;
          }

          AVPacket *packet = video_decode_ctx->packets[0];
          video_decode_ctx->packets.erase(video_decode_ctx->packets.span().slice(0, 1));
          video_decode_ctx->lock.unlock();

          error = avcodec_send_packet(video_decode_ctx->ctx, packet);

          av_packet_free(&packet);

          if (error != 0)
          {
            // handle error
          }

          while ((error = avcodec_receive_frame(video_decode_ctx->ctx, video_decode_ctx->frame)) == 0)
          {
            ctx->store_frame(video_decode_ctx->frame);
            nanoseconds delay = ctx->refresh(nanoseconds{audio_ctx->decode_ctx.clock.load(std::memory_order_relaxed)}, Clock::now());
            spdlog::info("sleeping for: {}ms", delay.count() / 1'000'000LL);
            auto begin = Clock::now();
            while ((Clock::now() - begin) < delay)
              std::this_thread::yield();
          }

          if (error == AVERROR(EAGAIN))
          {
            error = 0;
          }
          else if (error == AVERROR(EOF))
          {
          }
          else
          {
            ASH_LOG_FFMPEG_ERR(error);
            break;
          }
        }

        if (promise.fetch_cancel_request() == stx::CancelState::Canceled)
        {
          promise.notify_canceled();
          spdlog::info("video decode thread canceled");
        }
        else
        {
          promise.notify_completed();
          spdlog::info("video decode thread completed");
        }
      }};

  // fmt_ctx->chapters;
  // fmt_ctx->metadata;
  // AV_DISPOSITION_ATTACHED_PIC contains album art

  AppConfig cfg{.enable_validation_layers = false};
  App       app{std::move(cfg), new Image{ImageProps{.source = FileImageSource{.path = stx::string::make_static(argv[2])}, .border_radius = vec4{200, 200, 200, 200}, .resize_on_load = true}}};
  timepoint last_tick = Clock::now();
  while (true)
  {
    timepoint present = Clock::now();
    app.tick(present - last_tick);
    last_tick = present;
  }

  demuxer_thread.join();
  video_decode_thread.join();
  SDL_Quit();
  return 0;
}
