

#include "vlk/font_source.h"

#include <atomic>

#include "fmt/format.h"

namespace vlk {

uint64_t MemoryTypefaceSource::make_uid() {
  static std::atomic<uint64_t> latest_uid = 0;

  return latest_uid.fetch_add(1, std::memory_order_seq_cst);
}

FileTypefaceSource::FileTypefaceSource(std::filesystem::path path)
    : data{stx::rc::make_inplace<FileTypefaceSourceData const>(
               stx::os_allocator, path,
               fmt::format("FileTypeface(path: {})", path.c_str()))
               .unwrap()} {}

namespace {

inline auto make_typeface_source_data(std::vector<uint8_t>&& bytes) {
  uint64_t uid = MemoryTypefaceSource::make_uid();

  return stx::rc::make_inplace<MemoryTypefaceSourceData const>(
             stx::os_allocator, std::move(bytes),
             fmt::format("MemoryTypeface(uid: {})", uid))
      .unwrap();
}

inline auto make_system_font_data(stx::Option<std::string>&& font_family,
                                  FontStyle font_style) {
  auto font_family_str =
      font_family.match([](auto const& font) -> std::string { return font; },
                        []() -> std::string { return "SYSTEM_DEFAULT"; });

  auto tag = fmt::format("SystemFont(family: '{}', style: '{}')",
                         font_family_str, format(font_style));

  return stx::rc::make_inplace<SystemFontData const>(stx::os_allocator,
                                                     std::move(font_family),
                                                     font_style, std::move(tag))
      .unwrap();
}

inline auto make_file_font_source_data(std::string&& family_name,
                                       std::vector<FileFontFace>&& font_faces) {
  std::string tag = fmt::format("FileFont(family: {}, faces: [", family_name);

  for (auto const& face : font_faces) {
    tag += fmt::format("(typeface: {}, style: {}), ", format(face.source),
                       format(face.style));
  }

  tag += "])";

  return stx::rc::make_inplace<FileFontSourceData const>(
             stx::os_allocator, std::move(family_name), std::move(font_faces),
             std::move(tag))
      .unwrap();
}

inline auto make_memory_font_source_data(
    std::string&& family_name, std::vector<MemoryFontFace>&& font_faces) {
  std::string tag = fmt::format("FileFont(family: {}, faces: [", family_name);

  for (auto const& face : font_faces) {
    tag += fmt::format("(typeface: {}, style: {}), ", format(face.source),
                       format(face.style));
  }

  tag += "])";

  return stx::rc::make_inplace<MemoryFontSourceData const>(
             stx::os_allocator, std::move(family_name), std::move(font_faces),
             std::move(tag))
      .unwrap();
}

}  // namespace

// TODO(lamarrr): check for emptiness in loader (bytes and vectors?)
MemoryTypefaceSource::MemoryTypefaceSource(std::vector<uint8_t>&& bytes)
    : data{make_typeface_source_data(std::move(bytes))} {}

SystemFont::SystemFont(std::string font_family, FontStyle font_style)
    : data{make_system_font_data(stx::Some(std::move(font_family)),
                                 font_style)} {}

SystemFont::SystemFont(FontStyle font_style)
    : data{make_system_font_data(stx::None, font_style)} {}

FileFontSource::FileFontSource(std::string family_name,
                               std::vector<FileFontFace> font_faces)
    : data{make_file_font_source_data(std::move(family_name),
                                      std::move(font_faces))} {}

MemoryFontSource::MemoryFontSource(std::string family_name,
                                   std::vector<MemoryFontFace> font_faces)
    : data{make_memory_font_source_data(std::move(family_name),
                                        std::move(font_faces))} {}

}  // namespace vlk
