#include "SDL3/SDL.h"
#include "aom/aom_decoder.h"
#include "aom/aomdx.h"
#include "aom_ports/mem_ops.h"
#include "ashura/app.h"
#include "ashura/widget.h"
#include "ashura/widgets/image.h"
#include "common/ivfdec.h"
#include "common/obudec.h"
#include "common/tools_common.h"
#include "common/video_reader.h"
#include "common/webmdec.h"
#include "third_party/libwebm/mkvparser/mkvparser.h"
#include "third_party/libwebm/mkvparser/mkvreader.h"

using namespace ash;

struct MkvByteStream : public mkvparser::IMkvReader {
  virtual ~MkvByteStream() {}

  virtual int Read(long long position, long length, unsigned char *buffer) {
    if (position < 0 || length < 0) return -1;
    if (length == 0) return 0;
    if (position < 0) return -1;
    if (position >= this->buffer.size()) return -1;
    if (length > this->buffer.size()) return -1;

    std::memcpy(buffer, this->buffer.data(), length);

    return 0;
  }

  virtual int Length(long long *total, long long *available) {
    *total = -1;
    *available = buffer.size();
  }

  void append(stx::Span<char const> buffer) { this->buffer = buffer; }

  void swap(stx::Span<char const> buffer) { this->buffer = buffer; }

 FILE* file = nullptr;
};

enum class AudioCodec : u8 { None, Opus, Vorbis, Unrecognized };

enum class VideoCodec : u8 { None, AV1, VP8, VP9, Unrecognized };

struct rational {
  i32 numerator = 0;
  i32 denominator = 0;
};

struct VideoTrackInfo {};

struct WebMInfo {
  VideoCodec video_codec = VideoCodec::None;
  AudioCodec audio_codec = AudioCodec::None;
  u32 width = 0;
  u32 height = 0;
  rational framerate;
  u32 video_track_index = 0;
  stx::Option<u32> audio_track_index = stx::None;
};

struct WebMContext {
  WebMInfo info;
  mkvparser::Segment *segment = nullptr;
};

enum class WebMStreamError { None, TryAgain, NotWebM };

WebMStreamError is_webm_file(mkvparser::MkvReader &reader, WebMContext &ctx) {
  unsigned char bytes[4];

  if (reader.Read(0, 4, bytes) < 0) return WebMStreamError::TryAgain;

  // must be maktrosa container
  if (bytes[0] != 0x1A || bytes[1] != 0x45 || bytes[2] != 0xDF ||
      bytes[3] != 0xA3)
    return WebMStreamError::NotWebM;

  mkvparser::EBMLHeader header;
  long long pos = 0;

  if (header.Parse(&reader, pos) < 0) {
    return {};
  }

  if (mkvparser::Segment::CreateInstance(&reader, pos, ctx.segment)) {
    rewind_and_reset(webm_ctx, aom_ctx);
    return 0;
  }

  if (segment->Load() < 0) {
    rewind_and_reset(webm_ctx, aom_ctx);
    return 0;
  }

  mkvparser::Tracks const *tracks = segment->GetTracks();

  for (unsigned long i = 0; i < tracks->GetTracksCount(); ++i) {
    mkvparser::Track const *track = tracks->GetTrackByIndex(i);
    if (track->GetType() == mkvparser::Track::kVideo) {
      mkvparser::VideoTrack const *video_track =
          AS(mkvparser::VideoTrack const *, track);
      ctx.info.video_track_index = AS(i32, track->GetNumber());
      ctx.info.framerate.denominator = 0;
      ctx.info.framerate.numerator = 0;
      ctx.info.width = AS(u32, video_track->GetWidth());
      ctx.info.height = AS(u32, video_track->GetHeight());

      char const *codec_id = video_track->GetCodecId();
      ASH_CHECK(codec_id != nullptr);

      if (strncmp(codec_id, "V_AV1", 5) == 0) {
        ctx.info.video_codec = VideoCodec::AV1;
      } else if (strncmp(codec_id, "V_VP9", 5) == 0) {
        ctx.info.video_codec = VideoCodec::VP9;
      } else if (strncmp(codec_id, "V_VP8", 5) == 0) {
        ctx.info.video_codec = VideoCodec::VP8;
      } else {
        ctx.info.video_codec = VideoCodec::Unrecognized;
        // log
      }
    }

    if (track->GetType() == mkvparser::Track::kAudio) {
      mkvparser::AudioTrack const *audio_track =
          AS(mkvparser::AudioTrack const *, track);
      // audio_track->GetCodecId();
      // audio_track->GetChannels();
      // audio_track->GetBitDepth();
      // audio_track->GetSamplingRate();
      // audio_track->GetType();
      // track->GetNumber();

      char const *codec_id = audio_track->GetCodecId();
      ASH_CHECK(codec_id != nullptr);

      if (strncmp(codec_id, "A_VORBIS", 8) == 0) {
        ctx.info.audio_codec = AudioCodec::Vorbis;
      } else if (strncmp(codec_id, "A_OPUS", 6) == 0) {
        ctx.info.audio_codec = AudioCodec::Opus;
      } else {
        ctx.info.audio_codec = AudioCodec::Unrecognized;
        // log
      }
    }
  }

  if (video_track == NULL || video_track->GetCodecId() == NULL) {
    rewind_and_reset(webm_ctx, aom_ctx);
    return 0;
  }

  if (!strncmp(video_track->GetCodecId(), "V_AV1", 5)) {
    aom_ctx->fourcc = AV1_FOURCC;
  } else {
    rewind_and_reset(webm_ctx, aom_ctx);
    return 0;
  }

  get_first_cluster(webm_ctx);

  return WebMStreamError::None;
}

void decode_webm(AvxInputContext *input_ctx, WebmInputContext *webm_ctx,
                 char const *path) {
  std::ifstream stream{path, std::ios::binary | std::ios::ate};

  stx::Vec<char> bytes{stx::os_allocator};
  bytes.resize(stream.tellg()).unwrap();
  stream.seekg(0);
  stream.read(bytes.data(), bytes.size());




  //ASH_CHECK()
}

int main(int argc, char **argv) {
  ASH_CHECK(argc == 2);

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
