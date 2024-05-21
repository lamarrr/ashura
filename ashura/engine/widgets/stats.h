#pragma once

#include "ashura/primitives.h"
#include "ashura/stats.h"
#include "ashura/widget.h"
#include "ashura/widgets/flex.h"
#include "ashura/widgets/text.h"
#include "fmt/format.h"

namespace ash
{
namespace gui
{

struct StatsWidget : public Box
{
  StatsWidget() :
      Box{BoxProps{.background_color = colors::MAGENTA,
                   .padding          = EdgeInsets::all(10)},
          Flex{FlexProps{.direction   = Direction::V,
                         .wrap        = Wrap::Wrap,
                         .main_align  = MainAlign::Start,
                         .cross_align = CrossAlign::Start},
               Text{"", {}}, Text{"", {}}, Text{"", {}}, Text{"", {}}}}
  {
  }

  STX_DISABLE_COPY(StatsWidget)
  STX_DEFAULT_MOVE(StatsWidget)

  virtual void tick(Context &ctx, std::chrono::nanoseconds interval) override
  {
    TextProps props{.style = TextStyle{.foreground_color = colors::WHITE}};
    Span cols = this->get_children(ctx)[0]->get_children(ctx);
    dynamic_cast<Text *>(cols[0])->update_text(
        ::fmt::format("GPU time:  {:.2} ms",
                      ctx.frame_stats.gpu_time.count() / 1'000'000.0),
        props);
    dynamic_cast<Text *>(cols[1])->update_text(
        ::fmt::format("CPU time:  {:.2} ms",
                      ctx.frame_stats.cpu_time.count() / 1'000'000.0),
        props);
    dynamic_cast<Text *>(cols[2])->update_text(
        ::fmt::format("CPU-GPU sync time:  {:.2} ms",
                      ctx.frame_stats.gpu_sync_time.count() / 1'000'000.0),
        props);
    dynamic_cast<Text *>(cols[3])->update_text(
        ::fmt::format("{} vertices", ctx.frame_stats.input_assembly_vertices),
        props);
  }
};
}        // namespace gui
}        // namespace ash
