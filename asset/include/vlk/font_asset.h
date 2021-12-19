#pragma once

#include <filesystem>
#include <fstream>
#include <memory>

#include "include/core/SkData.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypeface.h"
#include "stx/async.h"
#include "stx/span.h"
#include "vlk/asset.h"
#include "vlk/font_style.h"

namespace vlk {

struct FontAsset final : public Asset {
  explicit FontAsset(sk_sp<SkTypeface> raw_typeface);

  auto get_raw() const { return raw_; }

  ~FontAsset() override {}

 private:
  sk_sp<SkTypeface> raw_;
};

enum class FontLoadError : uint8_t { InvalidPath, InvalidBytes, Loadfailed };

constexpr std::string_view format(FontLoadError error) {
  switch (error) {
    case FontLoadError::InvalidPath:
      return "Invalid Path";
    case FontLoadError::InvalidBytes:
      return "Invalid Bytes";
    case FontLoadError::Loadfailed:
      return "Load Failed";
    default:
      return "";
  }
}

namespace impl {

STX_FORCE_INLINE constexpr SkFontStyle to_skia(FontStyle const& style) {
  SkFontStyle::Weight weight =
      static_cast<SkFontStyle::Weight>(static_cast<int>(style.weight));
  SkFontStyle::Width width =
      static_cast<SkFontStyle::Width>(static_cast<int>(style.width));

  SkFontStyle::Slant slant = {};

  switch (style.slant) {
    case FontSlant::Italic:
      slant = (SkFontStyle::Slant)(slant | SkFontStyle::kItalic_Slant);
      break;
    case FontSlant::Oblique:
      slant = (SkFontStyle::Slant)(slant | SkFontStyle::kOblique_Slant);
      break;
    case FontSlant::Upright:
      slant = (SkFontStyle::Slant)(slant | SkFontStyle::kUpright_Slant);
      break;
    default:
      slant = SkFontStyle::Slant::kUpright_Slant;
  }

  return SkFontStyle{weight, width, slant};
}

inline stx::Result<sk_sp<SkTypeface>, FontLoadError> load_typeface_from_memory(
    stx::Span<uint8_t const> bytes) {
  // INVESTIGATE NOTE: skia seems to be performing a deferred decoding or
  // something
  sk_sp typeface = SkTypeface::MakeFromData(
      SkData::MakeWithCopy(bytes.data(), bytes.size_bytes()));

  if (typeface == nullptr) return stx::Err(FontLoadError::InvalidBytes);

  return stx::Ok(std::move(typeface));
}

inline stx::Result<sk_sp<SkTypeface>, FontLoadError> load_typeface_from_file(
    std::filesystem::path const& path) {
  if (!(std::filesystem::exists(path) &&
        std::filesystem::is_regular_file(path)))
    return stx::Err(FontLoadError::InvalidPath);

  std::ifstream stream{path, std::ios_base::in | std::ios_base::ate};

  auto const file_size = stream.tellg();

  std::unique_ptr<uint8_t[]> encoded_buffer(new uint8_t[file_size]);

  stream.seekg(0);

  stream.read(reinterpret_cast<char*>(encoded_buffer.get()), file_size);

  return load_typeface_from_memory(
      stx::Span<uint8_t const>(encoded_buffer.get(), file_size));
}

inline stx::Result<sk_sp<SkTypeface>, FontLoadError> load_system_typeface(
    stx::Option<std::string> const& family, FontStyle const& font_style) {
  auto font_mgr = SkFontMgr::RefDefault();
  if (font_mgr == nullptr) return stx::Err(FontLoadError::Loadfailed);

  SkTypeface* typeface = font_mgr->matchFamilyStyle(
      family.as_cref().match([](std::string const& str) { return str.c_str(); },
                             []() {
                               // get default system font
                               return nullptr;
                             }),
      to_skia(font_style));

  if (typeface == nullptr) return stx::Err(FontLoadError::Loadfailed);

  sk_sp sk_typeface = sk_ref_sp(typeface);

  return stx::Ok(std::move(sk_typeface));
}

}  // namespace impl
}  // namespace vlk
