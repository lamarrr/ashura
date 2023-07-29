#pragma once

#include "ashura/primitives.h"
#include "ashura/stats.h"
#include "ashura/widget.h"
#include "fmt/format.h"

namespace ash
{

template <typename T>
struct RingBuffer
{
  stx::Vec<T> content;
  usize       next_push = 0;
  usize       size      = 0;

  static RingBuffer make(usize capacity)
  {
    ASH_CHECK(capacity != 0);
    stx::Vec<T> content;
    content.resize(capacity).unwrap();
    return RingBuffer{.content = std::move(content), .next_push = 0, .size = 0};
  }

  void push(T value)
  {
    content[next_push] = value;
    next_push          = (next_push + 1) % content.size();
    size               = (size + 1) % content.size();
  }
};

struct StatsWidget : public Widget
{
  static constexpr vec2  ENTRY_EXTENT{150, 75};
  static constexpr vec2  ENTRY_PADDING{10, 10};
  static constexpr usize NFRAME_SAMPLES  = 64;
  static constexpr usize SAMPLE_INTERVAL = 64;

  static constexpr f32 inverse_lerp(nanoseconds a, nanoseconds b, nanoseconds value)
  {
    return ((f32) ((value - a).count())) / ((f32) ((b - a).count()));
  }

  explicit StatsWidget() :
      frame_stats{RingBuffer<FrameStats>::make(NFRAME_SAMPLES)}
  {
    vertices.resize(NFRAME_SAMPLES).unwrap();
  }

  STX_DISABLE_COPY(StatsWidget)
  STX_DEFAULT_MOVE(StatsWidget)

  virtual vec2 fit(Context &ctx, vec2 allocated_size, stx::Span<vec2 const> children_sizes, stx::Span<vec2> children_positions) override
  {
    return ENTRY_EXTENT;
  }

  virtual void draw(Context &ctx, gfx::Canvas &canvas) override
  {
    canvas.draw_rect_stroke(area, colors::WHITE, 1.25F);

    nanoseconds min_gpu_time = nanoseconds{stx::I64_MAX};
    nanoseconds max_gpu_time = nanoseconds{stx::I64_MIN};

    for (FrameStats const &stats : frame_stats.content)
    {
      min_gpu_time = std::min(min_gpu_time, stats.gpu_time);
      max_gpu_time = std::max(max_gpu_time, stats.gpu_time);
    }

    f32 spacing = ENTRY_EXTENT.x / ((f32) frame_stats.content.size());

    usize ivertex = 0;

    f32 x = 0;

    for (FrameStats const &stats : frame_stats.content.span().slice(frame_stats.next_push))
    {
      f32 y             = inverse_lerp(min_gpu_time, max_gpu_time, stats.gpu_time) * area.extent.y;
      vertices[ivertex] = vertex{.position = {x, y}, .color = colors::WHITE.to_vec()};
      ivertex++;
      x += spacing;
    }

    for (FrameStats const &stats : frame_stats.content.span().slice(0, frame_stats.next_push))
    {
      f32 y             = inverse_lerp(min_gpu_time, max_gpu_time, stats.gpu_time) * area.extent.y;
      vertices[ivertex] = vertex{.position = {x, y}, .color = colors::WHITE.to_vec()};
      ivertex++;
      x += spacing;
    }

    // canvas.draw_path(vertices, area.offset, {0, 0}, 1.25f, false);

    std::string gpu_time_str = fmt::format("{:.2} ms", ctx.frame_stats.gpu_time.count() / 1'000'000.0);

    TextRun runs[] = {
        TextRun{.size = (usize) -1, .style = 0}};

    TextBlock text_block{
        .text          = std::string_view{gpu_time_str.data(), gpu_time_str.size()},
        .runs          = runs,
        .styles        = {},
        .default_style = TextStyle{.font_height = 20, .foreground_color = colors::WHITE},
        .align         = TextAlign::Start,
        .direction     = TextDirection::LeftToRight,
        .language      = {}};

    text_layout.layout(text_block, ctx.text_scale_factor, ctx.font_bundle, 100);

    canvas.draw_text(text_block, text_layout, ctx.font_bundle, area.offset);
  }

  virtual void tick(Context &ctx, std::chrono::nanoseconds interval) override
  {
    frame_stats.push(ctx.frame_stats);
  }

  RingBuffer<FrameStats> frame_stats;
  TextLayout             text_layout;
  stx::Vec<vertex>       vertices;
};

}        // namespace ash
