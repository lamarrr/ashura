#include "vlk/ui/font_asset.h"

#include <atomic>

#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"

namespace vlk {
namespace ui {

uint64_t MemoryTypefaceSource::make_uid() {
  static std::atomic<uint64_t> latest_uid = 0;

  return latest_uid.fetch_add(1, std::memory_order_seq_cst);
}

namespace impl {

STX_FORCE_INLINE constexpr SkFontStyle to_skia(
    vlk::ui::FontStyle const& style) {
  SkFontStyle::Weight weight =
      static_cast<SkFontStyle::Weight>(static_cast<int>(style.weight));
  SkFontStyle::Width width =
      static_cast<SkFontStyle::Width>(static_cast<int>(style.width));

  SkFontStyle::Slant slant = {};

  switch (style.slant) {
    case ui::FontSlant::Italic:
      slant = (SkFontStyle::Slant)(slant | SkFontStyle::kItalic_Slant);
      break;
    case ui::FontSlant::Oblique:
      slant = (SkFontStyle::Slant)(slant | SkFontStyle::kOblique_Slant);
      break;
    case ui::FontSlant::Upright:
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

std::unique_ptr<Asset> TypefaceLoader::load(RenderContext const&,
                                            AssetLoadArgs const& args) const {
  auto const& load_args = upcast<TypefaceLoadArgs const&>(args);
  if (load_args.is_mem()) {
    auto const& source_data =
        std::get<std::shared_ptr<MemoryTypefaceSourceData const>>(
            load_args.data_ref());
    return std::make_unique<TypefaceAsset>(
        TypefaceAsset{load_typeface_from_memory(source_data->bytes)});
  } else if (load_args.is_file()) {
    auto const& source_data =
        std::get<std::shared_ptr<FileTypefaceSourceData const>>(
            load_args.data_ref());
    return std::make_unique<TypefaceAsset>(
        TypefaceAsset{load_typeface_from_file(source_data->path)});
  } else if (load_args.is_system()) {
    auto const& source_data =
        std::get<std::shared_ptr<SystemFontData const>>(load_args.data_ref());
    return std::make_unique<TypefaceAsset>(TypefaceAsset{
        load_system_typeface(source_data->family, source_data->style)});
  } else {
    VLK_PANIC();
  }
}

std::shared_ptr<TypefaceLoader const> TypefaceLoader::get_default() {
  static std::shared_ptr<TypefaceLoader const> loader =
      std::make_shared<TypefaceLoader const>(TypefaceLoader{});

  return loader;
}

}  // namespace impl

}  // namespace ui
}  // namespace vlk
