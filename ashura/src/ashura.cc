#include "SDL3/SDL.h"
#include "ashura/app.h"
#include "ashura/text.h"
#include "ashura/widget.h"
#include "ashura/widgets/checkbox.h"
#include "ashura/widgets/flex.h"
#include "ashura/widgets/image.h"
#include "ashura/widgets/slider.h"
#include "ashura/widgets/text.h"
#include "stx/try_some.h"

using namespace ash;

using Clock        = std::chrono::steady_clock;        // monotonic system clock
using timepoint    = Clock::time_point;
using nanoseconds  = std::chrono::nanoseconds;
using milliseconds = std::chrono::milliseconds;
using seconds      = std::chrono::seconds;

int main(int argc, char **argv)
{
  ASH_CHECK(SDL_Init(SDL_INIT_EVERYTHING) == 0);

  FontSpec fonts[] = {
      {.name = "Roboto", .path = R"(C:\Users\Basit\Documents\workspace\oss\ashura\assets\fonts\Roboto\Roboto-Regular.ttf)", .stroke_thickness = 2.5},
      {.name = "MaterialIcons", .path = R"(C:\Users\Basit\Documents\workspace\oss\ashura\assets\fonts\MaterialIcons\MaterialIcons-Regular.ttf)", .stroke_thickness = 0}};

  AppConfig cfg{.enable_validation_layers = false, .fonts = fonts};

  App       app{std::move(cfg),
          new Flex{FlexProps{},
                   CheckBox{},
                   Slider{},
                   Text{"verified", TextProps{.font = "MaterialIcons", .foreground_color = colors::YELLOW}},
                   Text{"User5346", TextProps{.font = "Roboto", .foreground_color = colors::WHITE}},
                   Text{"explicit", TextProps{.font = "MaterialIcons", .foreground_color = colors::WHITE}},
                   Image{ImageProps{
                             .source         = FileImageSource{.path = R"(C:\Users\Basit\Pictures\1288647.png)"},
                             .border_radius  = vec4{20, 20, 20, 20},
                             .aspect_ratio   = stx::Some(2.0f),
                             .resize_on_load = true}}}};
  timepoint last_tick = Clock::now();
  while (true)
  {
    timepoint present = Clock::now();
    app.tick(present - last_tick);
    last_tick = present;
  }

  SDL_Quit();
  return 0;
}
