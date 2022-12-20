#pragma once

#include "ashura/image.h"
#include "ashura/primitives.h"
#include "stb_truetype.h"
#include "stx/allocator.h"
#include "stx/span.h"
#include "stx/string.h"
#include "stx/vec.h"

namespace asr {

enum class TextAlign : u8 { Left, Right, Center };

enum class WordWrap : u8 { None, Wrap };

enum class TextDirection : u8 {
  LeftToRight,
  RightToLeft,
  TopToBottom,
  ButtomToTop
};

struct TextStyle {
  f32 font_height = 10;
  f32 letter_spacing = 0;
  f32 word_spacing = 0;
  TextDirection direction = TextDirection::LeftToRight;
  TextAlign align = TextAlign::Left;
  WordWrap word_wrap = WordWrap::None;
};

enum class TypefaceLoadError { PackFailed, InvalidData };

struct TypefaceAtlasInfo {
  f32 font_size = 40;
  extent extent{1024, 1024};
  u32 oversample_x = 2;
  u32 oversample_y = 2;
  u32 first_char = ' ';
  u32 char_count = '~' - ' ';
};

struct Typeface {
  TypefaceAtlasInfo info;
  stx::Vec<stbtt_packedchar> glyphs{stx::os_allocator};
  gfx::Image atlas;

  vec2 layout(stx::StringView text, TextStyle const& style);
};

}  // namespace asr