#pragma once
#include "ashura/std/types.h"

namespace ash
{

constexpr bool is_valid_utf8(Span<u8 const> text);

constexpr usize count_utf8_codepoints(Span<u8 const> text)
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
constexpr usize utf8_encode(Span<u32 const> decoded, Span<u8> encoded)
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

}        // namespace ash
