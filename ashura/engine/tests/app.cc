/// SPDX-License-Identifier: MIT
#include "SDL3/SDL.h"
#include "ashura/engine/color.h"
#include "ashura/engine/render_context.h"
#include "ashura/engine/renderer.h"
#include "ashura/engine/shader.h"
#include "ashura/engine/window.h"
#include "ashura/gpu/vulkan.h"
#include "ashura/std/fs.h"
#include "ashura/std/hash_map.h"
#include "stdlib.h"
#include <thread>

int main(int, char **)
{
  using namespace ash;
  logger->add_sink(&stdio_sink);
  defer log_{[&] { logger->info("Exiting"); }};

  // [ ] env/config to get paths for system: fonts, font cache, images, music,
  // etc.
  Vec<u8> font_data;
  CHECK(
      read_file(R"(../assets/fonts/Amiri/Amiri-Regular.ttf)"_span,
          font_data) == IoError::None);

  defer font_data_{[&] { font_data.uninit(); }};

  Font font = decode_font(span(font_data), 0, default_allocator).unwrap();

  CHECK(rasterize_font(font, 60, default_allocator));

  sdl_window_system->init();
  defer sdl_window_system_{[&] { sdl_window_system->uninit(); }};

  gpu::InstanceImpl instance =
      gpu::create_vulkan_instance(heap_allocator, false).unwrap();

  defer  instance_{[&] { instance->uninit(instance.self); }};
  Window win = sdl_window_system->create_window(instance, "Main"_span).unwrap();
  defer  win_{[&] { sdl_window_system->uninit_window(win); }};

  sdl_window_system->maximize(win);
  sdl_window_system->set_title(win, "Harro"_span);

  bool should_close = false;
  auto close_fn     = [&](WindowEvent const &) { should_close = true; };

  f32  tx     = 0;
  u32  rr     = 1;
  auto key_fn = [&](WindowEvent const &) {
    tx += 10;
    rr += 1;
  };

  sdl_window_system->listen(win, WindowEventTypes::CloseRequested,
                            fn(&close_fn));
  sdl_window_system->listen(win, WindowEventTypes::Key, fn(&key_fn));
  gpu::Surface    surface = sdl_window_system->get_surface(win);
  gpu::DeviceImpl device =
      instance
          ->create_device(instance.self, default_allocator,
                          span({
                              gpu::DeviceType::DiscreteGpu,
                              gpu::DeviceType::VirtualGpu,
                              gpu::DeviceType::IntegratedGpu,
                              gpu::DeviceType::Cpu,
                              gpu::DeviceType::Other,
                          }),
                          span({surface}), 2)
          .unwrap();
  defer device_{[&] { instance->uninit_device(instance.self, device.self); }};

  Vec<Tuple<Span<char const>, Vec<u32>>> spirvs;

  CHECK(
      pack_shaders(
          spirvs,
          span<ShaderUnit>(
              {{.id = "Ngon:FS"_span, .file = "ngon.frag"_span},
               {.id = "Ngon:VS"_span, .file = "ngon.vert"_span},
               {.id       = "Blur_UpSample:FS"_span,
                .file     = "blur.frag"_span,
                .preamble = "#define UPSAMPLE 1"_span},
               {.id       = "Blur_UpSample:VS"_span,
                .file     = "blur.vert"_span,
                .preamble = "#define UPSAMPLE 1"_span},
               {.id       = "Blur_DownSample:FS"_span,
                .file     = "blur.frag"_span,
                .preamble = "#define UPSAMPLE 0"_span},
               {.id       = "Blur_DownSample:VS"_span,
                .file     = "blur.vert"_span,
                .preamble = "#define UPSAMPLE 0"_span},
               {.id = "PBR:FS"_span, .file = "pbr.frag"_span},
               {.id = "PBR:VS"_span, .file = "pbr.vert"_span},
               {.id = "RRect:FS"_span, .file = "rrect.frag"_span},
               {.id = "RRect:VS"_span, .file = "rrect.vert"_span}}),
          R"(../ashura/shaders)"_span) ==
      ShaderCompileError::None);

  StrHashMap<gpu::Shader> shaders;
  defer                   shaders_{[&] { shaders.reset(); }};

  for (auto &[id, spirv] : spirvs)
  {
    bool exists;
    CHECK(shaders.insert(
        exists, nullptr, id,
        device
            ->create_shader(
                device.self,
                gpu::ShaderDesc{.label = id, .spirv_code = span(spirv)})
            .unwrap()));
    CHECK(!exists);
    spirv.reset();
  }

  spirvs.reset();

  logger->info("Finished Shader Compilation");

  gpu::ColorSpace  color_space_spec  = gpu::ColorSpace::DCI_P3_NONLINEAR;
  gpu::PresentMode present_mode_spec = gpu::PresentMode::Immediate;

  gpu::Swapchain swapchain            = nullptr;
  auto           invalidate_swapchain = [&] {
    gpu::SurfaceCapabilities capabilities =
        device->get_surface_capabilities(device.self, surface).unwrap();
    CHECK(has_bits(capabilities.image_usage,
                             gpu::ImageUsage::TransferDst |
                                 gpu::ImageUsage::ColorAttachment));

    Vec<gpu::SurfaceFormat> formats;
    defer                   formats_{[&] { formats.reset(); }};
    u32                     num_formats =
        device->get_surface_formats(device.self, surface, {}).unwrap();
    CHECK(num_formats != 0);
    formats.resize_uninit(num_formats).unwrap();
    CHECK(device->get_surface_formats(device.self, surface, span(formats))
                        .unwrap() == num_formats);

    Vec<gpu::PresentMode> present_modes;
    defer                 present_modes_{[&] { present_modes.reset(); }};
    u32                   num_present_modes =
        device->get_surface_present_modes(device.self, surface, {}).unwrap();
    CHECK(num_present_modes != 0);
    present_modes.resize_uninit(num_present_modes).unwrap();
    CHECK(device
                        ->get_surface_present_modes(device.self, surface,
                                                    span(present_modes))
                        .unwrap() == num_present_modes);

    Vec2U surface_extent = sdl_window_system->get_surface_size(win);
    surface_extent.x     = max(surface_extent.x, 1U);
    surface_extent.y     = max(surface_extent.y, 1U);

    gpu::ColorSpace preferred_color_spaces[] = {
        color_space_spec,
        gpu::ColorSpace::DCI_P3_NONLINEAR,
        gpu::ColorSpace::DISPLAY_P3_NONLINEAR,
        gpu::ColorSpace::DISPLAY_P3_LINEAR,
        gpu::ColorSpace::ADOBERGB_LINEAR,
        gpu::ColorSpace::ADOBERGB_NONLINEAR,
        gpu::ColorSpace::SRGB_NONLINEAR,
        gpu::ColorSpace::EXTENDED_SRGB_LINEAR,
        gpu::ColorSpace::EXTENDED_SRGB_NONLINEAR,
        gpu::ColorSpace::DOLBYVISION,
        gpu::ColorSpace::HDR10_ST2084,
        gpu::ColorSpace::HDR10_HLG,
        gpu::ColorSpace::BT709_LINEAR,
        gpu::ColorSpace::BT709_NONLINEAR,
        gpu::ColorSpace::BT2020_LINEAR,
        gpu::ColorSpace::PASS_THROUGH};

    gpu::PresentMode preferred_present_modes[] = {
        present_mode_spec, gpu::PresentMode::Immediate,
        gpu::PresentMode::Mailbox, gpu::PresentMode::Fifo,
        gpu::PresentMode::FifoRelaxed};

    bool               found_format = false;
    gpu::SurfaceFormat format;

    for (gpu::ColorSpace cp : preferred_color_spaces)
    {
      Span sel = find_if(span(formats), [&](gpu::SurfaceFormat a) {
        return a.color_space == cp;
      });
      if (!sel.is_empty())
      {
        found_format = true;
        format       = sel[0];
        break;
      }
    }

    CHECK(found_format);

    gpu::PresentMode present_mode       = gpu::PresentMode::Immediate;
    bool             found_present_mode = false;

    for (gpu::PresentMode pm : preferred_present_modes)
    {
      if (!find(span(present_modes), pm).is_empty())
      {
        found_present_mode = true;
        present_mode       = pm;
        break;
      }
    }

    CHECK(found_present_mode);

    gpu::CompositeAlpha alpha      = gpu::CompositeAlpha::None;
    gpu::CompositeAlpha alpha_spec = gpu::CompositeAlpha::Opaque;
    gpu::CompositeAlpha preferred_alpha[] = {
        alpha_spec,
        gpu::CompositeAlpha::Opaque,
        gpu::CompositeAlpha::Inherit,
        gpu::CompositeAlpha::Inherit,
        gpu::CompositeAlpha::PreMultiplied,
        gpu::CompositeAlpha::PostMultiplied};
    for (gpu::CompositeAlpha a : preferred_alpha)
    {
      if (has_bits(capabilities.composite_alpha, a))
      {
        alpha = a;
        break;
      }
    }

    gpu::SwapchainDesc desc{.label  = "Window Swapchain"_span,
                                      .format = format,
                                      .usage  = gpu::ImageUsage::TransferDst |
                                     gpu::ImageUsage::ColorAttachment,
                                      .preferred_buffering = 2,
                                      .present_mode        = present_mode,
                                      .preferred_extent    = surface_extent,
                                      .composite_alpha     = alpha};

    if (swapchain == nullptr)
    {
      swapchain = device->create_swapchain(device.self, surface, desc).unwrap();
    }
    else
    {
      device->invalidate_swapchain(device.self, swapchain, desc).unwrap();
    }
  };

  invalidate_swapchain();

  defer swapchain_{[&] { device->uninit_swapchain(device.self, swapchain); }};

  // job submission to render context to prepare resources ahead of frame.
  RenderContext ctx;
  ctx.init(device, true, 2, {1920, 1080}, shaders);
  shaders = {};
  defer ctx_{[&] { ctx.uninit(); }};

  PassContext pctx;
  pctx.init(ctx);
  defer pctx_{[&] { pctx.uninit(ctx); }};

  ctx.begin_frame(swapchain);

  CanvasRenderer renderer;
  renderer.init(ctx);
  defer renderer_{[&] { renderer.uninit(ctx); }};

  Canvas canvas;
  canvas.init();
  defer canvas_{[&] { canvas.uninit(); }};

  defer wait_{[&] { device->wait_idle(device.self).unwrap(); }};

  upload_font_to_device(font, ctx);
  defer font_{[&] { unload_font_from_device(font, ctx); }};

  // [ ] create transfer queue, calculate total required setup size
  u32       runs[]        = {U32_MAX};
  FontStyle font_styles[] = {
      {.font = font, .font_height = 30, .line_height = 1.25}};
  TextLayout text_layout;

  TextBlock text_block{.text      = UR"(
المادة 12.
لا يعرض أحد لتدخل تعسفي في حياته الخاصة أو أسرته أو مسكنه أو مراسلاته أو لحملات على شرفه وسمعته، ولكل شخص الحق في حماية القانون من مثل هذا التدخل أو تلك الحملات.
المادة 13.
( 1 ) لكل فرد حرية التنقل واختيار محل إقامته داخل حدود كل دولة.
( 2 ) يحق لكل فرد أن يغادر أية بلاد بما في ذلك بلده كما يحق له العودة إليه.
)"_utf,
                       .runs      = span(runs),
                       .fonts     = span(font_styles),
                       .direction = TextDirection::RightToLeft,
                       .language  = "en"_span};

  ctx.end_frame(swapchain);
  while (!should_close)
  {
    sdl_window_system->poll_events();
    ctx.begin_frame(swapchain);
    // [ ] maybe check for frame begin before accepting commands
    canvas.begin({1920, 1080});

    canvas.rrect({.center       = Vec2{1920 / 2, 1080 / 2},
                  .extent       = {1920, 1080},
                  .corner_radii = {0, 0, 0, 0},
                  .stroke       = 1,
                  .thickness    = 20,
                  .tint         = ColorGradient::all(colors::WHITE)});
    /*  for (u32 i = 0; i < 2000; i++)
        canvas.rect(
            {.center = Vec2{20, 20},
                      .extent = {160, 160},
                      .tint   = {f(colors::RED) / 255, f(colors::BLUE) / 255,
                                 f(colors::MAGENTA) / 255, f(colors::CYAN) /
       255}});*/
    /*canvas.line(
        {.center    = Vec2{0, 0},
                  .extent    = {800, 800},
                  .thickness = 20,
                  .tint      = {f(colors::RED) / 255, f(colors::BLUE) / 255,
                                f(colors::MAGENTA) / 255, f(colors::CYAN) /
       255}}, span({// Vec2{0, 0},
                 //
                 Vec2{1, 0},
                 //
                 Vec2{1, -1},
                 //
                 Vec2{0, -1},
                 //
                 Vec2{0, -0.5},
                 //
                 Vec2{-1, 1}}));*/
    layout_text(text_block, 1920, text_layout);

    canvas.text(
        {.center    = Vec2{1920 / 2.0f, 1080 / 2.0f},
         .transform = Mat4::identity(),
         .thickness = 20,
         .tint      = {colors::RED.norm(), colors::BLUE.norm(),
                       colors::MAGENTA.norm(), colors::CYAN.norm()}},
        text_block, text_layout,
        TextBlockStyle{
            .runs        = span<TextStyle>({TextStyle{
                       .underline_thickness     = 0,
                       .strikethrough_thickness = 0,
                       .shadow_scale            = 0,
                       .shadow_offset           = Vec2{1, 1},
                       .foreground = ColorGradient::y(colors::MAGENTA, colors::BLUE),
                       .background = ColorGradient::all(Vec4{0, 0, 0, 1}),
                       .underline  = ColorGradient::all(colors::WHITE),
                       .strikethrough = ColorGradient::all(colors::WHITE),
                       .shadow        = ColorGradient::all(colors::WHITE)}}),
            .alignment   = 0,
            .align_width = 1920});

    // [ ] add multi-sampling
    canvas.brect({.center       = {1920 / 2, 1080 / 2},
                  .extent       = {250, 250},
                  .corner_radii = Vec4::splat(0.125f),
                  .stroke       = 1,
                  .thickness    = 8,
                  .tint         = ColorGradient{
                              {colors::RED.norm(), colors::BLUE.norm(),
                               colors::YELLOW.norm(), colors::MAGENTA.norm()}}});
    canvas.squircle(
        {.center    = {1920 / 2 + 100, 1080 / 2 + 100},
         .extent    = {250, 250},
         .stroke    = 1,
         .thickness = 8,
         .tint =
             ColorGradient{{colors::RED.norm(), colors::BLUE.norm(),
                            colors::YELLOW.norm(), colors::MAGENTA.norm()}}},
        0.8, 128);
    canvas.rrect(
        {.center       = {1920 / 2 + 200, 1080 / 2 + 200},
         .extent       = {250, 250},
         .corner_radii = {35, 35, 35, 35},
         .stroke       = 0,
         .tint = ColorGradient{{colors::WHITE.norm(), colors::BLACK.norm(),
                                colors::WHITE.norm(), colors::WHITE.norm()}}});
    // canvas.blur(CRect{{1920 / 2, 1080 / 2}, {1920 / 1.25, 1080 / 1.25}}, 4);

    renderer.begin(
        ctx, pctx, canvas,
        gpu::RenderingInfo{.render_area       = {{0, 0}, {1920, 1080}},
                           .num_layers        = 1,
                           .color_attachments = span({gpu::RenderingAttachment{
                               .view = ctx.screen_fb.color.view}})},
        ctx.screen_fb.color_texture);
    renderer.render(
        ctx, pctx,
        gpu::RenderingInfo{.render_area       = {{0, 0}, {1920, 1080}},
                           .num_layers        = 1,
                           .color_attachments = span({gpu::RenderingAttachment{
                               .view = ctx.screen_fb.color.view}})},
        gpu::Viewport{.offset    = {0, 0},
                      .extent    = {1920, 1080},
                      .min_depth = 0,
                      .max_depth = 1},
        {1920, 1080}, ctx.screen_fb.color_texture, canvas);
    ctx.end_frame(swapchain);
    canvas.clear();
  }
}
