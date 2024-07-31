/// SPDX-License-Identifier: MIT
#pragma once

namespace ash
{
/// @brief default charset is ASCII
constexpr char const MIME_TEXT_PLAIN[]    = "text/plain";
constexpr char const MIME_TEXT_UTF8[]     = "text/plain;charset=UTF-8";
constexpr char const MIME_TEXT_CSS[]      = "text/css";
constexpr char const MIME_TEXT_CSV[]      = "text/csv";
constexpr char const MIME_TEXT_HTML[]     = "text/html";
constexpr char const MIME_TEXT_JS[]       = "text/javascript";
constexpr char const MIME_TEXT_MARKDOWN[] = "text/markdown";

constexpr char const MIME_IMAGE_AVIF[] = "image/avif";
constexpr char const MIME_IMAGE_BMP[]  = "image/bmp";
constexpr char const MIME_IMAGE_HEIF[] = "image/heif";
constexpr char const MIME_IMAGE_JPEG[] = "image/jpeg";
constexpr char const MIME_IMAGE_PNG[]  = "image/png";
constexpr char const MIME_IMAGE_SVG[]  = "image/svg+xml";
constexpr char const MIME_IMAGE_WEBP[] = "image/webp";

constexpr char const MIME_VIDEO_AV1[]      = "video/AV1";
constexpr char const MIME_VIDEO_H264[]     = "video/H264";
constexpr char const MIME_VIDEO_H265[]     = "video/H265";
constexpr char const MIME_VIDEO_H266[]     = "video/H266";
constexpr char const MIME_VIDEO_MATROSKA[] = "video/matroska";
constexpr char const MIME_VIDEO_MP4[]      = "video/mp4";
constexpr char const MIME_VIDEO_RAW[]      = "video/raw";
constexpr char const MIME_VIDEO_VP8[]      = "video/VP8";
constexpr char const MIME_VIDEO_VP9[]      = "video/VP9";

constexpr char const MIME_MODEL_GLTF_BINARY[] = "model/gltf+binary";
constexpr char const MIME_MODEL_GLTF_JSON[]   = "model/gltf+json";
constexpr char const MIME_MODEL_MESH[]        = "model/mesh";
constexpr char const MIME_MODEL_MTL[]         = "model/mtl";
constexpr char const MIME_MODEL_OBJ[]         = "model/obj";
constexpr char const MIME_MODEL_STL[]         = "model/stl";

constexpr char const MIME_FONT_OTF[]   = "font/otf";
constexpr char const MIME_FONT_SFNT[]  = "font/sfnt";
constexpr char const MIME_FONT_TTF[]   = "font/ttf";
constexpr char const MIME_FONT_WOFF[]  = "font/woff";
constexpr char const MIME_FONT_WOFF2[] = "font/woff2";

}        // namespace ash