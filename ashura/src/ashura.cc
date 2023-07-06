#include "SDL3/SDL.h"
#include "ashura/app.h"
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
      // {.name = "Roboto", .path = R"(C:\Users\Basit\Documents\workspace\oss\ashura\ashura\assets\fonts\Roboto\Roboto-Regular.ttf)"},
      {.name = "RobotoMono", .path = R"(C:\Users\Basit\Documents\workspace\oss\ashura\ashura\assets\fonts\RobotoMono\RobotoMono-Regular.ttf)"}
      // {.name = "MaterialIcons", .path = R"(C:\Users\Basit\Documents\workspace\oss\ashura\ashura\assets\fonts\MaterialIcons\MaterialIcons-Regular.ttf)"}
      
     };

  CanvasPipelineSpec pipelines[] = {
      {.name = DEFAULT_SHAPE_PIPELINE, .vertex_shader = gfx::vertex_shader_code, .fragment_shader = gfx::fragment_shader_code},
      {.name = DEFAULT_TEXT_PIPELINE, .vertex_shader = gfx::vertex_shader_code, .fragment_shader = gfx::fragment_shader_code}};

  AppConfig cfg{.enable_validation_layers = true, .fonts = fonts, .pipelines = pipelines};

  RadioState state{8};
  App        app{std::move(cfg),
          Flex{
              FlexProps{},
              Image{ImageProps{.source = FileImageSource{.path = R"(C:\Users\Basit\Desktop\pimping.png)"}, .aspect_ratio = stx::Some(1.0f), .resize_on_load = true}}, Text{R"(Tide, they dead, flies everywhere
Y'all know that, it's Lil Kodak, ay,
You play, you lay
Ain't no punk in me
Ain't no punk in me but I be pulling out the strap
Like I'm a dyke, or something

I be pulling out straps on these fuck niggas
I go Young M.A. on these dumb bitches
Like a dyke man, you niggas can't fuck with me
If a nigga says it's up, then it's stuck with me

What's the principle? Pimping ain't easy
I'm invincible, niggas can't beat me
Aye, what's the principle? Pimping ain't easy
I'm invincible, niggas can't beat me

I was in the 8-5, me and Pac hittin' it
I been out 6 months, made 4 million
Slide in the 6-4, windows tinted
Nigga had to get low, them boys start hittin'

Fuck around, hit the lil' hoe with no Jimmy
If a nigga say go, better go kill him
Said that Lil' Zo can cut throat with no feelings
Mama watch her lil' boy turn into a menace

I don't care, I go fed and get a life sentence
I want everybody dead, nigga no limit
I don't shake niggas hands, 'cause I ain't friendly
When I pull up to the crib, have no panties

I been leanin' to the right, like I'm on xannies
I been thuggin' all my life, I ain't romantic
When I whip out the .45, don't panic
When I whip out the .45, don't panic

I be pulling out straps on these fuck niggas
I go Young M.A. on these dumb bitches
Like a dyke man, you niggas can't fuck with me
If a nigga says it's up, then it's stuck with me

What's the principle? Pimping ain't easy
I'm invincible, niggas can't beat me
Aye, what's the principle? Pimping ain't easy
I'm invincible, niggas can't beat me

New AP, flood, water on my butt like a tub
I got my lil' gun in the club, don't worry about me, I'm a thug
You a kill a street nigga, get a dime
If you kill a rap nigga, get a dub

Big chain on my neck, don't budge
Fuckin' DeJ Loaf like a stud
I swapped out the mic for the gun
I swapped out the ice for the mud

I swapped out the spice for the bud
Fuckin' on a dyke, I'm in love
I'm fuckin' with a dyke, she the one
Kodak don't show no remorse

I be automatic tryin' get a nigga touched
Kodak on tour, with his boys
Say they got a whole gun store on the bus
Missy Elliott, come and sex me

Hopping off a jet, to a cheque, to a jet-ski
I beatbox a nigga like the music
I'm thugging in my Reebok, I never need Gucci
I don't even see the confusion

I'm fuckin' Young M.A., long as she got a coochie
Say she got the strap and the toolie
Say she put the crack in her booty

I be pulling out straps on these fuck niggas
I go Young M.A. on these dumb bitches
Like a dyke man, you niggas can't fuck with me
If a nigga says it's up, then it's stuck with me

What's the principle? Pimping ain't easy
I'm invincible, niggas can't beat me
Aye, what's the principle? Pimping ain't easy
I'm invincible, niggas can't beat me
)",
                                                                                                                                                                           TextProps{.font = "RobotoMono", .foreground_color = material::WHITE, .letter_spacing = 0, .word_spacing = 16}},
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
