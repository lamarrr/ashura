#include "ashura/engine/animation.h"
#include "ashura/engine/engine.h"
#include "ashura/engine/views.h"

struct Animated;

int main()
{
  using namespace ash;
  Logger::init();
  CHECK(logger->add_sink(&stdio_sink));
  Scheduler::init(default_allocator, std::this_thread::get_id(),
                  span<nanoseconds>({2ms, 2ms}),
                  span<nanoseconds>({2ms, 2ms, 2ms, 2ms}));

  Engine::init(
      default_allocator, nullptr,
      R"(C:\Users\rlama\Documents\workspace\oss\ashura\ashura\config.json)"_str,
      R"(C:\Users\rlama\Documents\workspace\oss\ashura\assets)"_str);

  // [ ] revamp views
  // [ ] forward pointer and key events to views

  defer engine_{[&] { Engine::uninit(); }};

  ScrollView         view;
  TextButton         btn;
  TextButton         btn_home;
  TextButton         btn2;
  Switch             sw[4];
  Slider             slider;
  ScalarBox          scalar;
  ComboBoxScrollView combo_box;
  RadioBox           radio;

  sw[0].on();

  scalar.frame(250, 100);

  slider.range(0, 100).interp(0).axis(Axis::Y);

  btn.text(U"replay RELOAD"_str)
      .style(TextStyle{.foreground = colors::WHITE, .background = colors::BLUE},
             FontStyle{.font = engine->assets.fonts["RobotoMono"_str].get(),
                       .font_height = 50,
                       .line_height = 1.2F})
      .style(TextStyle{.foreground = colors::WHITE, .background = colors::BLUE},
             FontStyle{.font = engine->assets.fonts["MaterialIcons"_str].get(),
                       .font_height = 40,
                       .line_height = 1},
             0, 6)
      .padding(10, 10);

  btn_home.text(U"home HOME"_str)
      .style(TextStyle{.foreground = colors::WHITE, .background = colors::BLUE},
             FontStyle{.font = engine->assets.fonts["RobotoMono"_str].get(),
                       .font_height = 50,
                       .line_height = 1.2F})
      .style(TextStyle{.foreground = colors::WHITE, .background = colors::BLUE},
             FontStyle{.font = engine->assets.fonts["MaterialIcons"_str].get(),
                       .font_height = 40,
                       .line_height = 1},
             0, 4)
      .frame(200, 200);

  btn2.text(U"بِسْمِ ٱللَّهِ ٱلرَّحْمَـٰنِ ٱلرَّحِيمِ"_str)
      .style(TextStyle{.foreground = colors::WHITE, .background = colors::NONE},
             FontStyle{.font        = engine->assets.fonts["Amiri"_str].get(),
                       .font_height = 25,
                       .line_height = 1.2F})
      .frame(200, 200)
      .padding(10, 10);

  FlexView flex{};
  flex.items({&btn, &btn2, &btn_home, sw + 0, sw + 1, sw + 2, &slider, &scalar,
              &radio, &combo_box})
      .axis(Axis::X)
      .cross_align(0)
      .main_align(MainAlign::SpaceBetween)
      .frame(1'920, 1'080);

  auto animation = StaggeredAnimation<f32>::make(4, 8, RippleStagger{});

  animation.timelines().v0.frame(1'920 * 0.25F, 1'920, 400ms, easing::out());

  // [ ] do we really need a tick function?
  auto loop = [&](time_point, nanoseconds delta) {
    animation.tick(delta);
    flex.frame(animation.animate(0).v0, 1'080);
  };

  engine->run(flex, fn(loop));
}
