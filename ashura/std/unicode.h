/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/types.h"

namespace ash
{

struct UnicodeRange
{
  u32 first = 0;
  u32 last  = 0;
};

namespace unicode_ranges
{
constexpr UnicodeRange BASIC_LATIN{0x0020, 0x007F};
constexpr UnicodeRange LATIN1_SUPPLEMENT{0x00A0, 0x00FF};
constexpr UnicodeRange LATIN_EXTENDED_A{0x0100, 0x017F};
constexpr UnicodeRange LATIN_EXTENDED_B{0x0180, 0x024F};
constexpr UnicodeRange COMBINING_DIACRITICAL_MARKS{0x0300, 0x036F};
constexpr UnicodeRange ARABIC{0x0600, 0x06FF};
constexpr UnicodeRange GENERAL_PUNCTUATION{0x2000, 0x206F};
constexpr UnicodeRange SUPERSCRIPTS_AND_SUBSCRIPTS{0x2070, 0x209F};
constexpr UnicodeRange CURRENCY_SYMBOLS{0x20A0, 0x20CF};
constexpr UnicodeRange NUMBER_FORMS{0x2150, 0x218F};
constexpr UnicodeRange ARROWS{0x2190, 0x21FF};
constexpr UnicodeRange MATHEMATICAL_OPERATORS{0x2200, 0x22FF};
constexpr UnicodeRange HIRAGANA{0x3040, 0x309F};
constexpr UnicodeRange KATAKANA{0x30A0, 0x30FF};
};        // namespace unicode_ranges

}        // namespace ash
