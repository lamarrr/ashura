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
#include "webm/buffer_reader.h"
#include "webm/callback.h"
#include "webm/file_reader.h"
#include "webm/reader.h"
#include "webm/webm_parser.h"

namespace ash {
constexpr mat3 make_yuv2rgb_color_matrix(f32 kr, f32 kb) {
  f32 kg = 1 - kr - kb;
  return mat3{
      vec3{1, 0, 2 - 2 * kr},
      vec3{1, -(kb / kg) * (2 - 2 * kb), -(kr / kg) * (2 - 2 * kr)},
      vec3{1, 2 - 2 * kb, 0},
  };
}

// Y′ ranges from 16 to 235.
// Cb and Cr range from 16 to 240.
// THE values from 0 to 15 are called footroom, while the values from 236 to 255
// are called headroom. https://en.wikipedia.org/wiki/YCbCr
//
//
// Y ranges from 0 to 1
// Pb ranges from -0.5 to 0.5
// Pr ranges from -0.5 to 0.5
//
// YPbPr is for analog images
// Y'CbCr is for digital images
//
// Conversion from analog YPbPr to digital 8-bit YCbCr ->
//
// Y' = 16 + 219Y
// Cb = 128 + 224Pb
// Cr = 128 + 224Pr
//
//
// Conversion from analog YPbPr to digital 16-bit YCbCr, by 16-bit to 8-bit
// scaling ->
//
// Y' = 65535/255 * (16 + 219Y)
// Cb = 65535/255 * (128 + 224Pb)
// Cr = 65535/255 * (128 + 224Pr)
//
constexpr mat3 ypbpr2rgb_coefficients[16] = {
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
    mat3::identity(),
    // AOM_CICP_MC_CHROMAT_CL: non-linear transformation
    mat3::identity(),
    // AOM_CICP_MC_ICTCP: invalid
    mat3::identity(),
    // AOM_CICP_MC_RESERVED_15
    mat3::identity()
    // [Future Use] 16-255
};

constexpr void yuv2rgb_bp12(mat3 const &coefficient, u8 y, u8 u, u8 v,
                            u8 *rgb) {
  vec3 r = coefficient *
           vec3{AS(f32, y) - 16, AS(f32, u) - 128, AS(f32, v) - 128} *
           vec3{255, 255, 255};
  rgb[0] = ASH_U8_CLAMP(r.x);
  rgb[1] = ASH_U8_CLAMP(r.y);
  rgb[2] = ASH_U8_CLAMP(r.z);
}

constexpr void yuv2rgb_bp16(mat3 const &coefficient, u16 y, u16 u, u16 v,
                            u8 *rgb) {
  vec3 r = coefficient *
           vec3{AS(f32, y) - 16 * 257, AS(f32, u) - 128 * 257,
                AS(f32, v) - 128 * 257} *
           vec3{255, 255, 255};
  rgb[0] = ASH_U8_CLAMP(r.x);
  rgb[1] = ASH_U8_CLAMP(r.y);
  rgb[2] = ASH_U8_CLAMP(r.z);
}

// planar format meaning y, u, and v stored in separate arrays
//
// AOM_IMG_FMT_YV12
// AOM_IMG_FMT_I420
// AOM_IMG_FMT_AOMI420
// AOM_IMG_FMT_AOMYV12
//
void yuv_420_12_to_rgb(aom_image_t const *img, u8 *rgb) {
  int PLANE_U = (img->fmt & AOM_IMG_FMT_UV_FLIP) == AOM_IMG_FMT_UV_FLIP
                    ? AOM_PLANE_V
                    : AOM_PLANE_U;
  int PLANE_V = (img->fmt & AOM_IMG_FMT_UV_FLIP) == AOM_IMG_FMT_UV_FLIP
                    ? AOM_PLANE_U
                    : AOM_PLANE_V;

  u8 const *y_plane = img->planes[AOM_PLANE_Y];
  u8 const *u_plane = img->planes[PLANE_U];
  u8 const *v_plane = img->planes[PLANE_V];
  int y_stride = img->stride[AOM_PLANE_Y];
  int u_stride = img->stride[PLANE_U];
  int v_stride = img->stride[PLANE_V];
  int y_width = aom_img_plane_width(img, AOM_PLANE_Y);
  int u_width = aom_img_plane_width(img, PLANE_U);
  int v_width = aom_img_plane_width(img, PLANE_V);
  int y_height = aom_img_plane_height(img, AOM_PLANE_Y);
  int u_height = aom_img_plane_height(img, PLANE_U);
  int v_height = aom_img_plane_height(img, PLANE_V);

  ASH_CHECK(u_width == v_width);
  ASH_CHECK(y_width == 2 * u_width);
  ASH_CHECK(u_height == v_height);
  ASH_CHECK(y_height == 2 * u_height);

  mat3 coefficient = ypbpr2rgb_coefficients[img->fmt];

  for (usize j = 0; j < y_height / 2; j++) {
    for (usize i = 0; i < y_width / 4; i++) {
      u8 y0_0 = y_plane[0];
      u8 y0_1 = y_plane[1];
      u8 y0_2 = y_plane[2];
      u8 y0_3 = y_plane[3];

      u8 y1_0 = (y_plane + y_stride)[0];
      u8 y1_1 = (y_plane + y_stride)[1];
      u8 y1_2 = (y_plane + y_stride)[2];
      u8 y1_3 = (y_plane + y_stride)[3];

      u8 u0_0 = u_plane[0];
      u8 v0_0 = v_plane[0];
      u8 u0_1 = u0_0;
      u8 v0_1 = v0_0;
      u8 u0_2 = u_plane[1];
      u8 v0_2 = v_plane[1];
      u8 u0_3 = u0_2;
      u8 v0_3 = v0_2;

      u8 u1_0 = u0_0;
      u8 v1_0 = v0_0;
      u8 u1_1 = u0_1;
      u8 v1_1 = v0_1;
      u8 u1_2 = u0_2;
      u8 v1_2 = v0_2;
      u8 u1_3 = u0_3;
      u8 v1_3 = v0_3;

      yuv2rgb_bp12(coefficient, y0_0, u0_0, v0_0, rgb);
      yuv2rgb_bp12(coefficient, y0_1, u0_1, v0_1, rgb + 3);
      yuv2rgb_bp12(coefficient, y0_2, u0_2, v0_2, rgb + 6);
      yuv2rgb_bp12(coefficient, y0_3, u0_3, v0_3, rgb + 6);
      yuv2rgb_bp12(coefficient, y1_0, u1_0, v1_0, rgb + 3 * y_width);
      yuv2rgb_bp12(coefficient, y1_1, u1_1, v1_1, rgb + 3 * y_width + 3);
      yuv2rgb_bp12(coefficient, y1_2, u1_2, v1_2, rgb + 3 * y_width + 6);
      yuv2rgb_bp12(coefficient, y1_3, u1_3, v1_3, rgb + 3 * y_width + 9);

      y_plane += 4;
      u_plane += 2;
      v_plane += 2;
      rgb += 4 * 3;
    }

    y_plane += (y_stride - y_width) + y_stride;
    u_plane += (u_stride - u_width);
    u_plane += (u_stride - u_width);
    rgb += y_width * 3;
  }
}

// AOM_IMG_FMT_NV12
void yuv_420_nv12_to_rgb(aom_image_t const *img, u8 *rgb) {
  u8 const *y_plane = img->planes[AOM_PLANE_Y];
  u8 const *uv_plane = img->planes[AOM_PLANE_U];
  int y_stride = img->stride[AOM_PLANE_Y];
  int uv_stride = img->stride[AOM_PLANE_U];
  int y_width = aom_img_plane_width(img, AOM_PLANE_Y);
  int u_width = aom_img_plane_width(img, AOM_PLANE_U);
  int v_width = aom_img_plane_width(img, AOM_PLANE_V);
  int y_height = aom_img_plane_height(img, AOM_PLANE_Y);
  int u_height = aom_img_plane_height(img, AOM_PLANE_U);
  int v_height = aom_img_plane_height(img, AOM_PLANE_V);

  ASH_CHECK(u_width == v_width);
  ASH_CHECK(y_width == 2 * u_width);
  ASH_CHECK(u_height == v_height);
  ASH_CHECK(y_height == 2 * u_height);

  mat3 coefficient = ypbpr2rgb_coefficients[img->fmt];

  for (usize j = 0; j < y_height / 2; j++) {
    for (usize i = 0; i < y_width / 4; i++) {
      u8 y0_0 = y_plane[0];
      u8 y0_1 = y_plane[1];
      u8 y0_2 = y_plane[2];
      u8 y0_3 = y_plane[3];

      u8 y1_0 = (y_plane + y_stride)[0];
      u8 y1_1 = (y_plane + y_stride)[1];
      u8 y1_2 = (y_plane + y_stride)[2];
      u8 y1_3 = (y_plane + y_stride)[3];

      u8 u0_0 = uv_plane[0];
      u8 v0_0 = uv_plane[1];
      u8 u0_1 = u0_0;
      u8 v0_1 = v0_0;
      u8 u0_2 = uv_plane[2];
      u8 v0_2 = uv_plane[3];
      u8 u0_3 = u0_2;
      u8 v0_3 = v0_2;

      u8 u1_0 = u0_0;
      u8 v1_0 = v0_0;
      u8 u1_1 = u0_1;
      u8 v1_1 = v0_1;
      u8 u1_2 = u0_2;
      u8 v1_2 = v0_2;
      u8 u1_3 = u0_3;
      u8 v1_3 = v0_3;

      yuv2rgb_bp12(coefficient, y0_0, u0_0, v0_0, rgb);
      yuv2rgb_bp12(coefficient, y0_1, u0_1, v0_1, rgb + 3);
      yuv2rgb_bp12(coefficient, y0_2, u0_2, v0_2, rgb + 6);
      yuv2rgb_bp12(coefficient, y0_3, u0_3, v0_3, rgb + 6);
      yuv2rgb_bp12(coefficient, y1_0, u1_0, v1_0, rgb + 3 * y_width);
      yuv2rgb_bp12(coefficient, y1_1, u1_1, v1_1, rgb + 3 * y_width + 3);
      yuv2rgb_bp12(coefficient, y1_2, u1_2, v1_2, rgb + 3 * y_width + 6);
      yuv2rgb_bp12(coefficient, y1_3, u1_3, v1_3, rgb + 3 * y_width + 9);

      y_plane += 4;
      uv_plane += 2 * 2;
      rgb += 4 * 3;
    }

    y_plane += (y_stride - y_width) + y_stride;
    uv_plane += uv_stride - (u_width + v_width);
    rgb += y_width * 3;
  }
}

// AOM_IMG_FMT_I422
void yuv_422_12_to_rgb(aom_image_t const *img, u8 *rgb) {
  u8 const *y_plane = img->planes[AOM_PLANE_Y];
  u8 const *u_plane = img->planes[AOM_PLANE_U];
  u8 const *v_plane = img->planes[AOM_PLANE_V];
  int y_stride = img->stride[AOM_PLANE_Y];
  int u_stride = img->stride[AOM_PLANE_U];
  int v_stride = img->stride[AOM_PLANE_V];
  int y_width = aom_img_plane_width(img, AOM_PLANE_Y);
  int u_width = aom_img_plane_width(img, AOM_PLANE_U);
  int v_width = aom_img_plane_width(img, AOM_PLANE_V);
  int y_height = aom_img_plane_height(img, AOM_PLANE_Y);
  int u_height = aom_img_plane_height(img, AOM_PLANE_U);
  int v_height = aom_img_plane_height(img, AOM_PLANE_V);

  ASH_CHECK(u_width == v_width);
  ASH_CHECK(y_width == 4 * u_width);
  ASH_CHECK(u_height == v_height);
  ASH_CHECK(y_height == u_height);

  mat3 coefficient = ypbpr2rgb_coefficients[img->fmt];

  for (usize j = 0; j < y_height / 2; j++) {
    for (usize i = 0; i < y_width / 4; i++) {
      u8 y0_0 = y_plane[0];
      u8 y0_1 = y_plane[1];
      u8 y0_2 = y_plane[2];
      u8 y0_3 = y_plane[3];

      u8 y1_0 = (y_plane + y_stride)[0];
      u8 y1_1 = (y_plane + y_stride)[1];
      u8 y1_2 = (y_plane + y_stride)[2];
      u8 y1_3 = (y_plane + y_stride)[3];

      u8 u0_0 = u_plane[0];
      u8 v0_0 = v_plane[0];
      u8 u0_1 = u0_0;
      u8 v0_1 = v0_0;
      u8 u0_2 = u_plane[1];
      u8 v0_2 = v_plane[1];
      u8 u0_3 = u0_2;
      u8 v0_3 = v0_2;

      u8 u1_0 = (u_plane + u_stride)[0];
      u8 v1_0 = (v_plane + v_stride)[0];
      u8 u1_1 = u1_0;
      u8 v1_1 = v1_0;
      u8 u1_2 = (u_plane + u_stride)[1];
      u8 v1_2 = (v_plane + v_stride)[1];
      u8 u1_3 = u1_2;
      u8 v1_3 = v1_2;

      yuv2rgb_bp12(coefficient, y0_0, u0_0, v0_0, rgb);
      yuv2rgb_bp12(coefficient, y0_1, u0_1, v0_1, rgb + 3);
      yuv2rgb_bp12(coefficient, y0_2, u0_2, v0_2, rgb + 6);
      yuv2rgb_bp12(coefficient, y0_3, u0_3, v0_3, rgb + 9);
      yuv2rgb_bp12(coefficient, y1_0, u1_0, v1_0, rgb + 3 * y_width);
      yuv2rgb_bp12(coefficient, y1_1, u1_1, v1_1, rgb + 3 * y_width + 3);
      yuv2rgb_bp12(coefficient, y1_2, u1_2, v1_2, rgb + 3 * y_width + 6);
      yuv2rgb_bp12(coefficient, y1_3, u1_3, v1_3, rgb + 3 * y_width + 9);

      y_plane += 4;
      u_plane += 2;
      v_plane += 2;
      rgb += 4 * 3;
    }

    y_plane += (y_stride - y_width) + y_stride;
    u_plane += (u_stride - u_width) + u_stride;
    u_plane += (u_stride - u_width) + v_stride;
    rgb += y_width * 3;
  }
}

// AOM_IMG_FMT_I444
void yuv_444_12_to_rgb(aom_image_t const *img, u8 *rgb) {
  u8 const *y_plane = img->planes[AOM_PLANE_Y];
  u8 const *u_plane = img->planes[AOM_PLANE_U];
  u8 const *v_plane = img->planes[AOM_PLANE_V];
  int y_stride = img->stride[AOM_PLANE_Y];
  int u_stride = img->stride[AOM_PLANE_U];
  int v_stride = img->stride[AOM_PLANE_V];
  int y_width = aom_img_plane_width(img, AOM_PLANE_Y);
  int u_width = aom_img_plane_width(img, AOM_PLANE_U);
  int v_width = aom_img_plane_width(img, AOM_PLANE_V);
  int y_height = aom_img_plane_height(img, AOM_PLANE_Y);
  int u_height = aom_img_plane_height(img, AOM_PLANE_U);
  int v_height = aom_img_plane_height(img, AOM_PLANE_V);

  ASH_CHECK(u_width == v_width);
  ASH_CHECK(y_width == u_width);
  ASH_CHECK(u_height == v_height);
  ASH_CHECK(y_height == u_height);

  mat3 coefficient = ypbpr2rgb_coefficients[img->fmt];

  for (usize j = 0; j < y_height; j++) {
    for (usize i = 0; i < y_width; i++) {
      u8 y = y_plane[j * y_stride + i];
      u8 u = u_plane[j * u_stride + i];
      u8 v = v_plane[j * v_stride + i];

      yuv2rgb_bp12(coefficient, y, u, v, rgb + j * y_width * 3 + i * 3);
    }
  }
}

// AOM_IMG_FMT_I42016
// AOM_IMG_FMT_YV1216
//
void yuv_420_16_to_rgb(aom_image_t const *img, u8 *rgb) {
  int PLANE_U = (img->fmt & AOM_IMG_FMT_UV_FLIP) == AOM_IMG_FMT_UV_FLIP
                    ? AOM_PLANE_V
                    : AOM_PLANE_U;
  int PLANE_V = (img->fmt & AOM_IMG_FMT_UV_FLIP) == AOM_IMG_FMT_UV_FLIP
                    ? AOM_PLANE_U
                    : AOM_PLANE_V;

  u8 const *y_plane = img->planes[AOM_PLANE_Y];
  u8 const *u_plane = img->planes[PLANE_U];
  u8 const *v_plane = img->planes[PLANE_V];
  int y_stride = img->stride[AOM_PLANE_Y];
  int u_stride = img->stride[PLANE_U];
  int v_stride = img->stride[PLANE_V];
  int y_width = aom_img_plane_width(img, AOM_PLANE_Y);
  int u_width = aom_img_plane_width(img, PLANE_U);
  int v_width = aom_img_plane_width(img, PLANE_V);
  int y_height = aom_img_plane_height(img, AOM_PLANE_Y);
  int u_height = aom_img_plane_height(img, PLANE_U);
  int v_height = aom_img_plane_height(img, PLANE_V);

  ASH_CHECK(u_width == v_width);
  ASH_CHECK(y_width == 2 * u_width);
  ASH_CHECK(u_height == v_height);
  ASH_CHECK(y_height == 2 * u_height);

  mat3 coefficient = ypbpr2rgb_coefficients[img->fmt];

  for (usize j = 0; j < y_height / 2; j++) {
    for (usize i = 0; i < y_width / 4; i++) {
      u16 y0_0 = AS(u16, y_plane[1]) << 8 | y_plane[0];
      u16 y0_1 = AS(u16, y_plane[3]) << 8 | y_plane[2];
      u16 y0_2 = AS(u16, y_plane[5]) << 8 | y_plane[4];
      u16 y0_3 = AS(u16, y_plane[7]) << 8 | y_plane[6];

      u16 y1_0 =
          AS(u16, (y_plane + y_stride)[1]) << 8 | (y_plane + y_stride)[0];
      u16 y1_1 =
          AS(u16, (y_plane + y_stride)[3]) << 8 | (y_plane + y_stride)[2];
      u16 y1_2 =
          AS(u16, (y_plane + y_stride)[5]) << 8 | (y_plane + y_stride)[4];
      u16 y1_3 =
          AS(u16, (y_plane + y_stride)[7]) << 8 | (y_plane + y_stride)[6];

      u16 u0_0 = AS(u16, u_plane[1]) << 8 | u_plane[0];
      u16 v0_0 = AS(u16, v_plane[1]) << 8 | v_plane[0];
      u16 u0_1 = u0_0;
      u16 v0_1 = v0_0;
      u16 u0_2 = AS(u16, u_plane[3]) << 8 | u_plane[2];
      u16 v0_2 = AS(u16, v_plane[3]) << 8 | v_plane[2];
      u16 u0_3 = u0_2;
      u16 v0_3 = v0_2;

      u16 u1_0 = u0_0;
      u16 v1_0 = v0_0;
      u16 u1_1 = u0_1;
      u16 v1_1 = v0_1;
      u16 u1_2 = u0_2;
      u16 v1_2 = v0_2;
      u16 u1_3 = u0_3;
      u16 v1_3 = v0_3;

      yuv2rgb_bp16(coefficient, y0_0, u0_0, v0_0, rgb);
      yuv2rgb_bp16(coefficient, y0_1, u0_1, v0_1, rgb + 3);
      yuv2rgb_bp16(coefficient, y0_2, u0_2, v0_2, rgb + 6);
      yuv2rgb_bp16(coefficient, y0_3, u0_3, v0_3, rgb + 9);
      yuv2rgb_bp16(coefficient, y1_0, u1_0, v1_0, rgb + y_width);
      yuv2rgb_bp16(coefficient, y1_1, u1_1, v1_1, rgb + y_width + 3);
      yuv2rgb_bp16(coefficient, y1_2, u1_2, v1_2, rgb + y_width + 6);
      yuv2rgb_bp16(coefficient, y1_3, u1_3, v1_3, rgb + y_width + 9);

      y_plane += 4 * 2;
      u_plane += 2 * 2;
      v_plane += 2 * 2;
      rgb += 4 * 3;
    }

    y_plane += (y_stride - y_width * 2) + y_stride;
    u_plane += (u_stride - u_width * 2);
    v_plane += (v_stride - v_width * 2);
    rgb += y_width * 3;
  }
}

// AOM_IMG_FMT_I42216
void yuv_422_16_to_rgb(aom_image_t const *img, u8 *rgb) {
  u8 const *y_plane = img->planes[AOM_PLANE_Y];
  u8 const *u_plane = img->planes[AOM_PLANE_U];
  u8 const *v_plane = img->planes[AOM_PLANE_V];
  int y_stride = img->stride[AOM_PLANE_Y];
  int u_stride = img->stride[AOM_PLANE_U];
  int v_stride = img->stride[AOM_PLANE_V];
  int y_width = aom_img_plane_width(img, AOM_PLANE_Y);
  int u_width = aom_img_plane_width(img, AOM_PLANE_U);
  int v_width = aom_img_plane_width(img, AOM_PLANE_V);
  int y_height = aom_img_plane_height(img, AOM_PLANE_Y);
  int u_height = aom_img_plane_height(img, AOM_PLANE_U);
  int v_height = aom_img_plane_height(img, AOM_PLANE_V);

  ASH_CHECK(u_width == v_width);
  ASH_CHECK(y_width == 4 * u_width);
  ASH_CHECK(u_height == v_height);
  ASH_CHECK(y_height == u_height);

  mat3 coefficient = ypbpr2rgb_coefficients[img->fmt];

  for (usize j = 0; j < y_height / 2; j++) {
    for (usize i = 0; i < y_width / 4; i++) {
      u16 y0_0 = AS(u16, y_plane[1]) << 8 | y_plane[0];
      u16 y0_1 = AS(u16, y_plane[3]) << 8 | y_plane[2];
      u16 y0_2 = AS(u16, y_plane[5]) << 8 | y_plane[4];
      u16 y0_3 = AS(u16, y_plane[7]) << 8 | y_plane[6];

      u16 y1_0 =
          AS(u16, (y_plane + y_stride)[1]) << 8 | (y_plane + y_stride)[0];
      u16 y1_1 =
          AS(u16, (y_plane + y_stride)[3]) << 8 | (y_plane + y_stride)[2];
      u16 y1_2 =
          AS(u16, (y_plane + y_stride)[5]) << 8 | (y_plane + y_stride)[4];
      u16 y1_3 =
          AS(u16, (y_plane + y_stride)[7]) << 8 | (y_plane + y_stride)[6];

      u16 u0_0 = AS(u16, u_plane[1]) << 8 | u_plane[0];
      u16 v0_0 = AS(u16, v_plane[1]) << 8 | v_plane[0];
      u16 u0_1 = u0_0;
      u16 v0_1 = v0_0;
      u16 u0_2 = AS(u16, u_plane[3]) << 8 | u_plane[2];
      u16 v0_2 = AS(u16, v_plane[3]) << 8 | v_plane[2];
      u16 u0_3 = u0_2;
      u16 v0_3 = v0_2;

      u16 u1_0 =
          AS(u16, (u_plane + u_stride)[1]) << 8 | (u_plane + u_stride)[0];
      u16 v1_0 =
          AS(u16, (v_plane + v_stride)[1]) << 8 | (v_plane + v_stride)[0];
      u16 u1_1 = u1_0;
      u16 v1_1 = v1_0;
      u16 u1_2 =
          AS(u16, (u_plane + u_stride)[1]) << 8 | (u_plane + u_stride)[0];
      u16 v1_2 =
          AS(u16, (v_plane + v_stride)[1]) << 8 | (v_plane + v_stride)[0];
      u16 u1_3 = u1_2;
      u16 v1_3 = v1_2;

      yuv2rgb_bp16(coefficient, y0_0, u0_0, v0_0, rgb);
      yuv2rgb_bp16(coefficient, y0_1, u0_1, v0_1, rgb + 3);
      yuv2rgb_bp16(coefficient, y0_2, u0_2, v0_2, rgb + 6);
      yuv2rgb_bp16(coefficient, y0_3, u0_3, v0_3, rgb + 9);
      yuv2rgb_bp16(coefficient, y1_0, u1_0, v1_0, rgb + y_width);
      yuv2rgb_bp16(coefficient, y1_1, u1_1, v1_1, rgb + y_width + 3);
      yuv2rgb_bp16(coefficient, y1_2, u1_2, v1_2, rgb + y_width + 6);
      yuv2rgb_bp16(coefficient, y1_3, u1_3, v1_3, rgb + y_width + 9);

      y_plane += 4 * 2;
      u_plane += 2 * 2;
      v_plane += 2 * 2;
      rgb += 4 * 3;
    }

    y_plane += (y_stride - y_width * 2) + y_stride;
    u_plane += (u_stride - u_width * 2) + u_stride;
    u_plane += (u_stride - u_width * 2) + v_stride;
    rgb += y_width * 3;
  }
}

// AOM_IMG_FMT_I44416
void yuv_444_16_to_rgb(aom_image_t const *img, u8 *rgb) {
  u8 const *y_plane = img->planes[AOM_PLANE_Y];
  u8 const *u_plane = img->planes[AOM_PLANE_U];
  u8 const *v_plane = img->planes[AOM_PLANE_V];
  int y_stride = img->stride[AOM_PLANE_Y];
  int u_stride = img->stride[AOM_PLANE_U];
  int v_stride = img->stride[AOM_PLANE_V];
  int y_width = aom_img_plane_width(img, AOM_PLANE_Y);
  int u_width = aom_img_plane_width(img, AOM_PLANE_U);
  int v_width = aom_img_plane_width(img, AOM_PLANE_V);
  int y_height = aom_img_plane_height(img, AOM_PLANE_Y);
  int u_height = aom_img_plane_height(img, AOM_PLANE_U);
  int v_height = aom_img_plane_height(img, AOM_PLANE_V);

  ASH_CHECK(u_width == v_width);
  ASH_CHECK(y_width == u_width);
  ASH_CHECK(u_height == v_height);
  ASH_CHECK(y_height == u_height);

  mat3 coefficient = ypbpr2rgb_coefficients[img->fmt];

  for (usize j = 0; j < y_height; j++) {
    for (usize i = 0; i < y_width; i++) {
      u16 y = AS(u16, y_plane[j * y_stride + i + 1]) << 8 |
              y_plane[j * y_stride + i];
      u16 u = AS(u16, u_plane[j * u_stride + i + 1]) << 8 |
              u_plane[j * u_stride + i];
      u16 v = AS(u16, v_plane[j * v_stride + i + 1]) << 8 |
              v_plane[j * v_stride + i];

      yuv2rgb_bp16(coefficient, y, u, v, rgb + j * y_width * 3 + i * 3);
    }
  }
}

}  // namespace ash

// // Chapters are a way to set predefined points to jump to in video or

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
/*
A block element is a basic element that contains the encoded data for one frame
of video or audio21. A block element has a timestamp that indicates its position
in the presentation time21. A block element can also have optional flags that
indicate its keyframe status, invisible status, discardable status and lacing
type21.

A blockgroup element is a container element that can contain one or more block
elements as well as additional information about them21. A blockgroup element
can have optional sub-elements such as duration, reference priority, reference
block, codec state and discard padding21.

The main difference between block and blockgroup elements is that a blockgroup
element can provide more information about the blocks it contains, such as their
dependencies, durations and codec states21. A blockgroup element can also group
multiple blocks together into a single logical unit21.

However, not all blocks need to be contained in a blockgroup element. For
example, webm files only use block elements for video tracks and do not use any
of the sub-elements of blockgroup elements2. Mkv files can use either block or
blockgroup elements depending on the codec and muxer settings3.
*/

using namespace webm;
using namespace ash;

class XCallback : public Callback {
 public:
  virtual Status OnTrackEntry(const ElementMetadata &metadata,
                              const TrackEntry &track_entry) {
    spdlog::info("codec: {}", track_entry.codec_id.value());
    return Status{Status::kOkCompleted};
  }

  virtual Status OnBlockBegin(const ElementMetadata &metadata,
                              const Block &block, Action *action) override {
    spdlog::info("block begin");
    // "V_AV1", "V_VP9", "V_VP8"
    // "A_VORBIS", "A_OPUS"
    *action = Action::kRead;
    return Status{Status::kOkCompleted};
  }

  virtual Status OnFrame(const FrameMetadata &f, Reader *reader,
                         std::uint64_t *bytes_remaining) override {
    if (*bytes_remaining == 0) return Status(Status::kOkCompleted);
    nframes++;
    u8 *out = new u8[f.size];
    u64 read = 0;
    Status status = reader->Read(*bytes_remaining, out, &read);
    ASH_CHECK(status.code == Status::kOkCompleted);
    ASH_CHECK(read == f.size);
    ASH_CHECK(*bytes_remaining == f.size);

    spdlog::info(
        "[block: {}, frame position: {}, nframes: {}] bytes_remaining: {}, "
        "read: {}",
        (int)f.parent_element.id, f.position, nframes, *bytes_remaining, read);

    *bytes_remaining = 0;

    aom_codec_stream_info_t info;
    info.is_annexb = false;
    auto err = aom_codec_peek_stream_info(decoder, out, f.size, &info);
    if (err == AOM_CODEC_OK) {
      err = aom_codec_decode(codec, out, f.size, nullptr);
      if (err != AOM_CODEC_OK) {
        char const *s = aom_codec_error_detail(codec);
        spdlog::error("video decode failed: {} {}", (int)err,
                      s == nullptr ? "" : s);
      } else {
        spdlog::info("video decode succeeded");
        aom_codec_iter_t iter = nullptr;
        aom_image_t *img = aom_codec_get_frame(codec, &iter);
        if (img != nullptr) {
          spdlog::info(
              "y_w: {}, u_w: {}, v_w: {}, y_h: {}, u_h: {}, "
              "v_h: "
              "{}, fmt: {}, "
              "mt: {}, bps: {} ",
              aom_img_plane_width(img, AOM_PLANE_Y),
              aom_img_plane_width(img, AOM_PLANE_U),
              aom_img_plane_width(img, AOM_PLANE_V),
              aom_img_plane_height(img, AOM_PLANE_Y),
              aom_img_plane_height(img, AOM_PLANE_U),
              aom_img_plane_height(img, AOM_PLANE_V), (int)img->fmt,
              (int)img->mc, img->bps);

          // ImageBuffer buffer{
          //     .memory = stx::mem::allocate(stx::os_allocator,
          //                                  AS(usize, y_width) * AS(usize,
          //                                  y_height) * 4)
          //                   .unwrap(),
          //     .extent = extent{.width = AS(u32, y_width), .height = AS(u32,
          //     y_height)}, .format = ImageFormat::rgb};

          // switch (img->fmt) {
          //   case AOM_IMG_FMT_NONE:
          //     break;
          //   case AOM_IMG_FMT_YV12:
          //   case AOM_IMG_FMT_I420:
          //   case AOM_IMG_FMT_AOMI420:
          //   case AOM_IMG_FMT_AOMYV12: {
          //     yuv_420_12_to_rgb(img, AS(u8 *, buffer.memory.handle));
          //   } break;
          //   case AOM_IMG_FMT_NV12: { // interleaved
          // } break;
          //   case AOM_IMG_FMT_I422: {
          //     yuv_422_12_to_rgb(img, AS(u8 *, buffer.memory.handle));
          //   } break;
          //   case AOM_IMG_FMT_I444: {
          //     yuv_444_12_to_rgb(img, AS(u8 *, buffer.memory.handle));
          //   } break;
          //   case AOM_IMG_FMT_I42016:
          //   case AOM_IMG_FMT_YV1216: {
          //     yuv_420_16_to_rgb(img, AS(u8 *, buffer.memory.handle));
          //   } break;
          //   case AOM_IMG_FMT_I42216: {
          //     yuv_422_16_to_rgb(img, AS(u8 *, buffer.memory.handle));
          //   } break;
          //   case AOM_IMG_FMT_I44416: {
          //     yuv_444_16_to_rgb(img, AS(u8 *, buffer.memory.handle));
          //   } break;
          //   default: {
          //     return -1;
          //   }
          // }
        }
      }
    } else {
      char const *s = aom_codec_error_detail(codec);
      spdlog::info("non-av1 frame found: {} {}", (int)err,
                   s == nullptr ? "" : s);
    }

    delete[] out;

    return Status{Status::kOkCompleted};
  }

  aom_codec_ctx_t *codec = nullptr;
  aom_codec_iface_t *decoder = nullptr;
  u64 nframes = 0;
  WebmParser parser;
};

extern "C" {
#include "libavformat/avformat.h"
}

int main(int argc, char **argv) {
  ASH_CHECK(argc == 2);

  auto *co = avcodec_find_decoder(AV_CODEC_ID_AV1);
  ASH_CHECK(co != nullptr);
  spdlog::info("codec name: {}, long name: {}", co->name, co->long_name);

  std::ifstream stream{argv[1], std::ios::binary | std::ios::ate};

  ASH_CHECK(stream.is_open());

  stx::Vec<char> bytes{stx::os_allocator};
  bytes.resize(stream.tellg()).unwrap();
  stream.seekg(0);
  stream.read(bytes.data(), bytes.size());

  aom_codec_iface_t *decoder = aom_codec_av1_dx();
  decoder->name;
  decoder->abi_version;
  aom_codec_ctx_t codec;
  aom_codec_dec_init(&codec, decoder, nullptr, 0);

  FileReader reader{std::fopen(argv[1], "rb")};
  XCallback callback;
  callback.codec = &codec;
  callback.decoder = decoder;
  WebmParser parser;
  Status status{Status::kOkPartial};
  while (status.code == Status::kOkPartial) {
    status = parser.Feed(&callback, &reader);
    spdlog::info("status: {}", status.code);
  }
  spdlog::info("final status: {}", status.code);

  AppConfig cfg{.enable_validation_layers = false};
  cfg.window_config.borderless = false;
  App app{
      std::move(cfg),
      new Image{ImageProps{
          .source = FileImageSource{.path = stx::string::make_static(argv[1])},
          .border_radius = vec4{200, 200, 200, 200},
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
