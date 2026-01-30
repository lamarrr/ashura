// #include "ashura/engine/animation.h"
#include "ashura/engine/views/text.h"
#include "ashura/engine/engine.h"
#include "ashura/std/log_sinks.h"

namespace ash
{

void init_sync_runtime();

}

int main()
{
    using namespace ash;

    init_sync_runtime();
    ILogger logger{span<LogSink>({&stdio_sink})};
    hook_logger(&logger);
    defer logger_{[&] { hook_logger(nullptr); }};

    static constexpr u8 JSON_CONFIG[] = {
#embed "ashura/config.json"
    };

    auto cfg =
      EngineCfg::parse_json(JSON_CONFIG, default_allocator, default_allocator).unwrap();

    ui::Text txt{default_allocator, U"Hello, Ashura!"_str, TextStyle{}, FontStyle{}};

    auto loop =
      dyn_lambda<WindowLoop>(
        default_allocator, [&](Engine, ui::Scope const &) -> ui::View & { return txt; })
        .unwrap();

    auto engine = IEngine::create(default_allocator, cfg, {}, std::move(loop));

    engine->run();
    defer engine_{[&] { engine->shutdown(); }};
}
