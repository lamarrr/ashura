#include "SDL3/SDL.h"
#include "ashura/app.h"
#include "ashura/text.h"
#include "ashura/widget.h"
#include "ashura/widgets/image.h"
#include "stx/try_some.h"
#include <thread>

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

using Clock         = std::chrono::steady_clock;        // monotonic system clock
using timepoint     = Clock::time_point;
using nanoseconds   = std::chrono::nanoseconds;
using milliseconds  = std::chrono::milliseconds;
using seconds       = std::chrono::seconds;
using media_session = u64;

constexpr u8          MIN_VOLUME            = 0;
constexpr u8          MAX_VOLUME            = 255;
constexpr nanoseconds SYNC_THRESHOLD        = milliseconds{16};
constexpr nanoseconds NO_SYNC_THRESHOLD     = seconds{10};
constexpr nanoseconds MAX_FRAME_DELAY       = seconds{1};
constexpr u32         NAUDIO_DIFF_AVERAGES  = 20;
constexpr u8          MAX_SAMPLE_CORRECTION = 10;
/*
enum class DemuxError : u8
{
  PathDoesNotExist,
  StreamNotFound,
  CodecNotSupported
};

enum class MediaError : u8
{
  PathDoesNotExist,
  InvalidSessionId,
  Buffering,
  NoStreamFound,
  NoVideoStreamFound,
  NoAudioStreamFound,
  VideoCodecNotSupported,
  AudioCodecNotSupported
};

enum class MediaProperties
{

};

enum class MediaRequest
{
  None,
  Play,
  Pause,
  Stop
};

enum class MediaSeek : u8
{
  Exact,
  Forward,
  Backward
};

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

constexpr nanoseconds timebase_to_ns(AVRational timebase)
{
  return nanoseconds{AS(nanoseconds::rep, 1'000'000'000LL * AS(f32, timebase.num) / AS(f32, timebase.den))};
}

constexpr i64 timebase_scale(AVRational timebase, nanoseconds duration)
{
  return ((duration.count() * timebase.den) / timebase.num) / 1'000'000'000L;
}

constexpr nanoseconds timebase_resolve(AVRational dst, i64 frame)
{
  return timebase_to_ns(dst) * frame;
}

constexpr void fill_silence(stx::Span<u8> samples, SDL_AudioFormat format)
{
  switch (format)
  {
    case SDL_AUDIO_U8:
      samples.fill(128);
      break;

    case SDL_AUDIO_S8:
    case SDL_AUDIO_S16LSB:
    case SDL_AUDIO_S16MSB:
    case SDL_AUDIO_S32LSB:
    case SDL_AUDIO_S32MSB:
    case SDL_AUDIO_F32LSB:
    case SDL_AUDIO_F32MSB:
      samples.fill(0);
      break;

    default:
      // guessing
      samples.fill(0);
      break;
  }
}

struct ResamplerConfig
{
  AVSampleFormat  fmt                = AV_SAMPLE_FMT_NONE;
  AVSampleFormat  dst_fmt            = AV_SAMPLE_FMT_NONE;
  int             sample_rate        = 0;
  int             dst_sample_rate    = 0;
  AVChannelLayout channel_layout     = AV_CHANNEL_LAYOUT_MONO;
  AVChannelLayout dst_channel_layout = AV_CHANNEL_LAYOUT_MONO;

  bool operator==(ResamplerConfig const &other) const
  {
    return fmt == other.fmt && dst_fmt == other.dst_fmt && sample_rate == other.sample_rate && dst_sample_rate == other.dst_sample_rate &&
           (av_channel_layout_compare(&channel_layout, &other.channel_layout) == 0) && (av_channel_layout_compare(&dst_channel_layout, &other.dst_channel_layout) == 0);
  }

  bool operator!=(ResamplerConfig const &other) const
  {
    return !(*this == other);
  }
};

struct RgbVideoFrame
{
  u8         *pixels = nullptr;
  ash::extent extent;
  nanoseconds pts{0};

  RgbVideoFrame()
  {}

  STX_MAKE_PINNED(RgbVideoFrame)

  ~RgbVideoFrame()
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

      /// NOTE: FFMPEG will segfault if you use a custom allocated memory that doesn't meet its alignment/size requirements
      /// as it will perform some aligned/packed SIMD operations (if supported, as in most cases)
      int nbytes = av_image_alloc(planes, linesizes, new_extent.width, new_extent.height, AV_PIX_FMT_RGB24, 1);
      ASH_CHECK(nbytes >= 0);
      pixels = planes[0];
      extent = new_extent;
    }
  }
};

/// NOTE: audio has the master clock and video has the slave clock
// TODO(lamarrr): add a flush packet and/or is_completed marker
struct AudioDecodeContext
{
  AVCodecContext               *codec  = nullptr;                // accessed only on the audio thread
  AVStream                     *stream = nullptr;                // accessed only on demuxer thread
  stx::SpinLock                 packets_lock;                    // lock for packets
  stx::Vec<AVPacket *>          packets;                         // accessed on decoder and demuxer thread
  AVFrame                      *frame = nullptr;                 // accessed only on decoder thread
  std::atomic<nanoseconds::rep> clock = 0;                       // accessed on main/presentation and decoder thread
  stx::Vec<u8>                  samples;                         // usually in the target device's sample format, accessed on decoder thread
  usize                         bytes_consumed = 0;              // portion of samples consumed, accessed only on decoder thread
  SwrContext                   *resampler      = nullptr;        // accessed only on decoder thread
  ResamplerConfig               resampler_cfg;                   // accessed only on decoder thread
  timepoint                     begin_timepoint;

  /// COMMANDS
  stx::SpinLock cmd_lock;
  bool          pause_requested = false;

  AudioDecodeContext(AVCodecContext *icodec, AVStream *istream, AVFrame *iframe, SwrContext *iresampler, ResamplerConfig iresampler_cfg, timepoint ibegin_timepoint) :
      codec{icodec}, stream{istream}, frame{iframe}, resampler{iresampler}, resampler_cfg{iresampler_cfg}, begin_timepoint{ibegin_timepoint}
  {}

  STX_MAKE_PINNED(AudioDecodeContext)

  ~AudioDecodeContext()
  {
    swr_free(&resampler);
    avcodec_free_context(&codec);
    av_frame_free(&frame);
    for (AVPacket *packet : packets)
    {
      av_packet_free(&packet);
    }
  }

  void play()
  {
    stx::LockGuard guard{cmd_lock};
    pause_requested = false;
  }

  void pause()
  {
    stx::LockGuard guard{cmd_lock};
    pause_requested = true;
  }
};

struct VideoDecodeContext
{
  AVCodecContext               *codec  = nullptr;                   // accessed only on decoder thread
  AVStream                     *stream = nullptr;                   // accessed only on demuxer thread
  stx::SpinLock                 packets_lock;                       // locks the packets
  stx::Vec<AVPacket *>          packets;                            // accessed on demuxer and decoder thread
  AVFrame                      *frame = nullptr;                    // accessed only on decoder thread
  stx::SpinLock                 rgb_frame_lock;                     // locks rgb_frame
  RgbVideoFrame                 rgb_frame;                          // accessed on decoder thread and main/presentation thread
  SwsContext                   *rescaler = nullptr;                 // only accessed on decoder thread
  nanoseconds                   timebase{0};                        // only accessed on main/presentation, only written to once
  nanoseconds                   last_frame_pts{0};                  // accessed only on main/presentation
  nanoseconds                   last_frame_pts_interval{0};         // accessed only on main/presentation
  std::atomic<nanoseconds::rep> last_frame_pts_timepoint{0};        // duration from begin_timepoint, accessed on audio thread, and main/presentation
  nanoseconds                   frame_timer{0};                     // accessed only on main/presentation
  timepoint                     begin_timepoint;                    // accessed only on main/presentation, immutable

                                                                    /// COMMANDS
  stx::SpinLock cmd_lock;
  bool          pause_requested = false;

  VideoDecodeContext(AVCodecContext *icodec, AVStream *istream, AVFrame *iframe, timepoint ibegin_timepoint) :
      codec{icodec}, stream{istream}, frame{iframe}, timebase{timebase_to_ns(istream->time_base)}, begin_timepoint{ibegin_timepoint}
  {}

  STX_MAKE_PINNED(VideoDecodeContext)

  ~VideoDecodeContext()
  {
    sws_freeContext(rescaler);
    avcodec_free_context(&codec);
    av_frame_free(&frame);
    for (AVPacket *packet : packets)
    {
      av_packet_free(&packet);
    }
  }

  void play()
  {
    stx::LockGuard guard{cmd_lock};
    pause_requested = false;
  }

  void pause()
  {
    stx::LockGuard guard{cmd_lock};
    pause_requested = true;
  }

  // interval between video frames, unlike audio samples, can be really long, so we need a more fine-grained clock.
  // i.e. a 30fps video has 33 ms intervals which can be large if the audio is trying to sync to it. some frames might also be
  // repeated and span over multiple cycles.
  nanoseconds get_clock_time() const
  {
    timepoint last_frame_pts_timepoint = begin_timepoint + nanoseconds{this->last_frame_pts_timepoint.load(std::memory_order_relaxed)};
    timepoint now                      = Clock::now();
    return last_frame_pts + (now - last_frame_pts_timepoint);
  }

  // accessed only on decoder thread
  void update_rgb_frame()
  {
    ASH_CHECK(frame->pts != AV_NOPTS_VALUE);
    rescaler = sws_getCachedContext(rescaler, frame->width, frame->height, AS(AVPixelFormat, frame->format), frame->width, frame->height, AV_PIX_FMT_RGB24, 0, nullptr, nullptr, nullptr);
    ASH_CHECK(rescaler != nullptr);

    rgb_frame_lock.lock();

    rgb_frame.pts = timebase * frame->pts;
    rgb_frame.fit(extent{.width = AS(u32, frame->width), .height = AS(u32, frame->height)});

    u8 *planes[4]  = {rgb_frame.pixels, nullptr, nullptr, nullptr};
    int strides[4] = {frame->width * 3, 0, 0, 0};

    sws_scale(rescaler, frame->data, frame->linesize, 0, frame->height, planes, strides);

    rgb_frame_lock.unlock();
  }

  // accessed on main/presentation thread only
  // returns delay from next frame given the current decoded frame.
  // this function should ideally not be called again until the returned duration has passed.
  nanoseconds tick(stx::Option<nanoseconds> audio_pts, timepoint current_timepoint)
  {
    // TODO(lamarrr): update frame
    // try lock if not available, continue
    rgb_frame_lock.lock();
    nanoseconds frame_pts = rgb_frame.pts;
    rgb_frame_lock.unlock();

    nanoseconds pts_interval = frame_pts - last_frame_pts;

    if (pts_interval <= nanoseconds{0} || pts_interval >= MAX_FRAME_DELAY)
    {
      // means delay is incorrect, we thus guess the frame delay by using the previous one
      pts_interval = last_frame_pts_interval;
    }

    last_frame_pts          = frame_pts;
    last_frame_pts_interval = pts_interval;
    nanoseconds delay       = pts_interval;

    if (audio_pts.is_some())
    {
      nanoseconds diff           = frame_pts - audio_pts.value();                                        // time difference between present audio and video frames
      nanoseconds sync_threshold = pts_interval > SYNC_THRESHOLD ? pts_interval : SYNC_THRESHOLD;        // skip or repeat the frame. Take delay into account we still doesn't "know if this is the best guess."

      if (std::chrono::abs(diff) < NO_SYNC_THRESHOLD)
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

    last_frame_pts_timepoint.store((current_timepoint - begin_timepoint).count(), std::memory_order_relaxed);

    return actual_delay;
  }
};

struct DecodeContext
{
  AVCodecContext *codec  = nullptr;
  AVStream       *stream = nullptr;
  AVFrame        *frame  = nullptr;
};

// Demuxer runs on main/presentation, fetches raw streams/packets from the files and dispatches them to the decoders
//
// the audio/video decode thread decodes audio/video frames, performs conversions/resampling and sends them to the renderer/audio device
//
struct VideoDemuxer
{
  static constexpr u32 AVIO_BUFFER_SIZE = 4096;

  AVIOContext     *io_ctx  = nullptr;
  AVFormatContext *fmt_ctx = nullptr;        // TODO(lamarrr): lock
  stx::SpinLock    fmt_ctx_lock;
  AVPacket        *packet = nullptr;
  FILE            *file   = nullptr;
  stx::String      path;

  VideoDemuxer(AVIOContext *iio_ctx, AVFormatContext *ictx, AVPacket *ipacket, FILE *ifile, stx::String ipath) :
      io_ctx{iio_ctx}, fmt_ctx{ictx}, packet{ipacket}, file{ifile}, path{std::move(ipath)}
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
      return stx::Err(DemuxError::PathDoesNotExist);
    }

    std::FILE *file = std::fopen(path.c_str(), "rb");
    ASH_CHECK(file != nullptr);

    void *avio_buffer = av_malloc(AVIO_BUFFER_SIZE);        // TODO(lamarrr): this memory is presently being leaked
    ASH_CHECK(avio_buffer != nullptr);

    stx::Rc demuxer = stx::rc::make_inplace<VideoDemuxer>(stx::os_allocator, nullptr, nullptr, nullptr, nullptr, stx::string::make(stx::os_allocator, path).unwrap()).unwrap();

    AVIOContext *io_ctx = avio_alloc_context(AS(uchar *, avio_buffer), AVIO_BUFFER_SIZE, 0, demuxer.handle, packet_file_read_callback, nullptr, packet_file_seek_callback);
    ASH_CHECK(io_ctx != nullptr);

    AVFormatContext *fmt_ctx = avformat_alloc_context();
    ASH_CHECK(fmt_ctx != nullptr);

    AVPacket *packet = av_packet_alloc();
    ASH_CHECK(packet != nullptr);

    fmt_ctx->pb      = io_ctx;
    demuxer->file    = file;
    demuxer->io_ctx  = io_ctx;
    demuxer->fmt_ctx = fmt_ctx;
    demuxer->packet  = packet;

    ASH_CHECK(avformat_open_input(&fmt_ctx, nullptr, nullptr, nullptr) >= 0);

    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0)
    {
      return stx::Err(DemuxError::StreamNotFound);
    }

    ASH_LOG_INFO(MediaPlayer, "Found Stream(s) in Media File {}. Dumping Metadata.", std::string_view{path});

    AVDictionaryEntry *prev = nullptr;
    do
    {
      prev = av_dict_get(fmt_ctx->metadata, "", prev, AV_DICT_IGNORE_SUFFIX);
      if (prev != nullptr)
      {
        ASH_LOG_INFO(MediaPlayer, "Field: {}={}", prev->key, prev->value);
      }
    } while (prev != nullptr);

    for (uint i = 0; i < fmt_ctx->nb_streams; i++)
    {
      AVStream *stream = fmt_ctx->streams[i];
      ASH_LOG_INFO(MediaPlayer, "Dumping Metadata for Media File -> {} Stream: {}, type={}, codec={}", std::string_view{path}, i, av_get_media_type_string(stream->codecpar->codec_type), avcodec_get_name(stream->codecpar->codec_id));
      AVDictionaryEntry *prev = nullptr;
      do
      {
        prev = av_dict_get(stream->metadata, "", prev, AV_DICT_IGNORE_SUFFIX);
        if (prev != nullptr)
        {
          ASH_LOG_INFO(MediaPlayer, "Field -> {}={}", prev->key, prev->value);
        }
      } while (prev != nullptr);
    }

    return stx::Ok(std::move(demuxer));
  }

  static stx::Result<DecodeContext, DemuxError> make_decoder_for_stream(std::string_view source, AVStream *stream)
  {
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

    return stx::Ok(DecodeContext{.codec = codec_ctx, .stream = stream, .frame = frame});
  }

  // AVMEDIA_TYPE_AUDIO
  // AVMEDIA_TYPE_VIDEO
  // AVMEDIA_TYPE_SUBTITLE
  stx::Result<DecodeContext, DemuxError> make_decoder(AVMediaType media_type) const
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

    return make_decoder_for_stream(path, stream);
  }
};

struct AudioDeviceInfo
{
  stx::String   name;
  SDL_AudioSpec spec{.freq = 48000, .format = SDL_AUDIO_S16SYS, .channels = 2, .samples = 4800, .size = 4800 * 2, .callback = nullptr, .userdata = nullptr};

  static stx::Vec<AudioDeviceInfo> enumerate()
  {
    stx::Vec<AudioDeviceInfo> devices;

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
  /// NOTE: this is called from a separate thread, the user should return true if its still open
  virtual bool mix(stx::Span<u8>, SDL_AudioSpec) = 0;
  virtual ~AudioSource()                         = 0;
};

struct AudioSourceEntry
{
  stx::Rc<AudioSource *> source;
  bool                   is_open = true;
};

struct AudioDevice
{
  SDL_AudioDeviceID          id = 0;
  AudioDeviceInfo            info;
  bool                       is_playing = false;
  stx::SpinLock              audio_sources_lock;
  stx::Vec<AudioSourceEntry> audio_sources;

  AudioDevice(SDL_AudioDeviceID iid, AudioDeviceInfo iinfo) :
      id{iid}, info{std::move(iinfo)}
  {}

  STX_MAKE_PINNED(AudioDevice)

  ~AudioDevice()
  {
    SDL_CloseAudioDevice(id);
  }

  void add_source(stx::Rc<AudioSource *> source)
  {
    audio_sources_lock.lock();
    audio_sources.push(AudioSourceEntry{.source = std::move(source), .is_open = true}).unwrap();
    audio_sources_lock.unlock();
  }

  /// NOTE: will be called on audio device thread managed by SDL, use SDL_LockAudioDevice and SDL_UnlockAudioDevice to prevent this callback from running
  static void audio_callback(void *userdata, u8 *pstream, int len)
  {
    AudioDevice *This = AS(AudioDevice *, userdata);
    stx::Span    stream{pstream, AS(usize, len)};

    fill_silence(stream, This->info.spec.format);

    This->audio_sources_lock.lock();
    for (AudioSourceEntry &info : This->audio_sources)
    {
      info.is_open = info.source->mix(stream, This->info.spec);
    }

    auto [open, closed] = This->audio_sources.span().partition([](AudioSourceEntry const &src) { return src.is_open; });
    (void) open;

    if (!closed.is_empty())
    {
      ASH_LOG_INFO(MediaPlayer, "Closing {} Audio Sources For Audio Device -> {} (id = {})", closed.size(), This->info.name.view(), This->id);
    }

    This->audio_sources.erase(closed);
    This->audio_sources_lock.unlock();
  }

  void play()
  {
    if (is_playing)
    {
      return;
    }
    int err = SDL_PlayAudioDevice(id);
    ASH_CHECK(err == 0);
    is_playing = true;
  }

  void pause()
  {
    int err = SDL_PauseAudioDevice(id);
    ASH_CHECK(err == 0);
    is_playing = false;
  }

  static SDL_AudioSpec make_spec(SDL_AudioSpec const spec, std::string_view name, void *userdata)
  {
    SDL_AudioSpec out;

    // FFMPEG only supports these, better not to support others
    switch (spec.format)
    {
      case SDL_AUDIO_U8:
      case SDL_AUDIO_S16SYS:
      case SDL_AUDIO_S32SYS:
      case SDL_AUDIO_F32SYS:
      {
        out.format = spec.format;
      }
      break;
      default:
      {
        out.format = SDL_AUDIO_S16SYS;
      }
      break;
    }

    switch (spec.channels)
    {
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 16:
      case 24:
      {
        out.channels = spec.channels;
      }
      break;
      default:
      {
        out.channels = 2;
        ASH_LOG_WARN(MediaPlayer, "Unrecognized number of channels = {} on audio device: {}, attempting to use: {}", spec.channels, name, out.channels);
        break;
      }
    }

    out.freq     = spec.freq;
    out.samples  = spec.samples;
    out.size     = 0;
    out.callback = audio_callback;
    out.userdata = userdata;

    return out;
  }

  static stx::Option<stx::Rc<AudioDevice *>> open_default()
  {
    TRY_SOME(info, AudioDeviceInfo::get_default());
    stx::Rc dev = stx::rc::make_inplace<AudioDevice>(stx::os_allocator, AS(SDL_AudioDeviceID, 0), AudioDeviceInfo{}).unwrap();

    SDL_AudioSpec desired_spec = make_spec(info.spec, "default", dev.handle);

    // .size is modified to the hardware buffer size
    // we are flexible with number of samples and frequency change
    SDL_AudioDeviceID id = SDL_OpenAudioDevice(nullptr, 0, &desired_spec, &dev->info.spec, SDL_AUDIO_ALLOW_ANY_CHANGE);

    if (id == 0)
    {
      ASH_LOG_ERR(MediaPlayer, "Failed To Open Default Audio Device, SDL Error: {}", SDL_GetError());
      return stx::None;
    }
    else
    {
      ASH_LOG_INFO(MediaPlayer, "Opened Audio Default Audio Device");
    }

    dev->info.name = stx::string::make_static("default");
    dev->id        = id;

    return stx::Some(std::move(dev));
  }

  static stx::Option<stx::Rc<AudioDevice *>> open(AudioDeviceInfo const &info)
  {
    stx::Rc dev = stx::rc::make_inplace<AudioDevice>(stx::os_allocator, AS(SDL_AudioDeviceID, 0), AudioDeviceInfo{}).unwrap();

    SDL_AudioSpec desired_spec = make_spec(info.spec, info.name, dev.handle);

    // .size is modified to the hardware buffer size
    // we are flexible with number of samples and frequency change
    SDL_AudioDeviceID id = SDL_OpenAudioDevice(info.name.c_str(), 0, &desired_spec, &dev->info.spec, SDL_AUDIO_ALLOW_ANY_CHANGE);

    if (id == 0)
    {
      ASH_LOG_ERR(MediaPlayer, "Failed To Open Audio Device: {}, SDL Error: {}", info.name.c_str(), SDL_GetError());
      return stx::None;
    }
    else
    {
      ASH_LOG_INFO(MediaPlayer, "Opened Audio Device: {}", info.name.c_str());
    }

    dev->info.name = stx::string::make(stx::os_allocator, info.name).unwrap();
    dev->id        = id;

    return stx::Some(std::move(dev));
  }
};

struct MediaPlayerAudioSource : public AudioSource
{
  stx::Promise<void>            promise;
  stx::Rc<AudioDecodeContext *> ctx;
  std::atomic<u8>               volume = MAX_VOLUME;

  MediaPlayerAudioSource(stx::Promise<void> ipromise, stx::Rc<AudioDecodeContext *> ictx) :
      promise{std::move(ipromise)}, ctx{std::move(ictx)}
  {}

  virtual ~MediaPlayerAudioSource() override = default;

  virtual bool mix(stx::Span<u8> stream, SDL_AudioSpec spec) override
  {
    int volume                     = AS(int, this->volume.load(std::memory_order_relaxed));
    volume                         = (volume * 128) / 255;        // convert to SDL volume range 0-128
    usize           bytes_written  = 0;
    AVSampleFormat  sample_fmt     = AV_SAMPLE_FMT_NONE;
    AVChannelLayout channel_layout = AV_CHANNEL_LAYOUT_MONO;
    bool            is_open        = true;
    int             error          = 0;

    switch (spec.format)
    {
      case SDL_AUDIO_U8:
      {
        sample_fmt = AV_SAMPLE_FMT_U8;
      }
      break;
      case SDL_AUDIO_S16SYS:
      {
        sample_fmt = AV_SAMPLE_FMT_S16;
      }
      break;
      case SDL_AUDIO_S32SYS:
      {
        sample_fmt = AV_SAMPLE_FMT_S32;
      }
      break;
      case SDL_AUDIO_F32SYS:
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

    // TODO(lamarrr): in the future we might want to allow the user select the output mode
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
      case 3:
      {
        channel_layout = AV_CHANNEL_LAYOUT_SURROUND;
      }
      break;
      case 4:
      {
        channel_layout = AV_CHANNEL_LAYOUT_QUAD;
      }
      break;
      case 5:
      {
        channel_layout = AV_CHANNEL_LAYOUT_4POINT1;
      }
      break;
      case 6:
      {
        channel_layout = AV_CHANNEL_LAYOUT_HEXAGONAL;
      }
      break;
      case 7:
      {
        channel_layout = AV_CHANNEL_LAYOUT_6POINT1;
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
      case 24:
      {
        channel_layout = AV_CHANNEL_LAYOUT_22POINT2;
      }
      break;
      default:
      {
        ASH_LOG_ERR(MediaPlayer, "encountered unsupported number of channels: {}", spec.channels);
        is_open = false;
      }
      break;
    }

    stx::CancelState expected_state = stx::CancelState::Executing;

    ctx->cmd_lock.lock();
    bool needs_pause = ctx->pause_requested;
    ctx->cmd_lock.unlock();

    if (!needs_pause)
      while (is_open && bytes_written < stream.size() && (expected_state = promise.fetch_cancel_request()) != stx::CancelState::Canceled)
      {
        if (ctx->bytes_consumed != ctx->samples.size())
        {
          usize bytes_left     = stream.size() - bytes_written;
          usize bytes_to_write = std::min(bytes_left, AS(usize, ctx->samples.size() - ctx->bytes_consumed));

          ASH_SDL_CHECK(SDL_MixAudioFormat(stream.data() + bytes_written, ctx->samples.data() + ctx->bytes_consumed, spec.format, bytes_to_write, volume) == 0);
          bytes_written += bytes_to_write;
          ctx->bytes_consumed += bytes_to_write;

          usize nsamples_written = (bytes_to_write / spec.channels) / av_get_bytes_per_sample(sample_fmt);

          ctx->clock.fetch_add(AS(nanoseconds::rep, 1'000'000'000LL * AS(f32, nsamples_written) / AS(f32, spec.freq)), std::memory_order_relaxed);
        }
        else
        {
          ctx->packets_lock.lock();
          if (ctx->packets.is_empty())
          {
            ctx->packets_lock.unlock();
            break;
          }

          AVPacket *packet = ctx->packets[0];
          ctx->packets.erase(ctx->packets.span().slice(0, 1));
          ctx->packets_lock.unlock();

          nanoseconds pts{ctx->clock.load(std::memory_order_relaxed)};

          if (packet->pts != AV_NOPTS_VALUE)
          {
            pts = timebase_resolve(ctx->stream->time_base, packet->pts);
          }

          error = avcodec_send_packet(ctx->codec, packet);

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

          error = avcodec_receive_frame(ctx->codec, ctx->frame);

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
              .fmt                = AS(AVSampleFormat, ctx->frame->format),
              .dst_fmt            = sample_fmt,
              .sample_rate        = ctx->frame->sample_rate,
              .dst_sample_rate    = spec.freq,
              .channel_layout     = ctx->frame->ch_layout,
              .dst_channel_layout = channel_layout};

          if (ctx->resampler_cfg != target_cfg || ctx->resampler == nullptr)
          {
            if (ctx->resampler != nullptr)
            {
              swr_free(&ctx->resampler);
            }

            error = swr_alloc_set_opts2(&ctx->resampler, &target_cfg.dst_channel_layout, target_cfg.dst_fmt, target_cfg.dst_sample_rate,
                                        &ctx->frame->ch_layout, target_cfg.fmt, target_cfg.sample_rate, 0, nullptr);

            if (error != 0)
            {
              ASH_LOG_FFMPEG_ERR(error);
              break;
            }

            ctx->resampler_cfg = target_cfg;

            error = swr_init(ctx->resampler);

            if (error != 0)
            {
              ASH_LOG_FFMPEG_ERR(error);
              break;
            }
          }

          int max_nsamples = swr_get_out_samples(ctx->resampler, ctx->frame->nb_samples);

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

          ctx->samples.unsafe_resize_uninitialized(max_buffer_size).unwrap();

          u8 *out = ctx->samples.data();

          int nsamples = swr_convert(ctx->resampler, &out, max_nsamples, (u8 const **) ctx->frame->data, ctx->frame->nb_samples);

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

          ctx->samples.unsafe_resize_uninitialized(buffer_size).unwrap();

          usize bytes_left     = stream.size() - bytes_written;
          usize bytes_to_write = std::min(bytes_left, AS(usize, buffer_size));

          ASH_SDL_CHECK(SDL_MixAudioFormat(stream.data() + bytes_written, ctx->samples.data(), spec.format, bytes_to_write, volume) == 0);

          ctx->bytes_consumed = bytes_to_write;

          bytes_written += bytes_to_write;

          usize nsamples_written = (bytes_to_write / spec.channels) / av_get_bytes_per_sample(target_cfg.dst_fmt);

          ctx->clock.store(pts.count() + AS(nanoseconds::rep, 1'000'000'000LL * AS(f32, nsamples_written) / AS(f32, spec.freq)), std::memory_order_relaxed);
        }
      }

    if (expected_state == stx::CancelState::Canceled)
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

struct MediaContext
{
  stx::Option<stx::Rc<AudioDecodeContext *>> audio_decode_ctx;
  stx::Option<stx::Rc<VideoDecodeContext *>> video_decode_ctx;
  stx::Option<stx::Rc<std::thread *>>        video_decode_thread;
};

struct MediaSession
{
  stx::String                          path;
  std::thread                          demux_thread;
  stx::Option<stx::Rc<VideoDemuxer *>> demuxer;
  stx::Option<gfx::image>              image;

  bool is_buffering = true;
};

struct Lyrics
{
  stx::Vec<std::string_view> content;
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
// TODO(lamarrr): main/presentation should be used for demuxing and another thread for decoding
//
struct MediaPlayer : public Plugin
{
  template <typename T>
  using Result = stx::Result<T, MediaError>;

  static void dump_supported_codecs()
  {
    ASH_LOG_INFO(MediaPlayer, "Enumerating Available Codecs");
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
    audio_device = AudioDevice::open_default();
  }

  virtual void tick(Context &context, std::chrono::nanoseconds interval)
  {
  }

  virtual void on_exit(Context &context)
  {}

  virtual std::string_view get_name()
  {
    return "MediaPlayer";
  }

  bool __try_open_audio_dev()
  {
    if (audio_device.is_some())
    {
      return true;
    }
    audio_device = AudioDevice::open_default();
    return audio_device.is_some();
  }

  void __begin_audio_dev_play()
  {
    if (__try_open_audio_dev() && !audio_device.value()->is_playing)
    {
      audio_device.value()->play();
    }
  }

  media_session create_session(std::string_view source)
  {
    media_session session_id = next_session_id;
    next_session_id++;
    sessions.emplace(session_id, MediaSession{.path = stx::string::make(stx::os_allocator, source).unwrap()});
    return session_id;
  }

  void begin_buffering();

  bool is_buffered();

  Result<stx::Void> play(media_session session, usize video_stream, usize audio_stream)
  {
    auto it = sessions.find(session);
    if (it == sessions.end())
    {
      return stx::Err(MediaError::InvalidSessionId);
    }

    if (it->second.audio_decode_ctx.is_some())
    {
      __begin_audio_dev_play();
      it->second.audio_decode_ctx.value()->play();
    }

    if (it->second.video_decode_ctx.is_some())
    {
      it->second.video_decode_ctx.value()->play();
    }

    return stx::Ok(stx::Void{});
  }

  Result<stx::Void> pause(media_session session)
  {
    auto it = sessions.find(session);
    if (it == sessions.end())
    {
      return stx::Err(MediaError::InvalidSession);
    }

    if (it->second.audio_decode_ctx.is_some())
    {
      it->second.audio_decode_ctx.value()->pause();
    }

    if (it->second.video_decode_ctx.is_some())
    {
      it->second.video_decode_ctx.value()->pause();
    }

    return stx::Ok(stx::Void{});
  }

  Result<stx::Void> seek_time(media_session session, nanoseconds timepoint, MediaSeek seek = MediaSeek::Exact)
  {
    auto pos = sessions.find(session);
    ASH_CHECK(pos != sessions.end());

    nanoseconds diff = timepoint - nanoseconds{pos->second.audio_decode_ctx.value()->clock.load(std::memory_order_relaxed)};

    // TODO(lamarrr): lock format contexts and decode contexts, also update the docs
    if (pos->second.audio_decode_ctx.is_some())
    {
      int error = av_seek_frame(pos->second.demuxer->fmt_ctx, pos->second.audio_decode_ctx.value()->stream->index, 0, diff.count() < 0 ? AVSEEK_FLAG_BACKWARD : 0);
    }
    else if (pos->second.video_decode_ctx.is_some())
    {
      int error = av_seek_frame(pos->second.demuxer->fmt_ctx, pos->second.video_decode_ctx.value()->stream->index, 0, diff.count() < 0 ? AVSEEK_FLAG_BACKWARD : 0);
    }
  }

  Result<stx::Void> seek_frame(media_session session, usize frame, MediaSeek seek = MediaSeek::Exact);

  Result<stx::Void> seek_preview_at_time(media_session session, nanoseconds timepoint);

  Result<stx::Void> seek_preview_at_frame(media_session session, usize frame);

  Result<stx::Void> set_volume(media_session session, u8 volume);

  Result<stx::Void> set_autoplay(media_session session, bool autoplay);

  Result<bool> is_playing(media_session session)
  {
  }

  // if has audio, check that the audio frame index is equal to number of audio frames - 1,
  // else if picture only, check that the frame index is equal to the number of picture frames
  Result<bool> is_play_ended(media_session session);

  // if has audio stream return true
  Result<bool> has_audio(media_session session);

  // if has video/picture stream return true
  Result<bool> has_video(media_session session);

  // TODO(lamarrr): we might not be able to tell yet as this is async
  // TODO(lamarrr): return metadata along with it
  //
  // returns the audio streams found in the media file
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

  Result<stx::Void> select_audio(media_session session, usize);

  Result<stx::Void> select_subtitle(media_session session, usize);

  u64                                   next_session_id = 0;
  std::map<media_session, MediaSession> sessions;
  stx::Option<stx::Rc<AudioDevice *>>   audio_device;
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
*/
int main(int argc, char **argv)
{
  /*
  // ASH_CHECK(argc == 3);
  ASH_CHECK(SDL_Init(SDL_INIT_EVERYTHING) == 0);
  spdlog::info("System theme: {}", (int) SDL_GetSystemTheme());
  stx::Vec devices = AudioDeviceInfo::enumerate();
  for (AudioDeviceInfo const &dev : devices)
  {
    spdlog::info("name: {}, channels: {}, format: {}, samplerate: {}, nsamples: {}",
                 dev.name.c_str(),
                 dev.spec.channels, dev.spec.format, dev.spec.freq, dev.spec.samples);
  }

  AudioDeviceInfo dev_info = AudioDeviceInfo::get_default().unwrap();
  spdlog::info("default device: {}, channels: {}, format: {}, samplerate: {}, nsamples: {}", dev_info.name.c_str(), dev_info.spec.channels, dev_info.spec.format,
               dev_info.spec.freq, dev_info.spec.samples);

  MediaPlayer::dump_supported_codecs();

  stx::Rc      demuxer          = VideoDemuxer::from_file(argv[1]).unwrap();
  stx::Rc      audio_decode_ctx = demuxer->make_audio_decoder().unwrap();
  stx::Rc      video_decode_ctx = demuxer->make_video_decoder().unwrap();
  stx::Promise promise          = stx::make_promise<void>(stx::os_allocator).unwrap();
  stx::Rc      audio_dev        = AudioDevice::open(dev_info).unwrap();

  spdlog::info("opened device: {}, channels: {}, format: {}, samplerate: {}, nsamples: {}, size: {}, silence: {}", dev_info.name.c_str(), dev_info.spec.channels,
               dev_info.spec.format, dev_info.spec.freq, dev_info.spec.samples, audio_dev->info.spec.size, (int) audio_dev->info.spec.silence);

  auto audio_src = stx::rc::make_inplace<MediaPlayerAudioSource>(stx::os_allocator, stx::make_promise<void>(stx::os_allocator).unwrap(),
                                                                 audio_decode_ctx.share())
                       .unwrap();
  audio_dev->add_source(stx::transmute(static_cast<AudioSource *>(audio_src.handle), audio_src.share()));
  audio_dev->play();
  audio_src->volume.store(25, std::memory_order_relaxed);

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

          error = avcodec_send_packet(video_decode_ctx->codec, packet);

          av_packet_free(&packet);

          if (error != 0)
          {
            // handle error
          }

          while ((error = avcodec_receive_frame(video_decode_ctx->codec, video_decode_ctx->frame)) == 0)
          {
            ctx->load_frame(video_decode_ctx->frame);
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
  */

  FontSpec fonts[] = {
      {.name = "Roboto", .path = R"(C:\Users\Basit\Documents\workspace\oss\ashura\assets\fonts\Roboto\Roboto-Regular.ttf)", .stroke_thickness = 2.5},
      {.name = "Arabic", .path = R"(C:\Users\Basit\Desktop\IBM_Plex_Sans_Arabic\IBMPlexSansArabic-Regular.ttf)", .stroke_thickness = 2.5},
      {.name = "JP", .path = R"(C:\Users\Basit\Desktop\Noto_Sans_HK\NotoSansHK-Regular.otf)", .stroke_thickness = 2.5, .max_atlas_extent = {8000, 8000}},
      {.name = "MaterialIcons", .path = R"(C:\Users\Basit\Documents\workspace\oss\ashura\assets\fonts\MaterialIcons\MaterialIcons-Regular.ttf)", .stroke_thickness = 0}};

  AppConfig cfg{.enable_validation_layers = false, .fonts = fonts};

  App       app{std::move(cfg), new Image{ImageProps{.source = FileImageSource{.path = R"(C:\Users\Basit\Pictures\1288647.png)"}, .border_radius = vec4{20, 20, 20, 20}, .aspect_ratio = stx::Some(2.0f), .resize_on_load = true}}};
  timepoint last_tick = Clock::now();
  while (true)
  {
    timepoint present = Clock::now();
    app.tick(present - last_tick);
    last_tick = present;
  }

  // demuxer_thread.join();
  // video_decode_thread.join();
  SDL_Quit();
  return 0;
}
