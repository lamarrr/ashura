#include "ashura/engine/engine.h"
#include "ashura/engine/views.h"

int main()
{
  using namespace ash;
  Logger::init();
  CHECK(logger->add_sink(&stdio_sink));
  Scheduler::init(default_allocator, std::this_thread::get_id(),
                  span({0ns, 0ns}), span({0ns, 0ns, 0ns, 0ns, 0ns, 0ns}));

  Engine::init(
      default_allocator, nullptr,
      R"(C:\Users\rlama\Documents\workspace\oss\ashura\ashura\config.json)"_span,
      R"(C:\Users\rlama\Documents\workspace\oss\ashura\assets)"_span);

  defer engine_{[&] { Engine::uninit(); }};

  ScrollView view;
  TextButton btn;
  TextButton btn2;

  btn.text(U"Hiya"_utf)
      .style(
          TextStyle{.foreground = colors::BLACK, .background = colors::WHITE},
          FontStyle{.font        = engine->default_font,
                    .font_height = 100,
                    .line_height = 1.2})
      .frame(200, 200);

  btn2.text(U"Gwawa"_utf)
      .style(
          TextStyle{.foreground = colors::BLACK, .background = colors::WHITE},
          FontStyle{.font        = engine->default_font,
                    .font_height = 100,
                    .line_height = 1.2})
      .frame(200, 200);

  FlexView flex{};
  flex.items({&btn, &btn2})
      .axis(Axis::X)
      .cross_align(0)
      .main_align(MainAlign::SpaceEvenly)
      .frame(1920, 1080);

  engine->run(flex);
}