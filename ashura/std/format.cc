#include "ashura/std/format.h"
#include <charconv>
#include <string.h>

namespace ash
{

template <typename IntT>
bool push_int(fmt::Context &ctx, fmt::Spec const &spec, IntT value)
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
    case fmt::Style::Bin:
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

  std::to_chars_result result =
      std::to_chars(ctx.scratch_buffer,
                    ctx.scratch_buffer + ctx.scratch_buffer_size, value, base);
  if (result.ec == std::errc{})
  {
    return ctx.push(ctx.scratch_buffer, result.ptr - ctx.scratch_buffer);
  }

  return false;
}

template <typename FloatT>
bool push_float(fmt::Context &ctx, fmt::Spec const &spec, FloatT value)
{
  std::chars_format format = std::chars_format::general;
  switch (spec.style)
  {
    case fmt::Style::General:
      format = std::chars_format::general;
      break;
    case fmt::Style::Scientific:
      format = std::chars_format::scientific;
      break;
    default:
      format = std::chars_format::general;
      break;
  }

  std::to_chars_result result{};
  if (spec.precision > 0)
  {
    result = std::to_chars(ctx.scratch_buffer,
                           ctx.scratch_buffer + ctx.scratch_buffer_size, value,
                           format, spec.precision);
  }
  else
  {
    result = std::to_chars(ctx.scratch_buffer,
                           ctx.scratch_buffer + ctx.scratch_buffer_size, value,
                           format);
  }

  if (result.ec == std::errc{})
  {
    return ctx.push(ctx.scratch_buffer, result.ptr - ctx.scratch_buffer);
  }
  return false;
}

inline bool fmt::push(fmt::Context &ctx, fmt::Spec const &, bool value)
{
  if (value)
  {
    return ctx.push("true", 4);
  }
  return ctx.push("false", 5);
}

bool fmt::push(fmt::Context &ctx, fmt::Spec const &spec, u8 value)
{
  return push_int(ctx, spec, value);
}

bool fmt::push(fmt::Context &ctx, fmt::Spec const &spec, u16 value)
{
  return push_int(ctx, spec, value);
}

bool fmt::push(fmt::Context &ctx, fmt::Spec const &spec, u32 value)
{
  return push_int(ctx, spec, value);
}

bool fmt::push(fmt::Context &ctx, fmt::Spec const &spec, u64 value)
{
  return push_int(ctx, spec, value);
}

bool fmt::push(fmt::Context &ctx, fmt::Spec const &spec, i8 value)
{
  return push_int(ctx, spec, value);
}

bool fmt::push(fmt::Context &ctx, fmt::Spec const &spec, i16 value)
{
  return push_int(ctx, spec, value);
}

bool fmt::push(fmt::Context &ctx, fmt::Spec const &spec, i32 value)
{
  return push_int(ctx, spec, value);
}

bool fmt::push(fmt::Context &ctx, fmt::Spec const &spec, i64 value)
{
  return push_int(ctx, spec, value);
}

bool fmt::push(fmt::Context &ctx, fmt::Spec const &spec, f32 value)
{
  return push_float(ctx, spec, value);
}

bool fmt::push(fmt::Context &ctx, fmt::Spec const &spec, f64 value)
{
  return push_float(ctx, spec, value);
}

bool fmt::push(fmt::Context &, fmt::Spec &spec, fmt::Spec const &value)
{
  spec = value;
  return true;
}

bool fmt::push(fmt::Context &ctx, fmt::Spec const &, Span<char const> str)
{
  return ctx.push(str.data(), str.size());
}

bool fmt::push(fmt::Context &ctx, fmt::Spec const &, std::string_view str)
{
  return ctx.push(str.data(), str.size());
}

bool fmt::push(fmt::Context &ctx, fmt::Spec const &, std::string const &str)
{
  return ctx.push(str.data(), str.size());
}

bool fmt::push(fmt::Context &ctx, fmt::Spec const &, char const *str)
{
  return ctx.push(str, strlen(str));
}

bool fmt::push(fmt::Context &ctx, fmt::Spec const &, void const *ptr)
{
  Spec const ptr_spec{.style = Style::Hex};
  return ctx.push("0x", 2) && push_int(ctx, ptr_spec, (uintptr_t) ptr);
}

}        // namespace ash