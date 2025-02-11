/// SPDX-License-Identifier: MIT
#include "ashura/std/format.h"
#include <charconv>

namespace ash
{

template <typename IntT>
void format_int(fmt::Sink sink, fmt::Spec spec, IntT const & value)
{
  int base = 0;

  switch (spec.style)
  {
    case fmt::Style::Decimal:
    {
      base = 10;
    }
    break;
    case fmt::Style::Octal:
    {
      base = 8;
    }
    break;
    case fmt::Style::Hex:
    {
      base = 16;
    }
    break;
    case fmt::Style::Binary:
    {
      base = 1;
    }
    break;
    default:
    {
      base = 10;
    }
    break;
  }

  char scratch_[512];
  Span scratch{scratch_};

  auto [end, ec] = std::to_chars(scratch.pbegin(), scratch.pend(), value, base);
  if (ec != std::errc{})
  {
    return;
  }

  sink({scratch_, end});
}

template <typename FloatT>
void format_float(fmt::Sink sink, fmt::Spec spec, FloatT const & value)
{
  std::chars_format format = std::chars_format::general;
  switch (spec.style)
  {
    case fmt::Style::Scientific:
      format = std::chars_format::scientific;
      break;
    default:
      format = std::chars_format::general;
      break;
  }

  char scratch_[512];
  Span scratch{scratch_};

  std::to_chars_result result{};
  if (spec.precision != fmt::NONE_PRECISION)
  {
    // [ ] doesn't do precision
    result = std::to_chars(scratch.pbegin(), scratch.pend(), value, format,
                           spec.precision);
  }
  else
  {
    result = std::to_chars(scratch.pbegin(), scratch.pend(), value, format);
  }

  if (result.ec != std::errc{})
  {
    return;
  }

  sink({scratch_, result.ptr});
}

}    // namespace ash

void ash::format(fmt::Sink sink, fmt::Spec, bool const & value)
{
  sink(value ? "true"_str : "false"_str);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, u8 const & value)
{
  format_int(sink, spec, value);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, u16 const & value)
{
  format_int(sink, spec, value);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, u32 const & value)
{
  format_int(sink, spec, value);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, u64 const & value)
{
  format_int(sink, spec, value);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, i8 const & value)
{
  format_int(sink, spec, value);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, i16 const & value)
{
  format_int(sink, spec, value);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, i32 const & value)
{
  format_int(sink, spec, value);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, i64 const & value)
{
  format_int(sink, spec, value);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, f32 const & value)
{
  format_float(sink, spec, value);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, f64 const & value)
{
  format_float(sink, spec, value);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, Vec2 const & value)
{
  sink("Vec2{"_str);
  format(sink, spec, value.x);
  sink(", "_str);
  format(sink, spec, value.y);
  sink("}"_str);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, Vec3 const & value)
{
  sink("Vec3{"_str);
  format(sink, spec, value.x);
  sink(", "_str);
  format(sink, spec, value.y);
  sink(", "_str);
  format(sink, spec, value.z);
  sink("}"_str);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, Vec4 const & value)
{
  sink("Vec4{"_str);
  format(sink, spec, value.x);
  sink(", "_str);
  format(sink, spec, value.y);
  sink(", "_str);
  format(sink, spec, value.z);
  sink(", "_str);
  format(sink, spec, value.w);
  sink("}"_str);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, Vec2I const & value)
{
  sink("Vec2I{"_str);
  format(sink, spec, value.x);
  sink(", "_str);
  format(sink, spec, value.y);
  sink("}"_str);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, Vec3I const & value)
{
  sink("Vec3I{"_str);
  format(sink, spec, value.x);
  sink(", "_str);
  format(sink, spec, value.y);
  sink(", "_str);
  format(sink, spec, value.z);
  sink("}"_str);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, Vec4I const & value)
{
  sink("Vec4I{"_str);
  format(sink, spec, value.x);
  sink(", "_str);
  format(sink, spec, value.y);
  sink(", "_str);
  format(sink, spec, value.z);
  sink(", "_str);
  format(sink, spec, value.w);
  sink("}"_str);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, Vec2U const & value)
{
  sink("Vec2U{"_str);
  format(sink, spec, value.x);
  sink(", "_str);
  format(sink, spec, value.y);
  sink("}"_str);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, Vec3U const & value)
{
  sink("Vec3U{"_str);
  format(sink, spec, value.x);
  sink(", "_str);
  format(sink, spec, value.y);
  sink(", "_str);
  format(sink, spec, value.z);
  sink("}"_str);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, Vec4U const & value)
{
  sink("Vec4U{"_str);
  format(sink, spec, value.x);
  sink(", "_str);
  format(sink, spec, value.y);
  sink(", "_str);
  format(sink, spec, value.z);
  sink(", "_str);
  format(sink, spec, value.w);
  sink("}"_str);
}

void ash::format(fmt::Sink sink, fmt::Spec, Span<char const> const & str)
{
  sink(str);
}

void ash::format(fmt::Sink sink, fmt::Spec, Span<char> const & str)
{
  sink(str);
}

void ash::format(fmt::Sink sink, fmt::Spec, void const * const & ptr)
{
  format_int(sink, fmt::Spec{.style = fmt::Style::Hex}, (uptr) ptr);
}
