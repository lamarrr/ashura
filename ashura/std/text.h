#pragma once
#include "ashura/std/types.h"

namespace ash
{

constexpr bool is_valid_utf8(Span<u8 const> text);

constexpr usize count_utf8_codepoints(Span<u8 const>);

/// @brief decode.size must be at least count_utf8_codepoints(encoded).
/// estimate: encoded.size() * 4
constexpr usize utf8_decode(Span<u8 const> encoded, Span<u32> decoded)
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
constexpr usize utf8_encode(Span<u32 const> decoded, Span<u8> encoded);

}        // namespace ash
