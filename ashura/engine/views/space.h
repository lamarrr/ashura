/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

namespace ui
{

struct Space : View
{
  struct Style
  {
    Frame frame{};
  } style_;

  Space()                          = default;
  Space(Space const &)             = delete;
  Space(Space &&)                  = default;
  Space & operator=(Space const &) = delete;
  Space & operator=(Space &&)      = default;
  virtual ~Space() override        = default;

  Space & frame(Frame frame);

  virtual Layout fit(f32x2 allocated, Span<f32x2 const> sizes,
                     Span<f32x2> centers) override;
};

}    // namespace ui

}    // namespace ash
