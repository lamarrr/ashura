#include "ashura/engine/engine.h"
#include "ashura/engine/views.h"

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
      R"(C:\Users\rlama\Documents\workspace\oss\ashura\ashura\config.json)"_span,
      R"(C:\Users\rlama\Documents\workspace\oss\ashura\assets)"_span);

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

  // Don't use whole font metrics for line metrics

  btn.text(U"replay RELOAD"_utf)
      .style(TextStyle{.foreground = colors::WHITE, .background = colors::BLUE},
             FontStyle{.font = engine->assets.fonts["RobotoMono"_span].get(),
                       .font_height = 50,
                       .line_height = 1.2F})
      .style(TextStyle{.foreground = colors::WHITE, .background = colors::BLUE},
             FontStyle{.font = engine->assets.fonts["MaterialIcons"_span].get(),
                       .font_height = 40,
                       .line_height = 1},
             0, 6)
      .padding(10, 10)
      .color(colors::CYAN);
  btn_home.text(U"home HOME"_utf)
      .style(TextStyle{.foreground = colors::WHITE, .background = colors::BLUE},
             FontStyle{.font = engine->assets.fonts["RobotoMono"_span].get(),
                       .font_height = 50,
                       .line_height = 1.2F})
      .style(TextStyle{.foreground = colors::WHITE, .background = colors::BLUE},
             FontStyle{.font = engine->assets.fonts["MaterialIcons"_span].get(),
                       .font_height = 40,
                       .line_height = 1},
             0, 4)
      .frame(200, 200)
      .color(colors::CYAN);

  btn2.text(
          U"بِسْمِ ٱللَّهِ ٱلرَّحْمَـٰنِ ٱلرَّحِيمِ ١ ٱلْحَمْدُ لِلَّهِ رَبِّ ٱلْعَـٰلَمِينَ ٢ ٱلرَّحْمَـٰنِ ٱلرَّحِيمِ ٣ مَـٰلِكِ يَوْمِ ٱلدِّينِ ٤ إِيَّاكَ نَعْبُدُ وَإِيَّاكَ نَسْتَعِينُ ٥ ٱهْدِنَا ٱلصِّرَٰطَ ٱلْمُسْتَقِيمَ ٦ صِرَٰطَ ٱلَّذِينَ أَنْعَمْتَ عَلَيْهِمْ غَيْرِ ٱلْمَغْضُوبِ عَلَيْهِمْ وَلَا ٱلضَّآلِّينَ ٧"_utf)
      .style(
          TextStyle{.foreground = colors::WHITE, .background = colors::YELLOW},
          FontStyle{.font        = engine->assets.fonts["Amiri"_span].get(),
                    .font_height = 50,
                    .line_height = 1.2F})
      .frame(200, 200)
      .color(colors::MAGENTA);

  FlexView flex{};
  flex.items({&btn, &btn2, &btn_home, sw + 0, sw + 1, sw + 2, &slider, &scalar,
              &radio, &combo_box})
      .axis(Axis::X)
      .cross_align(0)
      .main_align(MainAlign::SpaceBetween)
      .frame(1920, 1080);

  engine->run(flex);
}