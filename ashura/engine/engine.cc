/// SPDX-License-Identifier: MIT
#include "ashura/engine/engine.h"
#include "ashura/std/fs.h"
#include "simdjson.h"

namespace ash
{

ASH_C_LINKAGE ASH_DLL_EXPORT Engine *engine = nullptr;

EngineCfg EngineCfg::parse(Span<u8 const> json)
{
  EngineCfg                    cfg;
  simdjson::ondemand::parser   parser;
  simdjson::padded_string      str{json.as_char().data(), json.size()};
  simdjson::ondemand::document doc = parser.iterate(str);

  auto config = doc.get_object().value();

  auto version = config["version"].get_string().value();
  CHECK(version == "0.0.1");

  auto gpu           = config["gpu"].get_object().value();
  cfg.gpu.validation = gpu["validation"].get_bool().value();
  cfg.gpu.vsync      = gpu["vsync"].get_bool().value();

  auto gpu_prefs = gpu["preferences"].get_array().value();
  CHECK(gpu_prefs.count_elements().value() <= 5);

  for (auto pref : gpu_prefs)
  {
    std::string_view s = pref.get_string().value();
    if (s == "dgpu")
    {
      cfg.gpu.preferences.push(gpu::DeviceType::DiscreteGpu).unwrap();
    }
    else if (s == "vgpu")
    {
      cfg.gpu.preferences.push(gpu::DeviceType::VirtualGpu).unwrap();
    }
    else if (s == "igpu")
    {
      cfg.gpu.preferences.push(gpu::DeviceType::IntegratedGpu).unwrap();
    }
    else if (s == "other")
    {
      cfg.gpu.preferences.push(gpu::DeviceType::Other).unwrap();
    }
    else if (s == "cpu")
    {
      cfg.gpu.preferences.push(gpu::DeviceType::Cpu).unwrap();
    }
    else
    {
      CHECK(false);
    }
  }

  cfg.gpu.hdr = gpu["hdr"].get_bool().value();
  cfg.gpu.buffering =
      (u32) clamp(gpu["buffering"].get_int64().value(), (i64) 0, (i64) 4);

  auto window          = config["window"].get_object().value();
  cfg.window.resizable = window["resizable"].get_bool().value();
  cfg.window.maximized = window["maximized"].get_bool().value();
  cfg.window.width =
      (u32) clamp(window["width"].get_int64().value(), (i64) 0, (i64) U32_MAX);
  cfg.window.height =
      (u32) clamp(window["height"].get_int64().value(), (i64) 0, (i64) U32_MAX);

  auto shaders = config["shaders"].get_object().value();
  for (auto entry : shaders)
  {
    auto id   = entry.escaped_key().value();
    auto path = entry.value().get_string().value();
    bool exists;
    CHECK(cfg.shaders.insert(exists, nullptr, vec({}, span(id)).unwrap(),
                             vec({}, span(path)).unwrap()));
  }

  auto fonts = config["fonts"].get_object().value();
  for (auto entry : fonts)
  {
    auto id   = entry.escaped_key().value();
    auto path = entry.value().get_string().value();
    bool exists;
    CHECK(cfg.fonts.insert(exists, nullptr, vec({}, span(id)).unwrap(),
                           vec({}, span(path)).unwrap()));
  }

  return cfg;
}

void Engine::init(AllocatorImpl allocator, void *app,
                  Span<char const> config_path, Span<char const> assets_dir)
{
  CHECK(engine == nullptr);

  CHECK(logger != nullptr);
  CHECK(scheduler != nullptr);

  logger->trace("Loading Engine config file");

  Vec<u8> json{allocator};

  read_file(config_path, json).unwrap();

  EngineCfg cfg = EngineCfg::parse(span(json));

  logger->trace("Initializing Engine");

  gpu::InstanceImpl instance =
      gpu::create_vulkan_instance(allocator, cfg.gpu.validation).unwrap();

  gpu::DeviceImpl device = instance
                               ->create_device(instance.self, allocator,
                                               span(cfg.gpu.preferences), 2)
                               .unwrap();

  GpuContext gpu_ctx =
      GpuContext::create(allocator, device, cfg.gpu.hdr, cfg.gpu.buffering,
                         Vec2U{cfg.window.width, cfg.window.height});

  logger->trace("Initializing Renderer");

  Renderer renderer{allocator};

  Canvas canvas{allocator};

  ViewSystem view_system{allocator};

  ViewContext view_ctx{app, window_system->get_clipboard()};

  logger->trace("Initializing Window System");

  WindowSystem::init();

  logger->trace("Creating Root Window");

  Window window =
      window_system->create_window(instance, "Ashura"_span).unwrap();

  if (cfg.window.maximized)
  {
    window_system->maximize(window);
  }
  else
  {
    window_system->set_size(window, Vec2U{cfg.window.width, cfg.window.height});
  }

  if (cfg.window.resizable)
  {
    window_system->make_resizable(window);
  }
  else
  {
    window_system->make_unresizable(window);
  }

  gpu::Surface surface = window_system->get_surface(window);

  logger->trace("Initializing GPU Context");

  alignas(Engine) static u8 storage[sizeof(Engine)] = {};

  new (storage) Engine{allocator,
                       app,
                       instance,
                       device,
                       window,
                       surface,
                       cfg.gpu.vsync ? gpu::PresentMode::Mailbox :
                                       gpu::PresentMode::Immediate,
                       std::move(gpu_ctx),
                       std::move(renderer),
                       std::move(canvas),
                       std::move(view_system),
                       std::move(view_ctx)};

  window_system->listen(SystemEventTypes::All,
                        fn(engine, [](Engine *engine, SystemEvent const &) {

                        }));

  window_system->listen(
      window, WindowEventTypes::All,
      fn(engine, [](Engine *engine, WindowEvent const &event) {

      }));

  // [ ] setup window event listeners
  // [ ] recreate_swapchain
  // [ ] load shaders, pipelines, fonts

  Semaphore shaders_semaphores =
      create_semaphore(allocator, cfg.shaders.size() * 2).unwrap();
  Semaphore font_semaphores =
      create_semaphore(allocator, cfg.fonts.size() * 2).unwrap();

  cfg.shaders.iter([&](Vec<char> &id, Vec<char> &path) {
    Vec<char> resolved_path = vec(allocator, assets_dir).unwrap();
    path_append(resolved_path, span(path)).unwrap();

    async::once([shader_id   = vec(allocator, span(id)).unwrap(),
                 shader_path = std::move(resolved_path),
                 semaphore = shaders_semaphores.alias(), allocator]() mutable {
      logger->trace("Loading shader ", span(shader_id), " from ",
                    span(shader_path));

      Vec<u8> data{allocator};

      read_file(span(shader_path), data).unwrap();

      CHECK((data.size() & 3ULL) == 0);

      static_assert(std::endian::native == std::endian::little);

      Vec<u32> data_u32{allocator};

      data_u32.resize_uninit(data.size() >> 2).unwrap();

      mem::copy(span(data), span(data_u32).as_u8());

      semaphore->increment(1);

      logger->trace("Loaded shader ", span(shader_id), " from file");

      async::once(
          [shader_id = std::move(shader_id), semaphore = std::move(semaphore),
           data_u32 = std::move(data_u32)]() {
            logger->trace("Sending shader ", span(shader_id), " to GPU");

            bool exists;

            gpu::Shader shader =
                engine->device
                    ->create_shader(
                        engine->device.self,
                        gpu::ShaderInfo{.label      = "Shader"_span,
                                        .spirv_code = span(data_u32)})
                    .unwrap();

            CHECK(engine->shaders.insert(exists, nullptr, std::move(shader_id),
                                         shader));

            logger->trace("Shader", span(shader_id), " loaded to GPU");

            semaphore->increment(1);
          },
          async::Ready{}, TaskSchedule{.target = TaskTarget::Main});
    });
  });

  cfg.fonts.iter([&](Vec<char> &id, Vec<char> &path) {
    Vec<char> resolved_path = vec(allocator, assets_dir).unwrap();
    path_append(resolved_path, span(path)).unwrap();

    async::once([font_id   = vec(allocator, span(id)).unwrap(),
                 font_path = std::move(resolved_path),
                 semaphore = font_semaphores.alias(), allocator]() mutable {
      logger->trace("Loading font ", span(font_id), " from ", span(font_path));

      Vec<u8> data{allocator};

      read_file(span(font_path), data).unwrap();

      Dyn<Font *> font = Font::decode(span(data), 0, allocator).unwrap();

      logger->trace("Loaded font ", span(font_id), " from file");

      u32 const font_height = 64;

      logger->trace("Rasterizing font ", span(font_id), " @", font_height,
                    "px ");

      font->rasterize(font_height, allocator).unwrap();

      logger->trace("Rasterized font ", span(font_id));

      semaphore->increment(1);

      async::once(
          [font_id = std::move(font_id), semaphore = std::move(semaphore),
           font = std::move(font), allocator]() {
            logger->trace("Uploading font ", span(font_id), " to GPU");

            bool exists;

            font->upload_to_device(engine->gpu_ctx, allocator);

            CHECK(engine->fonts.insert(exists, nullptr, std::move(font_id),
                                       std::move(font)));

            logger->trace("Uploaded font ", span(font_id), " to GPU");

            semaphore->increment(1);
          },
          async::Ready{}, TaskSchedule{.target = TaskTarget::Main});
    });
  });

  while (!font_semaphores->is_completed() &&
         !shaders_semaphores->is_completed())
  {
    scheduler->execute_main_thread_loop(1ms, 2ms);
  }

  engine->renderer.acquire(engine->gpu_ctx);

  logger->trace("Engine Initialized");
}

void Engine::uninit()
{
  // check resources have been purged
  // TODO(lamarrr):
  //  shader_map.iter([&](Span<char const>, gpu::Shader shader) {
  //    device->uninit_shader(device.self, shader);
  //  });
  CHECK(engine != nullptr);
  logger->trace("Uninitializing Engine");
  engine->~Engine();
  engine = nullptr;
  logger->trace("Engine Uninitialized");
}

Engine::~Engine()
{
  device->uninit_swapchain(device.self, swapchain);
  window_system->uninit_window(window);
  logger->trace("Uninitializing Window System");
  WindowSystem::uninit();
  instance->uninit_device(instance.self, device.self);
  instance->uninit(instance.self);
}

void Engine::recreate_swapchain_()
{
  gpu::SurfaceCapabilities capabilities =
      device->get_surface_capabilities(device.self, surface).unwrap();
  CHECK(
      has_bits(capabilities.image_usage, gpu::ImageUsage::TransferDst |
                                             gpu::ImageUsage::ColorAttachment));

  Vec<gpu::SurfaceFormat> formats;
  u32                     num_formats =
      device->get_surface_formats(device.self, surface, {}).unwrap();
  CHECK(num_formats != 0);
  formats.resize_uninit(num_formats).unwrap();
  CHECK(device->get_surface_formats(device.self, surface, span(formats))
            .unwrap() == num_formats);

  Vec<gpu::PresentMode> present_modes;
  u32                   num_present_modes =
      device->get_surface_present_modes(device.self, surface, {}).unwrap();
  CHECK(num_present_modes != 0);
  present_modes.resize_uninit(num_present_modes).unwrap();
  CHECK(
      device
          ->get_surface_present_modes(device.self, surface, span(present_modes))
          .unwrap() == num_present_modes);

  Vec2U surface_extent = window_system->get_surface_size(window);
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
      present_mode_preference, gpu::PresentMode::Immediate,
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

  gpu::SwapchainInfo info{.label  = "Window Swapchain"_span,
                          .format = format,
                          .usage  = gpu::ImageUsage::TransferDst |
                                   gpu::ImageUsage::ColorAttachment,
                          .preferred_buffering = gpu_ctx.buffering,
                          .present_mode        = present_mode,
                          .preferred_extent    = surface_extent,
                          .composite_alpha     = alpha};

  if (swapchain == nullptr)
  {
    swapchain = device->create_swapchain(device.self, surface, info).unwrap();
  }
  else
  {
    device->invalidate_swapchain(device.self, swapchain, info).unwrap();
  }
}

void Engine::run(View &view)
{
  logger->trace("Starting Engine Run Loop");
  while (!should_shutdown)
  {
    window_system->poll_events();
    renderer.begin_frame(gpu_ctx, {}, canvas);
    // // view_system.tick(view_ctx, view, canvas);
    // renderer.render_frame(gpu_ctx);
    renderer.render_frame(gpu_ctx, {}, canvas);
    renderer.end_frame(gpu_ctx, {}, canvas);
  }
  logger->trace("Ended Engine Run Loop");
  // poll window events
  // preprocess window events
}

}        // namespace ash