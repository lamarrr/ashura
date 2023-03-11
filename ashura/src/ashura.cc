#include "SDL3/SDL.h"
#include "aom/aom_decoder.h"
#include "aom/aomdx.h"
#include "aom/internal/aom_codec_internal.h"
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

  virtual int Read(long long position, long length, uchar *buffer) {
    if (position < 0 || length < 0) return -1;
    if (length == 0) return 0;
    if (position < 0) return -1;
    if (position >= this->buffer.size()) return -1;
    if (length > this->buffer.size()) return -1;

    std::copy(this->buffer.data(), this->buffer.data() + length, buffer);

    return 0;
  }

  virtual int Length(long long *total, long long *available) {
    *total = -1;
    *available = buffer.size();
  }

  void append(stx::Span<char const> buffer) { this->buffer = buffer; }

  void swap(stx::Span<char const> buffer) { this->buffer = buffer; }

  stx::Span<char const> buffer;
};

enum class AudioCodec : u8 { None, Opus, Vorbis, Unrecognized };

enum class VideoCodec : u8 { None, AV1, VP8, VP9, Unrecognized };

struct rational {
  i32 numerator = 0;
  i32 denominator = 0;
};

// struct VideoTrackInfo {
//   char const *name_utf8 = nullptr;
//   char const *codec = nullptr;
//   char const *codec_name_utf8 = nullptr;
//   char const *language = nullptr;
//   u32 width = 0;
//   u32 height = 0;
//   double framerate = 0;
//   long track_index = 0;
// };

// struct AudioTrackInfo {
//   char const *name_utf8 = nullptr;
//   char const *codec = nullptr;
//   char const *codec_name_utf8 = nullptr;
//   char const *language = nullptr;
//   long long nchannels = 0;
//   long long bit_depth = 0;
//   double sample_rate = 0;
//   long track_index = 0;
// };

struct Info {
  // stx::Vec<VideoTrackInfo> video_tracks{stx::os_allocator};
  // stx::Vec<AudioTrackInfo> audio_tracks{stx::os_allocator};
};

struct Context {
  Info info;
};

// NOTE: video res may change across segments
struct DecodeContext {
  int target_video_track_index = 0;
  MkvByteStream *reader = nullptr;
  mkvparser::Segment *segment = nullptr;
  mkvparser::Cluster const *cluster = nullptr;
  mkvparser::Block const *block = nullptr;
  mkvparser::BlockEntry const *block_entry = nullptr;
  bool reached_end_of_stream = 0;
  int block_frame_index = 0;
  stx::Memory frame_buffer_mem;
  usize frame_buffer_mem_size = 0;
  usize frame_buffer_size = 0;
  // for every call to read_frame()
  i64 timestamp_ns = 0;
  bool is_key_frame = 0;
};

constexpr int E_NOT_WEBM = -2048;

namespace webm {
// Workflow:
// - receive video stream
// - try to parse header, if there's no header formed yet x is returned
//   if there's an header, 0 status is returned
// - if 0 status when parsing, we parse the segment and
//
int try_parse_header(mkvparser::MkvReader &reader, long long &pos) {
  uchar bytes[4];
  constexpr uchar const webm_magic_number[] = {0x1A, 0x45, 0xDF, 0xa3};

  if (reader.Read(0, 4, bytes) < 0) return E_NOT_WEBM;

  // must be matroksa container
  if (!stx::Span{bytes}.equals<uchar const>(webm_magic_number))
    return E_NOT_WEBM;

  mkvparser::EBMLHeader header;

  return header.Parse(&reader, pos);
}

stx::Result<mkvparser::Segment *, int> try_parse_segment(
    Context &ctx, mkvparser::MkvReader &reader, long long &segment_start) {
  // try to read a segment from the EBML document
  mkvparser::Segment *segment;
  long long read =
      mkvparser::Segment::CreateInstance(&reader, segment_start, segment);
  if (read < 0) {
    return stx::Err(AS(int, read));
  }

  segment_start += read;

  int status = segment->Load();
  if (status < 0) {
    return stx::Err(AS(int, status));
  }

  return stx::Ok(AS(mkvparser::Segment *, segment));
  // mkvparser::Tracks const *tracks = ctx.segment->GetTracks();
  //
  // if (strncmp(codec_id, "V_AV1", 5) == 0) {
  //   ctx.info.video_codec = VideoCodec::AV1;
  // } else if (strncmp(codec_id, "V_VP9", 5) == 0) {
  //   ctx.info.video_codec = VideoCodec::VP9;
  // } else if (strncmp(codec_id, "V_VP8", 5) == 0) {
  //   ctx.info.video_codec = VideoCodec::VP8;
  // } else {
  //   ctx.info.video_codec = VideoCodec::Unrecognized;
  //   // log
  // }
  // if (strncmp(codec_id, "A_VORBIS", 8) == 0) {
  //   ctx.info.audio_codec = AudioCodec::Vorbis;
  // } else if (strncmp(codec_id, "A_OPUS", 6) == 0) {
  //   ctx.info.audio_codec = AudioCodec::Opus;
  // } else {
  //   ctx.info.audio_codec = AudioCodec::Unrecognized;
  //   // log
  // }
  // for (unsigned long i = 0; i < tracks->GetTracksCount(); ++i) {
  //   mkvparser::Track const *track = tracks->GetTrackByIndex(i);
  //   if (track->GetType() == mkvparser::Track::kVideo) {
  //     mkvparser::VideoTrack const *video_track =
  //         AS(mkvparser::VideoTrack const *, track);
  //     ctx.info.video_tracks
  //         .push(VideoTrackInfo{
  //             .name_utf8 = video_track->GetNameAsUTF8(),
  //             .codec = video_track->GetCodecId(),
  //             .codec_name_utf8 = video_track->GetCodecNameAsUTF8(),
  //             .language = video_track->GetLanguage(),
  //             .width = AS(u32, video_track->GetWidth()),
  //             .height = AS(u32, video_track->GetHeight()),
  //             .framerate = video_track->GetFrameRate(),
  //             .track_index = video_track->GetNumber()})
  //         .unwrap();
  //   }

  //   if (track->GetType() == mkvparser::Track::kAudio) {
  //     mkvparser::AudioTrack const *audio_track =
  //         AS(mkvparser::AudioTrack const *, track);
  //     ctx.info.audio_tracks
  //         .push(AudioTrackInfo{
  //             .name_utf8 = audio_track->GetNameAsUTF8(),
  //             .codec = audio_track->GetCodecId(),
  //             .codec_name_utf8 = audio_track->GetCodecNameAsUTF8(),
  //             .language = audio_track->GetLanguage(),
  //             .nchannels = audio_track->GetChannels(),
  //             .bit_depth = audio_track->GetBitDepth(),
  //             .sample_rate = audio_track->GetSamplingRate(),
  //             .track_index = audio_track->GetNumber()})
  //         .unwrap();
  //   }
  // }

  // // Chapters are a way to set predefined points to jump to in video or
  // audio. mkvparser::Chapters const *chapters = ctx.segment->GetChapters();

  // for (int i = 0; i < chapters->GetEditionCount(); i++) {
  //   mkvparser::Chapters::Edition const *edition = chapters->GetEdition(i);
  //   for (int j = 0; j < edition->GetAtomCount(); j++) {
  //     mkvparser::Chapters::Atom const *atom = edition->GetAtom(j);
  //     atom->GetStartTime(chapters);
  //     atom->GetStartTimecode();
  //     atom->GetStopTime(chapters);
  //     atom->GetStopTimecode();
  //     atom->GetStringUID();
  //     atom->GetUID();
  //     for (int k = 0; k < atom->GetDisplayCount(); k++) {
  //       mkvparser::Chapters::Display const *display = atom->GetDisplay(k);
  //       display->GetCountry();
  //       display->GetLanguage();
  //       display->GetString();
  //     }
  //   }
  // }

  // mkvparser::Tags const *tags = ctx.segment->GetTags();
  // for (int i = 0; i < tags->GetTagCount(); i++) {
  //   mkvparser::Tags::Tag const *tag = tags->GetTag(i);
  //   for (int j = 0; j < tag->GetSimpleTagCount(); j++) {
  //     mkvparser::Tags::SimpleTag const *simple_tag = tag->GetSimpleTag(j);
  //     simple_tag->GetTagName();
  //     simple_tag->GetTagString();
  //   }
  // }

  // get_first_cluster(webm_ctx);
}

#define ASH_YUV_2_RGB(y, u, v, k0, k1, k2, k3) \
  f32 r = y + k0 * (v - c);                    \
  f32 g = y - k1 * (u - c) - k2 * (v - c);     \
  f32 b = y + k3 * (u - c);

// // Define a struct to store the matrix coefficients and offset vector for
// YCbCr to RGB conversion typedef struct {
//   f32 m11; // coefficient for Y' * R
//   f32 m12; // coefficient for Cb * R
//   f32 m13; // coefficient for Cr * R
//   f32 m21; // coefficient for Y' * G
//   f32 m22; // coefficient for Cb * G
//   f32 m23; // coefficient for Cr * G
//   f32 m31; // coefficient for Y' * B
//   f32 m32; // coefficient for Cb * B
//   f32 m33; // coefficient for Cr * B
//   f32 o1; // offset for R
//   f32 o2; // offset for G
//   f32 o3; // offset for B
// } coeff_t;

// // Define an enum type that represents the supported matrix coefficients
// values from aom_matrix_coefficients_t typedef enum {
//  AOM_CICP_MC_IDENTITY = -0,
//  AOM_CICP_MC_BT_709 = -1,
//  AOM_CICP_MC_UNSPECIFIED = -2,
//  AOM_CICP_MC_RESERVED_3 = -3,
//  AOM_CICP_MC_FCC = -4,
//  AOM_CICP_MC_BT_470_B_G = -5,
//  AOM_CICP_MC_BT_601 = -6,
//  AOM_CICP_MC_SMPTE_240 = -7,
//  AOM_CICP_MC_SMPTE_YCGCO = -8,
//  AOM_CICP_MC_BT_2020_NCL = -9,
//  AOM_CICP_MC_BT_2020_CL = -10,
//  AOM_CICP_MC_SMPTE_2085 = -11,
//  AOM_CICP_MC_CHROMAT_NCL = -12,
//  AOM_CICP_MC_CHROMAT_CL = -13,
//  AOM_CICP_MC_ICTCP = -14
// } mc_t;

// // Define a lookup table that maps each mc_t value to its corresponding
// coeff_t value (assuming full range) coeff_t lookup_table[] =
// {
//  {1.000,0.000,0.000,0.000,1.000,0.000,0.000,0.000,-1.000,-0,-0,-0}, //
//  identity matrix (no conversion)
//  {1.164,-0,-0,-1.164,-0.213,-2.112,-1.164,-2.532,--1--2--3--4--5--6--7--8--9-10-11-12-13-14-15},
//  {-16}, {-17}, {-18}, {-19}, {-20}, {-21}, {-22}, {-23}, {-24}
// };

// // Define a function that converts a YCbCr pixel to RGB using a given mc_t
// value to select the coeff_t value from the lookup table rgb_pixel_t
// ycbcr_to_rgb(ycbcr_pixel_t ycbcr, mc_t mc) {
//   // Initialize an empty RGB pixel
//   rgb_pixel_t rgb = {0,0,0};

//   // Convert the YCbCr values to f32 for calculation
//   f32 y = (f32)ycbcr.y;
//   f32 cb = (f32)ycbcr.cb;
//   f32 cr = (f32)ycbcr.cr;

//   // Get the coeff_t value from the lookup table using the mc_t value as an
//   index coeff_t coeff = lookup_table[mc];

//   // Apply the matrix multiplication and offset vector addition using the
//   coeff_t values f32 r = y * coeff.m11 + cb * coeff.m12 + cr * coeff.m13 +
//   coeff.o1; f32 g = y * coeff.m21 + cb * coeff.m22 + cr * coeff.m23 +
//   coeff.o2; f32 b = y * coeff.m31 + cb * coeff.m32 + cr * coeff.m33 +
//   coeff.o3;

//    // Clip the values to fit in the range of [0,255]
//    if (r < -0) r = -0;
//    if (r > -255) r = -255;
//    if (g < -0) g = -0;
//    if (g > -255) g = -255;
//    if (b < -0) b = -0;
//    if (b > -255) b = -255;

//    // Round the values to integer and assign them to the RGB pixel
//    rgb.r = (uchar)(r + .5);
//    rgb.g = (uchar)(g + .5);
//    rgb.b = (uchar)(b + .5);

//    // Return the RGB pixel
//    return rgb;
// }

typedef struct {
  i16 vY;
  i16 vVr;
  i16 vVg;
  i16 vUg;
  i16 vUb;
} yuv_table_row;

static const yuv_table_row yuv_table[256] = {
    //  Y    Vr    Vg    Ug    Ub     // #
    {-18, -204, 50, 104, -258},  // 0
    {-17, -202, 49, 103, -256},  // 1
    {-16, -201, 49, 102, -254},  // 2
    {-15, -199, 48, 101, -252},  // 3
    {-13, -197, 48, 100, -250},  // 4
    {-12, -196, 48, 99, -248},   // 5
    {-11, -194, 47, 99, -246},   // 6
    {-10, -193, 47, 98, -244},   // 7
    {-9, -191, 46, 97, -242},    // 8
    {-8, -189, 46, 96, -240},    // 9
    {-6, -188, 46, 95, -238},    // 10
    {-5, -186, 45, 95, -236},    // 11
    {-4, -185, 45, 94, -234},    // 12
    {-3, -183, 44, 93, -232},    // 13
    {-2, -181, 44, 92, -230},    // 14
    {-1, -180, 44, 91, -228},    // 15
    {0, -178, 43, 91, -226},     // 16
    {1, -177, 43, 90, -223},     // 17
    {2, -175, 43, 89, -221},     // 18
    {3, -173, 42, 88, -219},     // 19
    {4, -172, 42, 87, -217},     // 20
    {5, -170, 41, 86, -215},     // 21
    {6, -169, 41, 86, -213},     // 22
    {8, -167, 41, 85, -211},     // 23
    {9, -165, 40, 84, -209},     // 24
    {10, -164, 40, 83, -207},    // 25
    {11, -162, 39, 82, -205},    // 26
    {12, -161, 39, 82, -203},    // 27
    {13, -159, 39, 81, -201},    // 28
    {15, -158, 38, 80, -199},    // 29
    {16, -156, 38, 79, -197},    // 30
    {17, -154, 37, 78, -195},    // 31
    {18, -153, 37, 78, -193},    // 32
    {19, -151, 37, 77, -191},    // 33
    {20, -150, 36, 76, -189},    // 34
    {22, -148, 36, 75, -187},    // 35
    {23, -146, 35, 74, -185},    // 36
    {24, -145, 35, 73, -183},    // 37
    {25, -143, 35, 73, -181},    // 38
    {26, -142, 34, 72, -179},    // 39
    {27, -140, 34, 71, -177},    // 40
    {29, -138, 34, 70, -175},    // 41
    {30, -137, 33, 69, -173},    // 42
    {31, -135, 33, 69, -171},    // 43
    {32, -134, 32, 68, -169},    // 44
    {33, -132, 32, 67, -167},    // 45
    {34, -130, 32, 66, -165},    // 46
    {36, -129, 31, 65, -163},    // 47
    {37, -127, 31, 65, -161},    // 48
    {38, -126, 30, 64, -159},    // 49
    {39, -124, 30, 63, -157},    // 50
    {40, -122, 30, 62, -155},    // 51
    {41, -121, 29, 61, -153},    // 52
    {43, -119, 29, 60, -151},    // 53
    {44, -118, 28, 60, -149},    // 54
    {45, -116, 28, 59, -147},    // 55
    {46, -114, 28, 58, -145},    // 56
    {47, -113, 27, 57, -143},    // 57
    {48, -111, 27, 56, -141},    // 58
    {50, -110, 26, 56, -139},    // 59
    {51, -108, 26, 55, -137},    // 60
    {52, -106, 26, 54, -135},    // 61
    {53, -105, 25, 53, -133},    // 62
    {54, -103, 25, 52, -131},    // 63
    {55, -102, 25, 52, -129},    // 64
    {57, -100, 24, 51, -127},    // 65
    {58, -98, 24, 50, -125},     // 66
    {59, -97, 23, 49, -123},     // 67
    {60, -95, 23, 48, -121},     // 68
    {61, -94, 23, 47, -119},     // 69
    {62, -92, 22, 47, -117},     // 70
    {64, -90, 22, 46, -115},     // 71
    {65, -89, 21, 45, -113},     // 72
    {66, -87, 21, 44, -110},     // 73
    {67, -86, 21, 43, -108},     // 74
    {68, -84, 20, 43, -106},     // 75
    {69, -82, 20, 42, -104},     // 76
    {71, -81, 19, 41, -102},     // 77
    {72, -79, 19, 40, -100},     // 78
    {73, -78, 19, 39, -98},      // 79
    {74, -76, 18, 39, -96},      // 80
    {75, -75, 18, 38, -94},      // 81
    {76, -73, 17, 37, -92},      // 82
    {77, -71, 17, 36, -90},      // 83
    {79, -70, 17, 35, -88},      // 84
    {80, -68, 16, 34, -86},      // 85
    {81, -67, 16, 34, -84},      // 86
    {82, -65, 16, 33, -82},      // 87
    {83, -63, 15, 32, -80},      // 88
    {84, -62, 15, 31, -78},      // 89
    {86, -60, 14, 30, -76},      // 90
    {87, -59, 14, 30, -74},      // 91
    {88, -57, 14, 29, -72},      // 92
    {89, -55, 13, 28, -70},      // 93
    {90, -54, 13, 27, -68},      // 94
    {91, -52, 12, 26, -66},      // 95
    {93, -51, 12, 26, -64},      // 96
    {94, -49, 12, 25, -62},      // 97
    {95, -47, 11, 24, -60},      // 98
    {96, -46, 11, 23, -58},      // 99
    {97, -44, 10, 22, -56},      // 100
    {98, -43, 10, 21, -54},      // 101
    {100, -41, 10, 21, -52},     // 102
    {101, -39, 9, 20, -50},      // 103
    {102, -38, 9, 19, -48},      // 104
    {103, -36, 8, 18, -46},      // 105
    {104, -35, 8, 17, -44},      // 106
    {105, -33, 8, 17, -42},      // 107
    {107, -31, 7, 16, -40},      // 108
    {108, -30, 7, 15, -38},      // 109
    {109, -28, 7, 14, -36},      // 110
    {110, -27, 6, 13, -34},      // 111
    {111, -25, 6, 13, -32},      // 112
    {112, -23, 5, 12, -30},      // 113
    {114, -22, 5, 11, -28},      // 114
    {115, -20, 5, 10, -26},      // 115
    {116, -19, 4, 9, -24},       // 116
    {117, -17, 4, 8, -22},       // 117
    {118, -15, 3, 8, -20},       // 118
    {119, -14, 3, 7, -18},       // 119
    {121, -12, 3, 6, -16},       // 120
    {122, -11, 2, 5, -14},       // 121
    {123, -9, 2, 4, -12},        // 122
    {124, -7, 1, 4, -10},        // 123
    {125, -6, 1, 3, -8},         // 124
    {126, -4, 1, 2, -6},         // 125
    {128, -3, 0, 1, -4},         // 126
    {129, -1, 0, 0, -2},         // 127
    {130, 0, 0, 0, 0},           // 128
    {131, 1, 0, 0, 2},           // 129
    {132, 3, 0, -1, 4},          // 130
    {133, 4, -1, -2, 6},         // 131
    {135, 6, -1, -3, 8},         // 132
    {136, 7, -1, -4, 10},        // 133
    {137, 9, -2, -4, 12},        // 134
    {138, 11, -2, -5, 14},       // 135
    {139, 12, -3, -6, 16},       // 136
    {140, 14, -3, -7, 18},       // 137
    {142, 15, -3, -8, 20},       // 138
    {143, 17, -4, -8, 22},       // 139
    {144, 19, -4, -9, 24},       // 140
    {145, 20, -5, -10, 26},      // 141
    {146, 22, -5, -11, 28},      // 142
    {147, 23, -5, -12, 30},      // 143
    {148, 25, -6, -13, 32},      // 144
    {150, 27, -6, -13, 34},      // 145
    {151, 28, -7, -14, 36},      // 146
    {152, 30, -7, -15, 38},      // 147
    {153, 31, -7, -16, 40},      // 148
    {154, 33, -8, -17, 42},      // 149
    {155, 35, -8, -17, 44},      // 150
    {157, 36, -8, -18, 46},      // 151
    {158, 38, -9, -19, 48},      // 152
    {159, 39, -9, -20, 50},      // 153
    {160, 41, -10, -21, 52},     // 154
    {161, 43, -10, -21, 54},     // 155
    {162, 44, -10, -22, 56},     // 156
    {164, 46, -11, -23, 58},     // 157
    {165, 47, -11, -24, 60},     // 158
    {166, 49, -12, -25, 62},     // 159
    {167, 51, -12, -26, 64},     // 160
    {168, 52, -12, -26, 66},     // 161
    {169, 54, -13, -27, 68},     // 162
    {171, 55, -13, -28, 70},     // 163
    {172, 57, -14, -29, 72},     // 164
    {173, 59, -14, -30, 74},     // 165
    {174, 60, -14, -30, 76},     // 166
    {175, 62, -15, -31, 78},     // 167
    {176, 63, -15, -32, 80},     // 168
    {178, 65, -16, -33, 82},     // 169
    {179, 67, -16, -34, 84},     // 170
    {180, 68, -16, -34, 86},     // 171
    {181, 70, -17, -35, 88},     // 172
    {182, 71, -17, -36, 90},     // 173
    {183, 73, -17, -37, 92},     // 174
    {185, 75, -18, -38, 94},     // 175
    {186, 76, -18, -39, 96},     // 176
    {187, 78, -19, -39, 98},     // 177
    {188, 79, -19, -40, 100},    // 178
    {189, 81, -19, -41, 102},    // 179
    {190, 82, -20, -42, 104},    // 180
    {192, 84, -20, -43, 106},    // 181
    {193, 86, -21, -43, 108},    // 182
    {194, 87, -21, -44, 110},    // 183
    {195, 89, -21, -45, 113},    // 184
    {196, 90, -22, -46, 115},    // 185
    {197, 92, -22, -47, 117},    // 186
    {199, 94, -23, -47, 119},    // 187
    {200, 95, -23, -48, 121},    // 188
    {201, 97, -23, -49, 123},    // 189
    {202, 98, -24, -50, 125},    // 190
    {203, 100, -24, -51, 127},   // 191
    {204, 102, -25, -52, 129},   // 192
    {206, 103, -25, -52, 131},   // 193
    {207, 105, -25, -53, 133},   // 194
    {208, 106, -26, -54, 135},   // 195
    {209, 108, -26, -55, 137},   // 196
    {210, 110, -26, -56, 139},   // 197
    {211, 111, -27, -56, 141},   // 198
    {213, 113, -27, -57, 143},   // 199
    {214, 114, -28, -58, 145},   // 200
    {215, 116, -28, -59, 147},   // 201
    {216, 118, -28, -60, 149},   // 202
    {217, 119, -29, -60, 151},   // 203
    {218, 121, -29, -61, 153},   // 204
    {219, 122, -30, -62, 155},   // 205
    {221, 124, -30, -63, 157},   // 206
    {222, 126, -30, -64, 159},   // 207
    {223, 127, -31, -65, 161},   // 208
    {224, 129, -31, -65, 163},   // 209
    {225, 130, -32, -66, 165},   // 210
    {226, 132, -32, -67, 167},   // 211
    {228, 134, -32, -68, 169},   // 212
    {229, 135, -33, -69, 171},   // 213
    {230, 137, -33, -69, 173},   // 214
    {231, 138, -34, -70, 175},   // 215
    {232, 140, -34, -71, 177},   // 216
    {233, 142, -34, -72, 179},   // 217
    {235, 143, -35, -73, 181},   // 218
    {236, 145, -35, -73, 183},   // 219
    {237, 146, -35, -74, 185},   // 220
    {238, 148, -36, -75, 187},   // 221
    {239, 150, -36, -76, 189},   // 222
    {240, 151, -37, -77, 191},   // 223
    {242, 153, -37, -78, 193},   // 224
    {243, 154, -37, -78, 195},   // 225
    {244, 156, -38, -79, 197},   // 226
    {245, 158, -38, -80, 199},   // 227
    {246, 159, -39, -81, 201},   // 228
    {247, 161, -39, -82, 203},   // 229
    {249, 162, -39, -82, 205},   // 230
    {250, 164, -40, -83, 207},   // 231
    {251, 165, -40, -84, 209},   // 232
    {252, 167, -41, -85, 211},   // 233
    {253, 169, -41, -86, 213},   // 234
    {254, 170, -41, -86, 215},   // 235
    {256, 172, -42, -87, 217},   // 236
    {257, 173, -42, -88, 219},   // 237
    {258, 175, -43, -89, 221},   // 238
    {259, 177, -43, -90, 223},   // 239
    {260, 178, -43, -91, 226},   // 240
    {261, 180, -44, -91, 228},   // 241
    {263, 181, -44, -92, 230},   // 242
    {264, 183, -44, -93, 232},   // 243
    {265, 185, -45, -94, 234},   // 244
    {266, 186, -45, -95, 236},   // 245
    {267, 188, -46, -95, 238},   // 246
    {268, 189, -46, -96, 240},   // 247
    {270, 191, -46, -97, 242},   // 248
    {271, 193, -47, -98, 244},   // 249
    {272, 194, -47, -99, 246},   // 250
    {273, 196, -48, -99, 248},   // 251
    {274, 197, -48, -100, 250},  // 252
    {275, 199, -48, -101, 252},  // 253
    {277, 201, -49, -102, 254},  // 254
    {278, 202, -49, -103, 256}   // 255
};

#define YUYV_CONSTRAIN(v) ((v) < 0) ? 0 : (((v) > 255) ? 255 : (v))

void yuv2rgb(u8 y, u8 u, u8 v, u8 *r, u8 *g, u8 *b) {
  i16 ri = yuv_table[y].vY + yuv_table[v].vVr;
  i16 gi = yuv_table[y].vY + yuv_table[u].vUg + yuv_table[v].vVg;
  i16 bi = yuv_table[y].vY + yuv_table[u].vUb;

  *r = YUYV_CONSTRAIN(ri);
  *g = YUYV_CONSTRAIN(gi);
  *b = YUYV_CONSTRAIN(bi);
}

#define AOM_U8_CLAMP(...)    \
  ((__VA_ARGS__) < 0 ? (u8)0 \
                     : ((__VA_ARGS__) > 255 ? (u8)255 : (u8)(__VA_ARGS__)))

//
// TODO(lamarrr): add stride to rgb so we can set the alpha to 0xff ahead of
// time
//
//
// all planar format meaning stored in separate arrays
// AOM_IMG_FMT_YV12
// Y: 8 bpp
// U
void aom_yvu_420_12_to_rgb(aom_image_t *img, u8 const *rgb, bool output_alpha,
                           mat3 const &coefficient) {
  u8 const *y_plane = img->planes[AOM_PLANE_Y];
  u8 const *u_plane = img->planes[AOM_PLANE_U];
  u8 const *v_plane = img->planes[AOM_PLANE_V];
  int y_stride = img->stride[AOM_PLANE_Y];
  int u_stride = img->stride[AOM_PLANE_U];
  int v_stride = img->stride[AOM_PLANE_V];
  int y_width = aom_img_plane_width(img, AOM_PLANE_Y);
  int u_width = aom_img_plane_width(img, AOM_PLANE_U);
  int v_width = aom_img_plane_width(img, AOM_PLANE_V);
  int y_height = aom_img_plane_width(img, AOM_PLANE_Y);
  int u_height = aom_img_plane_height(img, AOM_PLANE_U);
  int v_height = aom_img_plane_height(img, AOM_PLANE_V);
  // bool has_color = u_width != 0 && v_width != 0;

  for (usize i = 0; i < y_width / 4; i++) {
    u8 y0_1 = y_plane[0];
    u8 y0_1 = y_plane[1];
    u8 y0_2 = y_plane[2];
    u8 y0_3 = y_plane[3];

    u8 y1_0 = (y_plane + y_stride)[0];
    u8 y1_1 = (y_plane + y_stride)[1];
    u8 y1_2 = (y_plane + y_stride)[2];
    u8 y1_3 = (y_plane + y_stride)[3];

    // TODO(lamarrr): check for correct values

    u8 u0_0_s = u_plane[0];
    u8 v0_0_s = v_plane[0];

    u8 u0_1_s = u_plane[1];
    u8 v0_1_s = v_plane[1];

    u8 u1_0_s = u_plane[0];
    u8 v1_0_s = v_plane[0];

    u8 u1_1_s = u_plane[1];
    u8 v1_1_s = v_plane[1];

    u8 u0 = u0_s;
    u8 v0 = v0_s;
    u8 u1 = u0_s;
    u8 v1 = v0_s;
    u8 u2 = u1_s;
    u8 v2 = v1_s;
    u8 u3 = u1_s;
    u8 v3 = v1_s;

    u8 r0 = y0;
    u8 g0 = u0;
    u8 b0 = v0;
    u8 r1 = y1;
    u8 g1 = u1;
    u8 b1 = v1;
    u8 r2 = y2;
    u8 g2 = u2;
    u8 b2 = v2;
    u8 r3 = y3;
    u8 g3 = u3;
    u8 b3 = v3;
  }
}

// AOM_IMG_FMT_I420
void aom_yuv_420_12_to_rgb(aom_image_t *img, u8 const *rgb, bool output_alpha) {
}

// AOM_IMG_FMT_AOMYV12
void aom_yvu_420_12_aom_to_rgb(aom_image_t *img, u8 const *rgb,
                               bool output_alpha) {}

// AOM_IMG_FMT_AOMI420
void aom_yuv_420_12_aom_to_rgb(aom_image_t *img, u8 const *rgb,
                               bool output_alpha) {}

// AOM_IMG_FMT_I422
void aom_yuv_422_12_to_rgb(aom_image_t *img, u8 const *rgb, bool output_alpha) {
}

// AOM_IMG_FMT_I444
void aom_yuv_444_12_to_rgb(aom_image_t *img, u8 const *rgb, bool output_alpha) {
}

// AOM_IMG_FMT_NV12
void aom_yuv_nv_420_12_to_rgb(aom_image_t *img, u8 const *rgb,
                              bool output_alpha) {}

// AOM_IMG_FMT_I42016
void aom_yuv_420_16_to_rgb(aom_image_t *img, u8 const *rgb, bool output_alpha) {
}

// AOM_IMG_FMT_YV1216
void aom_yvu_420_16_to_rgb(aom_image_t *img, u8 const *rgb, bool output_alpha) {
}

// AOM_IMG_FMT_I42216
void aom_yuv_422_16_to_rgb(aom_image_t *img, u8 const *rgb, bool output_alpha) {
}

// AOM_IMG_FMT_I44416
void aom_yuv_444_16_to_rgb(aom_image_t *img, u8 const *rgb, bool output_alpha) {
}

constexpr mat3 make_yuv2rgb_color_matrix(f32 kr, f32 kb) {
  f32 kg = 1 - kr - kb;
  return mat3{
      vec3{1, 0, 2 - 2 * kr},
      vec3{1, -(kb / kg) * (2 - 2 * kb), -(kr / kg) * (2 - 2 * kr)},
      vec3{1, 2 - 2 * kb, 0},
  };
}

constexpr mat3 yuv2rgb_matrix_coefficients[16] = {
    // AOM_CICP_MC_IDENTITY:
    mat3::identity(),
    // AOM_CICP_MC_BT_709: Table E-5 ITU-T Rec. H.264 (06/2019) Advanced video
    // coding for generic audiovisual services
    make_yuv2rgb_color_matrix(0.2126f, 0.0722f),
    // AOM_CICP_MC_UNSPECIFIED
    mat3::identity(),
    // AOM_CICP_MC_RESERVED_3
    mat3::identity(),
    // AOM_CICP_MC_FCC: Table E-5 ITU-T Rec. H.264 (06/2019) Advanced video
    // coding for generic audiovisual services
    make_yuv2rgb_color_matrix(0.30f, 0.11f),
    // AOM_CICP_MC_BT_470_B_G: Table E-5 ITU-T Rec. H.264 (06/2019) Advanced
    // video coding for generic audiovisual services
    make_yuv2rgb_color_matrix(0.299f, 0.114f),
    // AOM_CICP_MC_BT_601: Table E-5 ITU-T Rec. H.264 (06/2019) Advanced video
    // coding for generic audiovisual services
    make_yuv2rgb_color_matrix(0.299f, 0.114f),
    // AOM_CICP_MC_SMPTE_240:
    // https://www5.in.tum.de/lehre/vorlesungen/graphik/info/csc/COL_33.htm
    make_yuv2rgb_color_matrix(0.212f, 0.087f),
    // AOM_CICP_MC_SMPTE_YCGCO: Table E-5 ITU-T Rec. H.264 (06/2019) Advanced
    // video coding for generic audiovisual services
    make_yuv2rgb_color_matrix(0.2122f, 0.0865f),
    // AOM_CICP_MC_BT_2020_NCL: Table E-5 ITU-T Rec. H.264 (06/2019) Advanced
    // video coding for generic audiovisual services
    make_yuv2rgb_color_matrix(0.2627f, 0.0593f),
    // AOM_CICP_MC_BT_2020_CL: Table E-5 ITU-T Rec. H.264 (06/2019) Advanced
    // video coding for generic audiovisual services
    make_yuv2rgb_color_matrix(0.2627f, 0.0593f),
    // AOM_CICP_MC_SMPTE_2085: can't be represented
    mat3::identity(),
    // AOM_CICP_MC_CHROMAT_NCL: non-linear transformation
    make_yuv2rgb_color_matrix(000, 000),
    // AOM_CICP_MC_CHROMAT_CL: non-linear transformation
    mat3::identity(),
    // AOM_CICP_MC_ICTCP: invalid
    mat3::identity(),
    // AOM_CICP_MC_RESERVED_15
    mat3::identity()
    // [Future Use] 16-255
};

int xxxxxxxdecoder_init(DecodeContext &context, usize frame_size_bytes) {
  aom_codec_iface_t *decoder = aom_codec_av1_dx();
  decoder->name;
  aom_codec_ctx_t codec;
  aom_codec_dec_init(&codec, decoder, nullptr, 0);
  if (aom_codec_decode(&codec, AS(u8 const *, context.frame_buffer_mem.handle),
                       frame_size_bytes, nullptr)) {
    aom_codec_error_detail(&codec);
    return -1;
  }

  aom_codec_iter_t iter = nullptr;
  aom_image_t *img = aom_codec_get_frame(&codec, &iter);
  if (img == nullptr) return -1;
  u8 const *y_plane = img->planes[AOM_PLANE_Y];
  u8 const *u_plane = img->planes[AOM_PLANE_U];
  u8 const *v_plane = img->planes[AOM_PLANE_V];
  int y_stride = img->stride[AOM_PLANE_Y];
  int u_stride = img->stride[AOM_PLANE_U];
  int v_stride = img->stride[AOM_PLANE_V];
  int y_width = aom_img_plane_width(img, AOM_PLANE_Y);
  int u_width = aom_img_plane_width(img, AOM_PLANE_U);
  int v_width = aom_img_plane_width(img, AOM_PLANE_V);
  int y_height = aom_img_plane_width(img, AOM_PLANE_Y);
  int u_height = aom_img_plane_height(img, AOM_PLANE_U);
  int v_height = aom_img_plane_height(img, AOM_PLANE_V);
  bool has_color = u_width != 0 && v_width != 0;
  // normalize using img->bps, 12 bit, 16-bit

  ImageBuffer buffer{
      .memory = stx::mem::allocate(stx::os_allocator,
                                   AS(usize, y_width) * AS(usize, y_height) * 4)
                    .unwrap(),
      .extent = extent{.width = AS(u32, y_width), .height = AS(u32, y_height)},
      .format = ImageFormat::Rgba};

  switch (img->fmt) {
    case AOM_IMG_FMT_NONE:
      break;
    case AOM_IMG_FMT_YV12:
      aom_yvu_420_12_to_rgb(y_plane, u_plane, v_plane, );
      break;
    case AOM_IMG_FMT_I420:
      aom_yuv_420_12_to_rgb();
      break;
    case AOM_IMG_FMT_AOMYV12:
      aom_yvu_420_12_aom_to_rgb();
      break;
    case AOM_IMG_FMT_AOMI420:
      aom_yuv_420_12_aom_to_rgb();
      break;
    case AOM_IMG_FMT_I422:
      aom_yuv_422_12_to_rgb();
      break;
    case AOM_IMG_FMT_I444:
      aom_yuv_444_12_to_rgb();
      break;
    case AOM_IMG_FMT_NV12:
      aom_yuv_nv_420_12_to_rgb();
      break;
    case AOM_IMG_FMT_I42016:
      aom_yuv_420_16_to_rgb();
      break;
    case AOM_IMG_FMT_YV1216:
      aom_yvu_420_16_to_rgb();
      break;
    case AOM_IMG_FMT_I42216:
      aom_yuv_422_16_to_rgb();
      break;
    case AOM_IMG_FMT_I44416:
      break;
      aom_yuv_444_16_to_rgb();
    default:
      break;
  }

  //  for (int y = 0; y < height; y++) {
  // read buffer, buffer + width
  // buffer += stride;
  // }

  return 0;
}

// As an example, a simple Matroska file consisting of a single EBML Document
// could be represented like this:
//
// EBML Header
// Segment
//
//
// A more complex Matroska file consisting of an EBML Stream (consisting of two
// EBML Documents) could be represented like this:
//
// EBML Header
// Segment
// EBML Header
// Segment
int read_frame(DecodeContext &dec_ctx, long &bytes_read) {
  bytes_read = 0;
  // This check is needed for frame parallel decoding, in which case this
  // function could be called even after it has reached end of input stream.
  if (dec_ctx.reached_end_of_stream) {
    return 1;
  }

  mkvparser::Segment *segment = dec_ctx.segment;
  mkvparser::Cluster const *cluster = dec_ctx.cluster;
  mkvparser::Block const *block = dec_ctx.block;
  mkvparser::BlockEntry const *block_entry = dec_ctx.block_entry;

  // each frame belongs to a block_entry which
  // belongs to a block which belongs to a cluster
  bool block_entry_eos = false;
  do {
    long status = 0;
    bool get_new_block = false;
    if (block_entry == nullptr && !block_entry_eos) {
      status = cluster->GetFirst(block_entry);
      get_new_block = true;
    } else if (block_entry_eos || block_entry->EOS()) {
      cluster = segment->GetNext(cluster);
      if (cluster == nullptr || cluster->EOS()) {
        bytes_read = 0;
        dec_ctx.reached_end_of_stream = true;
        return 1;
      }
      status = cluster->GetFirst(block_entry);
      block_entry_eos = false;
      get_new_block = true;
    } else if (block == nullptr ||
               dec_ctx.block_frame_index == block->GetFrameCount() ||
               block->GetTrackNumber() != dec_ctx.target_video_track_index) {
      status = cluster->GetNext(block_entry, block_entry);
      if (block_entry == nullptr || block_entry->EOS()) {
        block_entry_eos = true;
        continue;
      }
      get_new_block = true;
    }
    if (status || block_entry == nullptr) {
      return -1;
    }
    if (get_new_block) {
      block = block_entry->GetBlock();
      if (block == nullptr) return -1;
      dec_ctx.block_frame_index = 0;
    }
  } while (block_entry_eos ||
           block->GetTrackNumber() != dec_ctx.target_video_track_index);

  dec_ctx.cluster = cluster;
  dec_ctx.block_entry = block_entry;
  dec_ctx.block = block;

  mkvparser::Block::Frame const &frame =
      block->GetFrame(dec_ctx.block_frame_index);
  dec_ctx.block_frame_index++;
  if (frame.len > static_cast<long>(dec_ctx.frame_buffer_mem_size)) {
    stx::mem::reallocate(dec_ctx.frame_buffer_mem, frame.len).unwrap();
    dec_ctx.frame_buffer_mem_size = frame.len;
    dec_ctx.frame_buffer_size = frame.len;
  }
  bytes_read = frame.len;
  dec_ctx.timestamp_ns = block->GetTime(cluster);
  dec_ctx.is_key_frame = block->IsKey();

  return frame.Read(dec_ctx.reader,
                    AS(uchar *, dec_ctx.frame_buffer_mem.handle))
             ? -1
             : 0;
}

void decode_webm(AvxInputContext *input_ctx, WebmInputContext *webm_ctx,
                 char const *path) {
  std::ifstream stream{path, std::ios::binary | std::ios::ate};

  stx::Vec<char> bytes{stx::os_allocator};
  bytes.resize(stream.tellg()).unwrap();
  stream.seekg(0);
  stream.read(bytes.data(), bytes.size());

  // ASH_CHECK()
}

}  // namespace webm

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
