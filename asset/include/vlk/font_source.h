#pragma once

#include <filesystem>
#include <string>
#include <variant>
#include <vector>

#include "stx/mem.h"
#include "stx/span.h"
#include "vlk/asset_tag.h"
#include "vlk/font_style.h"

namespace vlk {

struct FileTypefaceSourceData {
  std::filesystem::path path;
  std::string tag;
};

struct MemoryTypefaceSourceData {
  std::vector<uint8_t> bytes;
  std::string tag;
};

struct FileTypefaceSource {
  explicit FileTypefaceSource(std::filesystem::path path);

  FileTypefaceSource(FileTypefaceSource const& other)
      : data{other.data.share()} {}

  FileTypefaceSource& operator=(FileTypefaceSource const& other) {
    data = other.data.share();
    return *this;
  }

  FileTypefaceSource(FileTypefaceSource&&) = default;

  FileTypefaceSource& operator=(FileTypefaceSource&&) = default;

  auto get_tag() const {
    return AssetTag{
        stx::transmute(std::string_view{data.handle->tag}, data.share())};
  }

  auto get_path() const { return data.handle->path; }

  bool operator==(FileTypefaceSource const& other) const {
    return data.handle->tag == other.data.handle->tag;
  }

  bool operator!=(FileTypefaceSource const& other) const {
    return !(*this == other);
  }

  Rc<FileTypefaceSourceData const*> data;
};

inline std::string format(FileTypefaceSource const& source) {
  return source.data.handle->tag;
}

struct MemoryTypefaceSource {
  explicit MemoryTypefaceSource(std::vector<uint8_t>&& bytes);

  MemoryTypefaceSource(MemoryTypefaceSource const& other)
      : data{other.data.share()} {}

  MemoryTypefaceSource& operator=(MemoryTypefaceSource const& other) {
    data = other.data.share();
    return *this;
  }

  MemoryTypefaceSource(MemoryTypefaceSource&&) = default;

  MemoryTypefaceSource& operator=(MemoryTypefaceSource&&) = default;

  auto get_tag() const {
    return AssetTag{
        stx::transmute(std::string_view{data.handle->tag}, data.share())};
  }

  auto get_bytes() const {
    return stx::transmute(stx::Span<uint8_t const>{data.handle->bytes.data(),
                                                   data.handle->bytes.size()},
                          data.share());
  }

  bool operator==(MemoryTypefaceSource const& other) const {
    return data.handle->tag == other.data.handle->tag;
  }

  bool operator!=(MemoryTypefaceSource const& other) const {
    return !(*this == other);
  }

  static uint64_t make_uid();

  Rc<MemoryTypefaceSourceData const*> data;
};

inline std::string format(MemoryTypefaceSource const& source) {
  return source.data.handle->tag;
}

struct MemoryFontFace {
  MemoryTypefaceSource source;
  FontStyle style;

  bool operator==(MemoryFontFace const& other) const {
    return style == other.style;
  }

  bool operator!=(MemoryFontFace const& other) const {
    return !(*this == other);
  }
};

struct FileFontFace {
  FileTypefaceSource source;
  FontStyle style;

  bool operator==(FileFontFace const& other) const {
    return style == other.style;
  }

  bool operator!=(FileFontFace const& other) const { return !(*this == other); }
};

struct SystemFontData {
  stx::Option<std::string> family;

  /// style variant of system font to use
  FontStyle style;

  std::string tag;
};

struct FileFontSourceData {
  std::string family;
  std::vector<FileFontFace> faces;
  std::string tag;
};

struct MemoryFontSourceData {
  std::string family;
  std::vector<MemoryFontFace> faces;
  std::string tag;
};

struct SystemFont {
  explicit SystemFont(std::string font_family,
                      FontStyle font_style = FontStyle{});

  // uses the default system font
  explicit SystemFont(FontStyle font_style);

  SystemFont() : SystemFont{FontStyle{}} {}

  SystemFont(SystemFont const& other) : data{other.data.share()} {}

  SystemFont& operator=(SystemFont const& other) {
    data = other.data.share();
    return *this;
  }

  SystemFont(SystemFont&&) = default;

  SystemFont& operator=(SystemFont&&) = default;

  auto get_tag() const {
    return AssetTag{
        stx::transmute(std::string_view{data.handle->tag}, data.share())};
  }

  auto get_family() const { return data.handle->family; }

  auto get_style() const { return data.handle->style; }

  bool operator==(SystemFont const& other) const {
    return data.handle->tag == other.data.handle->tag;
  }

  bool operator!=(SystemFont const& other) const { return !(*this == other); }

  Rc<SystemFontData const*> data;
};

struct FileFontSource {
  FileFontSource(std::string family_name, std::vector<FileFontFace> font_faces);

  FileFontSource(FileFontSource const& other) : data{other.data.share()} {}

  FileFontSource& operator=(FileFontSource const& other) {
    data = other.data.share();
    return *this;
  }

  FileFontSource(FileFontSource&&) = default;

  FileFontSource& operator=(FileFontSource&&) = default;

  AssetTag get_tag() const {
    return AssetTag{
        stx::transmute(std::string_view{data.handle->tag}, data.share())};
  }

  auto get_family() const { return data.handle->family; }

  auto get_typefaces() const {
    return stx::transmute(
        stx::Span{data.handle->faces.data(), data.handle->faces.size()},
        data.share());
  }

  bool operator==(FileFontSource const& other) const {
    return data.handle->tag == other.data.handle->tag;
  }

  bool operator!=(FileFontSource const& other) const {
    return !(*this == other);
  }

  Rc<FileFontSourceData const*> data;
};

inline std::string format(FileFontSource const& source) {
  return source.data.handle->tag;
}

struct MemoryFontSource {
  explicit MemoryFontSource(std::string family_name,
                            std::vector<MemoryFontFace> font_faces);

  MemoryFontSource(MemoryFontSource const& other) : data{other.data.share()} {}

  MemoryFontSource& operator=(MemoryFontSource const& other) {
    data = other.data.share();
    return *this;
  }

  MemoryFontSource(MemoryFontSource&&) = default;

  MemoryFontSource& operator=(MemoryFontSource&&) = default;

  AssetTag get_tag() const {
    return AssetTag{
        stx::transmute(std::string_view{data.handle->tag}, data.share())};
  }

  auto get_family() const { return data.handle->family; }

  auto get_typefaces() const {
    return stx::transmute(
        stx::Span{data.handle->faces.data(), data.handle->faces.size()},
        data.share());
  }

  bool operator==(MemoryFontSource const& other) const {
    return data.handle->tag == other.data.handle->tag;
  }

  bool operator!=(MemoryFontSource const& other) const {
    return !(*this == other);
  }

  Rc<MemoryFontSourceData const*> data;
};

inline std::string format(MemoryFontSource const& source) {
  return source.data.handle->tag;
}

using FontSource =
    std::variant<SystemFont, FileTypefaceSource, MemoryTypefaceSource,
                 FileFontSource, MemoryFontSource>;

}  // namespace vlk
