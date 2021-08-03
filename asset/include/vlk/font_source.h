#pragma once

#include <filesystem>
#include <string>
#include <variant>
#include <vector>

#include "stx/mem.h"
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

  auto get_tag() const {
    return AssetTag{stx::transmute(std::string_view{data.get()->tag}, data)};
  }

  auto get_path() const { return data.get()->path; }

  bool operator==(FileTypefaceSource const& other) const {
    return data.get()->tag == other.data.get()->tag;
  }

  bool operator!=(FileTypefaceSource const& other) const {
    return !(*this == other);
  }

  stx::mem::Rc<FileTypefaceSourceData const> data;
};

inline std::string format(FileTypefaceSource const& source) {
  return source.data.get()->tag;
}

struct MemoryTypefaceSource {
  explicit MemoryTypefaceSource(std::vector<uint8_t>&& bytes);

  auto get_tag() const {
    return AssetTag{stx::transmute(std::string_view{data.get()->tag}, data)};
  }

  auto get_bytes() const {
    return stx::transmute(stx::Span{data.get()->bytes}, data);
  }

  bool operator==(MemoryTypefaceSource const& other) const {
    return data.get()->tag == other.data.get()->tag;
  }

  bool operator!=(MemoryTypefaceSource const& other) const {
    return !(*this == other);
  }

  static uint64_t make_uid();

  stx::mem::Rc<MemoryTypefaceSourceData const> data;
};

inline std::string format(MemoryTypefaceSource const& source) {
  return source.data.get()->tag;
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

  auto get_tag() const {
    return AssetTag{stx::transmute(std::string_view{data.get()->tag}, data)};
  }

  auto get_family() const { return data.get()->family; }

  auto get_style() const { return data.get()->style; }

  bool operator==(SystemFont const& other) const {
    return data.get()->tag == other.data.get()->tag;
  }

  bool operator!=(SystemFont const& other) const { return !(*this == other); }

  stx::mem::Rc<SystemFontData const> data;
};

struct FileFontSource {
  FileFontSource(std::string family_name, std::vector<FileFontFace> font_faces);

  AssetTag get_tag() const {
    return AssetTag{stx::transmute(std::string_view{data.get()->tag}, data)};
  }

  auto get_family() const { return data.get()->family; }

  auto get_typefaces() const {
    return stx::transmute(stx::Span{data.get()->faces}, data);
  }

  bool operator==(FileFontSource const& other) const {
    return data.get()->tag == other.data.get()->tag;
  }

  bool operator!=(FileFontSource const& other) const {
    return !(*this == other);
  }

  stx::mem::Rc<FileFontSourceData const> data;
};

inline std::string format(FileFontSource const& source) {
  return source.data.get()->tag;
}

struct MemoryFontSource {
  explicit MemoryFontSource(std::string family_name,
                            std::vector<MemoryFontFace> font_faces);

  AssetTag get_tag() const {
    return AssetTag{stx::transmute(std::string_view{data.get()->tag}, data)};
  }

  auto get_family() const { return data.get()->family; }

  auto get_typefaces() const {
    return stx::transmute(stx::Span{data.get()->faces}, data);
  }

  bool operator==(MemoryFontSource const& other) const {
    return data.get()->tag == other.data.get()->tag;
  }

  bool operator!=(MemoryFontSource const& other) const {
    return !(*this == other);
  }

  stx::mem::Rc<MemoryFontSourceData const> data;
};

inline std::string format(MemoryFontSource const& source) {
  return source.data.get()->tag;
}

using FontSource =
    std::variant<SystemFont, FileTypefaceSource, MemoryTypefaceSource,
                 FileFontSource, MemoryFontSource>;

}  // namespace vlk
