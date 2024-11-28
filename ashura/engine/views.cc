/// SPDX-License-Identifier: MIT
#include "ashura/engine/views.h"
#include "fast_float/fast_float.h"

namespace ash
{

void ScalarDragBox::scalar_parse(Span<u32 const> text, ScalarState &styling)
{
  if (text.is_empty())
  {
    return;
  }

  Vec<u8> utf8{default_allocator};
  utf8_encode(text, utf8).unwrap();

  char const *const first = (char const *) utf8.begin();
  char const *const last  = (char const *) utf8.end();

  switch (styling.base.type)
  {
    case ScalarInputType::i32:
    {
      i32 value      = 0;
      auto [ptr, ec] = fast_float::from_chars(first, last, value);
      if (ec != std::errc{} || value < styling.min.i32 ||
          value > styling.max.i32)
      {
        return;
      }
      styling.current = ScalarInput{.i32 = value, .type = ScalarInputType::i32};
    }
    break;

    case ScalarInputType::f32:
    {
      f32 value      = 0;
      auto [ptr, ec] = fast_float::from_chars(first, last, value);
      if (ec != std::errc{} || value < styling.min.f32 ||
          value > styling.max.f32)
      {
        return;
      }
      styling.current = ScalarInput{.f32 = value, .type = ScalarInputType::f32};
    }
    break;
  }
}

}        // namespace ash