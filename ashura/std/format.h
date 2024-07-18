/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/types.h"
#include <string>
#include <string_view>

namespace ash
{
namespace fmt
{

enum class Style : u8
{
  Decimal    = 0,
  General    = 0,
  Octal      = 1,
  Hex        = 2,
  Bin        = 3,
  Scientific = 4
};

struct Spec
{
  Style style     = Style::Decimal;
  i32   precision = 0;
};

/// @sratch_buffer: minimum recommended size of 256 bytes
struct Context
{
  Fn<bool(Span<char const>)> push = fn([](Span<char const>) { return true; });
  Span<char>                 scratch_buffer = {};
};

template <typename T>
bool push(Context &ctx, Spec const &, T const &)
{
  return ctx.push("{Unformatted Object}"_span);
}

bool push(Context &ctx, Spec const &, bool value);
bool push(Context &ctx, Spec const &spec, u8 value);
bool push(Context &ctx, Spec const &spec, u16 value);
bool push(Context &ctx, Spec const &spec, u32 value);
bool push(Context &ctx, Spec const &spec, u64 value);
bool push(Context &ctx, Spec const &spec, i8 value);
bool push(Context &ctx, Spec const &spec, i16 value);
bool push(Context &ctx, Spec const &spec, i32 value);
bool push(Context &ctx, Spec const &spec, i64 value);
bool push(Context &ctx, Spec const &spec, f32 value);
bool push(Context &ctx, Spec const &spec, f64 value);
bool push(Context &ctx, Spec const &spec, Vec2 const &value);
bool push(Context &ctx, Spec const &spec, Vec3 const &value);
bool push(Context &ctx, Spec const &spec, Vec4 const &value);
bool push(Context &ctx, Spec const &spec, Vec2I const &value);
bool push(Context &ctx, Spec const &spec, Vec3I const &value);
bool push(Context &ctx, Spec const &spec, Vec4I const &value);
bool push(Context &ctx, Spec const &spec, Vec2U const &value);
bool push(Context &ctx, Spec const &spec, Vec3U const &value);
bool push(Context &ctx, Spec const &spec, Vec4U const &value);
bool push(Context &, Spec &spec, Spec const &value);
bool push(Context &ctx, Spec const &, Span<char const> str);

template <usize N>
bool push(Context &ctx, Spec const &spec, char const (&str)[N])
{
  return push(ctx, spec, Span{str, N});
}

bool push(Context &ctx, Spec const &spec, char const *str);
bool push(Context &ctx, Spec const &spec, void const *ptr);

inline bool push(Context &ctx, Spec const &spec, std::string const &str)
{
  return push(ctx, spec, to_span(str));
}

inline bool push(Context &ctx, Spec const &spec, std::string_view str)
{
  return push(ctx, spec, to_span(str));
}

template <typename... Args>
bool format(Context &ctx, Args const &...args)
{
  Spec spec;
  return true && (push(ctx, spec, args) && ...);
}

}        // namespace fmt
}        // namespace ash
