/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/passes.h"
#include "ashura/engine/systems.h"
#include "ashura/std/dyn.h"

namespace ash
{

struct Canvas;

struct PassContext
{
  ref<BloomPass>   bloom;
  ref<BlurPass>    blur;
  ref<NgonPass>    ngon;
  ref<PBRPass>     pbr;
  ref<RRectPass>   rrect;
  Vec<Dyn<Pass *>> all;

  static PassContext create(AllocatorRef allocator);

  PassContext(BloomPass & bloom, BlurPass & blur, NgonPass & ngon,
              PBRPass & pbr, RRectPass & rrect, Vec<Dyn<Pass *>> all) :
    bloom{bloom},
    blur{blur},
    ngon{ngon},
    pbr{pbr},
    rrect{rrect},
    all{std::move(all)}
  {
  }

  PassContext(PassContext const &)             = delete;
  PassContext(PassContext &&)                  = default;
  PassContext & operator=(PassContext const &) = delete;
  PassContext & operator=(PassContext &&)      = default;
  ~PassContext()                               = default;

  void acquire();

  void release();

  void add_pass(Dyn<Pass *> pass);
};

// [ ] REMOVE TASK QUEUE
struct FrameGraph
{
  typedef Dyn<Fn<void(FrameGraph & graph, gpu::CommandEncoder & enc,
                      PassContext & passes, Canvas const & canvas)>>
    PassFn;

  struct Pass
  {
    Span<char const> label;
    PassFn           pass;
  };

  struct FrameData
  {
    SSBO ssbo{.label = "Frame Graph SSBO"_str};
  };

  typedef InplaceVec<FrameData, gpu::MAX_FRAME_BUFFERING> BufferedFrameData;

  BufferedFrameData frame_data_;
  u32               frame_index_;
  bool              uploaded_;
  Vec<u8>           ssbo_data_;
  Vec<Slice32>      ssbo_entries_;
  PassContext *     pass_ctx_;
  Vec<Pass>         passes_;
  ArenaPool         arena_;

  FrameGraph(AllocatorRef allocator, PassContext & pass_ctx) :
    frame_data_{},
    frame_index_{0},
    uploaded_{false},
    ssbo_data_{allocator},
    ssbo_entries_{allocator},
    pass_ctx_{&pass_ctx},
    passes_{allocator},
    arena_{allocator}
  {
  }

  FrameGraph(FrameGraph const &)             = delete;
  FrameGraph & operator=(FrameGraph const &) = delete;
  FrameGraph(FrameGraph &&)                  = default;
  FrameGraph & operator=(FrameGraph &&)      = default;
  ~FrameGraph()                              = default;

  u32 push_ssbo(Span<u8 const> data);

  SSBOSpan get_ssbo(u32 id);

  void add_pass(Pass pass);

  template <typename Lambda>
  void add_pass(Span<char const> label, Lambda task)
  {
    // relocate lambda to heap
    Dyn<Lambda *> lambda = dyn(arena_, static_cast<Lambda &&>(task)).unwrap();
    // allocator is noop-ed but destructor still runs when the dynamic object is
    // uninitialized. the memory is freed by at the end of the frame anyway so
    // no need to free it
    lambda.allocator_    = noop_allocator;

    auto f = fn(*lambda);

    return add_pass(
      Pass{.label = label, .pass = transmute(std::move(lambda), f)});
  }

  void execute(Canvas const & canvas);

  void acquire();

  void release();
};

struct BlurRenderParam
{
  RectU         area          = {};
  Vec2U         radius        = {};
  Vec4          corner_radii  = {};
  Mat4          transform     = Mat4::identity();
  f32           aspect_ratio  = 1;
  RectU         scissor       = {};
  gpu::Viewport viewport      = {};
  Mat4          world_to_view = {};
};

struct BlurRenderer
{
  static void render(FrameGraph & graph, Framebuffer const & fb,
                     BlurRenderParam const & param);
};

struct Renderer
{
  Dyn<PassContext *> passes_;
  FrameGraph         graph_;

  static Renderer create(AllocatorRef allocator);

  Renderer(AllocatorRef allocator, Dyn<PassContext *> passes) :
    passes_{std::move(passes)},
    graph_{allocator, *passes_}
  {
  }

  Renderer(Renderer const &)             = delete;
  Renderer(Renderer &&)                  = default;
  Renderer & operator=(Renderer const &) = delete;
  Renderer & operator=(Renderer &&)      = default;
  ~Renderer()                            = default;

  void acquire();

  void release();

  void render_canvas(Framebuffer const & fb, Canvas const & canvas);
};

}    // namespace ash
