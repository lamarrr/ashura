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

void format_float(fmt::Sink sink, fmt::Spec spec, f64 value)
{
  char  scratch_[512];
  Span  scratch{scratch_};
  usize written = 0;

  if (spec.precision != fmt::NONE_PRECISION)
  {
    // [ ] allow setting format 0-padding in format api
    if (int status =
          snprintf(scratch.data(), scratch.size(),
                   (spec.style == fmt::Style::Default) ? "%0.*f" : "%0.*g",
                   (int) spec.precision, (f32) value);
        status > 0)
    {
      written = (usize) status;
    }
  }
  else
  {
    if (int status = snprintf(
          scratch.data(), scratch.size(),
          (spec.style == fmt::Style::Default) ? "%0.f" : "%0.g", (f32) value);
        status > 0)
    {
      written = (usize) status;
    }
  }

  sink({scratch_, written});
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

void ash::format(fmt::Sink sink, fmt::Spec spec, f32x2 const & value)
{
  sink("f32x2{"_str);
  format(sink, spec, value.x());
  sink(", "_str);
  format(sink, spec, value.y());
  sink("}"_str);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, f32x3 const & value)
{
  sink("f32x3{"_str);
  format(sink, spec, value.x());
  sink(", "_str);
  format(sink, spec, value.y());
  sink(", "_str);
  format(sink, spec, value.z());
  sink("}"_str);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, f32x4 const & value)
{
  sink("f32x4{"_str);
  format(sink, spec, value.x());
  sink(", "_str);
  format(sink, spec, value.y());
  sink(", "_str);
  format(sink, spec, value.z());
  sink(", "_str);
  format(sink, spec, value.w());
  sink("}"_str);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, i32x2 const & value)
{
  sink("i32x2{"_str);
  format(sink, spec, value.x());
  sink(", "_str);
  format(sink, spec, value.y());
  sink("}"_str);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, i32x3 const & value)
{
  sink("i32x3{"_str);
  format(sink, spec, value.x());
  sink(", "_str);
  format(sink, spec, value.y());
  sink(", "_str);
  format(sink, spec, value.z());
  sink("}"_str);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, i32x4 const & value)
{
  sink("i32x4{"_str);
  format(sink, spec, value.x());
  sink(", "_str);
  format(sink, spec, value.y());
  sink(", "_str);
  format(sink, spec, value.z());
  sink(", "_str);
  format(sink, spec, value.w());
  sink("}"_str);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, u32x2 const & value)
{
  sink("u32x2{"_str);
  format(sink, spec, value.x());
  sink(", "_str);
  format(sink, spec, value.y());
  sink("}"_str);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, u32x3 const & value)
{
  sink("u32x3{"_str);
  format(sink, spec, value.x());
  sink(", "_str);
  format(sink, spec, value.y());
  sink(", "_str);
  format(sink, spec, value.z());
  sink("}"_str);
}

void ash::format(fmt::Sink sink, fmt::Spec spec, u32x4 const & value)
{
  sink("u32x4{"_str);
  format(sink, spec, value.x());
  sink(", "_str);
  format(sink, spec, value.y());
  sink(", "_str);
  format(sink, spec, value.z());
  sink(", "_str);
  format(sink, spec, value.w());
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
