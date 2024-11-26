/// SPDX-License-Identifier: MIT
#include "ashura/engine/engine.h"
#include "ashura/std/fs.h"
#include "simdjson.h"

namespace ash
{

ASH_C_LINKAGE ASH_DLL_EXPORT Engine *engine = nullptr;

EngineCfg EngineCfg::parse(AllocatorImpl allocator, Span<u8 const> json)
{
  EngineCfg                    out;
  simdjson::ondemand::parser   parser;
  simdjson::padded_string      str{json.as_char().data(), json.size()};
  simdjson::ondemand::document doc = parser.iterate(str);

  auto config = doc.get_object().value();

  auto version = config["version"].get_string().value();
  CHECK(version == "0.0.1");

  auto gpu           = config["gpu"].get_object().value();
  out.gpu.validation = gpu["validation"].get_bool().value();
  out.gpu.vsync      = gpu["vsync"].get_bool().value();

  auto gpu_prefs = gpu["preferences"].get_array().value();
  CHECK(gpu_prefs.count_elements().value() <= 5);

  for (auto pref : gpu_prefs)
  {
    std::string_view s = pref.get_string().value();
    if (s == "dgpu")
    {
      out.gpu.preferences.push(gpu::DeviceType::DiscreteGpu).unwrap();
    }
    else if (s == "vgpu")
    {
      out.gpu.preferences.push(gpu::DeviceType::VirtualGpu).unwrap();
    }
    else if (s == "igpu")
    {
      out.gpu.preferences.push(gpu::DeviceType::IntegratedGpu).unwrap();
    }
    else if (s == "other")
    {
      out.gpu.preferences.push(gpu::DeviceType::Other).unwrap();
    }
    else if (s == "cpu")
    {
      out.gpu.preferences.push(gpu::DeviceType::Cpu).unwrap();
    }
    else
    {
      CHECK(false);
    }
  }

  out.gpu.hdr = gpu["hdr"].get_bool().value();
  out.gpu.buffering =
      (u32) clamp(gpu["buffering"].get_int64().value(), (i64) 0, (i64) 4);

  auto window            = config["window"].get_object().value();
  out.window.resizable   = window["resizable"].get_bool().value();
  out.window.maximized   = window["maximized"].get_bool().value();
  out.window.full_screen = window["full_screen"].get_bool().value();
  out.window.width =
      (u32) clamp(window["width"].get_int64().value(), (i64) 0, (i64) U32_MAX);
  out.window.height =
      (u32) clamp(window["height"].get_int64().value(), (i64) 0, (i64) U32_MAX);

  auto shaders = config["shaders"].get_object().value();
  for (auto entry : shaders)
  {
    auto id   = entry.escaped_key().value();
    auto path = entry.value().get_string().value();
    out.shaders
        .insert(vec(allocator, span(id)).unwrap(),
                vec(allocator, span(path)).unwrap())
        .unwrap();
  }

  auto fonts = config["fonts"].get_object().value();
  for (auto entry : fonts)
  {
    auto id   = entry.escaped_key().value();
    auto path = entry.value().get_string().value();
    out.fonts
        .insert(vec(allocator, span(id)).unwrap(),
                vec(allocator, span(path)).unwrap())
        .unwrap();
  }

  std::string_view default_font_sv =
      config["default_font"].get_string().value();
  out.default_font = vec(allocator, span(default_font_sv)).unwrap();

  // check that it is a valid entry
  fonts[default_font_sv].get_string().value();

  auto images = config["images"].get_object().value();
  for (auto entry : images)
  {
    auto id   = entry.escaped_key().value();
    auto path = entry.value().get_string().value();
    out.images
        .insert(vec(allocator, span(id)).unwrap(),
                vec(allocator, span(path)).unwrap())
        .unwrap();
  }

  return out;
}

void Engine::init(AllocatorImpl allocator, void *app,
                  Span<char const> config_path, Span<char const> assets_dir)
{
  if (logger == nullptr)
  {
    abort();
  }
  CHECK(scheduler != nullptr);
  CHECK(engine == nullptr);

  logger->trace("Initializing Window System");

  WindowSystem::init();

  logger->trace("Loading Engine config file");

  Vec<u8> json{allocator};

  read_file(config_path, json).unwrap();

  EngineCfg cfg = EngineCfg::parse(allocator, span(json));

  logger->trace("Initializing Engine");

  Dyn<gpu::Instance *> instance =
      gpu::create_vulkan_instance(allocator, cfg.gpu.validation).unwrap();

  gpu::Device *device =
      instance->create_device(allocator, span(cfg.gpu.preferences), 2).unwrap();

  GpuContext gpu_ctx =
      GpuContext::create(allocator, device, cfg.gpu.hdr, cfg.gpu.buffering,
                         Vec2U{cfg.window.width, cfg.window.height});

  logger->trace("Initializing Renderer");

  Renderer renderer = Renderer::create(allocator);

  Canvas canvas{allocator};

  ViewSystem view_system{allocator};

  ViewContext view_ctx{app, window_system->get_clipboard()};

  logger->trace("Creating Root Window");

  Window window =
      window_system->create_window(*instance, "Ashura"_span).unwrap();

  if (cfg.window.maximized)
  {
    window_system->maximize(window);
  }
  else
  {
    window_system->set_size(window, Vec2U{cfg.window.width, cfg.window.height});
  }

  if (cfg.window.full_screen)
  {
    window_system->make_fullscreen(window);
  }
  else
  {
    window_system->make_windowed(window);
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

  engine = new (storage) Engine{allocator,
                                app,
                                std::move(instance),
                                device,
                                window,
                                surface,
                                cfg.gpu.vsync ? gpu::PresentMode::Fifo :
                                                gpu::PresentMode::Immediate,
                                std::move(gpu_ctx),
                                std::move(renderer),
                                std::move(canvas),
                                std::move(view_system),
                                std::move(view_ctx)};

  window_system->listen(
      SystemEventTypes::All,
      fn(engine, [](Engine *engine, SystemEvent const &event) {
        if (event.type == SystemEventTypes::ThemeChanged)
        {
          engine->view_ctx.theme = event.theme;
        }
      }));

  window_system->listen(
      window, WindowEventTypes::All,
      fn(engine, [](Engine *engine, WindowEvent const &event) {
        if (event.type == WindowEventTypes::CloseRequested)
        {
          engine->should_shutdown = true;
        }
      }));

  engine->device->begin_frame(nullptr).unwrap();

  // [ ] setup window event listeners
  // [ ] recreate_swapchain

  Semaphore sem =
      create_semaphore(allocator, cfg.shaders.size() + cfg.fonts.size())
          .unwrap();

  cfg.shaders.iter([&](Vec<char> &id, Vec<char> &path) {
    Vec<char> resolved_path = vec(allocator, assets_dir).unwrap();
    path_append(resolved_path, span(path)).unwrap();

    async::once([shader_id   = vec(allocator, span(id)).unwrap(),
                 shader_path = std::move(resolved_path), sem = sem.alias(),
                 allocator]() mutable {
      logger->trace("Loading shader ", span(shader_id), " from ",
                    span(shader_path));

      Vec<u8> data{allocator};

      if (Result result = read_file(span(shader_path), data); !result)
      {
        logger->error("Unable to load shader at ", span(shader_path),
                      ", IO Error: ", result.err());
        sem->increment(1);
        return;
      }

      CHECK((data.size() & 3ULL) == 0);

      static_assert(std::endian::native == std::endian::little);

      Vec<u32> data_u32{allocator};

      data_u32.resize_uninit(data.size() >> 2).unwrap();

      mem::copy(span(data), span(data_u32).as_u8());

      logger->trace("Loaded shader ", span(shader_id), " from file");

      async::once(
          [shader_id = std::move(shader_id), sem = std::move(sem),
           data_u32 = std::move(data_u32)]() mutable {
            logger->trace("Sending shader ", span(shader_id), " to GPU");

            gpu::Shader shader =
                engine->device
                    ->create_shader(gpu::ShaderInfo{
                        .label = "Shader"_span, .spirv_code = span(data_u32)})
                    .unwrap();

            engine->assets.shaders.insert(std::move(shader_id), shader)
                .unwrap();

            sem->increment(1);
          },
          async::Ready{}, TaskSchedule{.target = TaskTarget::Main});
    });
  });

  cfg.fonts.iter([&](Vec<char> &id, Vec<char> &path) {
    Vec<char> resolved_path = vec(allocator, assets_dir).unwrap();
    path_append(resolved_path, span(path)).unwrap();

    async::once([font_id   = vec(allocator, span(id)).unwrap(),
                 font_path = std::move(resolved_path), sem = sem.alias(),
                 allocator]() mutable {
      logger->trace("Loading font ", span(font_id), " from ", span(font_path));

      Vec<u8> data{allocator};

      Result read_result = read_file(span(font_path), data);

      if (!read_result)
      {
        logger->error("Unable to load font at ", span(font_path),
                      ", IO Error: ", read_result.err());
        sem->increment(1);
        return;
      }

      Result decode_result = Font::decode(span(data), 0, allocator);

      if (!decode_result)
      {
        logger->error("Unable to decode font at ", span(font_path),
                      "Error: ", decode_result.err());
        sem->increment(1);
        return;
      }

      Dyn<Font *> font = decode_result.unwrap();

      logger->trace("Loaded font ", span(font_id), " from file");

      u32 const font_height = 64;

      logger->trace("Rasterizing font ", span(font_id), " @", font_height,
                    "px ");

      font->rasterize(font_height, allocator).unwrap();

      logger->trace("Rasterized font ", span(font_id));

      async::once(
          [font_id = std::move(font_id), sem = std::move(sem),
           font = std::move(font), allocator]() mutable {
            logger->trace("Uploading font ", span(font_id), " to GPU");

            font->upload_to_device(engine->gpu_ctx, allocator);

            engine->assets.fonts.insert(std::move(font_id), std::move(font))
                .unwrap();

            sem->increment(1);
          },
          async::Ready{}, TaskSchedule{.target = TaskTarget::Main});
    });
  });

  while (!sem->is_completed())
  {
    scheduler->execute_main_thread_loop(1ms, 2ms);
  }

  engine->default_font_name = vec(allocator, span(cfg.default_font)).unwrap();
  engine->default_font = engine->assets.fonts[engine->default_font_name].get();

  engine->renderer.acquire(engine->gpu_ctx, engine->assets);

  engine->device->submit_frame(nullptr).unwrap();

  logger->trace("Engine Initialized");
}

void Engine::uninit()
{
  CHECK(engine != nullptr);
  logger->trace("Uninitializing Engine");
  engine->~Engine();
  engine = nullptr;
  logger->trace("Engine Uninitialized");
}

Engine::~Engine()
{
  // [ ] renderer must be uninit before device
  // [ ] canvas
  assets.shaders.iter(
      [&](Vec<char> &, gpu::Shader shader) { device->uninit_shader(shader); });
  assets.shaders.clear();
  assets.fonts.iter([&](Vec<char> &, Dyn<Font *> &font) {
    font->unload_from_device(gpu_ctx);
  });
  assets.fonts.clear();
  device->uninit_swapchain(swapchain);
  window_system->uninit_window(window);
  logger->trace("Uninitializing Window System");
  WindowSystem::uninit();
  instance->uninit_device(device);
}

void Engine::recreate_swapchain_()
{
  gpu::SurfaceCapabilities capabilities =
      device->get_surface_capabilities(surface).unwrap();
  CHECK(
      has_bits(capabilities.image_usage, gpu::ImageUsage::TransferDst |
                                             gpu::ImageUsage::ColorAttachment));

  Vec<gpu::SurfaceFormat> formats{allocator};
  device->get_surface_formats(surface, formats).unwrap();

  Vec<gpu::PresentMode> present_modes{allocator};
  device->get_surface_present_modes(surface, present_modes).unwrap();

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
    swapchain = device->create_swapchain(surface, info).unwrap();
  }
  else
  {
    device->invalidate_swapchain(swapchain, info).unwrap();
  }
}

void Engine::run(View &view)
{
  view_ctx.timestamp = steady_clock::now();
  view_ctx.timedelta = 0ms;

  logger->trace("Starting Engine Run Loop");

  if (swapchain == nullptr)
  {
    recreate_swapchain_();
  }

  while (!should_shutdown)
  {
    // [ ] preprocess window events
    //
    // [ ] recreate frame buffers when extent changed
    //
    time_point const timestamp = steady_clock::now();
    view_ctx.timedelta         = timestamp - view_ctx.timestamp;
    view_ctx.timestamp         = timestamp;

    window_system->poll_events();
    gpu_ctx.begin_frame(swapchain);

    // [ ] VIEWS should not be ticked often, they should not be ticked when
    // there's no viewport

    view_ctx.viewport_extent = as_vec2(gpu_ctx.screen_fb.extent);

    // prepare view_ctx for new frame:  clear events

    // view_ctx.viewport_extent;
    // view_ctx.text_input;
    // view_ctx.drag_payload
    // view_ctx.keyboard;
    // view_ctx.mouse;
    //
    // enum class DragSource{ ext; view; };
    //
    // drop type:
    //
    // outside drag and drop?
    //
    //
    // [ ] canvas recording needs to happen before renderer render frame, two
    // separate thingaa
    //
    //
    //

    gpu::RenderingAttachment attachments[] = {
        {.view         = gpu_ctx.screen_fb.color.view,
         .resolve      = nullptr,
         .resolve_mode = gpu::ResolveModes::None,
         .load_op      = gpu::LoadOp::Load,
         .store_op     = gpu::StoreOp::Store,
         .clear        = {}}};

    RenderTarget rt{
        .info =
            gpu::RenderingInfo{
                .render_area        = {.offset = {},
                                       .extent = gpu_ctx.screen_fb.extent},
                .num_layers         = 1,
                .color_attachments  = span(attachments),
                .depth_attachment   = {},
                .stencil_attachment = {}},
        .viewport           = gpu::Viewport{.offset = {0, 0},
                                            .extent = as_vec2(gpu_ctx.screen_fb.extent),
                                            .min_depth = 0,
                                            .max_depth = 1},
        .extent             = gpu_ctx.screen_fb.extent,
        .color_descriptor   = gpu_ctx.screen_fb.color_texture,
        .depth_descriptor   = nullptr,
        .stencil_descriptor = nullptr};

    canvas.begin_recording(Vec2{(f32) rt.extent.x, (f32) rt.extent.y},
                           rt.extent);

    view_system.tick(view_ctx, view, canvas);

    canvas.end_recording();

    renderer.begin_frame(gpu_ctx, rt, canvas);
    renderer.render_frame(gpu_ctx, rt, canvas);
    renderer.end_frame(gpu_ctx, rt, canvas);
    gpu_ctx.submit_frame(swapchain);
  }
  logger->trace("Ended Engine Run Loop");
}

}        // namespace ash