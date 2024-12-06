/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/error.h"
#include "ashura/std/result.h"
#include "ashura/std/tuple.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

[[nodiscard]] constexpr bool is_valid_utf8(Span<c8 const> text);

/// @brief count number of utf8 codepoints found in the text. does no
/// utf8-validation
[[nodiscard]] constexpr usize count_utf8_codepoints(Span<c8 const> text)
{
  c8 const * in    = text.data();
  usize      count = 0;
  while (in != text.pend())
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
[[nodiscard]] constexpr usize utf8_decode(Span<c8 const> encoded,
                                          Span<c32>      decoded)
{
  c8 const * in  = encoded.data();
  c32 *      out = decoded.data();
  while (in != encoded.pend())
  {
    if ((*in & 0xF8) == 0xF0)
    {
      c32 c1 = *in++;
      c32 c2 = *in++;
      c32 c3 = *in++;
      c32 c4 = *in++;
      *out++ = c1 << 24 | c2 << 16 | c3 << 8 | c4;
    }
    else if ((*in & 0xF0) == 0xE0)
    {
      c32 c1 = *in++;
      c32 c2 = *in++;
      c32 c3 = *in++;
      *out++ = c1 << 16 | c2 << 8 | c3;
    }
    else if ((*in & 0xE0) == 0xC0)
    {
      c32 c1 = *in++;
      c32 c2 = *in++;
      *out++ = c1 << 8 | c2;
    }
    else
    {
      c32 c1 = *in++;
      *out++ = c1;
    }
  }
  return out - decoded.pbegin();
}

/// @brief encoded.size must be at least decoded.size * 4
[[nodiscard]] constexpr usize utf8_encode(Span<c32 const> decoded,
                                          Span<c8>        encoded)
{
  c8 *        out = encoded.data();
  c32 const * in  = decoded.data();

  while (in != decoded.pend())
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
    if (*in <= 0x10'FFFF)
    {
      out[0] = 0xF0 | (*in >> 18);
      out[1] = 0x80 | ((*in >> 12) & 0x3F);
      out[2] = 0x80 | ((*in >> 6) & 0x3F);
      out[3] = 0x80 | (*in & 0x3F);
      out += 4;
    }
  }

  return out - encoded.pbegin();
}

/// @brief converts UTF-8 text from @p encoded to UTF-32 and appends into @p
/// `decoded`
inline Result<> utf8_decode(Span<c8 const> encoded, Vec<c32> & decoded)
{
  usize const first = decoded.size();
  usize const count = count_utf8_codepoints(encoded);
  if (!decoded.extend_uninit(count))
  {
    return Err{};
  }
  (void) utf8_decode(encoded, decoded.view().slice(first, count));
  return Ok{};
}

/// @brief converts UTF-32 text from @p decoded to UTF-8 and appends into @p
/// `encoded`
[[nodiscard]] inline Result<> utf8_encode(Span<c32 const> decoded,
                                          Vec<c8> &       encoded)
{
  usize const first     = encoded.size();
  usize const max_count = decoded.size();
  if (!encoded.extend_uninit(max_count))
  {
    return Err{};
  }
  usize const count =
      utf8_encode(decoded, encoded.view().slice(first, max_count));
  encoded.resize_uninit(first + count).unwrap();
  return Ok{};
}

constexpr void replace_invalid_codepoints(Span<c32 const> input,
                                          Span<c32> output, c32 replacement)
{
  c32 const * in  = input.pbegin();
  c32 *       out = output.pbegin();

  while (in < input.pend())
  {
    if (*in > 0x10'FFFF) [[unlikely]]
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
namespace utf
{
inline constexpr Tuple<u32, u32> BASIC_LATIN{0x0020, 0x007F};
inline constexpr Tuple<u32, u32> LATIN1_SUPPLEMENT{0x00A0, 0x00FF};
inline constexpr Tuple<u32, u32> LATIN_EXTENDED_A{0x0100, 0x017F};
inline constexpr Tuple<u32, u32> LATIN_EXTENDED_B{0x0180, 0x024F};
inline constexpr Tuple<u32, u32> COMBINING_DIACRITICAL_MARKS{0x0300, 0x036F};
inline constexpr Tuple<u32, u32> ARABIC{0x0600, 0x06FF};
inline constexpr Tuple<u32, u32> GENERAL_PUNCTUATION{0x2000, 0x206F};
inline constexpr Tuple<u32, u32> SUPERSCRIPTS_AND_SUBSCRIPTS{0x2070, 0x209F};
inline constexpr Tuple<u32, u32> CURRENCY_SYMBOLS{0x20A0, 0x20CF};
inline constexpr Tuple<u32, u32> NUMBER_FORMS{0x2150, 0x218F};
inline constexpr Tuple<u32, u32> ARROWS{0x2190, 0x21FF};
inline constexpr Tuple<u32, u32> MATHEMATICAL_OPERATORS{0x2200, 0x22FF};
inline constexpr Tuple<u32, u32> HIRAGANA{0x3040, 0x309F};
inline constexpr Tuple<u32, u32> KATAKANA{0x30A0, 0x30FF};
}        // namespace utf

}        // namespace ash
