/// SPDX-License-Identifier: MIT
#include "ashura/engine/engine.h"
#include "ashura/std/fs.h"
#include "simdjson.h"

namespace ash
{

ASH_C_LINKAGE ASH_DLL_EXPORT Engine * engine = nullptr;

EngineCfg EngineCfg::parse(AllocatorImpl allocator, Span<u8 const> json)
{
  EngineCfg                    out;
  simdjson::ondemand::parser   parser;
  simdjson::padded_string      str{json.as_char().data(), json.size()};
  simdjson::ondemand::document doc = parser.iterate(str);

  auto config = doc.get_object().value();

  std::string_view version = config["version"].get_string().value();
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
    std::string_view id   = entry.escaped_key().value();
    std::string_view path = entry.value().get_string().value();
    out.shaders
        .insert(vec(span(id), allocator).unwrap(),
                vec(span(path), allocator).unwrap())
        .unwrap();
  }

  auto fonts = config["fonts"].get_object().value();
  for (auto entry : fonts)
  {
    std::string_view id   = entry.escaped_key().value();
    std::string_view path = entry.value().get_string().value();
    out.fonts
        .insert(vec(span(id), allocator).unwrap(),
                vec(span(path), allocator).unwrap())
        .unwrap();
  }

  std::string_view default_font_sv =
      config["default_font"].get_string().value();
  out.default_font = vec<char>(default_font_sv, allocator).unwrap();

  // check that it is a valid entry
  fonts[default_font_sv].get_string().value();

  auto images = config["images"].get_object().value();
  for (auto entry : images)
  {
    std::string_view id   = entry.escaped_key().value();
    std::string_view path = entry.value().get_string().value();
    out.images
        .insert(vec(span(id), allocator).unwrap(),
                vec(span(path), allocator).unwrap())
        .unwrap();
  }

  return out;
}

void Engine::init(AllocatorImpl allocator, void * app,
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

  EngineCfg cfg = EngineCfg::parse(allocator, json);

  logger->trace("Initializing Engine");

  Dyn<gpu::Instance *> instance =
      gpu::create_vulkan_instance(allocator, cfg.gpu.validation).unwrap();

  gpu::Device * device =
      instance->create_device(allocator, cfg.gpu.preferences, 2).unwrap();

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
      window_system->create_window(*instance, "Ashura"_str).unwrap();

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

  window_system->listen(fn(
      engine, +[](Engine * engine, SystemEvent const & event) {
        event.match(
            [&](SystemTheme theme) {
              FrameState & state = engine->view_ctx.state_buffer;
              state.theme        = theme;
            },
            [](SystemEventType) {});
      }));

  window_system->listen(
      window, fn(
                  engine, +[](Engine * engine, WindowEvent const & event) {
                    FrameState & state = engine->view_ctx.state_buffer;

                    event.match(
                        [&](KeyEvent e) {
                          switch (e.action)
                          {
                            case KeyAction::Press:
                            {
                              state.key.any_down = true;
                              set_bit(state.key.downs, (u32) e.key_code);
                              set_bit(state.key.scan_downs, (u32) e.scan_code);
                              state.key.modifiers |= e.modifiers;
                            }
                            break;
                            case KeyAction::Release:
                            {
                              state.key.any_up = true;
                              set_bit(state.key.ups, (u32) e.key_code);
                              set_bit(state.key.scan_ups, (u32) e.scan_code);
                              state.key.modifiers |= e.modifiers;
                            }
                            break;
                            default:
                              break;
                          }
                        },
                        [&](MouseMotionEvent e) {
                          state.mouse.moved    = true;
                          state.mouse.position = e.position;
                          state.mouse.translation += e.translation;
                        },
                        [&](MouseClickEvent e) {
                          state.mouse.num_clicks[(u32) e.button] = e.clicks;
                          state.mouse.position                   = e.position;
                          switch (e.action)
                          {
                            case KeyAction::Press:
                              set_bit(state.mouse.downs, (u32) e.button);
                              state.mouse.any_down = true;
                              break;
                            case KeyAction::Release:
                              set_bit(state.mouse.ups, (u32) e.button);
                              state.mouse.any_up = true;
                              break;
                            default:
                              break;
                          }
                        },
                        [&](MouseWheelEvent e) {
                          state.mouse.wheel_scrolled = true;
                          state.mouse.position       = e.position;
                          state.mouse.translation += e.translation;
                        },
                        [&](TextInputEvent e) {
                          state.text_input = true;
                          state.text.extend(e.text).unwrap();
                        },
                        [&](WindowEventType e) {
                          switch (e)
                          {
                            case WindowEventType::Shown:
                            case WindowEventType::Hidden:
                            case WindowEventType::Exposed:
                            case WindowEventType::Moved:
                              break;
                            case WindowEventType::Resized:
                              state.resized = true;
                              break;
                            case WindowEventType::SurfaceResized:
                              state.surface_resized = true;
                              break;
                            case WindowEventType::Minimized:
                            case WindowEventType::Maximized:
                            case WindowEventType::Restored:
                              break;
                            case WindowEventType::MouseEnter:
                              state.mouse.in      = true;
                              state.mouse_focused = true;
                              break;
                            case WindowEventType::MouseLeave:
                              state.mouse.out     = true;
                              state.mouse_focused = false;
                              break;
                            case WindowEventType::KeyboardFocusIn:
                              state.key.in      = true;
                              state.key_focused = true;
                              break;
                            case WindowEventType::KeyboardFocusOut:
                              state.key.out     = true;
                              state.key_focused = false;
                              break;
                            case WindowEventType::CloseRequested:
                              state.close_requested = true;
                              break;
                            case WindowEventType::Occluded:
                            case WindowEventType::EnterFullScreen:
                            case WindowEventType::LeaveFullScreen:
                            case WindowEventType::Destroyed:
                              break;
                            default:
                              break;
                          }
                        },
                        [&](DropEvent const & e) {
                          e.match(
                              [&](DropEventType e) {
                                switch (e)
                                {
                                  case DropEventType::DropBegin:
                                    break;
                                  case DropEventType::DropComplete:
                                    state.dropped = true;
                                    break;
                                  default:
                                    break;
                                }
                              },
                              [&](DropPositionEvent e) {
                                state.drop_hovering  = true;
                                state.mouse.position = e.pos;
                              },
                              [&](DropFileEvent e) {
                                state.drop_data.clear();
                                state.drop_data.extend(e.path.as_u8()).unwrap();
                                state.drop_type = DropType::FilePath;
                              },
                              [&](DropTextEvent e) {
                                state.drop_data.clear();
                                state.drop_data.extend(e.text.as_u8()).unwrap();
                                state.drop_type = DropType::Bytes;
                              });
                        });
                  }));

  engine->device->begin_frame(nullptr).unwrap();

  Semaphore sem =
      create_semaphore(allocator, cfg.shaders.size() + cfg.fonts.size())
          .unwrap();

  for (auto const & [id, path] : cfg.shaders)
  {
    Vec<char> resolved_path = vec(assets_dir, allocator).unwrap();
    path_append(resolved_path, path).unwrap();

    async::once([shader_id   = vec<char>(id, allocator).unwrap(),
                 shader_path = std::move(resolved_path), sem = sem.alias(),
                 allocator]() mutable {
      logger->trace("Loading shader ", shader_id, " from ", shader_path);

      Vec<u8> data{allocator};

      if (Result result = read_file(shader_path, data); !result)
      {
        logger->error("Unable to load shader at ", shader_path,
                      ", IO Error: ", result.err());
        sem->increment(1);
        return;
      }

      CHECK((data.size() & 3ULL) == 0);

      static_assert(std::endian::native == std::endian::little);

      Vec<u32> data_u32{allocator};

      data_u32.resize_uninit(data.size() >> 2).unwrap();

      mem::copy(data.view(), data_u32.view().as_u8());

      logger->trace("Loaded shader ", shader_id, " from file");

      async::once(
          [shader_id = std::move(shader_id), sem = std::move(sem),
           data_u32 = std::move(data_u32)]() mutable {
            logger->trace("Sending shader ", shader_id, " to GPU");

            gpu::Shader shader =
                engine->device
                    ->create_shader(gpu::ShaderInfo{.label      = "Shader"_str,
                                                    .spirv_code = data_u32})
                    .unwrap();

            engine->assets.shaders.insert(std::move(shader_id), shader)
                .unwrap();

            sem->increment(1);
          },
          async::Ready{}, TaskSchedule{.target = TaskTarget::Main});
    });
  }

  for (auto const & [id, path] : cfg.fonts)
  {
    Vec<char> resolved_path = vec(assets_dir, allocator).unwrap();
    path_append(resolved_path, path).unwrap();

    async::once([font_id   = vec<char>(id, allocator).unwrap(),
                 font_path = std::move(resolved_path), sem = sem.alias(),
                 allocator]() mutable {
      logger->trace("Loading font ", font_id, " from ", font_path);

      Vec<u8> data{allocator};

      Result read_result = read_file(font_path, data);

      if (!read_result)
      {
        logger->error("Unable to load font at ", font_path,
                      ", IO Error: ", read_result.err());
        sem->increment(1);
        return;
      }

      Result decode_result = Font::decode(data, 0, allocator);

      if (!decode_result)
      {
        logger->error("Unable to decode font at ", font_path,
                      "Error: ", decode_result.err());
        sem->increment(1);
        return;
      }

      Dyn<Font *> font = decode_result.unwrap();

      logger->trace("Loaded font ", font_id, " from file");

      u32 const font_height = 64;

      logger->trace("Rasterizing font ", font_id, " @", font_height, "px ");

      font->rasterize(font_height, allocator).unwrap();

      logger->trace("Rasterized font ", font_id);

      async::once(
          [font_id = std::move(font_id), sem = std::move(sem),
           font = std::move(font), allocator]() mutable {
            logger->trace("Uploading font ", font_id, " to GPU");

            font->upload_to_device(engine->gpu_ctx, allocator);

            engine->assets.fonts.insert(std::move(font_id), std::move(font))
                .unwrap();

            sem->increment(1);
          },
          async::Ready{}, TaskSchedule{.target = TaskTarget::Main});
    });
  }

  while (!sem->is_completed())
  {
    scheduler->execute_main_thread_loop(1ms, 2ms);
  }

  engine->default_font_name = vec<char>(cfg.default_font, allocator).unwrap();
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
  device->wait_idle().unwrap();
  canvas.reset();

  for (auto const & [_, shader] : assets.shaders)
  {
    device->uninit_shader(shader);
  }

  assets.shaders.clear();

  for (auto const & [_, font] : assets.fonts)
  {
    font->unload_from_device(gpu_ctx);
  }

  assets.fonts.clear();

  renderer.release(gpu_ctx, assets);

  gpu_ctx.uninit();

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
    Span sel = find_if(formats.view(), [&](gpu::SurfaceFormat a) {
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
    if (!find(present_modes.view(), pm).is_empty())
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

  gpu::SwapchainInfo info{.label  = "Window Swapchain"_str,
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

void Engine::run(View & view, Fn<void(time_point, nanoseconds)> loop)
{
  logger->trace("Starting Engine Run Loop");

  if (swapchain == nullptr)
  {
    recreate_swapchain_();
  }

  time_point timestamp = steady_clock::now();

  // [ ] remove !view_ctx.close_requested once we have proper cooperative shutdown
  while (!should_shutdown && !view_ctx.close_requested)
  {
    time_point const  current_time = steady_clock::now();
    nanoseconds const time_delta   = current_time - timestamp;
    timestamp                      = current_time;

    view_ctx.swap_frame_state_();
    auto & state = view_ctx.state_buffer;

    state.clear_frame_();

    window_system->poll_events();

    // [ ] EVENTS NEED TO BE UPDATED 1 FRAME LATER, NOT IMMEDIATELY AS RECEIVED

    // [ ] get mouse position?

    window_system->get_mouse_state(state.mouse.states);
    window_system->get_keyboard_state(state.key.states);

    gpu_ctx.begin_frame(swapchain);

    if (view_ctx.resized || view_ctx.surface_resized)
    {
      Vec2U const surface_size = window_system->get_surface_size(window);
      gpu_ctx.recreate_framebuffers(surface_size);
    }

    view_ctx.viewport_extent = as_vec2(gpu_ctx.screen_fb.extent);

    gpu::RenderingAttachment attachments[] = {
        {.view         = gpu_ctx.screen_fb.color.view,
         .resolve      = nullptr,
         .resolve_mode = gpu::ResolveModes::None,
         .load_op      = gpu::LoadOp::Load,
         .store_op     = gpu::StoreOp::Store,
         .clear        = {}}
    };

    RenderTarget rt{
        .info =
            gpu::RenderingInfo{
                               .render_area        = {.offset = {},
                                       .extent = gpu_ctx.screen_fb.extent},
                               .num_layers         = 1,
                               .color_attachments  = attachments,
                               .depth_attachment   = {},
                               .stencil_attachment = {}},
        .viewport           = gpu::Viewport{.offset = {0, 0},
                               .extent = as_vec2(gpu_ctx.screen_fb.extent),
                               .min_depth = 0,
                               .max_depth = 1},
        .extent             = gpu_ctx.screen_fb.extent,
        .color_descriptor   = gpu_ctx.screen_fb.color_texture,
        .depth_descriptor   = nullptr,
        .stencil_descriptor = nullptr
    };

    view_ctx.begin_frame_(timestamp, time_delta);

    canvas.begin_recording(Vec2{(f32) rt.extent.x, (f32) rt.extent.y},
                           rt.extent);

    loop(timestamp, time_delta);

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
