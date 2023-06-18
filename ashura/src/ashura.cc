#include "SDL3/SDL.h"
#include "ashura/app.h"
#include "ashura/text.h"
#include "ashura/widget.h"
#include "ashura/widgets/box.h"
#include "ashura/widgets/checkbox.h"
#include "ashura/widgets/flex.h"
#include "ashura/widgets/image.h"
#include "ashura/widgets/radio.h"
#include "ashura/widgets/slider.h"
#include "ashura/widgets/text.h"
#include "stx/try_some.h"

using namespace ash;

int main(int argc, char **argv)
{
  ASH_CHECK(SDL_Init(SDL_INIT_EVERYTHING) == 0);

  FontSpec fonts[] = {
      {.name = "Roboto", .path = R"(C:\Users\Basit\Documents\workspace\oss\ashura\assets\fonts\Roboto\Roboto-Regular.ttf)", .stroke_thickness = 2.5},
      {.name = "RobotoMono", .path = R"(C:\Users\Basit\Documents\workspace\oss\ashura\assets\fonts\RobotoMono\RobotoMono-Regular.ttf)", .stroke_thickness = 2.5},
      {.name = "MaterialIcons", .path = R"(C:\Users\Basit\Documents\workspace\oss\ashura\assets\fonts\MaterialIcons\MaterialIcons-Regular.ttf)", .stroke_thickness = 0}};

  AppConfig cfg{.enable_validation_layers = false, .fonts = fonts};

  RadioContext ctx{8};
  App          app{std::move(cfg),
          FlexBox{FlexBoxProps{},
                  CheckBox{},
                  Slider{},
                  Box{BoxProps{.width            = constraint::absolute(200),
                                        .height           = constraint::absolute(200),
                                        .background_color = colors::WHITE,
                                        .border_thickness = 2,
                                        .border_color     = colors::YELLOW,
                                        .border_radius    = {20, 20, 20, 20}},
                      Text{"Click Me!", TextProps{.foreground_color = colors::MAGENTA}}},
                  Radio(5, ctx),
                  Radio(6, ctx),
                  Radio(8, ctx),
                  Text{"verified", TextProps{.font = "MaterialIcons", .foreground_color = colors::YELLOW}},
                  Text{"Edgy Ashura ", TextProps{.font = "Roboto", .foreground_color = colors::CYAN}},
                  Text{"Engine ", TextProps{.font = "RobotoMono", .foreground_color = colors::GREEN}},
                  Text{"explicit", TextProps{.font = "MaterialIcons", .foreground_color = colors::WHITE}},
                  Image{ImageProps{
                               .source         = FileImageSource{.path = R"(C:\Users\Basit\Desktop\pxfuel.jpg)"},
                               .border_radius  = vec4{20, 20, 20, 20},
                               .aspect_ratio   = stx::Some(2.0f),
                               .resize_on_load = true}}}};
  timepoint    last_tick = Clock::now();
  while (true)
  {
    timepoint present = Clock::now();
    app.tick(present - last_tick);
    last_tick = present;
  }

  SDL_Quit();
  return 0;
}
