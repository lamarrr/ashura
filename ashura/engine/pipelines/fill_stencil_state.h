/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/pipeline.h"
#include "ashura/engine/shaders.gen.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

constexpr auto fill_stencil_state(FillRule fill_rule, bool invert,
                                  u32 write_mask)
{
  auto even_odd_pass_op = gpu::StencilOp::Invert;
  auto even_odd_fail_op = gpu::StencilOp::Keep;

  auto non_zero_front_pass_op = gpu::StencilOp::IncrementAndWrap;
  auto non_zero_front_fail_op = gpu::StencilOp::Keep;

  auto non_zero_back_pass_op = gpu::StencilOp::DecrementAndWrap;
  auto non_zero_back_fail_op = gpu::StencilOp::Keep;

  auto front_fail_op = (fill_rule == FillRule::EvenOdd) ?
                         even_odd_fail_op :
                         non_zero_front_fail_op;

  auto front_pass_op = (fill_rule == FillRule::EvenOdd) ?
                         even_odd_pass_op :
                         non_zero_front_pass_op;

  auto back_fail_op =
    (fill_rule == FillRule::EvenOdd) ? even_odd_fail_op : non_zero_back_fail_op;

  auto back_pass_op =
    (fill_rule == FillRule::EvenOdd) ? even_odd_pass_op : non_zero_back_pass_op;

  if (invert)
  {
    swap(front_fail_op, front_pass_op);
    swap(back_fail_op, back_pass_op);
  }

  return Tuple{
    gpu::StencilState{.fail_op       = front_fail_op,
                      .pass_op       = front_pass_op,
                      .depth_fail_op = gpu::StencilOp::Keep,
                      .compare_op    = gpu::CompareOp::Never,
                      .compare_mask  = 0,
                      .write_mask    = write_mask,
                      .reference     = 0},

    gpu::StencilState{.fail_op       = back_fail_op,
                      .pass_op       = back_pass_op,
                      .depth_fail_op = gpu::StencilOp::Keep,
                      .compare_op    = gpu::CompareOp::Never,
                      .compare_mask  = 0,
                      .write_mask    = write_mask,
                      .reference     = 0}
  };
}

}    // namespace ash
