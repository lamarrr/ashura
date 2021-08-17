#pragma once

#include "include/core/SkRefCnt.h"
#include "vlk/asset.h"

struct SkTypeface;

namespace vlk {

struct FontAsset final : public Asset {
  explicit FontAsset(sk_sp<SkTypeface> raw_typeface);

  auto get_raw() const { return raw_; }

  ~FontAsset() override {}

 private:
  sk_sp<SkTypeface> raw_;
};

}  // namespace vlk

//

// namespace vlk {
/*


namespace impl {

struct TypefaceLoadArgs : public AssetLoadArgs {
  explicit TypefaceLoadArgs(
      std::shared_ptr<MemoryTypefaceSourceData const> source_data) {
    data_ = std::move(source_data);
  }

  explicit TypefaceLoadArgs(
      std::shared_ptr<FileTypefaceSourceData const> source_data) {
    data_ = std::move(source_data);
  }

  explicit TypefaceLoadArgs(std::shared_ptr<SystemFontData const> system_font) {
    data_ = std::move(system_font);
  }

  bool is_mem() const {
    return std::holds_alternative<
        std::shared_ptr<MemoryTypefaceSourceData const>>(data_);
  }

  bool is_file() const {
    return std::holds_alternative<
        std::shared_ptr<FileTypefaceSourceData const>>(data_);
  }

  bool is_system() const {
    return std::holds_alternative<std::shared_ptr<SystemFontData const>>(data_);
  }

  auto const& data_ref() const { return data_; }

 private:
  std::variant<std::shared_ptr<MemoryTypefaceSourceData const>,
               std::shared_ptr<FileTypefaceSourceData const>,
               std::shared_ptr<SystemFontData const>>
      data_;
};

struct TypefaceLoader : public AssetLoader {
  virtual std::unique_ptr<Asset> load(AssetLoadArgs const& args) const override;

  static std::shared_ptr<TypefaceLoader const> get_default();
};

}  // namespace impl



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
*/
/*
 */

/*
inline auto add_font_asset(AssetManager& asset_manager,
                           FileTypefaceSource const& typeface_source) {
  return asset_manager.add(
      typeface_source.get_tag(),
      std::make_unique<TypefaceLoadArgs>(
          TypefaceLoadArgs{typeface_source.data_ref()}),
      TypefaceLoader::get_default());
}

inline auto add_font_asset(AssetManager& asset_manager,
                           MemoryTypefaceSource const& typeface_source) {
  return asset_manager.add(
      typeface_source.get_tag(),
      std::make_unique<TypefaceLoadArgs>(
          TypefaceLoadArgs{typeface_source.data_ref()}),
      TypefaceLoader::get_default());
}

inline auto add_font_asset(AssetManager& asset_manager,
                           SystemFont const& system_font) {
  return asset_manager.add(system_font.get_tag(),
                           std::make_unique<TypefaceLoadArgs>(
                               TypefaceLoadArgs{system_font.data_ref()}),
                           TypefaceLoader::get_default());
}

inline auto add_font_asset(AssetManager& asset_manager,
                           FileFont const& file_font) {
  return add_font_asset(asset_manager, file_font.source);
}

inline auto add_font_asset(AssetManager& asset_manager,
                           MemoryFont const& memory_font) {
  return add_font_asset(asset_manager, memory_font.source);
}

inline stx::Result<std::shared_ptr<Typeface const>, AssetError>
get_font_asset(AssetManager& asset_manager,
               FileTypefaceSource const& typeface_source) {
  TRY_OK(asset, asset_manager.get(typeface_source.get_tag()));
  auto typeface_asset = std::dynamic_pointer_cast<Typeface const>(asset);
  VLK_ENSURE(typeface_asset != nullptr);
  return stx::Ok(std::move(typeface_asset));
}

inline stx::Result<std::shared_ptr<Typeface const>, AssetError>
get_font_asset(AssetManager& asset_manager, SystemFont const& system_font) {
  TRY_OK(asset, asset_manager.get(system_font.get_tag()));
  auto typeface_asset = std::dynamic_pointer_cast<Typeface const>(asset);
  VLK_ENSURE(typeface_asset != nullptr);
  return stx::Ok(std::move(typeface_asset));
}

inline stx::Result<std::shared_ptr<Typeface const>, AssetError>
get_font_asset(AssetManager& asset_manager,
               MemoryTypefaceSource const& typeface_source) {
  TRY_OK(asset, asset_manager.get(typeface_source.get_tag()));
  auto typeface_asset = std::dynamic_pointer_cast<Typeface const>(asset);
  VLK_ENSURE(typeface_asset != nullptr);
  return stx::Ok(std::move(typeface_asset));
}

inline auto add_font_asset(AssetManager& asset_manager,
                           FileFontSource const& font_source) {
  std::vector<stx::Result<stx::NoneType, AssetError>> results;
  for (auto const& font_face : font_source.data_ref()->faces) {
    results.push_back(add_font_asset(asset_manager, font_face.source));
  }
  return results;
}

inline auto add_font_asset(AssetManager& asset_manager,
                           MemoryFontSource const& font_source) {
  std::vector<stx::Result<stx::NoneType, AssetError>> results;
  for (auto const& font_face : font_source.data_ref()->faces) {
    results.push_back(add_font_asset(asset_manager, font_face.source));
  }
  return results;
}

inline auto get_font_asset(AssetManager& asset_manager,
                           FileFont const& file_font) {
  return get_font_asset(asset_manager, file_font.source);
}

inline auto get_font_asset(AssetManager& asset_manager,
                           MemoryFont const& memory_font) {
  return get_font_asset(asset_manager, memory_font.source);
}

*/

// }  // namespace vlk
