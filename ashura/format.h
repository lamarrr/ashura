

#include "ashura/types.h"
#include "stx/fn.h"
#include <string>
#include <string_view>

namespace ash
{
namespace fmtx
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
  stx::Fn<bool(char const *buffer, usize size)> push;
  char                                         *scratch_buffer      = nullptr;
  usize                                         scratch_buffer_size = 0;
};

template <typename T>
bool push(Context &ctx, Spec const &, T const &value)
{
  return ctx.push("{?}", 3);
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

}        // namespace fmtx
}        // namespace ash
