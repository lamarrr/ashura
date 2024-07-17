/// SPDX-License-Identifier: MIT
#include "ashura/engine/color.h"
#include "ashura/engine/render_context.h"
#include "ashura/engine/renderer.h"
#include "ashura/engine/shader.h"
#include "ashura/engine/window.h"
#include "ashura/gfx/vulkan.h"
#include "ashura/std/hash_map.h"
#include "ashura/std/io.h"
#include "stdlib.h"
#include <thread>

namespace ash
{
Logger *default_logger;
}

int main(int, char **)
{
  using namespace ash;
  StdioSink sink;
  default_logger = create_logger(to_span<LogSink *>({&sink}), heap_allocator);
  defer default_logger_del{[&] { destroy_logger(default_logger); }};
  defer shutdown{[&] { default_logger->info("Shutting down"); }};

  Vec<u8> font_data;
  CHECK(
      read_file(
          R"(C:\Users\rlama\Documents\workspace\oss\ashura\assets\fonts\Amiri\Amiri-Regular.ttf)"_span,
          font_data) == IoError::None);

  defer font_data_del{[&] { font_data.uninit(); }};

  Font font = load_font(to_span(font_data), 0, default_allocator).unwrap();

  FontAtlas font_atlas;
  CHECK(rasterize_font(font, 60, font_atlas, default_allocator));

  WindowSystem *win_sys = init_sdl_window_system();
  CHECK(win_sys != nullptr);

  gfx::InstanceImpl instance =
      gfx::create_vulkan_instance(heap_allocator, false).unwrap();

  defer  instance_del{[&] { instance->destroy(instance.self); }};
  Window win = win_sys->create_window(instance, "Main"_span).unwrap();
  defer  win_del{[&] { win_sys->destroy_window(win); }};

  win_sys->maximize(win);
  win_sys->set_title(win, "Harro"_span);

  bool should_close = false;
  auto close_fn     = [&](WindowEvent const &) { should_close = true; };

  f32  tx     = 0;
  u32  rr     = 1;
  auto key_fn = [&](WindowEvent const &) {
    tx += 10;
    rr += 1;
  };

  win_sys->listen(win, WindowEventTypes::CloseRequested, fn(&close_fn));
  win_sys->listen(win, WindowEventTypes::Key, fn(&key_fn));
  gfx::Surface    surface = win_sys->get_surface(win);
  gfx::DeviceImpl device =
      instance
          ->create_device(instance.self, default_allocator,
                          to_span({
                              gfx::DeviceType::DiscreteGpu,
                              gfx::DeviceType::VirtualGpu,
                              gfx::DeviceType::IntegratedGpu,
                              gfx::DeviceType::Cpu,
                              gfx::DeviceType::Other,
                          }),
                          to_span({surface}), 2)
          .unwrap();
  defer device_del{
      [&] { instance->destroy_device(instance.self, device.self); }};

  Vec<Tuple<Span<char const>, Vec<u32>>> spirvs;

  CHECK(
      pack_shaders(
          spirvs,
          to_span<ShaderUnit>(
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
          R"(C:\Users\rlama\Documents\workspace\oss\ashura\ashura\shaders)"_span) ==
      ShaderCompileError::None);

  StrHashMap<gfx::Shader> shaders;
  defer                   shaders_del{[&] { shaders.reset(); }};

  for (auto &[id, spirv] : spirvs)
  {
    bool exists;
    CHECK(shaders.insert(
        exists, nullptr, id,
        device
            ->create_shader(
                device.self,
                gfx::ShaderDesc{.label = id, .spirv_code = to_span(spirv)})
            .unwrap()));
    CHECK(!exists);
    spirv.reset();
  }

  spirvs.reset();

  default_logger->info("Finished Shader Compilation");

  gfx::ColorSpace  color_space_spec  = gfx::ColorSpace::DCI_P3_NONLINEAR;
  gfx::PresentMode present_mode_spec = gfx::PresentMode::Immediate;

  gfx::Swapchain swapchain            = nullptr;
  auto           invalidate_swapchain = [&] {
    gfx::SurfaceCapabilities capabilities =
        device->get_surface_capabilities(device.self, surface).unwrap();
    CHECK(has_bits(capabilities.image_usage,
                             gfx::ImageUsage::TransferDst |
                                 gfx::ImageUsage::ColorAttachment));

    Vec<gfx::SurfaceFormat> formats;
    defer                   formats_del{[&] { formats.reset(); }};
    u32                     num_formats =
        device->get_surface_formats(device.self, surface, {}).unwrap();
    CHECK(num_formats != 0);
    CHECK(formats.resize_uninitialized(num_formats));
    CHECK(device->get_surface_formats(device.self, surface, to_span(formats))
                        .unwrap() == num_formats);

    Vec<gfx::PresentMode> present_modes;
    defer                 present_modes_del{[&] { present_modes.reset(); }};
    u32                   num_present_modes =
        device->get_surface_present_modes(device.self, surface, {}).unwrap();
    CHECK(num_present_modes != 0);
    CHECK(present_modes.resize_uninitialized(num_present_modes));
    CHECK(device
                        ->get_surface_present_modes(device.self, surface,
                                                    to_span(present_modes))
                        .unwrap() == num_present_modes);

    Vec2U surface_extent = win_sys->get_surface_size(win);
    surface_extent.x     = max(surface_extent.x, 1U);
    surface_extent.y     = max(surface_extent.y, 1U);

    gfx::ColorSpace preferred_color_spaces[] = {
        color_space_spec,
        gfx::ColorSpace::DCI_P3_NONLINEAR,
        gfx::ColorSpace::DISPLAY_P3_NONLINEAR,
        gfx::ColorSpace::DISPLAY_P3_LINEAR,
        gfx::ColorSpace::ADOBERGB_LINEAR,
        gfx::ColorSpace::ADOBERGB_NONLINEAR,
        gfx::ColorSpace::SRGB_NONLINEAR,
        gfx::ColorSpace::EXTENDED_SRGB_LINEAR,
        gfx::ColorSpace::EXTENDED_SRGB_NONLINEAR,
        gfx::ColorSpace::DOLBYVISION,
        gfx::ColorSpace::HDR10_ST2084,
        gfx::ColorSpace::HDR10_HLG,
        gfx::ColorSpace::BT709_LINEAR,
        gfx::ColorSpace::BT709_NONLINEAR,
        gfx::ColorSpace::BT2020_LINEAR,
        gfx::ColorSpace::PASS_THROUGH};

    gfx::PresentMode preferred_present_modes[] = {
        present_mode_spec, gfx::PresentMode::Immediate,
        gfx::PresentMode::Mailbox, gfx::PresentMode::Fifo,
        gfx::PresentMode::FifoRelaxed};

    bool               found_format = false;
    gfx::SurfaceFormat format;

    for (gfx::ColorSpace cp : preferred_color_spaces)
    {
      Span sel = find_if(to_span(formats), [&](gfx::SurfaceFormat a) {
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

    gfx::PresentMode present_mode       = gfx::PresentMode::Immediate;
    bool             found_present_mode = false;

    for (gfx::PresentMode pm : preferred_present_modes)
    {
      if (!find(to_span(present_modes), pm).is_empty())
      {
        found_present_mode = true;
        present_mode       = pm;
        break;
      }
    }

    CHECK(found_present_mode);

    gfx::CompositeAlpha alpha      = gfx::CompositeAlpha::None;
    gfx::CompositeAlpha alpha_spec = gfx::CompositeAlpha::Opaque;
    gfx::CompositeAlpha preferred_alpha[] = {
        alpha_spec,
        gfx::CompositeAlpha::Opaque,
        gfx::CompositeAlpha::Inherit,
        gfx::CompositeAlpha::Inherit,
        gfx::CompositeAlpha::PreMultiplied,
        gfx::CompositeAlpha::PostMultiplied};
    for (gfx::CompositeAlpha a : preferred_alpha)
    {
      if (has_bits(capabilities.composite_alpha, a))
      {
        alpha = a;
        break;
      }
    }

    gfx::SwapchainDesc desc{.label  = "Window Swapchain"_span,
                                      .format = format,
                                      .usage  = gfx::ImageUsage::TransferDst |
                                     gfx::ImageUsage::ColorAttachment,
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

  defer swapchain_del{
      [&] { device->destroy_swapchain(device.self, swapchain); }};

  // job submission to render context to prepare resources ahead of frame.
  RenderContext ctx;
  ctx.init(device, true, 2, {1920, 1080}, shaders);
  shaders = {};
  defer ctx_del{[&] { ctx.uninit(); }};

  PassContext pctx;
  pctx.init(ctx);
  defer pctx_del{[&] { pctx.uninit(ctx); }};

  ctx.begin_frame(swapchain);

  CanvasRenderer renderer;
  renderer.init(ctx);
  defer renderer_del{[&] { renderer.uninit(ctx); }};

  Canvas canvas;
  canvas.init();
  defer canvas_del{[&] { canvas.uninit(); }};

  defer dev_wait{[&] { device->wait_idle(device.self).unwrap(); }};

  FontAtlasResource font_resource;

  font_resource.init(ctx, font_atlas, default_allocator);
  defer fr_del{[&] { font_resource.release(ctx); }};

  // TODO(lamarrr): create transfer queue, calculate total required setup size
  u32       runs[]        = {U32_MAX};
  FontStyle font_styles[] = {
      {.font = font, .font_height = 30, .line_height = 1.25}};
  TextLayout text_layout;

  TextBlock text_block{.text      = utf(UR"(
المادة 12.
لا يعرض أحد لتدخل تعسفي في حياته الخاصة أو أسرته أو مسكنه أو مراسلاته أو لحملات على شرفه وسمعته، ولكل شخص الحق في حماية القانون من مثل هذا التدخل أو تلك الحملات.
المادة 13.
( 1 ) لكل فرد حرية التنقل واختيار محل إقامته داخل حدود كل دولة.
( 2 ) يحق لكل فرد أن يغادر أية بلاد بما في ذلك بلده كما يحق له العودة إليه.
)"_span),
                       .runs      = to_span(runs),
                       .fonts     = to_span(font_styles),
                       .direction = TextDirection::RightToLeft,
                       .language  = "en"_span};

  ctx.end_frame(swapchain);
  while (!should_close)
  {
    win_sys->poll_events();
    ctx.begin_frame(swapchain);
    // TODO(lamarrr): maybe check for frame begin before accepting commands
    canvas.begin({1920, 1080});

    canvas.rrect(ShapeDesc{.center       = Vec2{1920 / 2, 1080 / 2},
                           .extent       = {1920, 1080},
                           .border_radii = {0, 0, 0, 0},
                           .stroke       = 1,
                           .thickness    = 20,
                           .tint = ColorGradient::uniform(colors::WHITE)});
    /*  for (u32 i = 0; i < 2000; i++)
        canvas.rect(
            ShapeDesc{.center = Vec2{20, 20},
                      .extent = {160, 160},
                      .tint   = {f(colors::RED) / 255, f(colors::BLUE) / 255,
                                 f(colors::MAGENTA) / 255, f(colors::CYAN) /
       255}});*/
    /*canvas.line(
        ShapeDesc{.center    = Vec2{0, 0},
                  .extent    = {800, 800},
                  .thickness = 20,
                  .tint      = {f(colors::RED) / 255, f(colors::BLUE) / 255,
                                f(colors::MAGENTA) / 255, f(colors::CYAN) /
       255}}, to_span({// Vec2{0, 0},
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
        ShapeDesc{.center    = Vec2{1920 / 2.0f, 1080 / 2.0f},
                  .transform = Mat4::identity(),
                  .thickness = 20,
                  .tint      = {colors::RED.norm(), colors::BLUE.norm(),
                                colors::MAGENTA.norm(), colors::CYAN.norm()}},
        text_block, text_layout,
        TextBlockStyle{
            .runs        = to_span<TextStyle>({TextStyle{
                       .underline_thickness     = 0,
                       .strikethrough_thickness = 0,
                       .shadow_scale            = 0,
                       .shadow_offset           = Vec2{1, 1},
                       .foreground    = ColorGradient::y(colors::RED, colors::YELLOW),
                       .background    = ColorGradient::uniform(Vec4{0, 0, 0, 1}),
                       .underline     = ColorGradient::uniform(colors::WHITE),
                       .strikethrough = ColorGradient::uniform(colors::WHITE),
                       .shadow        = ColorGradient::uniform(colors::WHITE)}}),
            .alignment   = 0,
            .align_width = 1920},
        to_span<FontAtlasResource const *>({&font_resource}));
    // canvas.blur({.scissor = {{0, 0}, {U32_MAX, U32_MAX}}}, rr);
    /*canvas.triangles(
        ShapeDesc{.center    = Vec2{0, 0},
                  .extent    = {800, 800},
                  .thickness = 0,
                  .tint      = {f(colors::RED) / 255, f(colors::BLUE) / 255,
                                f(colors::MAGENTA) / 255, f(colors::CYAN) /
       255}},

        to_span({Vec2{-1, -1}, Vec2{1, -1}, Vec2{1, 1}, Vec2{1, 1}, Vec2{-1, 1},
                 Vec2{-1, -1}}));*/

    Vec<Vec2> squircle;
    Path::squircle(squircle, 2048);
    canvas.line(
        ShapeDesc{.center    = {1920 / 2, 1080 / 2},
                  .extent    = {400, 400},
                  .stroke    = 1,
                  .thickness = 4,
                  .tint = ColorGradient::y(colors::MAGENTA, colors::YELLOW)},
        to_span(squircle));
    squircle.reset();

    renderer.begin(ctx, pctx, canvas,
                   gfx::RenderingInfo{
                       .render_area       = {{0, 0}, {1920, 1080}},
                       .num_layers        = 1,
                       .color_attachments = to_span({gfx::RenderingAttachment{
                           .view = ctx.screen_fb.color.view}})},
                   ctx.screen_fb.color_texture);
    renderer.render(ctx, pctx,
                    gfx::RenderingInfo{
                        .render_area       = {{0, 0}, {1920, 1080}},
                        .num_layers        = 1,
                        .color_attachments = to_span({gfx::RenderingAttachment{
                            .view = ctx.screen_fb.color.view}})},
                    gfx::Viewport{.offset    = {0, 0},
                                  .extent    = {1920, 1080},
                                  .min_depth = 0,
                                  .max_depth = 1},
                    {1920, 1080}, ctx.screen_fb.color_texture, canvas);
    ctx.end_frame(swapchain);
    canvas.clear();
  }
  default_logger->info("closing");
}
