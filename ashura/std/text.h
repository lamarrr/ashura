/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/error.h"
#include "ashura/std/result.h"
#include "ashura/std/tuple.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

[[nodiscard]] constexpr bool is_valid_utf8(Str8 text);

/// @brief count number of utf8 codepoints found in the text. does no
/// utf8-validation
[[nodiscard]] constexpr usize count_utf8_codepoints(Str8 text)
{
  c8 const * in    = text.data();
  c8 const * end   = text.pend();
  usize      count = 0;
  while (in != end)
  {
    if ((*in & 0xc0) != 0x80)
    {
      count++;
    }
    in++;
  }
  return count;
}

/// @brief `decoded.size()` must be at least `encoded.size()`
[[nodiscard]] constexpr usize utf8_decode(Str8 text, MutStr32 decoded)
{
  c8 const * in  = text.data();
  c8 const * end = text.pend();
  c32 *      out = decoded.data();

  while (in != end)
  {
    c32 const c0 = in[0];

    if ((c0 & 0xF8) == 0xF0)
    {
      c32 const c1 = in[1];
      c32 const c2 = in[2];
      c32 const c3 = in[3];
      *out =
        ((c0 & 0x07) << 18) | ((c1 & 0x3F) << 12) | ((c2 & 0x3F) << 6) | c3;
      in += 4;
    }
    else if ((c0 & 0xF0) == 0xE0)
    {
      c32 const c1 = in[1];
      c32 const c2 = in[2];
      *out         = ((c0 & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0X3F);
      in += 3;
    }
    else if ((c0 & 0xE0) == 0xC0)
    {
      c32 const c1 = in[1];
      *out         = ((c0 & 0x1F) << 6) | (c1 & 0x3F);
      in += 2;
    }
    else
    {
      *out = c0;
      in += 1;
    }

    out++;
  }

  return out - decoded.pbegin();
}

/// @brief `encoded.size()` must be at least `text.size() * 4`
[[nodiscard]] constexpr usize utf8_encode(Str32 text, MutStr8 encoded)
{
  c8 *        out = encoded.data();
  c32 const * in  = text.data();
  c32 const * end = text.pend();

  while (in != end)
  {
    c32 const c = *in;

    if (c <= 0x7F)
    {
      out[0] = c;
      out += 1;
    }
    else if (c <= 0x7FF)
    {
      out[0] = 0xC0 | (c >> 6);
      out[1] = 0x80 | (c & 0x3F);
      out += 2;
    }
    else if (c <= 0xFFFF)
    {
      out[0] = 0xE0 | (c >> 12);
      out[1] = 0x80 | ((c >> 6) & 0x3F);
      out[2] = 0x80 | (c & 0x3F);
      out += 3;
    }
    else if (c <= 0x10'FFFF)
    {
      out[0] = 0xF0 | (c >> 18);
      out[1] = 0x80 | ((c >> 12) & 0x3F);
      out[2] = 0x80 | ((c >> 6) & 0x3F);
      out[3] = 0x80 | (c & 0x3F);
      out += 4;
    }

    in++;
  }

  return out - encoded.pbegin();
}

/// @brief converts UTF-8 text from @p encoded to UTF-32 and appends into @p
/// `decoded`
inline Result<> utf8_decode(Str8 text, Vec<c32> & decoded)
{
  usize const first     = decoded.size();
  usize const max_count = text.size();
  if (!decoded.extend_uninit(max_count))
  {
    return Err{};
  }
  usize const count = utf8_decode(text, decoded.view().slice(first, max_count));
  decoded.resize_uninit(first + count).unwrap();
  return Ok{};
}

/// @brief converts UTF-32 text from @p decoded to UTF-8 and appends into @p
/// `encoded`
[[nodiscard]] inline Result<> utf8_encode(Str32 text, Vec<c8> & encoded)
{
  usize const first     = encoded.size();
  usize const max_count = text.size() * 4;
  if (!encoded.extend_uninit(max_count))
  {
    return Err{};
  }
  usize const count = utf8_encode(text, encoded.view().slice(first, max_count));
  encoded.resize_uninit(first + count).unwrap();
  return Ok{};
}

constexpr void replace_invalid_codepoints(Str32 input, MutStr32 output,
                                          c32 replacement)
{
  c32 const * in  = input.pbegin();
  c32 const * end = input.pend();
  c32 *       out = output.pbegin();

  while (in != end)
  {
    auto cp = *in;
    if (cp >= UTF32_MIN && cp <= UTF32_MAX) [[likely]]
    {
      *out = cp;
    }
    else
    {
      *out = replacement;
    }
    in++;
    out++;
  }
}

/// Unicode Ranges
namespace utf
{
inline constexpr Tuple<c32, c32> ALL{UTF32_MIN, UTF32_MAX};
inline constexpr Tuple<c32, c32> BASIC_LATIN{0x0020, 0x007F};
inline constexpr Tuple<c32, c32> LATIN1_SUPPLEMENT{0x00A0, 0x00FF};
inline constexpr Tuple<c32, c32> LATIN_EXTENDED_A{0x0100, 0x017F};
inline constexpr Tuple<c32, c32> LATIN_EXTENDED_B{0x0180, 0x024F};
inline constexpr Tuple<c32, c32> COMBINING_DIACRITICAL_MARKS{0x0300, 0x036F};
inline constexpr Tuple<c32, c32> ARABIC{0x0600, 0x06FF};
inline constexpr Tuple<c32, c32> GENERAL_PUNCTUATION{0x2000, 0x206F};
inline constexpr Tuple<c32, c32> SUPERSCRIPTS_AND_SUBSCRIPTS{0x2070, 0x209F};
inline constexpr Tuple<c32, c32> CURRENCY_SYMBOLS{0x20A0, 0x20CF};
inline constexpr Tuple<c32, c32> NUMBER_FORMS{0x2150, 0x218F};
inline constexpr Tuple<c32, c32> ARROWS{0x2190, 0x21FF};
inline constexpr Tuple<c32, c32> MATHEMATICAL_OPERATORS{0x2200, 0x22FF};
inline constexpr Tuple<c32, c32> HIRAGANA{0x3040, 0x309F};
inline constexpr Tuple<c32, c32> KATAKANA{0x30A0, 0x30FF};
}    // namespace utf

}    // namespace ash
