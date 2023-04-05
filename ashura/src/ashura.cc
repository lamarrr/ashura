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

enum class Error : i64
{
  Eof          = AVERROR_EOF,
  NeedsPackets = AVERROR(EAGAIN),
  Invalid      = AVERROR(EINVAL),
  Ok           = 0
};

#define ASH_LOG_FFMPEG_ERR(err_exp)                                                                                           \
  do                                                                                                                          \
  {                                                                                                                           \
    char ASH_LOG_FFMPEG_ERR_error_buffer[256];                                                                                \
    int  ASH_LOG_FFMPEG_ERR_error = (err_exp);                                                                                \
    if (av_strerror(ASH_LOG_FFMPEG_ERR_error, ASH_LOG_FFMPEG_ERR_error_buffer, sizeof(ASH_LOG_FFMPEG_ERR_error_buffer)) == 0) \
    {                                                                                                                         \
      spdlog::error("[FFMPEG] {}: {}", ASH_LOG_FFMPEG_ERR_error, ASH_LOG_FFMPEG_ERR_error_buffer);                            \
    }                                                                                                                         \
    else                                                                                                                      \
    {                                                                                                                         \
      spdlog::error("[FFMPEG] unidentified ffmpeg error: {}", ASH_LOG_FFMPEG_ERR_error);                                      \
    }                                                                                                                         \
  } while (0)

// volume ranges from 0 to 255
#define ASH_ADJUST_VOLUME(sample, volume) ((sample) = ((sample) *AS(::ash::i64, (volume))) / AS(::ash::i64, MAX_VOLUME))
#define ASH_ADJUST_VOLUME_U8(sample, volume) ((sample) = ((((sample) -AS(::ash::i32, 128)) * AS(::ash::i32, volume)) / AS(::ash::i32, MAX_VOLUME)) + AS(::ash::i32, 128))

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

void Ashura_SDL_ScaleAudioFormat(stx::Span<u8> samples, SDL_AudioFormat format, u8 volume)
{
  if (volume == MIN_VOLUME)
  {
    fill_silence(samples, format);
    return;
  }
  else if (volume == MAX_VOLUME)
  {
    return;
  }

  switch (format)
  {
    case AUDIO_U8:
      for (u8 &sample : samples)
      {
        ASH_ADJUST_VOLUME_U8(sample, volume);
      }
      break;

    case AUDIO_S8:
      for (i8 &sample : samples.transmute<i8>())
      {
        ASH_ADJUST_VOLUME(sample, volume);
      }
      break;

    case AUDIO_S16LSB:
      for (i16 &sample : samples.transmute<i16>())
      {
        sample = SDL_SwapLE16(sample);
        ASH_ADJUST_VOLUME(sample, volume);
        sample = SDL_SwapLE16(sample);
      }
      break;

    case AUDIO_S16MSB:
      for (i16 &sample : samples.transmute<i16>())
      {
        sample = SDL_SwapBE16(sample);
        ASH_ADJUST_VOLUME(sample, volume);
        sample = SDL_SwapBE16(sample);
      }
      break;

    case AUDIO_S32LSB:
      for (i32 &sample : samples.transmute<i32>())
      {
        sample = SDL_SwapLE32(sample);
        ASH_ADJUST_VOLUME(sample, volume);
        sample = SDL_SwapLE32((u32) sample);
      }
      break;

    case AUDIO_S32MSB:
      for (i32 &sample : samples.transmute<i32>())
      {
        sample = SDL_SwapBE32(sample);
        ASH_ADJUST_VOLUME(sample, volume);
        sample = SDL_SwapBE32((u32) sample);
      }
      break;

    case AUDIO_F32LSB:
    {
      f32 scale = volume / AS(f32, MAX_VOLUME);
      for (f32 &sample : samples.transmute<f32>())
      {
        sample = SDL_SwapFloatLE(sample) * scale;
        sample = SDL_SwapFloatLE(sample);
      }
    }
    break;

    case AUDIO_F32MSB:
    {
      f32 scale = volume / AS(f32, MAX_VOLUME);
      for (f32 &sample : samples.transmute<f32>())
      {
        sample = SDL_SwapFloatBE(sample) * scale;
        sample = SDL_SwapFloatBE(sample);
      }
    }
    break;

    default:
      break;
  }
}

constexpr nanoseconds timebase_to_ns(AVRational timebase)
{
  return nanoseconds{AS(nanoseconds::rep, 1'000'000'000LL * AS(f32, timebase.num) / AS(f32, timebase.den))};
}

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
  ash::extent extent;
  u8         *pixels = nullptr;
  nanoseconds pts{0};

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

  void push_packet(AVPacket *packet)
  {
    packets.push_inplace(packet).unwrap();
  }
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
    av_freep(io_ctx->buf_ptr);
    avio_context_free(&io_ctx);
    avformat_close_input(&fmt_ctx);
    av_packet_free(&packet);
  }

  static int packet_file_read_callback(void *opaque, u8 *buffer, int buffer_size)
  {
    // AVERROR(EAGAIN);
    std::FILE *file = AS(VideoDemuxer *, opaque)->file;

    usize read = std::fread(buffer, 1, buffer_size, file);

    if (std::ferror(file))
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
    return std::fseek(AS(VideoDemuxer *, opaque)->file, AS(long, offset), whence);
  }

  static stx::Option<stx::Rc<VideoDemuxer *>> from_file(char const *path)
  {
    if (!std::filesystem::exists(path))
    {
      return stx::None;
    }

    FILE *file = fopen(path, "rb");
    ASH_CHECK(file != nullptr);

    void *avio_buffer = av_malloc(AVIO_BUFFER_SIZE);
    ASH_CHECK(avio_buffer != nullptr);

    stx::Rc demuxer = stx::rc::make_inplace<VideoDemuxer>(stx::os_allocator, nullptr, nullptr, nullptr, nullptr).unwrap();

    AVIOContext *io_ctx = avio_alloc_context(AS(uchar *, avio_buffer), AVIO_BUFFER_SIZE, 0, demuxer.handle, packet_file_read_callback, nullptr, nullptr /* packet_file_seek_callbackcan be null if seeking not supported, TODO(lamarrr): check if av_seek_frame requires this*/);
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
    ASH_CHECK(avformat_find_stream_info(fmt_ctx, nullptr) >= 0);

    return stx::Some(std::move(demuxer));
  }

  stx::Option<stx::Rc<DecodeContext *>> make_decoder(AVMediaType media_type) const
  {
    int stream_index = av_find_best_stream(fmt_ctx, media_type, -1, -1, nullptr, 0);

    if (stream_index < 0)
    {
      return stx::None;
    }

    AVStream *stream = fmt_ctx->streams[stream_index];

    if (stream == nullptr)
    {
      return stx::None;
    }

    AVCodec const *codec = avcodec_find_decoder(stream->codecpar->codec_id);

    if (codec == nullptr)
    {
      return stx::None;
    }

    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);

    ASH_CHECK(codec_ctx != nullptr);
    ASH_CHECK(avcodec_parameters_to_context(codec_ctx, stream->codecpar) >= 0);
    ASH_CHECK(avcodec_open2(codec_ctx, codec, nullptr) >= 0);

    AVFrame *frame = av_frame_alloc();
    ASH_CHECK(frame != nullptr);

    return stx::Some(stx::rc::make_inplace<DecodeContext>(stx::os_allocator, codec_ctx, stream, frame).unwrap());
  }

  stx::Option<stx::Rc<DecodeContext *>> make_video_decoder() const
  {
    return make_decoder(AVMEDIA_TYPE_VIDEO);
  }

  stx::Option<stx::Rc<DecodeContext *>> make_audio_decoder() const
  {
    return make_decoder(AVMEDIA_TYPE_AUDIO);
  }

  stx::Option<stx::Rc<DecodeContext *>> make_subtitle_decoder() const
  {
    return make_decoder(AVMEDIA_TYPE_SUBTITLE);
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

  void tick(nanoseconds interval);

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

struct AudioDevice
{
  SDL_AudioDeviceID        id = 0;
  AudioDeviceInfo          info;
  stx::Promise<void>       promise;
  stx::Rc<DecodeContext *> ctx;
  AudioDecodeContext       decode_ctx;
  std::atomic<u8>          volume = 255;        // ranges from 0 to 255

  AudioDevice(SDL_AudioDeviceID iid, AudioDeviceInfo iinfo, stx::Promise<void> ipromise, stx::Rc<DecodeContext *> ictx, SwrContext *iresampler, ResamplerConfig iresampler_cfg) :
      id{iid}, info{std::move(iinfo)}, promise{std::move(ipromise)}, ctx{std::move(ictx)}, decode_ctx{iresampler, iresampler_cfg}
  {}

  STX_MAKE_PINNED(AudioDevice)

  ~AudioDevice()
  {
    SDL_CloseAudioDevice(id);
  }

  // will be called on a different thread, use SDL_LockAudioDevice and SDL_UnlockAudioDevice to prevent it from running
  static void audio_callback(void *userdata, u8 *pstream, int len)
  {
    AudioDevice    *This   = AS(AudioDevice *, userdata);
    u8              volume = This->volume.load(std::memory_order_relaxed);
    nanoseconds     clock{0};
    stx::Span       stream{pstream, AS(usize, len)};
    usize           bytes_written  = 0;
    int             error          = 0;
    AVSampleFormat  sample_fmt     = AV_SAMPLE_FMT_NONE;
    AVChannelLayout channel_layout = AV_CHANNEL_LAYOUT_MONO;

    switch (This->info.spec.format)
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
        spdlog::error("encountered unsupported number of channels: {}", This->info.spec.channels);
        fill_silence(stream, This->info.spec.format);
        return;
      }
      break;
    }

    switch (This->info.spec.channels)
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
        spdlog::error("encountered unsupported number of channels: {}", This->info.spec.channels);
        fill_silence(stream, This->info.spec.format);
        return;
      }
      break;
    }

    while (bytes_written < len && This->promise.fetch_cancel_request() != stx::CancelState::Canceled)
    {
      if (This->decode_ctx.bytes_consumed != This->decode_ctx.samples.size())
      {
        usize bytes_left     = len - bytes_written;
        usize bytes_to_write = std::min(bytes_left, AS(usize, This->decode_ctx.samples.size() - This->decode_ctx.bytes_consumed));

        stream.slice(bytes_written).copy(This->decode_ctx.samples.span().slice(This->decode_ctx.bytes_consumed, bytes_to_write));
        bytes_written += bytes_to_write;
        This->decode_ctx.bytes_consumed += bytes_to_write;

        usize nsamples_written = (bytes_to_write / This->info.spec.channels) / av_get_bytes_per_sample(sample_fmt);

        clock = nanoseconds{This->decode_ctx.clock.load(std::memory_order_relaxed)};
        clock += nanoseconds{AS(nanoseconds::rep, 1'000'000'000LL * AS(f32, nsamples_written) / AS(f32, This->info.spec.freq))};
      }
      else
      {
        This->ctx->lock.lock();
        if (This->ctx->packets.is_empty())
        {
          This->ctx->lock.unlock();
          break;
        }

        AVPacket *packet = This->ctx->packets[0];
        This->ctx->packets.erase(This->ctx->packets.span().slice(0, 1));
        This->ctx->lock.unlock();

        if (packet->pts != AV_NOPTS_VALUE)
        {
          clock = timebase_to_ns(This->ctx->stream->time_base) * packet->pts;
        }

        error = avcodec_send_packet(This->ctx->ctx, packet);

        av_packet_free(&packet);

        if (error != 0)
        {
          fill_silence(stream, This->info.spec.format);
          break;
        }

        error = avcodec_receive_frame(This->ctx->ctx, This->ctx->frame);

        if (error != 0)
        {
          if (error == AVERROR(EOF))
          {
            This->promise.notify_completed();
          }
          fill_silence(stream, This->info.spec.format);
          break;
        }

        ResamplerConfig target_cfg{
            .src_fmt            = AS(AVSampleFormat, This->ctx->frame->format),
            .dst_fmt            = sample_fmt,
            .src_sample_rate    = This->ctx->frame->sample_rate,
            .dst_sample_rate    = This->info.spec.freq,
            .src_channel_layout = This->ctx->frame->ch_layout,
            .dst_channel_layout = channel_layout};

        if (This->decode_ctx.resampler_cfg != target_cfg || This->decode_ctx.resampler == nullptr)
        {
          if (This->decode_ctx.resampler != nullptr)
          {
            swr_free(&This->decode_ctx.resampler);
            This->decode_ctx.resampler = nullptr;
          }

          error = swr_alloc_set_opts2(&This->decode_ctx.resampler, &target_cfg.dst_channel_layout,
                                      target_cfg.dst_fmt,
                                      target_cfg.dst_sample_rate, &This->ctx->frame->ch_layout,
                                      target_cfg.src_fmt, target_cfg.src_sample_rate, 0, nullptr);

          if (error != 0)
          {
            fill_silence(stream, This->info.spec.format);
            ASH_LOG_FFMPEG_ERR(error);
            break;
          }

          This->decode_ctx.resampler_cfg = target_cfg;

          error = swr_init(This->decode_ctx.resampler);

          if (error != 0)
          {
            fill_silence(stream, This->info.spec.format);
            ASH_LOG_FFMPEG_ERR(error);
            break;
          }
        }

        int max_nsamples = swr_get_out_samples(This->decode_ctx.resampler, This->ctx->frame->nb_samples);

        if (max_nsamples < 0)
        {
          error = max_nsamples;
          fill_silence(stream, This->info.spec.format);
          ASH_LOG_FFMPEG_ERR(error);
          break;
        }

        int max_buffer_size = av_samples_get_buffer_size(nullptr, This->info.spec.channels, max_nsamples, target_cfg.dst_fmt, 1);

        if (max_buffer_size < 0)
        {
          error = max_buffer_size;
          fill_silence(stream, This->info.spec.format);
          ASH_LOG_FFMPEG_ERR(error);
          break;
        }

        This->decode_ctx.samples.resize(max_buffer_size).unwrap();

        u8 *out = This->decode_ctx.samples.data();

        int nsamples = swr_convert(This->decode_ctx.resampler, &out, max_nsamples, (u8 const **) This->ctx->frame->data, This->ctx->frame->nb_samples);

        av_frame_unref(This->ctx->frame);

        if (nsamples < 0)
        {
          error = nsamples;
          fill_silence(stream, This->info.spec.format);
          ASH_LOG_FFMPEG_ERR(error);
          break;
        }

        int buffer_size = av_samples_get_buffer_size(nullptr, This->info.spec.channels, nsamples, target_cfg.dst_fmt, 1);

        if (buffer_size < 0)
        {
          error = buffer_size;
          fill_silence(stream, This->info.spec.format);
          ASH_LOG_FFMPEG_ERR(error);
          break;
        }

        This->decode_ctx.samples.resize(buffer_size).unwrap();

        usize bytes_left     = len - bytes_written;
        usize bytes_to_write = std::min(bytes_left, AS(usize, buffer_size));

        stream.slice(bytes_written).copy(This->decode_ctx.samples.span().slice(0, bytes_to_write));

        This->decode_ctx.bytes_consumed = bytes_to_write;

        bytes_written += AS(int, bytes_to_write);

        usize nsamples_written = (bytes_to_write / This->info.spec.channels) / av_get_bytes_per_sample(target_cfg.dst_fmt);

        clock += nanoseconds{AS(nanoseconds::rep, 1'000'000'000LL * AS(f32, nsamples_written) / AS(f32, This->info.spec.freq))};
      }
    }

    Ashura_SDL_ScaleAudioFormat(stream, This->info.spec.format, This->volume.load(std::memory_order_relaxed));

    This->decode_ctx.clock.store(clock.count(), std::memory_order_relaxed);

    if (This->promise.fetch_cancel_request() == stx::CancelState::Canceled)
    {
      if (bytes_written < len)
      {
        fill_silence(stream.slice(bytes_written), This->info.spec.format);
      }
      This->promise.notify_canceled();
    }
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

  static stx::Option<stx::Rc<AudioDevice *>> open(AudioDeviceInfo const &info, u8 nchannels,
                                                  stx::Rc<DecodeContext *> const &ctx)
  {
    stx::Rc dev = stx::rc::make_inplace<AudioDevice>(stx::os_allocator, AS(SDL_AudioDeviceID, 0), AudioDeviceInfo{}, stx::make_promise<void>(stx::os_allocator).unwrap(), ctx.share(), nullptr, ResamplerConfig{}).unwrap();

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

void dump_ffmpeg_info()
{
  uint version = avformat_version();
  spdlog::info("FFMPEG avformat version: {}.{}.{}\n Available Codecs:", AV_VERSION_MAJOR(version), AV_VERSION_MINOR(version),
               AV_VERSION_MICRO(version));

  void          *iter  = nullptr;
  AVCodec const *codec = nullptr;

  do
  {
    codec = av_codec_iterate(&iter);
    if (codec != nullptr)
    {
      spdlog::info("name: {}, long name: {}, media type: {}", codec->name, codec->long_name, codec->type);
    }
  } while (codec != nullptr);
}

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

enum class VideoState
{
  NotLoaded
};

struct VideoPlayer;

enum class VideoSessionError
{
  NotLoaded
};

enum class SeekType
{
  Exact,
  Forward,
  Backward
};

// TODO(lamarrr): make loading audio/video optional
struct VideoSessionData
{
  stx::Rc<VideoDemuxer *>             demuxer;
  stx::Rc<DecodeContext *>            audio_decode_ctx;
  stx::Rc<DecodeContext *>            video_decode_ctx;
  stx::Rc<VideoDecodeContext *>       video_decode_ctx2;
  gfx::image                          image = 0;
  stx::Option<stx::Rc<AudioDevice *>> audio_device;
};

struct VideoSession
{
  template <typename T>
  using Result = stx::Result<T, VideoSessionError>;

  u64                           id = 0;
  stx::String                   path;
  VideoPlayer                  *player = nullptr;
  stx::Option<VideoSessionData> data;

  // TODO(lamarrr): we might not be able to tell yet as this is async
  Result<stx::Vec<usize>> get_video_streams();

  Result<stx::Vec<usize>> get_audio_streams();

  Result<stx::Vec<usize>> get_subtitles();

  Result<nanoseconds> get_duration();

  Result<nanoseconds> get_time();

  Result<usize> get_frame();

  Result<stx::Void> play(usize video_stream, usize audio_stream);

  Result<stx::Void> select_subtitle(usize);

  Result<stx::Void> pause();

  Result<stx::Void> stop();

  Result<stx::Void> seek_time(nanoseconds timepoint, SeekType seek = SeekType::Exact);

  Result<stx::Void> seek_frame(usize frame, SeekType seek = SeekType::Exact);

  Result<gfx::image> get_image();

  Result<gfx::image> get_preview_image();

  Result<gfx::image> get_subtitle();

  Result<stx::Void> seek_preview_at_time(nanoseconds timepoint);

  Result<stx::Void> seek_preview_at_frame(usize frame);

  Result<stx::Void> set_volume(u8 volume)
  {
  }

  Result<stx::Void> mute()
  {
    return set_volume(0);
  }

  Result<bool> is_playing();

  Result<bool> is_play_ended();
};

// TODO(lamarrr): handle audio device defaulting and updating
struct VideoPlayer : public Plugin
{
  virtual void on_startup(Context &context)
  {
    task_scheduler = context.task_scheduler;
    // open the audio device, handle audio device changing
  }

  virtual void tick(Context &context, std::chrono::nanoseconds interval)
  {}

  virtual void on_exit(Context &context)
  {}

  virtual std::string_view get_name()
  {
  }

  stx::Rc<VideoSession *> create_session(std::string_view source)
  {
    // TODO(lamarrr): do not do demuxing or stream seeking on main thread, it is not clear how long that will take
    stx::Rc session = stx::rc::make_inplace<VideoSession>(stx::os_allocator, next_session_id, stx::string::make(stx::os_allocator, source).unwrap(), this).unwrap();
    sessions.emplace(next_session_id, session.share());
    next_session_id++;
    // preferably don't use locks on the data? at leas the video session
    return session;
  }

  u64                                    next_session_id = 0;
  std::map<u64, stx::Rc<VideoSession *>> sessions;
  stx::Option<stx::Future<void>>         demuxer_promise;
  stx::Option<stx::Future<void>>         video_decode_promise;
  stx::TaskScheduler                    *task_scheduler = nullptr;
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

  dump_ffmpeg_info();

  stx::Rc      demuxer          = VideoDemuxer::from_file(argv[1]).unwrap();
  stx::Rc      audio_decode_ctx = demuxer->make_audio_decoder().unwrap();
  stx::Rc      video_decode_ctx = demuxer->make_video_decoder().unwrap();
  stx::Promise promise          = stx::make_promise<void>(stx::os_allocator).unwrap();
  stx::Rc      audio_device     = AudioDevice::open(dev, 2, audio_decode_ctx).unwrap();

  spdlog::info("opened device: {}, channels: {}, format: {}, samplerate: {}, nsamples: {}, size: {}, silence: {}", dev.name.c_str(), dev.spec.channels,
               dev.spec.format, dev.spec.freq, dev.spec.samples, audio_device->info.spec.size, (int) audio_device->info.spec.silence);

  audio_device->play();

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
          video_decode_ctx->push_packet(packet);
          video_decode_ctx->lock.unlock();
        }
        else if (packet->stream_index == audio_decode_ctx->stream->index)
        {
          audio_decode_ctx->lock.lock();
          audio_decode_ctx->push_packet(packet);
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
       audio_device     = audio_device.share(),
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
            nanoseconds delay = ctx->refresh(nanoseconds{audio_device->decode_ctx.clock.load(std::memory_order_relaxed)}, Clock::now());
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
