// #include "ashura/engine/animation.h"
#include "ashura/engine/engine.h"
#include "ashura/engine/views.h"
#include "ashura/std/async.h"

struct Animated;

struct App
{
};

int main()
{
  using namespace ash;
  Logger logger_obj;
  hook_logger(&logger_obj);
  defer logger_{[&] { hook_logger(nullptr); }};

  CHECK(logger->add_sink(&stdio_sink));

  Dyn<Engine *> engine = Engine::create(
    default_allocator,
    R"(C:\Users\rlama\Documents\workspace\oss\ashura\ashura\config.json)"_str,
    R"(C:\Users\rlama\Documents\workspace\oss\ashura\)"_str);

  defer engine_{[&] { engine->shutdown(); }};

  FontId const RobotoMono    = sys->font.get("RobotoMono"_str).info().id;
  // FontId const Roboto        = sys->font.get("Roboto"_str).info().id;
  FontId const MaterialIcons = sys->font.get("MaterialIcons"_str).info().id;
  // FontId const Amiri         = sys->font.get("Amiri"_str).info().id;

  // (void) Amiri;

  ui::theme.head_font = RobotoMono;
  ui::theme.body_font = RobotoMono;
  ui::theme.icon_font = MaterialIcons;

  // [ ] forward pointer and key events to views

  ui::Flex flex;

  ui::Stack      stack;
  ui::Text       text;
  ui::Input      input;
  ui::TextButton btn;
  ui::CheckBox   check_box;
  ui::Slider     slider;
  ui::Switch     switch_box;
  ui::Radio      radio;
  ui::ScalarBox  scalar;
  ui::Space      space;
  ui::ScrollView scroll{space};
  ui::Combo      combo;
  ui::Image      img;
  ui::Image      img2;

  btn.text(U"playlist_add ADD Meeeeeee"_str)
    .run({.color = colors::WHITE}, {.font        = RobotoMono,
                                    .height      = ui::theme.body_font_height,
                                    .line_height = 1})
    .run({.color = colors::WHITE},
         {.font        = MaterialIcons,
          .height      = ui::theme.body_font_height,
          .line_height = 1},
         0, 12)
    .padding({5, 5});

  img.source(sys->image.get("birdie"_str).id)
    .frame({250, 250})
    .corner_radii(ui::CornerRadii::all(25));
  img2.source(sys->image.get("mountains"_str).id)
    .frame({800, 500})
    .corner_radii(ui::CornerRadii::all(25));

  slider.range(0, 100).interp(0.25).axis(Axis::X);

  flex
    .items({stack, text, input, btn, check_box, slider, switch_box, radio,
            scalar, space, scroll, combo, img, img2})
    .axis(Axis::X)
    .cross_align(0)
    .main_align(ui::MainAlign::SpaceBetween);

  ui::List        list;
  ui::Table       table;
  ui::ColorPicker picker;
  ui::Plot        plot;
  ui::ProgressBar progress;
  // [ ] store current cursor type in inputbuffer

  // auto animation = StaggeredAnimation<f32>::make(4, 8, RippleStagger{});

  // animation.timelines().v0.frame(1'920 * 0.25F, 1'920, 400ms, easing::out());

  auto loop = [&](ui::ViewContext const & ctx) {
    // animation.tick(ctx.timedelta);
    // flex.frame(animation.animate(0).v0, 1'080);
  };

  engine->run(flex, fn(loop));
}
