// #include "ashura/engine/animation.hpp"
#include "ashura/engine/views/text.hpp"
#include "ashura/engine/engine.hpp"
#include "ashura/std/log_sinks.hpp"

namespace ash
{

void init_sync_runtime();

}

using namespace ash;

struct Sample final : ash::ui::View
{
    constexpr Sample()
    {
    }

    constexpr virtual ui::ViewState tick(ui::Scope const &, ui::Events const &,
                                         Fn<void(View &)>) override
    {
        return ui::ViewState{.pointable = true};
    }

    constexpr virtual void size(ui::Scope const &, f32x2 allocated,
                                Span<f32x2> sizes) override
    {
        fill(sizes, allocated);
    }

    constexpr virtual ui::Layout fit(ui::Scope const &, f32x2, Span<f32x2 const>,
                                     Span<f32x2> centers) override
    {
        fill(centers, f32x2{0, 0});
        return ui::Layout{
          .extent = f32x2{250, 250},
            .viewport_extent = f32x2{250, 250}
        };
    }

    constexpr virtual i32 layer(ui::Scope const &, i32, Span<i32> indices) override
    {
        fill(indices, 0);
        return 0;
    }

    constexpr virtual i32 z_index(ui::Scope const &, i32, Span<i32> indices) override
    {
        fill(indices, 0);
        return 0;
    }

    constexpr virtual void render(ui::Scope const &, Canvas c,
                                  ui::RenderInfo const & info) override
    {
        c->rrect(
          Shape{.world_transform = transform2d_to_3d(info.canvas_transform).to_mat(),
                .area            = info.canvas_region,
                .bbox_extent     = info.canvas_region.extent,
                .tint            = ColorGradient{colors::RED}});
    }

    constexpr virtual Cursor cursor(ui::Scope const &, f32x2, f32x2) override
    {
        return Cursor::Pointer;
    }
};

int main()
{
    using namespace ash;

    init_sync_runtime();
    ILogger logger{span<LogSink>({&stdio_sink})};
    hook_logger(&logger);
    defer logger_{[&] { hook_logger(nullptr); }};

    
    static constexpr u8 JSON_CONFIG[] = {
#embed "config.json"
    };

    auto cfg = EngineCfg::parse_json(JSON_CONFIG, default_allocator).unwrap();

    // ui::Text txt{default_allocator, U"Hello, Ashura!"_s, TextStyle{}, FontStyle{}};
    Sample sample;

    auto loop =
      dyn_lambda<WindowLoop>(
        default_allocator, [&](Engine, ui::Scope const &) -> ui::View & { return sample; })
        .unwrap();

    auto engine = IEngine::create(default_allocator, cfg, {}, std::move(loop));

    engine->run();
    defer engine_{[&] { engine->shutdown(); }};
}
