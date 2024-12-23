/// SPDX-License-Identifier: MIT
#include "ashura/engine/views.h"
#include "fast_float/fast_float.h"

namespace ash
{

void ScalarDragBox::scalar_parse(Span<c32 const> text, ScalarSpec const & spec,
                                 Scalar & scalar)
{
  if (text.is_empty())
  {
    return;
  }

  spec.match(
    [&](F32InputSpec const & spec) {
      f32 value = 0;
      auto [ptr, ec] =
        fast_float::from_chars(text.pbegin(), text.pend(), value);
      if (ec != std::errc{} || value < spec.min || value > spec.max)
      {
        return;
      }
      scalar = value;
    },
    [&](I32InputSpec const & spec) {
      i32 value = 0;
      auto [ptr, ec] =
        fast_float::from_chars(text.pbegin(), text.pend(), value);
      if (ec != std::errc{} || value < spec.min || value > spec.max)
      {
        return;
      }
      scalar = value;
    });
}

}    // namespace ash
