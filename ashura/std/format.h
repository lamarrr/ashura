#pragma once
#include "ashura/std/types.h"
#include <string>
#include <string_view>
#include <typeinfo>

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

/// @sratch_buffer: recommended size of 256 bytes
struct Context
{
  Fn<bool(Span<char const>)> push =
      to_fn([](Span<char const>) { return true; });
  Span<char> scratch_buffer = {};
};

bool push_object_hex(Context &ctx, Spec const &spec, char const *name,
                     Span<u8 const> rep);

template <typename T>
bool push(Context &ctx, Spec const &spec, T const &value)
{
  return push_object_hex(ctx, spec, typeid(value).name(),
                         Span<T const>{&value, 1}.as_u8());
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
bool push(Context &ctx, Spec const &, std::string_view str);
bool push(Context &ctx, Spec const &, char const *str);
bool push(Context &ctx, Spec const &spec, void const *ptr);
bool push(Context &ctx, Spec const &, std::string const &str);

template <typename... Args>
bool format(Context &ctx, Args const &...args)
{
  Spec spec;
  return true && (push(ctx, spec, args) && ...);
}

}        // namespace fmt
}        // namespace ash
