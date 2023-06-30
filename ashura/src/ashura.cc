#include "SDL3/SDL.h"
#include "ashura/app.h"
#include "ashura/hvk.h"
#include "ashura/text.h"
#include "ashura/uuid.h"
#include "ashura/widget.h"
#include "ashura/widget_tree.h"
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
      {.name = "Roboto", .path = R"(C:\Users\Basit\Documents\workspace\oss\ashura\ashura\assets\fonts\Roboto\Roboto-Regular.ttf)", .stroke_thickness = 2.5},
      {.name = "RobotoMono", .path = R"(C:\Users\Basit\Documents\workspace\oss\ashura\ashura\assets\fonts\RobotoMono\RobotoMono-Regular.ttf)", .stroke_thickness = 2.5},
      {.name = "MaterialIcons", .path = R"(C:\Users\Basit\Documents\workspace\oss\ashura\ashura\assets\fonts\MaterialIcons\MaterialIcons-Regular.ttf)", .stroke_thickness = 0}};

  AppConfig cfg{.enable_validation_layers = true, .fonts = fonts};

  RadioState state{8};
  App        app{std::move(cfg),
          Flex{FlexProps{},
               CheckBox{},
               Slider{},
               Box{BoxProps{.width            = constraint::absolute(200),
                                   .height           = constraint::absolute(200),
                                   .background_color = colors::WHITE,
                                   .border_thickness = 2,
                                   .border_color     = colors::GREEN,
                                   .border_radius    = {5, 5, 5, 5}},
                   Text{"Click Me!", TextProps{.foreground_color = colors::RED}}},
               Radio(5, state),
               Radio(6, state),
               Radio(8, state),
               Text{"verified", TextProps{.font = "MaterialIcons", .foreground_color = colors::YELLOW}},
               Text{R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum)", TextProps{.font = "Roboto", .foreground_color = material::BLUE_500, .background_color = material::GRAY_100}},
               Text{R"(I didn't wanna say anything, but this game seems lame)", TextProps{.font = "Roboto", .font_height = 30, .foreground_color = material::BLUE_500, .background_color = material::GRAY_100}},
               Text{"explicit", TextProps{.font = "MaterialIcons", .foreground_color = colors::GREEN}},
               Image{ImageProps{.source = FileImageSource{.path = R"(C:\Users\Basit\Desktop\wallpaperflare.com_wallpaper.jpg)"}, .border_radius = vec4{20, 20, 20, 20}, .aspect_ratio = stx::Some(2.0f), .resize_on_load = true}}}};
  timepoint  last_tick = Clock::now();
  while (true)
  {
    timepoint present = Clock::now();
    app.tick(present - last_tick);
    last_tick = present;
  }

  SDL_Quit();
  return 0;
}
