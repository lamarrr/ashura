// #include "ashura/engine/animation.h"
#include "ashura/engine/animation.h"
#include "ashura/engine/engine.h"
#include "ashura/engine/views.h"

struct Animated;

struct App
{
};

int main()
{
  using namespace ash;

  Dyn<Engine *> engine = Engine::create(
    default_allocator,
    R"(C:\Users\rlama\Documents\workspace\oss\ashura\ashura\config.json)"_str,
    R"(C:\Users\rlama\Documents\workspace\oss\ashura\)"_str);

  defer engine_{[&] { engine->shutdown(); }};

  FontId const RobotoMono     = sys->font.get("RobotoMono"_str).v().id;
  FontId const Roboto         = sys->font.get("Roboto"_str).v().id;
  FontId const MaterialIcons  = sys->font.get("MaterialIcons"_str).v().id;
  FontId const CupertinoIcons = sys->font.get("CupertinoIcons"_str).v().id;
  FontId const Amiri          = sys->font.get("Amiri"_str).v().id;
  FontId const TX_02          = sys->font.get("TX-02"_str).v().id;

  ui::theme.head_font = TX_02;
  ui::theme.body_font = TX_02;
  ui::theme.icon_font = CupertinoIcons;

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
  ui::Combo      combo;
  ui::Image      img;
  ui::Image      img2;
  ui::Image      img3;
  ui::Image      img4;
  ui::FocusView  focus_view;

  input.stub(U"Input Text Here"_str);

  text.text(U"This is a text item"_str)
    .run({.color = colors::WHITE}, {.font        = RobotoMono,
                                    .height      = ui::theme.body_font_height,
                                    .line_height = 1})
    .copyable(true);

  // [ ] drag box cursor

  btn.text(U"doc_text_search camera_viewfinder ADD TO PLAYLIST"_str)
    .run({.color = colors::WHITE}, {.font        = RobotoMono,
                                    .height      = ui::theme.body_font_height,
                                    .line_height = 1})
    .run({.color = colors::WHITE},
         {.font        = CupertinoIcons,
          .height      = ui::theme.body_font_height,
          .line_height = 1},
         0, 15)
    .run({.color = colors::WHITE},
         {.font        = CupertinoIcons,
          .height      = ui::theme.body_font_height,
          .line_height = 1},
         16, 17)
    .padding(ui::Padding::all(5))
    .rrect(ui::CornerRadii::all(15));

  img.source(sys->image.get("goku"_str).v().id)
    .frame(ui::Frame{}.abs(500, 500))
    .corner_radii(ui::CornerRadii::all(25));
  img2.source(sys->image.get("mountains"_str).v().id)
    .frame(ui::Frame{}.abs(400, 400))
    .corner_radii(ui::CornerRadii::all(400));
  img3.source(sys->image.get("bankside"_str).v().id)
    .frame(ui::Frame{}.abs(400, 400))
    .corner_radii(ui::CornerRadii::all(25));
  img4.source(sys->image.get("sunset"_str).v().id)
    .frame(ui::Frame{}.abs(400, 400))
    .corner_radii(ui::CornerRadii::all(25));

  scalar.format("Distance: {.2}m"_str);

  slider.range(0, 100).interp(0.25).axis(Axis::X);

  flex
    .items({stack, text, input, btn, check_box, slider, switch_box, radio,
            scalar, space, combo, img, img2, img3, img4, focus_view})
    .axis(Axis::X)
    .cross_align(0)
    .main_align(ui::MainAlign::SpaceBetween);

  ui::List        list;
  ui::Table       table;
  ui::ColorPicker picker;
  ui::Plot        plot;
  ui::ProgressBar progress;

  list.generator(
    [](Allocator allocator, usize i) -> Option<Dyn<ui::View *>> {
      if (i >= 20)
      {
        return none;
      }

      auto text = dyn<ui::Text>(inplace, allocator, U"Item"_str).unwrap();

      return cast<ui::View *>(std::move(text));
    });

  auto animation = StaggeredAnimation<f32>::make(4, 8, RippleStagger{});

  animation.timelines().v0.frame(500, 1'920, 150ms, easing::out());

  auto loop = [&](ui::Ctx const & ctx) {
    animation.tick(ctx.timedelta);
    flex.frame(ui::Frame{}.abs({animation.animate(0).v0, 500}));
  };

  ui::ScrollView scroll{flex};

  scroll.view_frame(ui::Frame{}.abs(1'920, 1'080));

  engine->run(scroll, &loop);
}
