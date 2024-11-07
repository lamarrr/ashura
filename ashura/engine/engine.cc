/// SPDX-License-Identifier: MIT
#include "ashura/engine/engine.h"

namespace ash
{

static void recreate_swapchain(Engine *e)
{
  gpu::SurfaceCapabilities capabilities =
      e->device->get_surface_capabilities(e->device.self, e->surface).unwrap();
  CHECK(
      has_bits(capabilities.image_usage, gpu::ImageUsage::TransferDst |
                                             gpu::ImageUsage::ColorAttachment));

  Vec<gpu::SurfaceFormat> formats;
  defer                   formats_{[&] { formats.uninit(); }};
  u32                     num_formats =
      e->device->get_surface_formats(e->device.self, e->surface, {}).unwrap();
  CHECK(num_formats != 0);
  formats.resize_uninit(num_formats).unwrap();
  CHECK(
      e->device->get_surface_formats(e->device.self, e->surface, span(formats))
          .unwrap() == num_formats);

  Vec<gpu::PresentMode> present_modes;
  defer                 present_modes_{[&] { present_modes.uninit(); }};
  u32                   num_present_modes =
      e->device->get_surface_present_modes(e->device.self, e->surface, {})
          .unwrap();
  CHECK(num_present_modes != 0);
  present_modes.resize_uninit(num_present_modes).unwrap();
  CHECK(e->device
            ->get_surface_present_modes(e->device.self, e->surface,
                                        span(present_modes))
            .unwrap() == num_present_modes);

  Vec2U surface_extent = sdl_window_system->get_surface_size(e->window);
  surface_extent.x     = max(surface_extent.x, 1U);
  surface_extent.y     = max(surface_extent.y, 1U);

  gpu::ColorSpace preferred_color_spaces[] = {
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
      e->present_mode_preference, gpu::PresentMode::Immediate,
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

  gpu::CompositeAlpha alpha             = gpu::CompositeAlpha::None;
  gpu::CompositeAlpha alpha_spec        = gpu::CompositeAlpha::Opaque;
  gpu::CompositeAlpha preferred_alpha[] = {alpha_spec,
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

  if (e->swapchain == nullptr)
  {
    e->swapchain =
        e->device->create_swapchain(e->device.self, e->surface, desc).unwrap();
  }
  else
  {
    e->device->invalidate_swapchain(e->device.self, e->swapchain, desc)
        .unwrap();
  }
}

void Engine::init()
{
  logger->trace("Initializing Engine");
  // [ ] flags to enable validation layer
  instance = gpu::create_vulkan_instance(heap_allocator, false).unwrap();

  // [ ] read device preference from config
  device =
      instance
          ->create_device(
              instance.self, default_allocator,
              span({gpu::DeviceType::DiscreteGpu, gpu::DeviceType::VirtualGpu,
                    gpu::DeviceType::IntegratedGpu, gpu::DeviceType::Cpu,
                    gpu::DeviceType::Other}),
              2)
          .unwrap();

  logger->trace("Initializing Window System");
  sdl_window_system->init();
  window = sdl_window_system->create_window(instance, "Ashura"_span).unwrap();
  sdl_window_system->maximize(window);

  // get window config
  // color space config
  // present mode config
  // setup window event listeners
  // load default shaders? in SPIRV mode!!!
  // compile shaders if necessary

  surface = sdl_window_system->get_surface(window);

  // recreate_swapchain

  logger->trace("Initializing GPU Context");
  gpu_ctx.init(device, true, 2, {{}}, {});
  logger->trace("Initializing Renderer");
  renderer.init(gpu_ctx);
  canvas.init();
  renderer.canvas = &canvas;
  view_system.init();
  logger->trace("Engine Initialized");
}

void Engine::uninit()
{
  logger->trace("Uninitializing Engine");
  view_system.uninit();
  canvas.uninit();
  logger->trace("Uninitializing Renderer");
  renderer.uninit(gpu_ctx);
  logger->trace("Uninitializing GPU Context");
  gpu_ctx.uninit();
  sdl_window_system->uninit_window(window);
  logger->trace("Uninitializing Window System");
  sdl_window_system->uninit();
  instance->uninit_device(instance.self, device.self);
  instance->uninit(instance.self);
  logger->trace("Engine Uninitialized");
}

void Engine::run(void *app, View &view)
{
  logger->trace("Starting Engine Run Loop");
  while (false)
  {
    renderer.begin_frame(gpu_ctx);
    // view_system.tick(view_ctx, view, canvas);
    renderer.render_frame(gpu_ctx);
    renderer.end_frame(gpu_ctx);
  }

  logger->trace("Ended Engine Run Loop");

  // poll window events
  // preprocess window events
}

}        // namespace ash