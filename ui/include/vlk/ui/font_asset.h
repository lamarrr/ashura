#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "fmt/format.h"
#include "include/core/SkTypeface.h"
#include "stx/result.h"
#include "stx/span.h"
#include "vlk/utils/utils.h"

//

#include "vlk/ui/asset_manager.h"
#include "vlk/ui/font.h"

namespace vlk {
namespace ui {

enum class FontLoadError : uint8_t { InvalidPath, InvalidBytes, Loadfailed };

constexpr std::string_view format(FontLoadError const& error) {
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

struct FileTypefaceSourceData {
  std::filesystem::path path;
  std::string identifier;
};

struct MemoryTypefaceSourceData {
  std::vector<uint8_t> bytes;
  std::string identifier;
};

}  // namespace impl

struct FileTypefaceSource {
  explicit FileTypefaceSource(std::filesystem::path path) {
    std::string identifier = std::string("FileTypefaceSource{path: ") +
                             std::string(path) + std::string("}");
    data_ = std::make_shared<impl::FileTypefaceSourceData>(
        impl::FileTypefaceSourceData{std::move(path), std::move(identifier)});
  }

  auto data() const { return data_; }

  // this is used to prevent the volatile atomic ref count that would occur when
  // copying from data() as compilers are required to not optimize volatile ops.
  auto const& data_ref() const { return data_; }

  bool operator==(FileTypefaceSource const& other) const {
    return data_->identifier == other.data_->identifier;
  }

  bool operator!=(FileTypefaceSource const& other) const {
    return !(*this == other);
  }

 private:
  std::shared_ptr<impl::FileTypefaceSourceData const> data_;
};

inline std::string format(FileTypefaceSource const& source) {
  return source.data_ref()->identifier;
}

struct MemoryTypefaceSource {
  explicit MemoryTypefaceSource(std::vector<uint8_t> bytes) {
    VLK_ENSURE(!bytes.empty());
    uint64_t uid = make_uid();
    std::string identifier = std::string("MemoryTypefaceSource{uid: ") +
                             std::to_string(uid) + std::string("}");
    data_ = std::make_shared<impl::MemoryTypefaceSourceData>(
        impl::MemoryTypefaceSourceData{std::move(bytes),
                                       std::move(identifier)});
  }

  auto data() const { return data_; }

  auto const& data_ref() const { return data_; }

  bool operator==(MemoryTypefaceSource const& other) const {
    return data_ref()->identifier == other.data_ref()->identifier;
  }

  bool operator!=(MemoryTypefaceSource const& other) const {
    return !(*this == other);
  }

 private:
  static uint64_t make_uid();

  std::shared_ptr<impl::MemoryTypefaceSourceData const> data_;
};

inline std::string format(MemoryTypefaceSource const& source) {
  return source.data_ref()->identifier;
}

template <typename TypefaceSource>
struct FontFace {
  TypefaceSource source;
  FontStyle style;

  bool operator==(FontFace const& other) const { return style == other.style; }

  bool operator!=(FontFace const& other) const { return !(*this == other); }
};

namespace impl {
struct FileFontSourceData {
  std::string family;
  std::vector<FontFace<FileTypefaceSource>> faces;
  std::string debug_identifier;
};

struct MemoryFontSourceData {
  std::string family;
  std::vector<FontFace<MemoryTypefaceSource>> faces;
  std::string debug_identifier;
};

struct SystemFontData {
  stx::Option<std::string> family = stx::None;

  //! style variant of system font to use
  FontStyle style = FontStyle{};

  std::string identifier;
};

}  // namespace impl

//! system font to use. uses default system font by default
struct SystemFont {
  SystemFont(std::string font_family, FontStyle font_style) {
    std::string identifier = "SystemFont{font: " + font_family + ", style: (" +
                             format(font_style) + ")}";
    data_ = std::make_shared<impl::SystemFontData const>(
        impl::SystemFontData{stx::Some(std::move(font_family)),
                             std::move(font_style), std::move(identifier)});
  }

  explicit SystemFont(std::string font_family)
      : SystemFont{std::move(font_family), FontStyle{}} {}

  explicit SystemFont(FontStyle font_style) {
    std::string identifier =
        "DefaultSystemFont{style: (" + format(font_style) + ")}";
    data_ = std::make_shared<impl::SystemFontData const>(impl::SystemFontData{
        stx::None, std::move(font_style), std::move(identifier)});
  }

  SystemFont() : SystemFont{FontStyle{}} {}

  auto data() const { return data_; }

  auto const& data_ref() const { return data_; }

  bool operator==(SystemFont const& other) const {
    return data_ref()->identifier == other.data_ref()->identifier;
  }

  bool operator!=(SystemFont const& other) const { return !(*this == other); }

 private:
  std::shared_ptr<impl::SystemFontData const> data_;
};

struct FileFontSource {
  FileFontSource(std::string family_name,
                 std::vector<FontFace<FileTypefaceSource>> font_faces) {
    std::string debug_identifier =
        fmt::format("FileFontSourcefamily(family: {}, faces: [", family_name);

    for (auto const& face : font_faces) {
      debug_identifier +=
          fmt::format("(id: {}, style: {}), ",
                      face.source.data_ref()->identifier, format(face.style));
    }

    debug_identifier += "])";

    data_ = std::make_shared<impl::FileFontSourceData>(
        impl::FileFontSourceData{std::move(family_name), std::move(font_faces),
                                 std::move(debug_identifier)});
  }

  auto data() const { return data_; }

  auto const& data_ref() const { return data_; }

  bool operator==(FileFontSource const& other) const {
    return data_ref()->debug_identifier == other.data_ref()->debug_identifier;
  }

  bool operator!=(FileFontSource const& other) const {
    return !(*this == other);
  }

 private:
  std::shared_ptr<impl::FileFontSourceData const> data_;
};

inline std::string format(FileFontSource const& source) {
  return source.data_ref()->debug_identifier;
}

struct MemoryFontSource {
  explicit MemoryFontSource(
      std::string family_name,
      std::vector<FontFace<MemoryTypefaceSource>> font_faces) {
    VLK_ENSURE(!font_faces.empty(), "font faces can not be empty");

    std::string debug_identifier =
        fmt::format("FileFontSourcefamily(family: {}, faces: [", family_name);

    for (auto const& face : font_faces) {
      debug_identifier +=
          fmt::format("(id: {}, style: {}), ",
                      face.source.data_ref()->identifier, format(face.style));
    }

    debug_identifier += "])";

    data_ =
        std::make_shared<impl::MemoryFontSourceData>(impl::MemoryFontSourceData{
            std::move(family_name), std::move(font_faces),
            std::move(debug_identifier)});
  }

  auto data() const { return data_; }

  auto const& data_ref() const { return data_; }

  bool operator==(MemoryFontSource const& other) const {
    return data_ref()->debug_identifier == other.data_ref()->debug_identifier;
  }

  bool operator!=(MemoryFontSource const& other) const {
    return !(*this == other);
  }

 private:
  std::shared_ptr<impl::MemoryFontSourceData const> data_;
};

inline std::string format(MemoryFontSource const& source) {
  return source.data_ref()->debug_identifier;
}

namespace impl {
inline uint64_t get_typeface_size(sk_sp<SkTypeface const> const& typeface) {
  uint64_t byte_size = 0U;
  uint64_t num_tables = typeface->countTables();
  std::vector<SkFontTableTag> table_tags;
  table_tags.resize(num_tables);

  for (uint64_t i = 0; i < num_tables; i++) {
    typeface->getTableTags(table_tags.data());
  }

  for (auto const& table_tag : table_tags) {
    byte_size += typeface->getTableSize(table_tag);
  }

  return byte_size;
}
}  // namespace impl

struct TypefaceAsset : public Asset {
  explicit TypefaceAsset(
      stx::Result<sk_sp<SkTypeface>, FontLoadError>&& load_result)
      : load_result_{std::move(load_result)} {
    load_result_.as_cref().match(
        [&](sk_sp<SkTypeface> const& typeface) {
          Asset::update_size(impl::get_typeface_size(typeface));
        },
        [&](FontLoadError) { Asset::update_size(0); });
  }

  auto const& load_result_ref() const { return load_result_; }

 private:
  stx::Result<sk_sp<SkTypeface>, FontLoadError> load_result_;
};

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
  virtual std::unique_ptr<Asset> load(RenderContext const&,
                                      AssetLoadArgs const& args) const override;

  static std::shared_ptr<TypefaceLoader const> get_default();
};

}  // namespace impl

namespace impl {
template <typename FontSourceType>
auto const& get_typeface_source(FontSourceType const& font_source,
                                FontStyle style) {
  auto const& faces = font_source.data_ref()->faces;
  VLK_ENSURE(!faces.empty());
  auto const pos =
      std::find_if(faces.begin(), faces.end(),
                   [&](auto const& face) { return face.style == style; });
  if (pos == faces.end()) {
    VLK_WARN(
        "specified font style: {}, does not match any of the styles in "
        "the specified "
        "typefaces of the font source: {}. "
        "The first font in the typefaces of the font source will be used",
        format(style), font_source.data_ref()->debug_identifier);
    return faces[0].source;
  } else {
    return pos->source;
  }
}
}  // namespace impl

struct FileFont {
  explicit FileFont(FileFontSource font_source, FontStyle style = FontStyle{})
      : source{impl::get_typeface_source(font_source, style)} {}

  explicit FileFont(FileTypefaceSource typeface_source)
      : source{std::move(typeface_source)} {}

  FileTypefaceSource source;

  bool operator==(FileFont const& other) const {
    return source == other.source;
  }

  bool operator!=(FileFont const& other) const { return !(*this == other); }
};

struct MemoryFont {
  explicit MemoryFont(MemoryFontSource font_source,
                      FontStyle style = FontStyle{})
      : source{impl::get_typeface_source(font_source, style)} {}

  explicit MemoryFont(MemoryTypefaceSource typeface_source)
      : source{std::move(typeface_source)} {}

  MemoryTypefaceSource source;

  bool operator==(MemoryFont const& other) const {
    return source == other.source;
  }

  bool operator!=(MemoryFont const& other) const { return !(*this == other); }
};

inline auto add_font_asset(AssetManager& asset_manager,
                           FileTypefaceSource const& typeface_source) {
  return asset_manager.add(
      typeface_source.data_ref()->identifier,
      std::make_unique<impl::TypefaceLoadArgs>(
          impl::TypefaceLoadArgs{typeface_source.data_ref()}),
      impl::TypefaceLoader::get_default());
}

inline auto add_font_asset(AssetManager& asset_manager,
                           MemoryTypefaceSource const& typeface_source) {
  return asset_manager.add(
      typeface_source.data_ref()->identifier,
      std::make_unique<impl::TypefaceLoadArgs>(
          impl::TypefaceLoadArgs{typeface_source.data_ref()}),
      impl::TypefaceLoader::get_default());
}

inline auto add_font_asset(AssetManager& asset_manager,
                           SystemFont const& system_font) {
  return asset_manager.add(system_font.data_ref()->identifier,
                           std::make_unique<impl::TypefaceLoadArgs>(
                               impl::TypefaceLoadArgs{system_font.data_ref()}),
                           impl::TypefaceLoader::get_default());
}

inline auto add_font_asset(AssetManager& asset_manager,
                           FileFont const& file_font) {
  return add_font_asset(asset_manager, file_font.source);
}

inline auto add_font_asset(AssetManager& asset_manager,
                           MemoryFont const& memory_font) {
  return add_font_asset(asset_manager, memory_font.source);
}

inline stx::Result<std::shared_ptr<TypefaceAsset const>, AssetError>
get_font_asset(AssetManager& asset_manager,
               FileTypefaceSource const& typeface_source) {
  TRY_OK(asset, asset_manager.get(typeface_source.data_ref()->identifier));
  auto typeface_asset = std::dynamic_pointer_cast<TypefaceAsset const>(asset);
  VLK_ENSURE(typeface_asset != nullptr);
  return stx::Ok(std::move(typeface_asset));
}

inline stx::Result<std::shared_ptr<TypefaceAsset const>, AssetError>
get_font_asset(AssetManager& asset_manager, SystemFont const& system_font) {
  TRY_OK(asset, asset_manager.get(system_font.data_ref()->identifier));
  auto typeface_asset = std::dynamic_pointer_cast<TypefaceAsset const>(asset);
  VLK_ENSURE(typeface_asset != nullptr);
  return stx::Ok(std::move(typeface_asset));
}

inline stx::Result<std::shared_ptr<TypefaceAsset const>, AssetError>
get_font_asset(AssetManager& asset_manager,
               MemoryTypefaceSource const& typeface_source) {
  TRY_OK(asset, asset_manager.get(typeface_source.data_ref()->identifier));
  auto typeface_asset = std::dynamic_pointer_cast<TypefaceAsset const>(asset);
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

}  // namespace ui
}  // namespace vlk
