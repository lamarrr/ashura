/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/error.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

[[nodiscard]] constexpr bool is_valid_utf8(Span<u8 const> text);

/// @brief count number of utf8 codepoints found in the text. does no
/// utf8-validation
[[nodiscard]] constexpr usize count_utf8_codepoints(Span<u8 const> text)
{
  u8 const *in    = text.data();
  usize     count = 0;
  while (in != text.end())
  {
    if ((*in & 0xc0) != 0x80)
    {
      count++;
    }
    in++;
  }
  return count;
}

/// @brief decoded.size() must be at least count_utf8_codepoints(encoded).
/// estimate: encoded.size()
[[nodiscard]] constexpr usize utf8_decode(Span<u8 const> encoded,
                                          Span<u32>      decoded)
{
  u8 const *in  = encoded.data();
  u32      *out = decoded.data();
  while (in != encoded.end())
  {
    if ((*in & 0xF8) == 0xF0)
    {
      u32 c1 = *in++;
      u32 c2 = *in++;
      u32 c3 = *in++;
      u32 c4 = *in++;
      *out++ = c1 << 24 | c2 << 16 | c3 << 8 | c4;
    }
    else if ((*in & 0xF0) == 0xE0)
    {
      u32 c1 = *in++;
      u32 c2 = *in++;
      u32 c3 = *in++;
      *out++ = c1 << 16 | c2 << 8 | c3;
    }
    else if ((*in & 0xE0) == 0xC0)
    {
      u32 c1 = *in++;
      u32 c2 = *in++;
      *out++ = c1 << 8 | c2;
    }
    else
    {
      u32 c1 = *in++;
      *out++ = c1;
    }
  }
  return out - decoded.begin();
}

/// @brief encoded.size must be at least decoded.size * 4
[[nodiscard]] constexpr usize utf8_encode(Span<u32 const> decoded,
                                          Span<u8>        encoded)
{
  u8        *out = encoded.data();
  u32 const *in  = decoded.data();

  while (in != decoded.end())
  {
    if (*in <= 0x7F)
    {
      *out = *in;
      out += 1;
    }
    if (*in <= 0x7FF)
    {
      out[0] = 0xC0 | (*in >> 6);
      out[1] = 0x80 | (*in & 0x3F);
      out += 2;
    }
    if (*in <= 0xFFFF)
    {
      out[0] = 0xE0 | (*in >> 12);
      out[1] = 0x80 | ((*in >> 6) & 0x3F);
      out[2] = 0x80 | (*in & 0x3F);
      out += 3;
    }
    if (*in <= 0x10FFFF)
    {
      out[0] = 0xF0 | (*in >> 18);
      out[1] = 0x80 | ((*in >> 12) & 0x3F);
      out[2] = 0x80 | ((*in >> 6) & 0x3F);
      out[3] = 0x80 | (*in & 0x3F);
      out += 4;
    }
  }

  return out - encoded.begin();
}

/// @brief converts UTF-8 text from @p encoded to UTF-32 and appends into @p
/// `decoded`
inline Result<Void, Void> utf8_decode(Span<u8 const> encoded, Vec<u32> &decoded)
{
  usize const first = decoded.size();
  usize const count = count_utf8_codepoints(encoded);
  if (!decoded.extend_uninit(count))
  {
    return Err{};
  }
  (void) utf8_decode(encoded, span(decoded).slice(first, count));
  return Ok{};
}

/// @brief converts UTF-32 text from @p decoded to UTF-8 and appends into @p
/// `encoded`
[[nodiscard]] inline Result<Void, Void> utf8_encode(Span<u32 const> decoded,
                                                    Vec<u8>        &encoded)
{
  usize const first     = encoded.size();
  usize const max_count = decoded.size();
  if (!encoded.extend_uninit(max_count))
  {
    return Err{};
  }
  usize const count =
      utf8_encode(decoded, span(encoded).slice(first, max_count));
  CHECK(!encoded.resize_defaulted(first + count));
  return Ok{};
}

constexpr void replace_invalid_codepoints(Span<u32 const> input,
                                          Span<u32> output, u32 replacement)
{
  u32 const *in  = input.begin();
  u32       *out = output.begin();

  while (in < input.end())
  {
    if (*in > 0x10FFFF) [[unlikely]]
    {
      *out = replacement;
    }
    else
    {
      *out = *in;
    }
    in++;
    out++;
  }
}

/// Unicode Ranges

constexpr Slice32 UTF_BASIC_LATIN{0x0020, 0x007F - 0x0020};
constexpr Slice32 UTF_LATIN1_SUPPLEMENT{0x00A0, 0x00FF - 0x00A0};
constexpr Slice32 UTF_LATIN_EXTENDED_A{0x0100, 0x017F - 0x0100};
constexpr Slice32 UTF_LATIN_EXTENDED_B{0x0180, 0x024F - 0x0180};
constexpr Slice32 UTF_COMBINING_DIACRITICAL_MARKS{0x0300, 0x036F - 0x0300};
constexpr Slice32 UTF_ARABIC{0x0600, 0x06FF - 0x0600};
constexpr Slice32 UTF_GENERAL_PUNCTUATION{0x2000, 0x206F - 0x2000};
constexpr Slice32 UTF_SUPERSCRIPTS_AND_SUBSCRIPTS{0x2070, 0x209F - 0x2070};
constexpr Slice32 UTF_CURRENCY_SYMBOLS{0x20A0, 0x20CF - 0x20A0};
constexpr Slice32 UTF_NUMBER_FORMS{0x2150, 0x218F - 0x2150};
constexpr Slice32 UTF_ARROWS{0x2190, 0x21FF - 0x2190};
constexpr Slice32 UTF_MATHEMATICAL_OPERATORS{0x2200, 0x22FF - 0x2200};
constexpr Slice32 UTF_HIRAGANA{0x3040, 0x309F - 0x3040};
constexpr Slice32 UTF_KATAKANA{0x30A0, 0x30FF - 0x30A0};

}        // namespace ash
