#pragma once
#include <filesystem>
#include <string_view>

#include "ashura/image.h"
#include "ashura/primitives.h"
#include "freetype/freetype.h"
#include "stb_truetype.h"
#include "stx/allocator.h"
#include "stx/limits.h"
#include "stx/span.h"
#include "stx/string.h"
#include "stx/vec.h"

namespace asr {

enum class TextAlign : u8 { Left, Right, Center };

enum class WordWrap : u8 { None, Wrap };

enum class TextDirection : u8 { LeftToRight, RightToLeft };

struct TextStyle {
  f32 font_height = 10;
  f32 letter_spacing = 0;
  f32 word_spacing = 0;
  TextDirection direction = TextDirection::LeftToRight;
  TextAlign align = TextAlign::Left;
  WordWrap word_wrap = WordWrap::None;
  u32 num_tab_spaces = 4;
};

enum class TypefaceLoadError { PackFailed, InvalidData };

namespace unicode_ranges {
constexpr range ENGLISH{0x020, 0x007F};  // a.k.a. LATIN ASCII
constexpr range EMOTICONS{0x1F600, 0x1F64F};
constexpr range CURRENCY_SYMBOLS{0x20A0, 0x20CF};
constexpr range ARROWS{0x2190, 0x21FF};
constexpr range MATHEMATICAL_OPERATORS{0x2200, 0x22FF};
constexpr range ARABIC{0x0600, 0x06FF};
constexpr range HIRAGANA{0x3040, 0x309F};
constexpr range KATAKANA{0x30A0, 0x30FF};
}  // namespace unicode_ranges

// caching strategy:
//
//

// TODO(lamarrr): font rendering algorithm
//
//
// - is character in availabe unicode value range? if so use, if not load the
// glyphs for the unicode range into memory
//
//
//
//
/*struct TypefaceAtlasConfig {
  f32 atlas_font_height = 40;
  extent atlas_extent{1024, 1024};
  u32 oversample_x = 2;
  u32 oversample_y = 2;
  u32 first_char = ' ';
  u32 char_count = '~' - ' ';

  constexpr bool has_character(u32 character) const {
    return character >= first_char && character < (first_char + char_count);
  }
};
*/

struct GlyphPack {};

struct Font {};
enum class FontLoadError { InvalidPath };

stx::Result<Font, FontLoadError> load_font(std::string_view path,
                                           range unicode_range) {
  if (!std::filesystem::exists(path)) {
    return stx::Err(FontLoadError::I)
  }
}

struct Typeface {
  TypefaceAtlasConfig config;
  stx::Vec<stbtt_packedchar> glyphs{stx::os_allocator};
  Image atlas;

  vec2 layout();

  vec2 layout(stx::StringView text, TextStyle const& style,
              vec2 max_width = {stx::f32_max, stx::f32_max}) const {
    vec2 extent{0, 0};

    for (char character : text) {
      switch (character) {
        case ' ': {
          switch (style.direction) {
            case TextDirection::LeftToRight: {
              switch (style.word_wrap) {}
              vec2 new_extent = extent + vec2{AS_F32(word_spacing), 0} + vec2{};
              // new_extent.x >

            } break;

            default: {
              ASR_PANIC(unsupported text direction);
            } break;
          }
        } break;

        case '\t': {
          switch (style.direction) {
            case TextDirection::LeftToRight: {
            } break;
            default: {
              ASR_PANIC(unsupported text direction);
            } break;
          }
        }

        default: {
          if (!info.has_character(character)) {
            continue;
          }
        }
      }
    }
  }
};

}  // namespace asr