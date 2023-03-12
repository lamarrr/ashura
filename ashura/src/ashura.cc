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
                            u8 *rgba) {
  vec3 rgb = coefficient *
             vec3{AS(f32, y) - 16, AS(f32, u) - 128, AS(f32, v) - 128} *
             vec3{255, 255, 255};
  rgba[0] = ASH_U8_CLAMP(rgb.x);
  rgba[1] = ASH_U8_CLAMP(rgb.y);
  rgba[2] = ASH_U8_CLAMP(rgb.z);
  rgba[3] = 255;
}

constexpr void yuv2rgb_bp16(mat3 const &coefficient, u16 y, u16 u, u16 v,
                            u8 *rgba) {
  vec3 rgb = coefficient *
             vec3{AS(f32, y) - 16 * 257, AS(f32, u) - 128 * 257,
                  AS(f32, v) - 128 * 257} *
             vec3{255, 255, 255};
  rgba[0] = ASH_U8_CLAMP(rgb.x);
  rgba[1] = ASH_U8_CLAMP(rgb.y);
  rgba[2] = ASH_U8_CLAMP(rgb.z);
  rgba[3] = 255;
}

// planar format meaning y, u, and v stored in separate arrays
//
// AOM_IMG_FMT_YV12
// AOM_IMG_FMT_I420
// AOM_IMG_FMT_AOMI420
// AOM_IMG_FMT_AOMYV12
// AOM_IMG_FMT_NV12
//
void yuv_420_12_to_rgb(aom_image_t const *img, u8 *rgba) {
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

      yuv2rgb_bp12(coefficient, y0_0, u0_0, v0_0, rgba);
      yuv2rgb_bp12(coefficient, y0_1, u0_1, v0_1, rgba + 4);
      yuv2rgb_bp12(coefficient, y0_2, u0_2, v0_2, rgba + 8);
      yuv2rgb_bp12(coefficient, y0_3, u0_3, v0_3, rgba + 12);
      yuv2rgb_bp12(coefficient, y1_0, u1_0, v1_0, rgba + y_width);
      yuv2rgb_bp12(coefficient, y1_1, u1_1, v1_1, rgba + y_width + 4);
      yuv2rgb_bp12(coefficient, y1_2, u1_2, v1_2, rgba + y_width + 8);
      yuv2rgb_bp12(coefficient, y1_3, u1_3, v1_3, rgba + y_width + 12);

      y_plane += 4;
      u_plane += 2;
      v_plane += 2;
      rgba += 4 * 4;
    }

    y_plane += (y_stride - y_width) + y_stride;
    u_plane += (u_stride - u_width);
    u_plane += (u_stride - u_width);
    rgba += y_width * 4;
  }
}

// AOM_IMG_FMT_I422
void yuv_422_12_to_rgb(aom_image_t const *img, u8 *rgba) {
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

      yuv2rgb_bp12(coefficient, y0_0, u0_0, v0_0, rgba);
      yuv2rgb_bp12(coefficient, y0_1, u0_1, v0_1, rgba + 4);
      yuv2rgb_bp12(coefficient, y0_2, u0_2, v0_2, rgba + 8);
      yuv2rgb_bp12(coefficient, y0_3, u0_3, v0_3, rgba + 12);
      yuv2rgb_bp12(coefficient, y1_0, u1_0, v1_0, rgba + y_width);
      yuv2rgb_bp12(coefficient, y1_1, u1_1, v1_1, rgba + y_width + 4);
      yuv2rgb_bp12(coefficient, y1_2, u1_2, v1_2, rgba + y_width + 8);
      yuv2rgb_bp12(coefficient, y1_3, u1_3, v1_3, rgba + y_width + 12);

      y_plane += 4;
      u_plane += 2;
      v_plane += 2;
      rgba += 4 * 4;
    }

    y_plane += (y_stride - y_width) + y_stride;
    u_plane += (u_stride - u_width) + u_stride;
    u_plane += (u_stride - u_width) + v_stride;
    rgba += y_width * 4;
  }
}

// AOM_IMG_FMT_I444
void yuv_444_12_to_rgb(aom_image_t const *img, u8 *rgba) {
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

      yuv2rgb_bp12(coefficient, y, u, v, rgba + j * y_width * 4 + i * 4);
    }
  }
}

// AOM_IMG_FMT_I42016
// AOM_IMG_FMT_YV1216
//
void yuv_420_16_to_rgb(aom_image_t const *img, u8 *rgba) {
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

      yuv2rgb_bp16(coefficient, y0_0, u0_0, v0_0, rgba);
      yuv2rgb_bp16(coefficient, y0_1, u0_1, v0_1, rgba + 4);
      yuv2rgb_bp16(coefficient, y0_2, u0_2, v0_2, rgba + 8);
      yuv2rgb_bp16(coefficient, y0_3, u0_3, v0_3, rgba + 12);
      yuv2rgb_bp16(coefficient, y1_0, u1_0, v1_0, rgba + y_width);
      yuv2rgb_bp16(coefficient, y1_1, u1_1, v1_1, rgba + y_width + 4);
      yuv2rgb_bp16(coefficient, y1_2, u1_2, v1_2, rgba + y_width + 8);
      yuv2rgb_bp16(coefficient, y1_3, u1_3, v1_3, rgba + y_width + 12);

      y_plane += 4 * 2;
      u_plane += 2 * 2;
      v_plane += 2 * 2;
      rgba += 4 * 4;
    }

    y_plane += (y_stride - y_width * 2) + y_stride;
    u_plane += (u_stride - u_width * 2);
    v_plane += (v_stride - v_width * 2);
    rgba += y_width * 4;
  }
}

// AOM_IMG_FMT_I42216
void yuv_422_16_to_rgb(aom_image_t const *img, u8 *rgba) {
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

      yuv2rgb_bp16(coefficient, y0_0, u0_0, v0_0, rgba);
      yuv2rgb_bp16(coefficient, y0_1, u0_1, v0_1, rgba + 4);
      yuv2rgb_bp16(coefficient, y0_2, u0_2, v0_2, rgba + 8);
      yuv2rgb_bp16(coefficient, y0_3, u0_3, v0_3, rgba + 12);
      yuv2rgb_bp16(coefficient, y1_0, u1_0, v1_0, rgba + y_width);
      yuv2rgb_bp16(coefficient, y1_1, u1_1, v1_1, rgba + y_width + 4);
      yuv2rgb_bp16(coefficient, y1_2, u1_2, v1_2, rgba + y_width + 8);
      yuv2rgb_bp16(coefficient, y1_3, u1_3, v1_3, rgba + y_width + 12);

      y_plane += 4 * 2;
      u_plane += 2 * 2;
      v_plane += 2 * 2;
      rgba += 4 * 4;
    }

    y_plane += (y_stride - y_width * 2) + y_stride;
    u_plane += (u_stride - u_width * 2) + u_stride;
    u_plane += (u_stride - u_width * 2) + v_stride;
    rgba += y_width * 4;
  }
}

// AOM_IMG_FMT_I44416
void yuv_444_16_to_rgb(aom_image_t const *img, u8 *rgba) {
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

      yuv2rgb_bp16(coefficient, y, u, v, rgba + j * y_width * 4 + i * 4);
    }
  }
}

namespace webm {

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
    *total = buffer.size();
    *available = buffer.size();
    return 0;
  }

  void append(stx::Span<char const> buffer) { this->buffer = buffer; }

  void swap(stx::Span<char const> buffer) { this->buffer = buffer; }

  stx::Span<char const> buffer;
};

enum class AudioCodec : u8 { None, Opus, Vorbis, Unrecognized };

enum class VideoCodec : u8 { None, AV1, VP8, VP9, Unrecognized };

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

// Workflow:
// - receive video stream
// - try to parse header, if there's no valid header or an error occured, we
// return a negative value, if successful we return 0
// - pos is advanced by the number of bytes read. it is still incremented even
// if an error occured.
int try_parse_header(mkvparser::IMkvReader &reader, long long &pos) {
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
    mkvparser::IMkvReader &reader, long long &segment_start) {
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

int decoder_init(DecodeContext &context, usize frame_size_bytes) {
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
  int y_width = aom_img_plane_width(img, AOM_PLANE_Y);
  int y_height = aom_img_plane_width(img, AOM_PLANE_Y);

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
    case AOM_IMG_FMT_I420:
    case AOM_IMG_FMT_AOMI420:
    case AOM_IMG_FMT_AOMYV12:
    case AOM_IMG_FMT_NV12: {
      yuv_420_12_to_rgb(img, AS(u8 *, buffer.memory.handle));
    } break;
    case AOM_IMG_FMT_I422: {
      yuv_422_12_to_rgb(img, AS(u8 *, buffer.memory.handle));
    } break;
    case AOM_IMG_FMT_I444: {
      yuv_444_12_to_rgb(img, AS(u8 *, buffer.memory.handle));
    } break;
    case AOM_IMG_FMT_I42016:
    case AOM_IMG_FMT_YV1216: {
      yuv_420_16_to_rgb(img, AS(u8 *, buffer.memory.handle));
    } break;
    case AOM_IMG_FMT_I42216: {
      yuv_422_16_to_rgb(img, AS(u8 *, buffer.memory.handle));
    } break;
    case AOM_IMG_FMT_I44416: {
      yuv_444_16_to_rgb(img, AS(u8 *, buffer.memory.handle));
    } break;
    default: {
      return -1;
    }
  }

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

}  // namespace webm
}  // namespace ash

int main(int argc, char **argv) {
  ASH_CHECK(argc == 2);

  std::ifstream stream{argv[1], std::ios::binary | std::ios::ate};

  stx::Vec<char> bytes{stx::os_allocator};
  bytes.resize(stream.tellg()).unwrap();
  stream.seekg(0);
  stream.read(bytes.data(), bytes.size());

  ash::webm::MkvByteStream bstream;
  bstream.buffer = bytes;
  long long pos = 0;
  int error = 0;
  error = ash::webm::try_parse_header(bstream, pos);

  if (error < 0) {
    spdlog::error("error: {}", error);
    return 0;
  }

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
