
#include "ashura/engine/shader.h"
#include "ashura/engine/window.h"
#include "ashura/gfx/vulkan.h"
#include "ashura/renderer/render_context.h"
#include "ashura/renderer/renderer.h"
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

  WindowSystem *win_sys = init_sdl_window_system();
  CHECK(win_sys != nullptr);

  gfx::InstanceImpl instance =
      gfx::create_vulkan_instance(heap_allocator, default_logger, true)
          .unwrap();

  defer instance_del{[&] { instance->destroy(instance.self); }};
  u32   win = win_sys->create_window(instance, "Main").unwrap();
  defer win_del{[&] { win_sys->destroy_window(win); }};

  win_sys->maximize(win);
  win_sys->set_title(win, "Harro");

  bool should_close = false;
  auto close_fn     = [&](WindowEvent const &) { should_close = true; };

  win_sys->listen(win, WindowEventTypes::CloseRequested, to_fn_ref(close_fn));
  gfx::Surface    surface = win_sys->get_surface(win);
  gfx::DeviceImpl device =
      instance
          ->create_device(
              instance.self, default_allocator,
              to_span({gfx::DeviceType::Cpu, gfx::DeviceType::VirtualGpu,
                       gfx::DeviceType::Other, gfx::DeviceType::DiscreteGpu,
                       gfx::DeviceType::IntegratedGpu}),
              to_span({surface}), 2)
          .unwrap();
  defer device_del{
      [&] { instance->destroy_device(instance.self, device.self); }};

  Vec<Tuple<Span<char const>, Vec<u32>>> spirvs;

  CHECK(
      pack_shaders(
          spirvs,
          to_span<ShaderPackEntry>(
              {{.id = "ConvexPoly:FS"_span, .file = "convex_poly.frag"_span},
               {.id = "ConvexPoly:VS"_span, .file = "convex_poly.vert"_span},
               {.id       = "KawaseBlur_UpSample:FS"_span,
                .file     = "kawase_blur.frag"_span,
                .preamble = "#define UPSAMPLE 1"_span},
               {.id       = "KawaseBlur_UpSample:VS"_span,
                .file     = "kawase_blur.vert"_span,
                .preamble = "#define UPSAMPLE 1"_span},
               {.id       = "KawaseBlur_DownSample:FS"_span,
                .file     = "kawase_blur.frag"_span,
                .preamble = "#define UPSAMPLE 0"_span},
               {.id       = "KawaseBlur_DownSample:VS"_span,
                .file     = "kawase_blur.vert"_span,
                .preamble = "#define UPSAMPLE 0"_span},
               {.id = "PBR:FS"_span, .file = "pbr.frag"_span},
               {.id = "PBR:VS"_span, .file = "pbr.vert"_span},
               {.id = "RRect:FS"_span, .file = "rrect.frag"_span},
               {.id = "RRect:VS"_span, .file = "rrect.vert"_span}}),
          "C:\\Users\\rlama\\Documents\\workspace\\oss\\ashura\\ashura\\shaders"_span) ==
      ShaderCompileError::None)

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

  Renderer renderer;
  renderer.init(device, true, 2, {1920, 1080}, shaders);
  shaders = {};
  defer renderer_del{[&] { renderer.uninit(); }};

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
                                     gfx::ImageUsage::ColorAttachment |
                                     gfx::ImageUsage::InputAttachment,
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

  // TODO(lamarrr): update preferred extent

  while (!should_close)
  {
    win_sys->poll_events();
    renderer.begin_frame(swapchain);
    renderer.record_frame();
    renderer.end_frame(swapchain);
  }
  default_logger->info("closing");
}
