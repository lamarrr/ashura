#pragma once
#include "ashura/primitives.h"
#include <string_view>

namespace ash
{

struct unicode_range
{
  u32 first = 0;
  u32 last  = 0;
};

namespace unicode_ranges
{
constexpr unicode_range BASIC_LATIN                 = unicode_range{0x0020, 0x007F};
constexpr unicode_range LATIN1_SUPPLEMENT           = unicode_range{0x00A0, 0x00FF};
constexpr unicode_range LATIN_EXTENDED_A            = unicode_range{0x0100, 0x017F};
constexpr unicode_range LATIN_EXTENDED_B            = unicode_range{0x0180, 0x024F};
constexpr unicode_range COMBINING_DIACRITICAL_MARKS = unicode_range{0x0300, 0x036F};
constexpr unicode_range ARABIC                      = unicode_range{0x0600, 0x06FF};
constexpr unicode_range GENERAL_PUNCTUATION         = unicode_range{0x2000, 0x206F};
constexpr unicode_range SUPERSCRIPTS_AND_SUBSCRIPTS = unicode_range{0x2070, 0x209F};
constexpr unicode_range CURRENCY_SYMBOLS            = unicode_range{0x20A0, 0x20CF};
constexpr unicode_range NUMBER_FORMS                = unicode_range{0x2150, 0x218F};
constexpr unicode_range ARROWS                      = unicode_range{0x2190, 0x21FF};
constexpr unicode_range MATHEMATICAL_OPERATORS      = unicode_range{0x2200, 0x22FF};
constexpr unicode_range HIRAGANA                    = unicode_range{0x3040, 0x309F};
constexpr unicode_range KATAKANA                    = unicode_range{0x30A0, 0x30FF};
};        // namespace unicode_ranges

};        // namespace ash