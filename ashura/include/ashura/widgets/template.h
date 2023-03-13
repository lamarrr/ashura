
VkExtent2D extent =
    window.value()->surface.value()->swapchain.value().window_extent;

gfx::Canvas &c     = canvas;
static int   x     = 0;
static auto  start = std::chrono::steady_clock::now();

auto d = std::chrono::duration_cast<std::chrono::seconds>(
             std::chrono::steady_clock::now() - start)
             .count();

c.restart({AS(f32, extent.width), AS(f32, extent.height)});
c.brush.color = colors::WHITE;
c.clear();
c.brush.fill  = true;
c.brush.color = colors::GREEN;
c.draw_rect({{800, 800}, {300, 100}});
c.brush.line_thickness = 2;
c.brush.fill           = false;
c.brush.color          = colors::RED;
c.draw_rect({{90, 400}, {320, 120}});
c.brush.color = colors::WHITE;
auto str      = std::format(
    "Hello World! Examples Ashura Engine Demo.\n Starting in {}", d);
char arstr[]  = {0xd9, 0x84, 0xd8, 0xa7, 0x20, 0xd8, 0xa5, 0xd9, 0x84, 0xd9,
                 0x87, 0x20, 0xd8, 0xa5, 0xd9, 0x84, 0xd8, 0xa7, 0x20, 0xd8,
                 0xa7, 0xd9, 0x84, 0xd9, 0x84, 0xd9, 0x87, 0x20, 0xd9, 0x88,
                 0xd8, 0xa7, 0xd9, 0x84, 0xd9, 0x84, 0xd9, 0x87, 0x20, 0xd8,
                 0xa3, 0xd9, 0x83, 0xd8, 0xa8, 0xd8, 0xb1, 0};
char jpstr[]  = {0xe7, 0xa4, 0xbe, 0xe4, 0xbc, 0x9a, 0xe3, 0x81, 0xae, 0xe5,
                 0x90, 0x84, 0xe5, 0x80, 0x8b, 0xe4, 0xba, 0xba, 0xe5, 0x8f,
                 0x8a, 0xe3, 0x81, 0xb3, 0xe5, 0x90, 0x84, 0xe6, 0xa9, 0x9f,
                 0xe9, 0x96, 0xa2, 0xe3, 0x81, 0x8c, 0x20, 0xe3, 0x81, 0x93,
                 0xe3, 0x81, 0xae, 0xe4, 0xb8, 0x96, 0xe7, 0x95, 0x8c, 0xe4,
                 0xba, 0xba, 0xe6, 0xa8, 0xa9, 0xe5, 0xae, 0xa3, 0xe8, 0xa8,
                 0x80, 0xe3, 0x82, 0x92, 0xe5, 0xb8, 0xb8, 0xe3, 0x81, 0xab,
                 0xe5, 0xbf, 0xb5, 0xe9, 0xa0, 0xad, 0};
char emojis[] = {
    0xf0, 0x9f, 0x98, 0x80, 0x20, 0xf0, 0x9f, 0x98, 0x83, 0x20, 0xf0, 0x9f,
    0x98, 0x84, 0x20, 0xf0, 0x9f, 0x98, 0x81, 0x20, 0xf0, 0x9f, 0x98, 0x86,
    0x20, 0xf0, 0x9f, 0x98, 0x85, 0x20, 0xf0, 0x9f, 0x98, 0x82, 0x20, 0xf0,
    0x9f, 0xa4, 0xa3, 0x20, 0xf0, 0x9f, 0xa5, 0xb2, 0x20, 0xf0, 0x9f, 0xa5,
    0xb9, 0x20, 0xe2, 0x98, 0xba, 0xef, 0xb8, 0x8f, 0x20, 0xf0, 0x9f, 0x98,
    0x8a, 0x20, 0xf0, 0x9f, 0x98, 0x87, 0x20, 0xf0, 0x9f, 0x99, 0x82, 0x20,
    0xf0, 0x9f, 0x99, 0x83, 0x20, 0xf0, 0x9f, 0x98, 0x89, 0x20, 0xf0, 0x9f,
    0x98, 0x8c, 0x20, 0xf0, 0x9f, 0x98, 0x8d, 0x20, 0};
TextRun runs[] = {
    {.text  = str,
     .font  = 0,
     .style = TextStyle{.font_height      = 30,
                        .letter_spacing   = 1,
                        .word_spacing     = 16,
                        .foreground_color = colors::CYAN,
                        .background_color = ios::DARK_GRAY}},
    {.text = str,
     .font = 0,
     .style =
         TextStyle{.font_height         = 18,
                   .foreground_color    = colors::BLACK,
                   .background_color    = color::from_rgb(0x33, 0x33, 0x33),
                   .underline_color     = colors::GREEN,
                   .underline_thickness = 1}},
    {.text      = arstr,
     .font      = 2,
     .style     = TextStyle{.font_height         = 30,
                            .letter_spacing      = 0,
                            .foreground_color    = colors::BLACK,
                            .background_color    = colors::GREEN,
                            .underline_color     = colors::MAGENTA,
                            .underline_thickness = 1},
     .direction = TextDirection::RightToLeft,
     .script    = Script::Arabic,
     .language  = languages::ARABIC},
    {.text  = emojis,
     .font  = 1,
     .style = TextStyle{.font_height      = 20,
                        .letter_spacing   = 0,
                        .word_spacing     = 15,
                        .foreground_color = colors::WHITE,
                        .background_color = colors::BLACK.with_alpha(0)}},
    {.text  = "Face with "
              "Tears "
              "of "
              "Joy",
     .font  = 1,
     .style = TextStyle{.font_height      = 50,
                        .letter_spacing   = 0,
                        .word_spacing     = 15,
                        .foreground_color = colors::WHITE,
                        .background_color = colors::BLACK}},
    {.text     = jpstr,
     .font     = 4,
     .style    = TextStyle{.font_height      = 50,
                           .letter_spacing   = 0,
                           .word_spacing     = 15,
                           .foreground_color = colors::WHITE,
                           .background_color = ios::DARK_PURPLE},
     .script   = Script::Katakana,
     .language = languages::JAPANESE},
    {.text = "verified",
     .font = 3,
     .style =
         TextStyle{.font_height      = 50,
                   .letter_spacing   = 0,
                   .word_spacing     = 15,
                   .foreground_color = color::from_rgb(29, 155, 240)}}};
Paragraph                   paragraph{.runs = runs, .align = TextAlign::Right};
stx::Vec<gfx::RunSubWord>   subwords{stx::os_allocator};
stx::Vec<gfx::SubwordGlyph> glyphs{stx::os_allocator};
c.draw_text(paragraph, stx::Span{font, 5}, {100, 500}, 300, subwords,
            glyphs);

c.brush.color = colors::WHITE.with_alpha(255);
c.brush.fill  = true;
c.scale(4, 4);
c.brush.texture = img;
c.draw_round_rect({{0, 0}, {100, 100}}, {25, 25, 25, 25}, 360);