#include "SDL3/SDL.h"
#include "ashura/app.h"
#include "ashura/text.h"
#include "ashura/widget.h"
#include "ashura/widgets/flex.h"
#include "ashura/widgets/image.h"
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
      {.name = "Arabic", .path = R"(C:\Users\Basit\Desktop\IBM_Plex_Sans_Arabic\IBMPlexSansArabic-Regular.ttf)", .stroke_thickness = 2.5},
      {.name = "JP", .path = R"(C:\Users\Basit\Desktop\Noto_Sans_HK\NotoSansHK-Regular.otf)", .stroke_thickness = 2.5, .max_atlas_extent = {8000, 8000}},
      {.name = "MaterialIcons", .path = R"(C:\Users\Basit\Documents\workspace\oss\ashura\assets\fonts\MaterialIcons\MaterialIcons-Regular.ttf)", .stroke_thickness = 0}};

  AppConfig cfg{.enable_validation_layers = false, .fonts = fonts};

  App       app{std::move(cfg),
          new Flex{FlexProps{},
                   Text{ "Hi there!",  TextProps{.font = "Roboto", .font_height = 20, .foreground_color = colors::WHITE}
},
                   Image{
                       ImageProps{
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
