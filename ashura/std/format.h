/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/buffer.h"
#include "ashura/std/math.h"
#include "ashura/std/range.h"
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

/// @param push function to be called to insert text into the format context.
/// should return true when it is successful.
/// @param sratch scratch buffer. minimum recommended size of 256 bytes
struct Context
{
  Fn<bool(Span<char const>)> push    = [](Span<char const>) { return true; };
  Span<char>                 scratch = {};
};

inline Context buffer(Buffer<char> & b, Span<char> scratch)
{
  auto f = [](Buffer<char> * b, Span<char const> in) { return b->extend(in); };
  return Context{.push = fn(&b, +f), .scratch = scratch};
}

template <typename T>
bool push(Context const & ctx, Spec const &, T const &)
{
  return ctx.push("<unformatted object>"_str);
}

bool push(Context const & ctx, Spec const &, bool value);
bool push(Context const & ctx, Spec const & spec, u8 value);
bool push(Context const & ctx, Spec const & spec, u16 value);
bool push(Context const & ctx, Spec const & spec, u32 value);
bool push(Context const & ctx, Spec const & spec, u64 value);
bool push(Context const & ctx, Spec const & spec, i8 value);
bool push(Context const & ctx, Spec const & spec, i16 value);
bool push(Context const & ctx, Spec const & spec, i32 value);
bool push(Context const & ctx, Spec const & spec, i64 value);
bool push(Context const & ctx, Spec const & spec, f32 value);
bool push(Context const & ctx, Spec const & spec, f64 value);
bool push(Context const & ctx, Spec const & spec, Vec2 const & value);
bool push(Context const & ctx, Spec const & spec, Vec3 const & value);
bool push(Context const & ctx, Spec const & spec, Vec4 const & value);
bool push(Context const & ctx, Spec const & spec, Vec2I const & value);
bool push(Context const & ctx, Spec const & spec, Vec3I const & value);
bool push(Context const & ctx, Spec const & spec, Vec4I const & value);
bool push(Context const & ctx, Spec const & spec, Vec2U const & value);
bool push(Context const & ctx, Spec const & spec, Vec3U const & value);
bool push(Context const & ctx, Spec const & spec, Vec4U const & value);
bool push(Context const &, Spec & spec, Spec const & value);
bool push(Context const & ctx, Spec const &, Span<char const> str);

inline bool push(Context const & ctx, Spec const & spec, Span<char> str)
{
  return push(ctx, spec, str.as_const());
}

template <usize N>
bool push(Context const & ctx, Spec const & spec, char const (&str)[N])
{
  return push(ctx, spec, Span{str, N});
}

bool push(Context const & ctx, Spec const & spec, char const * str);
bool push(Context const & ctx, Spec const & spec, void const * ptr);

template <typename T>
bool push(Context const & ctx, Spec const & spec, T * ptr)
{
  return push(ctx, spec, (void const *) ptr);
}

inline bool push(Context const & ctx, Spec const & spec,
                 std::string const & str)
{
  return push(ctx, spec, span(str));
}

inline bool push(Context const & ctx, Spec const & spec, std::string_view str)
{
  return push(ctx, spec, span(str));
}

template <typename... Args>
bool format(Context const & ctx, Args const &... args)
{
  Spec spec;
  return (true && ... && push(ctx, spec, args));
}

}        // namespace fmt
}        // namespace ash
