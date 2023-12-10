#pragma once
#include "ashura/span.h"
#include "ashura/types.h"

namespace ash
{

enum class FormatStyle : u8
{
  Decimal        = 0,
  Binary         = 1,
  Octal          = 2,
  Hex            = 4,
  Exponential    = 3,
  ExponentialCap = 4,
  Char
};

//
// print @u - as unicode codepoint, @u8 as utf8 codepoint
//
// specifier for when exceed maximum possible width? not exceedable by core
// types what about for strings?
//
// max width of 256 for integers
// padding to width with align left or right
//
// 0 padding of floats?
//
// {[< | > | ^]width:.precision:+:#[0](x | X | b | d):$(u | u8)}
//
// format string must always be ascii??? NAH!!!
//
struct FormatSpec
{
  u16         width     = 0;
  u8          precision = 0;
  FormatStyle style : 2 = FormatStyle::Decimal;
};

void format_float(FormatSpec const &spec);

void xformat(FormatSpec spec, void const *args);

// must be of core type
enum class FormatArgType
{
  U8     = 0,
  I8     = 1,
  U16    = 2,
  I16    = 3,
  U32    = 4,
  I32    = 5,
  U64    = 6,
  I64    = 7,
  F32    = 8,
  F64    = 9,
  Custom = 10
};

// buffer size must always be known at compile time
template <typename T>
struct Formatter;

template <>
struct Formatter<f32>
{
  template <typename Context>
  constexpr void operator()(f32 const &value, Context &context)
  {
    // branching?
    context.format("vec2{{x: {}, y :{}}}", value, value);
  }
};

struct FormatSizeContext
{
  usize size_estimate;

  void format()
};

// multi-pass requires that the objects are const-safe
//
//
// this works for integral types
// how do we estimate the size requirement for custom types?
// upper bound computation SAFELY
//
// walk through the provided formatter format spec and pretend to format it
//
typedef void (*PFN_format_write_pass)(void const    *object,
                                      FormatContext *context);
typedef void (*PFN_format_size_pass)(void const    *object,
                                     FormatSizeContext *context);

struct FormatArg
{
  FormatArgType type  = FormatArgType::U8;
  void (*formatter)() = nullptr;        // only for non-core types
};

// scratch buffers, allocator
// custom formatter for the custom types
//
// there a {} apples at coordinate {}
//
//
// compute maximum format size
//
template <typename... Args>
  usize format(Span<char> output, char const *format_string,
                       Args const &...args)
{
  FormatArg format_args[sizeof...(args) + 1];
  return 0;
}

template <typename... Args>
  usize format_estimate_size()
{
}

struct ParseSpec
{
};

// macros

/// struct Vec4{   };
///
///
/// STRUCT_FORMAT_SPEC(ash::Vec4) {
///     MEMBER_FORMAT_SPEC(x, Vec4::x);
///     MEMBER_FORMAT_SPEC(y, Vec4::y);
/// }
///

}        // namespace ash