#pragma once
#include "ashura/primitives.h"

namespace ash
{

struct UnicodeRange
{
  u32 first = 0;
  u32 last  = 0;
};

namespace unicode_ranges
{
constexpr UnicodeRange BASIC_LATIN                 = UnicodeRange{0x0020, 0x007F};
constexpr UnicodeRange LATIN1_SUPPLEMENT           = UnicodeRange{0x00A0, 0x00FF};
constexpr UnicodeRange LATIN_EXTENDED_A            = UnicodeRange{0x0100, 0x017F};
constexpr UnicodeRange LATIN_EXTENDED_B            = UnicodeRange{0x0180, 0x024F};
constexpr UnicodeRange COMBINING_DIACRITICAL_MARKS = UnicodeRange{0x0300, 0x036F};
constexpr UnicodeRange ARABIC                      = UnicodeRange{0x0600, 0x06FF};
constexpr UnicodeRange GENERAL_PUNCTUATION         = UnicodeRange{0x2000, 0x206F};
constexpr UnicodeRange SUPERSCRIPTS_AND_SUBSCRIPTS = UnicodeRange{0x2070, 0x209F};
constexpr UnicodeRange CURRENCY_SYMBOLS            = UnicodeRange{0x20A0, 0x20CF};
constexpr UnicodeRange NUMBER_FORMS                = UnicodeRange{0x2150, 0x218F};
constexpr UnicodeRange ARROWS                      = UnicodeRange{0x2190, 0x21FF};
constexpr UnicodeRange MATHEMATICAL_OPERATORS      = UnicodeRange{0x2200, 0x22FF};
constexpr UnicodeRange HIRAGANA                    = UnicodeRange{0x3040, 0x309F};
constexpr UnicodeRange KATAKANA                    = UnicodeRange{0x30A0, 0x30FF};
};        // namespace unicode_ranges

};        // namespace ash