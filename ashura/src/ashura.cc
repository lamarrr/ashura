#include "SDL3/SDL.h"
#include "ashura/app.h"
#include "ashura/text.h"
#include "ashura/utf8string.h"
#include "ashura/uuid.h"
#include "ashura/widget.h"
#include "ashura/widget_tree.h"
#include "ashura/widgets/box.h"
#include "ashura/widgets/checkbox.h"
#include "ashura/widgets/flex.h"
#include "ashura/widgets/grid.h"
#include "ashura/widgets/image.h"
#include "ashura/widgets/padding.h"
#include "ashura/widgets/progress_bar.h"
#include "ashura/widgets/radio.h"
#include "ashura/widgets/scroll_box.h"
#include "ashura/widgets/slider.h"
#include "ashura/widgets/stack.h"
#include "ashura/widgets/stats.h"
#include "ashura/widgets/switch.h"
#include "ashura/widgets/text.h"
#include "stx/try_some.h"

using namespace ash;

int main(int argc, char **argv)
{
  ASH_CHECK(SDL_Init(SDL_INIT_EVERYTHING) == 0);

  FontSpec fonts[] = {{.name = "Roboto", .path = R"(C:\Users\Basit\Documents\workspace\oss\ashura\ashura\assets\fonts\Roboto\Roboto-Regular.ttf)"},
                      {.name = "RobotoMono", .path = R"(C:\Users\Basit\Desktop\JetBrainsMono-2.304\fonts\ttf\JetBrainsMono-Regular.ttf)"},
                      {.name = "MaterialIcons", .path = R"(C:\Users\Basit\Documents\workspace\oss\ashura\ashura\assets\fonts\MaterialIcons\MaterialIcons-Regular.ttf)"},
                      {.name = "NotoSans", .path = R"(C:\Users\Basit\Desktop\Noto_Sans_Arabic\static\NotoSansArabic-Regular.ttf)"}};

  CanvasPipelineSpec pipelines[] = {
      {.name = DEFAULT_SHAPE_PIPELINE, .vertex_shader = gfx::vertex_shader_code, .fragment_shader = gfx::fragment_shader_code},
      {.name = DEFAULT_GLYPH_PIPELINE, .vertex_shader = gfx::glyph_vertex_shader_code, .fragment_shader = gfx::glyph_fragment_shader_code}};

  AppConfig cfg{.enable_validation_layers = false, .fonts = fonts, .pipelines = pipelines};

  char const text[] = R"(
1. بِسْمِ اللَّهِ الرَّحْمَٰنِ الرَّحِيمِ
2. الْحَمْدُ لِلَّهِ رَبِّ الْعَالَمِينَ
3. الرَّحْمَٰنِ الرَّحِيمِ
4. مَالِكِ يَوْمِ الدِّينِ
5. إِيَّاكَ نَعْبُدُ وَإِيَّاكَ نَسْتَعِينُ
6. اهْدِنَا الصِّرَاطَ الْمُسْتَقِيمَ
7. صِرَاطَ الَّذِينَ أَنْعَمْتَ عَلَيْهِمْ غَيْرِ الْمَغْضُوبِ عَلَيْهِمْ وَلَا الضَّالِّينَ)";

  char const greeting[] = R"( ٱلسَّلَامُ عَلَيْكُمْ )";

  stx::Vec<GridItem> items;
  GridItem           items_tmp[] = {
      {.column = 0, .column_span = 2, .row = 0, .row_span = 2},
      {.column = 2, .column_span = 1, .row = 0, .row_span = 1},
      {.column = 2, .column_span = 1, .row = 1, .row_span = 1},
  };

  items.extend(items_tmp).unwrap();

  RadioState state{8};
  App        app{std::move(cfg),
          Flex{
              FlexProps{},
              Image{
                  ImageProps{.source         = FileImageSource{.path = R"(C:\Users\Basit\Desktop\pimping.png)"},
                                    .aspect_ratio   = stx::Some(1.0f),
                                    .resize_on_load = true}},
              Text{"cruelty_free", TextProps{.style = TextStyle{.font             = "MaterialIcons",
                                                                       .font_height      = 25,
                                                                       .foreground_color = material::BLACK,
                                                                       .background_color = colors::WHITE,
                                                                       .line_height      = 1.0f}}},
              Text{std::string_view{(char *) greeting, sizeof(greeting)}, TextProps{.style = TextStyle{.font             = "NotoSans",
                                                                                                              .font_height      = 20,
                                                                                                              .foreground_color = material::BLACK,
                                                                                                              .background_color = colors::WHITE}}},
              CheckBox{},
              Slider{stx::fn::rc::make_unique_static([](Slider &slider, Context &ctx, f32 value) {
                ctx.text_scale_factor = value * 5;
              })},
              Switch{},
              StatsWidget{},
              ProgressBar{},
              Grid{GridProps{.columns = 3, .rows = 2, .column_gap = 10, .row_gap = 10, .alignment = ALIGN_CENTER, .items = std::move(items), .frame = SizeConstraint::absolute(600, 400)},
                   Image{
                       ImageProps{.source         = FileImageSource{.path = R"(C:\Users\Basit\Desktop\26050398.jpg)"},
                                         .aspect_ratio   = stx::Some(1.0f),
                                         .resize_on_load = true}},
                   Image{
                       ImageProps{.source         = FileImageSource{.path = R"(C:\Users\Basit\Desktop\26050398.jpg)"},
                                         .aspect_ratio   = stx::Some(1.0f),
                                         .resize_on_load = true}},
                   Image{
                       ImageProps{.source         = FileImageSource{.path = R"(C:\Users\Basit\Desktop\26050398.jpg)"},
                                         .aspect_ratio   = stx::Some(1.0f),
                                         .resize_on_load = true}}},
              Stack{
                  StackProps{.alignment = ALIGN_BOTTOM_CENTER},
                  Box{
                      BoxProps{.padding          = EdgeInsets::all(2.5f),
                                      .border_thickness = 2.5f,
                                      .border_color     = material::CYAN_500,
                                      .border_radius    = BorderRadius::relative(1)},
                      Image{
                          ImageProps{.source         = FileImageSource{.path = R"(C:\Users\Basit\Desktop\profile.png)"},
                                            .border_radius  = BorderRadius::relative(1, 1, 1, 1),
                                            .aspect_ratio   = stx::Some(1.0f),
                                            .resize_on_load = true}}},
                  Box{
                      BoxProps{.background_color = material::RED_500,
                                      .padding          = EdgeInsets::horizontal(5),
                                      .border_thickness = 5,
                                      .border_color     = colors::BLACK,
                                      .border_radius    = BorderRadius::absolute(7.5f)},
                      Text{"LIVE", TextProps{.style = TextStyle{.font_height      = 15,
                                                                       .foreground_color = colors::WHITE}}}}},
              Stack{
                  StackProps{.alignment = ALIGN_CENTER},
                  Box{
                      BoxProps{
                                 .background_gradient = LinearColorGradient{.begin = material::GREEN_500, .end = material::GREEN_500.with_alpha(10), .angle = 0},
                                 .padding             = EdgeInsets::all(50),
                                 .border_radius       = BorderRadius::absolute(7.5f)},
                      Text{"FE!N FE!N FE!N FE!N FE!N", TextProps{.style = TextStyle{.foreground_color = colors::WHITE},
                                                                        .frame = SizeConstraint::relative(1, 1)}}},
                  Padding{
                      EdgeInsets::all(20),
                      Box{BoxProps{.background_color = material::RED_500.with_alpha(0xCC),
                                          .padding          = EdgeInsets::all(5),
                                          .border_thickness = 5,
                                          .border_color     = colors::BLACK,
                                          .border_radius    = BorderRadius::absolute(7.5f),
                                          .corner_shape     = BoxCornerShape::Bevel},
                          Text{"For You", TextProps{.style = TextStyle{.foreground_color = colors::WHITE}}}}}},
              Box{BoxProps{.background_color = color::from_rgb(0x33, 0x33, 0x33),
                                  .padding          = EdgeInsets::all(5),
                                  .border_thickness = 1,
                                  .border_color     = color::from_rgb(0xFF, 0xFF, 0xFF),
                                  .border_radius    = BorderRadius::absolute(7.5f)},
                  Text{"For You", TextProps{.style = TextStyle{.foreground_color = colors::WHITE}}}},
              Radio(5, state),
              Radio(6, state),
              Radio(8, state),
              Text{"verified", TextProps{.style = TextStyle{.font = "MaterialIcons", .foreground_color = colors::YELLOW}}},
              Text{R"(I didn't wanna say anything, but this game seems lame)", TextProps{.style = TextStyle{.font             = "Roboto",
                                                                                                                   .font_height      = 30,
                                                                                                                   .foreground_color = material::WHITE,
                                                                                                                   .shadow_color     = colors::BLACK,
                                                                                                                   .shadow_scale     = 1,
                                                                                                                   .shadow_offset    = 2,
                                                                                                                   .background_color = material::GRAY_100}}},
              Text{R"([2023-07-31 13:26:08.632] [Init] [info] WINDOW RESIZED
[2023-07-31 13:26:08.632] [ImageLoader] [info] Loading image from path: C:\Users\Basit\Desktop\pimping.png
[2023-07-31 13:26:08.632] [ImageLoader] [info] Loading image from path: C:\Users\Basit\Desktop\profile.png
[2023-07-31 13:26:08.633] [ImageLoader] [info] Loading image from path: C:\Users\Basit\Desktop\wallpaperflare.com_wallpaper.jpg
[2023-07-31 13:26:08.637] [Vulkan_RenderResourceManager] [info] Uploaded pending images
[2023-07-31 13:26:08.668] [ImageLoader] [info] Loaded and decoded 70x70 image at path: C:\Users\Basit\Desktop\profile.png with size=19600 bytes
[2023-07-31 13:26:08.674] [Vulkan_RenderResourceManager] [info] Copied Image #9 to Host Visible Staging Buffer in 0.0108 ms
[2023-07-31 13:26:08.674] [Vulkan_RenderResourceManager] [info] Created non-real-time 70x70 Image #9 with format=VK_FORMAT_R8G8B8A8_UNORM and size=40960 bytes
[2023-07-31 13:26:08.675] [ImageLoader] [info] Loaded and decoded 563x570 image at path: C:\Users\Basit\Desktop\pimping.png with size=1283640 bytes
[2023-07-31 13:26:08.675] [Vulkan_RenderResourceManager] [info] Uploaded pending images
[2023-07-31 13:26:08.681] [Vulkan_RenderResourceManager] [info] Copied Image #10 to Host Visible Staging Buffer in 0.6121 ms
[2023-07-31 13:26:08.681] [Vulkan_RenderResourceManager] [info] Created non-real-time 563x570 Image #10 with format=VK_FORMAT_R8G8B8A8_UNORM and size=1474560 bytes
[2023-07-31 13:26:08.682] [Vulkan_RenderResourceManager] [info] Uploaded pending images
[2023-07-31 13:26:08.683] [ImageLoader] [info] Loaded and decoded 1920x1080 image at path: C:\Users\Basit\Desktop\wallpaperflare.com_wallpaper.jpg with size=6220800 bytes
[2023-07-31 13:26:08.691] [Vulkan_RenderResourceManager] [info] Copied Image #11 to Host Visible Staging Buffer in 2.7849 ms
[2023-07-31 13:26:08.691] [Vulkan_RenderResourceManager] [info] Created non-real-time 1920x1080 Image #11 with format=VK_FORMAT_R8G8B8A8_UNORM and size=8847360 bytes
[2023-07-31 13:26:08.695] [Vulkan_RenderResourceManager] [info] Uploaded pending image
[2023-07-31 13:26:08.632] [Init] [info] WINDOW RESIZED
[2023-07-31 13:26:08.632] [ImageLoader] [info] Loading image from path: C:\Users\Basit\Desktop\pimping.png
[2023-07-31 13:26:08.632] [ImageLoader] [info] Loading image from path: C:\Users\Basit\Desktop\profile.png
[2023-07-31 13:26:08.633] [ImageLoader] [info] Loading image from path: C:\Users\Basit\Desktop\wallpaperflare.com_wallpaper.jpg
[2023-07-31 13:26:08.637] [Vulkan_RenderResourceManager] [info] Uploaded pending images
[2023-07-31 13:26:08.668] [ImageLoader] [info] Loaded and decoded 70x70 image at path: C:\Users\Basit\Desktop\profile.png with size=19600 bytes
[2023-07-31 13:26:08.674] [Vulkan_RenderResourceManager] [info] Copied Image #9 to Host Visible Staging Buffer in 0.0108 ms
[2023-07-31 13:26:08.674] [Vulkan_RenderResourceManager] [info] Created non-real-time 70x70 Image #9 with format=VK_FORMAT_R8G8B8A8_UNORM and size=40960 bytes
[2023-07-31 13:26:08.675] [ImageLoader] [info] Loaded and decoded 563x570 image at path: C:\Users\Basit\Desktop\pimping.png with size=1283640 bytes
[2023-07-31 13:26:08.675] [Vulkan_RenderResourceManager] [info] Uploaded pending images
[2023-07-31 13:26:08.681] [Vulkan_RenderResourceManager] [info] Copied Image #10 to Host Visible Staging Buffer in 0.6121 ms
[2023-07-31 13:26:08.681] [Vulkan_RenderResourceManager] [info] Created non-real-time 563x570 Image #10 with format=VK_FORMAT_R8G8B8A8_UNORM and size=1474560 bytes
[2023-07-31 13:26:08.682] [Vulkan_RenderResourceManager] [info] Uploaded pending images
[2023-07-31 13:26:08.683] [ImageLoader] [info] Loaded and decoded 1920x1080 image at path: C:\Users\Basit\Desktop\wallpaperflare.com_wallpaper.jpg with size=6220800 bytes
[2023-07-31 13:26:08.691] [Vulkan_RenderResourceManager] [info] Copied Image #11 to Host Visible Staging Buffer in 2.7849 ms
[2023-07-31 13:26:08.691] [Vulkan_RenderResourceManager] [info] Created non-real-time 1920x1080 Image #11 with format=VK_FORMAT_R8G8B8A8_UNORM and size=8847360 bytes
[2023-07-31 13:26:08.695] [Vulkan_RenderResourceManager] [info] Uploaded pending image[2023-07-31 13:26:08.632] [Init] [info] WINDOW RESIZED
[2023-07-31 13:26:08.632] [ImageLoader] [info] Loading image from path: C:\Users\Basit\Desktop\pimping.png
[2023-07-31 13:26:08.632] [ImageLoader] [info] Loading image from path: C:\Users\Basit\Desktop\profile.png
[2023-07-31 13:26:08.633] [ImageLoader] [info] Loading image from path: C:\Users\Basit\Desktop\wallpaperflare.com_wallpaper.jpg
[2023-07-31 13:26:08.637] [Vulkan_RenderResourceManager] [info] Uploaded pending images
[2023-07-31 13:26:08.668] [ImageLoader] [info] Loaded and decoded 70x70 image at path: C:\Users\Basit\Desktop\profile.png with size=19600 bytes
[2023-07-31 13:26:08.674] [Vulkan_RenderResourceManager] [info] Copied Image #9 to Host Visible Staging Buffer in 0.0108 ms
[2023-07-31 13:26:08.674] [Vulkan_RenderResourceManager] [info] Created non-real-time 70x70 Image #9 with format=VK_FORMAT_R8G8B8A8_UNORM and size=40960 bytes
[2023-07-31 13:26:08.675] [ImageLoader] [info] Loaded and decoded 563x570 image at path: C:\Users\Basit\Desktop\pimping.png with size=1283640 bytes
[2023-07-31 13:26:08.675] [Vulkan_RenderResourceManager] [info] Uploaded pending images
[2023-07-31 13:26:08.681] [Vulkan_RenderResourceManager] [info] Copied Image #10 to Host Visible Staging Buffer in 0.6121 ms
[2023-07-31 13:26:08.681] [Vulkan_RenderResourceManager] [info] Created non-real-time 563x570 Image #10 with format=VK_FORMAT_R8G8B8A8_UNORM and size=1474560 bytes
[2023-07-31 13:26:08.682] [Vulkan_RenderResourceManager] [info] Uploaded pending images
[2023-07-31 13:26:08.683] [ImageLoader] [info] Loaded and decoded 1920x1080 image at path: C:\Users\Basit\Desktop\wallpaperflare.com_wallpaper.jpg with size=6220800 bytes
[2023-07-31 13:26:08.691] [Vulkan_RenderResourceManager] [info] Copied Image #11 to Host Visible Staging Buffer in 2.7849 ms
[2023-07-31 13:26:08.691] [Vulkan_RenderResourceManager] [info] Created non-real-time 1920x1080 Image #11 with format=VK_FORMAT_R8G8B8A8_UNORM and size=8847360 bytes
[2023-07-31 13:26:08.695] [Vulkan_RenderResourceManager] [info] Uploaded pending image[2023-07-31 13:26:08.632] [Init] [info] WINDOW RESIZED
[2023-07-31 13:26:08.632] [ImageLoader] [info] Loading image from path: C:\Users\Basit\Desktop\pimping.png
[2023-07-31 13:26:08.632] [ImageLoader] [info] Loading image from path: C:\Users\Basit\Desktop\profile.png
[2023-07-31 13:26:08.633] [ImageLoader] [info] Loading image from path: C:\Users\Basit\Desktop\wallpaperflare.com_wallpaper.jpg
[2023-07-31 13:26:08.637] [Vulkan_RenderResourceManager] [info] Uploaded pending images
[2023-07-31 13:26:08.668] [ImageLoader] [info] Loaded and decoded 70x70 image at path: C:\Users\Basit\Desktop\profile.png with size=19600 bytes
[2023-07-31 13:26:08.674] [Vulkan_RenderResourceManager] [info] Copied Image #9 to Host Visible Staging Buffer in 0.0108 ms
[2023-07-31 13:26:08.674] [Vulkan_RenderResourceManager] [info] Created non-real-time 70x70 Image #9 with format=VK_FORMAT_R8G8B8A8_UNORM and size=40960 bytes
[2023-07-31 13:26:08.675] [ImageLoader] [info] Loaded and decoded 563x570 image at path: C:\Users\Basit\Desktop\pimping.png with size=1283640 bytes
[2023-07-31 13:26:08.675] [Vulkan_RenderResourceManager] [info] Uploaded pending images
[2023-07-31 13:26:08.681] [Vulkan_RenderResourceManager] [info] Copied Image #10 to Host Visible Staging Buffer in 0.6121 ms
[2023-07-31 13:26:08.681] [Vulkan_RenderResourceManager] [info] Created non-real-time 563x570 Image #10 with format=VK_FORMAT_R8G8B8A8_UNORM and size=1474560 bytes
[2023-07-31 13:26:08.682] [Vulkan_RenderResourceManager] [info] Uploaded pending images
[2023-07-31 13:26:08.683] [ImageLoader] [info] Loaded and decoded 1920x1080 image at path: C:\Users\Basit\Desktop\wallpaperflare.com_wallpaper.jpg with size=6220800 bytes
[2023-07-31 13:26:08.691] [Vulkan_RenderResourceManager] [info] Copied Image #11 to Host Visible Staging Buffer in 2.7849 ms
[2023-07-31 13:26:08.691] [Vulkan_RenderResourceManager] [info] Created non-real-time 1920x1080 Image #11 with format=VK_FORMAT_R8G8B8A8_UNORM and size=8847360 bytes
[2023-07-31 13:26:08.695] [Vulkan_RenderResourceManager] [info] Uploaded pending image
)",
                   TextProps{.style = TextStyle{.font             = "Roboto",
                                                       .font_height      = 30,
                                                       .foreground_color = material::BLUE_500,
                                                       .background_color = material::GRAY_100}}},
              Text{"explicit", TextProps{.style = TextStyle{.font             = "MaterialIcons",
                                                                   .foreground_color = colors::GREEN}}},
              ScrollBox{ScrollBoxProps{}, Image{ImageProps{.source         = FileImageSource{.path = R"(C:\Users\Basit\Desktop\wallpaperflare.com_wallpaper.jpg)"},
                                                                  .border_radius  = BorderRadius::relative(.25f, .25f, .25f, .25f),
                                                                  .aspect_ratio   = stx::Some(2.0f),
                                                                  .resize_on_load = true}}}}};

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
