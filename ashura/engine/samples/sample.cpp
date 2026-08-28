// #include "ashura/engine/animation.hpp"
#include "ashura/engine/animation.hpp"
#include "ashura/engine/engine.hpp"
#include "ashura/engine/views.hpp"
#include "ashura/std/log_sinks.hpp"

using namespace ash;

struct Sample final : ash::ui::View
{
    ui::Text txt;

    constexpr Sample(Span<char32_t const> text) :
      txt{default_allocator, text, TextStyle{.foreground = mdc::GRAY_100},
          FontStyle{.height = 20}}
    {
        txt.highlightable(true)
          .alignment(-.5)
          .copyable(true)
          .editable(true)
          .accept_tab_input(false)
          .enter_submits(false)
          .enable_multiline_input(false)
          .enable_undo_redo(true);
    }

    constexpr virtual ui::ViewState tick(ui::Scope const &, ui::Events const &,
                                         Fn<void(View &)> build) override
    {
        build(txt);
        return ui::ViewState{.pointable = true};
    }

    constexpr virtual void size(ui::Scope const &, f32x2 allocated,
                                Span<f32x2> sizes) override
    {
        fill(sizes, allocated);
    }

    constexpr virtual ui::Layout fit(ui::Scope const &, f32x2, Span<f32x2 const> sizes,
                                     Span<f32x2> centers) override
    {
        fill(centers, f32x2{0, 0});
        return ui::Layout{
          .extent          = sizes[0],
          .viewport_extent = sizes[0],
        };
    }

    constexpr virtual void render(ui::Scope const & s, Canvas c,
                                  ui::RenderInfo const & info) override
    {
        // animate the text color using the timestamp
        auto time    = s.timestamp();
        auto t       = fmod(time.time_since_epoch().count() / 1'000'000'000.0F, 1.0F);
        auto b       = ash::easing::cubic_bezier(0.42F, 0, 0.58F, 1.0F);
        auto eased_t = b(t) * 0.25 * static_cast<int>(View::id());

        // c->rrect(Shape{
        //   .world_transform = transform2d_to_3d(info.canvas_transform).to_mat(),
        //   .area = {info.canvas_region.center, info.canvas_region.extent * eased_t},
        //   .bbox_extent = info.canvas_region.extent,
        //   .shade       = ShadeType::Feathered,
        //   .feather     = 50,
        //   .tint        = ColorGradient{colors::CYAN.w(80)}
        // });

        c->rrect(
          Shape{.world_transform = transform2d_to_3d(info.canvas_transform).to_mat(),
                .area            = info.canvas_region,
                .bbox_extent     = info.canvas_region.extent,
                .shade           = ShadeType::Stroked,
                .feather         = 50,
                .tint            = ColorGradient{colors::RED.w(80)}});

        txt.alignment(eased_t - 0.5F);
    }

    constexpr virtual Cursor cursor(ui::Scope const &, f32x2, f32x2) override
    {
        return Cursor::Text;
    }
};

int main()
{
    using namespace ash;

    // TODO: remove
    static constexpr u8 JSON_CONFIG[] = {
#embed "config.json"
    };

    auto cfg = EngineCfg::parse_json(JSON_CONFIG, default_allocator).unwrap();

    auto engine = IEngine::create(default_allocator, cfg, {});

    // TODO: how to run future after engine init and trivially poll
    auto fut1 = sys.image->load_from_path(
      "Test Image"_s, R"***(C:\Users\rlama\Pictures\1298210.jpg)***"_s);
    auto fut2 = sys.image->load_from_path(
      "Test Image"_s, R"***(C:\Users\rlama\Pictures\1359482.png)***"_s);
    auto fut3 = sys.image->load_from_path(
      "Test Image"_s, R"***(C:\Users\rlama\Pictures\1378704.png)***"_s);

    while (!fut1.poll() || !fut2.poll() || !fut3.poll())
    {
        engine->run(10ms);
    }

    ImageId image1_id = fut1.poll().unwrap()->unwrap().id;
    ImageId image2_id = fut2.poll().unwrap()->unwrap().id;
    ImageId image3_id = fut3.poll().unwrap()->unwrap().id;

    Sample sample1{UR"***(auto & focus();
    auto & icon(...);
    auto & image(...);)***"};
    Sample sample2{UR"***(auto & input(...);
    auto & list(...);
    auto & modal(...);
    auto & plot(...);)***"};
    Sample sample3{UR"***(auto & radio(...);
    auto & scalar_box(...);
    auto & scroll_view(...);)***"};
    Sample sample4{UR"***(Space()                          = default;
    Space(Space const &)             = delete;
    Space(Space &&)                  = default;
    Space & operator=(Space const &) = delete;
    Space & operator=(Space &&)      = default;
    virtual ~Space() override        = default;)***"_s};
    Sample sample5{UR"***(auto & button(...);
    auto & check_box(...);
    auto & color_picker();
    auto & combo(...);
    auto & flex(...);)***"_s};

    ui::Space space;
    space.extent(f32x2{100, 100});

    ui::Stack stack{default_allocator};
    stack.items({sample5, sample4});

    ui::Image image1{image1_id};
    image1.radii(ui::CornerRadii::all(60))
      .frame(ui::Frame{}.abs(750, 750))
      .tint(ColorGradient{colors::WHITE, colors::RED, 1.5F * PI, 0});

    ui::Image image2{image2_id};
    image2.radii(ui::CornerRadii::all(60))
      .frame(ui::Frame{}.abs(750, 750))
      .tint(ColorGradient{colors::WHITE, colors::RED, 1.5F * PI, 0});

    ui::Text button_text{default_allocator};
    button_text.str(UR"***(Sample Button)***"_s);

    // TODO: relative corner radii
    ui::Button button;
    button.item(button_text)
      .padding(ui::Padding::all(10))
      .on_click([](u32 count) {
          info("Button clicked {} times"_s, count);
    })
      // .squircle() <- not good
      .rrect(ui::CornerRadii::all(20))
      .shadow(ui::Shadow{
        .offset  = f32x2{5, 5},
        .feather = 10,
        .tint    = mdc::GRAY_900.w(80),
      });

    // TODO: pointer is not being set back to default when leaving the clicked button

    ui::Text box_text1{default_allocator};
    box_text1.str(UR"***(Lorem Ipsum Dolor Sit Amet)***"_s);
    ui::Box text_box;
    text_box.item(box_text1)
      .background_color(mdc::GRAY_800)
      .radii(ui::CornerRadii::all(20))
      .border_thickness(1)
      .border_color(mdc::GRAY_700)
      .padding(ui::Padding::all(20));

    ui::Text box_text2{default_allocator};
    box_text2.str(UR"***(A quick brown fox jumps over the lazy dog)***"_s)
      .style(TextStyle{.foreground = mdc::RED_500}, FontStyle{.height = 20});
    ui::Box text_box2;
    text_box2.item(box_text2)
      .radii(ui::CornerRadii::all(20))
      .border_thickness(1)
      .border_color(mdc::YELLOW_500)
      .padding(ui::Padding::all(20))
      .background_blur(8)
      .frame(ui::Frame{}.abs(250, 300));

    ui::Image image3{image3_id};
    image3.radii(ui::CornerRadii::all(60)).frame(ui::Frame{}.abs(750, 750));

    ui::Stack stack2{default_allocator};
    stack2.items({image3, text_box2});

    ui::Flex flex{default_allocator};
    flex.axis(Axis::X)
      .items({sample1, sample2, sample3, stack, image1, space, image2, button, stack2})
      .main_align(ui::MainAlign::SpaceBetween)
      .cross_align(0);

    ui::Box box;
    box.item(flex)
      .background_color(mdc::GRAY_400)
      .radii(ui::CornerRadii::all(20))
      .border_thickness(1)
      .border_color(mdc::GRAY_700)
      .padding(ui::Padding::all(20));

    ui::Box box2;
    box2
      .frame(ui::Frame{
    }
               .abs(500, 500)
               .constrain(false, false))
      .background_blur(4)
      .background_color(mdc::RED_500.w(80))
      .border_thickness(1)
      .border_color(mdc::GRAY_400)
      .radii(ui::CornerRadii::all(20))
      .shadow(ui::Shadow{
        .offset = f32x2{10, 10}, .feather = 20, .tint = mdc::GRAY_900.w(80)});


        ui::CheckBox check_box;
        
    ui::Stack stack3{default_allocator};
    stack3.items({box, box2});

    engine->run(stack3, nanoseconds::max());
}
