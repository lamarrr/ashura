/// SPDX-License-Identifier: MIT
#include "ashura/engine/engine.h"
#include "ashura/std/async.h"
#include "ashura/std/fs.h"
#include "ashura/std/trace.h"
#include "simdjson.h"

namespace ash
{

EngineCfg EngineCfg::parse(AllocatorRef allocator, Span<u8 const> json)
{
  EngineCfg                    out{.shaders{allocator},
                                   .fonts{allocator},
                                   .images{allocator},
                                   .pipeline_cache{allocator}};
  simdjson::ondemand::parser   parser;
  simdjson::padded_string      str{json.as_char().data(), json.size()};
  simdjson::ondemand::document doc = parser.iterate(str);

  auto cfg = doc.get_object().value();

  std::string_view version = cfg["version"].get_string().value();
  CHECK(version == "0.0.1", "");

  out.gpu.validation = cfg["gpu.validation"].get_bool().value();
  out.gpu.vsync      = cfg["gpu.vsync"].get_bool().value();

  auto gpu_prefs = cfg["gpu.preferences"].get_array().value();
  CHECK(gpu_prefs.count_elements().value() <= 5, "");

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
      CHECK(false, "");
    }
  }

  out.gpu.hdr = cfg["gpu.hdr"].get_bool().value();
  out.gpu.buffering =
    (u32) clamp(cfg["gpu.buffering"].get_int64().value(), (i64) 0, (i64) 4);

  switch (cfg["gpu.msaa.level"].get_int64().value())
  {
    case 1:
      out.gpu.msaa_level = gpu::SampleCount::C1;
      break;
    case 2:
      out.gpu.msaa_level = gpu::SampleCount::C2;
      break;
    case 4:
      out.gpu.msaa_level = gpu::SampleCount::C4;
      break;
    case 8:
      out.gpu.msaa_level = gpu::SampleCount::C8;
      break;
    case 16:
      out.gpu.msaa_level = gpu::SampleCount::C16;
      break;
    default:
      out.gpu.msaa_level = gpu::SampleCount::C4;
      break;
  }

  if (auto fps = cfg["gpu.max_fps"].get_int64();
      fps.error() == simdjson::error_code::SUCCESS)
  {
    out.gpu.max_fps = fps.value();
  }

  out.window.resizable   = cfg["window.resizable"].get_bool().value();
  out.window.maximized   = cfg["window.maximized"].get_bool().value();
  out.window.full_screen = cfg["window.full_screen"].get_bool().value();
  out.window.borderless  = cfg["window.borderless"].get_bool().value();
  out.window.width       = (u32) clamp(cfg["window.width"].get_int64().value(),
                                       (i64) 0, (i64) U32_MAX);
  out.window.height      = (u32) clamp(cfg["window.height"].get_int64().value(),
                                       (i64) 0, (i64) U32_MAX);

  auto shaders = cfg["shaders"].get_object().value();
  for (auto entry : shaders)
  {
    std::string_view id   = entry.escaped_key().value();
    std::string_view path = entry.value().get_string().value();
    out.shaders
      .insert(vec(allocator, span(id)).unwrap(),
              vec(allocator, span(path)).unwrap())
      .unwrap();
  }

  auto fonts = cfg["fonts"].get_object().value();
  for (auto entry : fonts)
  {
    std::string_view id   = entry.escaped_key().value();
    std::string_view path = entry.value().get_string().value();
    out.fonts
      .insert(vec(allocator, span(id)).unwrap(),
              vec(allocator, span(path)).unwrap())
      .unwrap();
  }

  out.font_height =
    clamp((u32) cfg["fonts.height"].get_int64().value(), 16U, 256U);

  auto images = cfg["images"].get_object().value();
  for (auto entry : images)
  {
    std::string_view id   = entry.escaped_key().value();
    std::string_view path = entry.value().get_string().value();
    out.images
      .insert(vec(allocator, span(id)).unwrap(),
              vec(allocator, span(path)).unwrap())
      .unwrap();
  }

  std::string_view pipeline_cache_path =
    cfg["cache.pipeline.path"].get_string().value();

  out.pipeline_cache.extend(pipeline_cache_path).unwrap();

  return out;
}

static void window_event_listener(Engine * engine, WindowEvent const & event)
{
  InputState & f = engine->input_buffer;

  event.match(
    [&](KeyEvent e) {
      switch (e.action)
      {
        case KeyAction::Press:
        {
          f.key.any_down = true;
          set_bit(f.key.key_downs, (usize) e.key_code);
          set_bit(f.key.scan_downs, (usize) e.scan_code);
          f.key.modifier_downs |= e.modifiers;
        }
        break;
        case KeyAction::Release:
        {
          f.key.any_up = true;
          set_bit(f.key.key_ups, (usize) e.key_code);
          set_bit(f.key.scan_ups, (usize) e.scan_code);
          f.key.modifier_ups |= e.modifiers;
        }
        break;
        default:
          break;
      }
    },
    [&](MouseMotionEvent e) {
      f.mouse.moved    = true;
      f.mouse.position = e.position;
      f.mouse.translation += e.translation;
    },
    [&](MouseClickEvent e) {
      f.mouse.num_clicks[(u32) e.button] = e.clicks;
      f.mouse.position                   = e.position;
      switch (e.action)
      {
        case KeyAction::Press:
          f.mouse.downs |= MouseButtons{1U << (u32) e.button};
          f.mouse.any_down = true;
          break;
        case KeyAction::Release:
          f.mouse.ups |= MouseButtons{1U << (u32) e.button};
          f.mouse.any_up = true;
          break;
        default:
          break;
      }
    },
    [&](MouseWheelEvent e) {
      f.mouse.wheel_scrolled = true;
      f.mouse.position       = e.position;
      f.mouse.translation += e.translation;
    },
    [&](TextInputEvent e) {
      f.text_input = true;
      f.text.extend(e.text).unwrap();
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
          f.resized = true;
          break;
        case WindowEventType::SurfaceResized:
          f.surface_resized = true;
          break;
        case WindowEventType::Minimized:
        case WindowEventType::Maximized:
        case WindowEventType::Restored:
          break;
        case WindowEventType::MouseEnter:
          f.mouse.in      = true;
          f.mouse_focused = true;
          break;
        case WindowEventType::MouseLeave:
          f.mouse.out     = true;
          f.mouse_focused = false;
          break;
        case WindowEventType::KeyboardFocusIn:
          f.key.in      = true;
          f.key_focused = true;
          break;
        case WindowEventType::KeyboardFocusOut:
          f.key.out     = true;
          f.key_focused = false;
          break;
        case WindowEventType::CloseRequested:
          f.close_requested = true;
          f.closing         = true;
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
              f.dropped = true;
              break;
            default:
              break;
          }
        },
        [&](DropPositionEvent e) {
          f.drop_hovering  = true;
          f.mouse.position = e.pos;
        },
        [&](DropFileEvent e) {
          f.drop_data.clear();
          f.drop_data.extend(e.path.as_u8()).unwrap();
          f.drop_type = DropType::FilePath;
        },
        [&](DropTextEvent e) {
          f.drop_data.clear();
          f.drop_data.extend(e.text.as_u8()).unwrap();
          f.drop_type = DropType::Bytes;
        });
    });
}

Dyn<Engine *> Engine::create(AllocatorRef allocator, Str config_path,
                             Str working_dir)
{
  Dyn<Logger *> logger =
    dyn<Logger>(inplace, default_allocator, span<LogSink *>({&stdio_sink}))
      .unwrap();
  hook_logger(logger);
  trace("Initializing Engine, config_path: {} and working dir: {} ",
        config_path, working_dir);

  trace("Loading Engine config file");

  Vec<u8> json{allocator};

  read_file(config_path, json).unwrap("Error opening config file");

  EngineCfg cfg = EngineCfg::parse(allocator, json);

  trace("Initializing Core Systems");

  FileSystem file_sys{allocator};

  Dyn<gpu::Instance *> instance =
    gpu::create_vulkan_instance(allocator, cfg.gpu.validation).unwrap();

  gpu::Device * device =
    instance->create_device(allocator, cfg.gpu.preferences, 2).unwrap();

  trace("Loading Pipeline cache from disk");

  Vec<u8> pipeline_cache{allocator};
  read_file(cfg.pipeline_cache, pipeline_cache)
    .match([](Void) {},
           [](IoErr err) {
             CHECK(err == IoErr::InvalidFileOrDir, "Io Error Occured");
           });

  GpuSystem gpu_sys = GpuSystem::create(
    allocator, *device, pipeline_cache, cfg.gpu.hdr, cfg.gpu.buffering,
    cfg.gpu.msaa_level, Vec2U{cfg.window.width, cfg.window.height});

  ImageSystem image_sys{allocator};

  Dyn<FontSystem *> font_sys = FontSystem::create(allocator);

  ShaderSystem shader_sys{allocator};

  Dyn<WindowSystem *> window_sys = WindowSystem::create_SDL(allocator);

  trace("Creating Root Window");

  Window window = window_sys->create_window(*instance, "Ashura"_str).unwrap();

  if (cfg.window.maximized)
  {
    window_sys->maximize(window);
  }
  else
  {
    window_sys->set_extent(window, Vec2U{cfg.window.width, cfg.window.height});
  }

  if (cfg.window.full_screen)
  {
    window_sys->make_fullscreen(window);
  }
  else
  {
    window_sys->make_windowed(window);
  }

  if (cfg.window.borderless)
  {
    window_sys->make_borderless(window);
  }
  else
  {
    window_sys->make_bordered(window);
  }

  if (cfg.window.resizable)
  {
    window_sys->make_resizable(window);
  }
  else
  {
    window_sys->make_unresizable(window);
  }

  ClipBoard & clipboard = window_sys->get_clipboard();

  gpu::Surface surface = window_sys->get_surface(window);

  gpu::PresentMode present_mode =
    cfg.gpu.vsync ? gpu::PresentMode::Fifo : gpu::PresentMode::Immediate;

  Renderer renderer = Renderer::create(allocator);

  Canvas canvas{allocator};

  ViewSystem view_sys{allocator};

  Vec<char> working_dir_copy = vec(allocator, working_dir).unwrap();

  u32 const hardware_concurrency = std::thread::hardware_concurrency();

  u32 const num_worker_threads = max(hardware_concurrency, 2U) - 1U;

  nanoseconds const max_thread_sleep = 5ms;

  Vec<nanoseconds> worker_thread_sleep{allocator};

  for (u32 i = 0; i < num_worker_threads; i++)
  {
    worker_thread_sleep.push(max_thread_sleep).unwrap();
  }

  Dyn<Scheduler *> scheduler = Scheduler::create(
    allocator, std::this_thread::get_id(), {}, worker_thread_sleep);

  trace("All Core Systems Initialized");

  nanoseconds min_frame_interval = 0ns;

  if (cfg.gpu.max_fps.is_some())
  {
    f64 const max_fpns = cfg.gpu.max_fps.value() * (1 / 1'000'000'000.0);
    f64 const min_frame_time_ns = 1 / max_fpns;
    min_frame_interval = nanoseconds{(nanoseconds::rep) min_frame_time_ns};
  }

  Dyn<Engine *> engine =
    dyn<Engine>(inplace, allocator, allocator, std::move(logger),
                std::move(scheduler), std::move(file_sys), std::move(instance),
                *device, std::move(gpu_sys), std::move(image_sys),
                std::move(font_sys), std::move(shader_sys),
                std::move(window_sys), window, clipboard, surface, present_mode,
                std::move(renderer), std::move(canvas), std::move(view_sys),
                std::move(working_dir_copy), std::move(cfg.pipeline_cache),
                min_frame_interval)
      .unwrap();

  hook_engine(engine);

  engine->engage_(cfg);

  return engine;
}

void Engine::engage_(EngineCfg const & cfg)
{
  window_sys->listen(fn(
    this, +[](Engine * engine, SystemEvent const & event) {
      event.match(
        [&](SystemTheme theme) {
          InputState & f = engine->input_buffer;
          f.theme        = theme;
        },
        [](SystemEventType) {});
    }));

  window_sys->listen(window, fn(this, window_event_listener));

  Vec<AnyFuture> futures{allocator};

  Vec<char> resolved_path{allocator};

  for (auto & [label, path] : cfg.shaders)
  {
    resolved_path.clear();
    path_join(working_dir, path, resolved_path).unwrap();
    trace("Loading shader: {} from : {}", label, resolved_path);
    futures.push(shader_sys.load_from_path(std::move(label), resolved_path))
      .unwrap();
  }

  for (auto & [label, path] : cfg.fonts)
  {
    resolved_path.clear();
    path_join(working_dir, path, resolved_path).unwrap();
    trace("Loading font: {} from: {}", label, resolved_path);
    futures
      .push(font_sys->load_from_path(std::move(label), resolved_path,
                                     cfg.font_height, 0))
      .unwrap();
  }

  for (auto & [label, path] : cfg.images)
  {
    resolved_path.clear();
    path_join(working_dir, path, resolved_path).unwrap();
    trace("Loading image: {}  from: {}", label, resolved_path);
    futures.push(image_sys.load_from_path(std::move(label), resolved_path))
      .unwrap();
  }

  trace("Waiting for resources");
  while (!await_futures(futures, 0ns))
  {
    gpu_sys.begin_frame(nullptr);
    scheduler->run_main_loop(1ms, 1ms);
    gpu_sys.submit_frame(nullptr);
  }

  trace("All resources loaded");

  renderer.acquire();
}

void Engine::shutdown()
{
  trace("Shutting down engine");

  scheduler->shutdown();

  device->wait_idle().unwrap();

  renderer.release();
  device->uninit(swapchain);
  swapchain = nullptr;

  window_sys->uninit_window(window);
  window = nullptr;
  window_sys->shutdown();

  shader_sys.shutdown();
  font_sys->shutdown();
  image_sys.shutdown();

  Vec<u8> pipeline_cache{allocator};

  gpu_sys.shutdown(pipeline_cache);

  if (!pipeline_cache.is_empty())
  {
    write_to_file(pipeline_cache_path, pipeline_cache, false)
      .match(
        [&](Void) {
          trace("Saved pipeline cache to: {}", pipeline_cache_path);
        },
        [&](IoErr err) {
          error("Error {} writing pipeline cache to {}", err,
                pipeline_cache_path);
        });
  }

  canvas.reset();

  window_sys->shutdown();
  shader_sys.shutdown();
  font_sys->shutdown();
  image_sys.shutdown();

  instance->uninit(device.ptr());

  trace("Engine Uninitialized");
}

void Engine::recreate_swapchain_()
{
  gpu::SurfaceCapabilities capabilities =
    device->get_surface_capabilities(surface).unwrap();
  CHECK(has_bits(capabilities.image_usage, gpu::ImageUsage::TransferDst |
                                             gpu::ImageUsage::ColorAttachment),
        "");

  Vec<gpu::SurfaceFormat> formats{allocator};
  device->get_surface_formats(surface, formats).unwrap();

  Vec<gpu::PresentMode> present_modes{allocator};
  device->get_surface_present_modes(surface, present_modes).unwrap();

  Vec2U surface_extent = window_sys->get_surface_extent(window);
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

  CHECK(found_format, "");

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

  CHECK(found_present_mode, "");

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
                          .preferred_buffering = gpu_sys.buffering_,
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

void Engine::run(ui::View & view, ui::View & focus_view,
                 Fn<void(InputState const &)> loop)
{
  trace("Starting Engine Run Loop");

  if (swapchain == nullptr)
  {
    recreate_swapchain_();
  }

  time_point timestamp       = steady_clock::now();
  bool       should_close    = false;
  Cursor     cursor          = Cursor::Default;
  bool       text_input_mode = false;
  window_sys->set_cursor(cursor);

  while (!should_close)
  {
    ScopeTrace frame_trace{{"frame"_str}};
    auto const frame_start = steady_clock::now();
    auto const timedelta   = frame_start - timestamp;
    timestamp              = frame_start;

    input_buffer.clear();
    input_buffer.stamp(timestamp, timedelta);

    window_sys->get_mouse_state(input_buffer.mouse.states,
                                input_buffer.mouse.position);
    window_sys->get_keyboard_state(input_buffer.key.scan_states,
                                   input_buffer.key.key_states,
                                   input_buffer.key.modifier_states);

    {
      ScopeTrace poll_trace{
        {"frame.event_poll"_str, 0}
      };
      window_sys->poll_events();
    }

    Vec2U const surface_extent = window_sys->get_surface_extent(window);
    Vec2U const window_extent  = window_sys->get_extent(window);

    input_buffer.window_extent  = window_extent;
    input_buffer.surface_extent = surface_extent;

    if (input_buffer.resized || input_buffer.surface_resized)
    {
      gpu_sys.recreate_framebuffers(surface_extent);
    }

    ScopeTrace record_trace{{"frame.record"_str}};
    gpu_sys.begin_frame(swapchain);

    canvas.begin_recording(
      gpu::Viewport{
        .offset{0, 0},
        .extent    = as_vec2(surface_extent),
        .min_depth = 0,
        .max_depth = 1
    },
      as_vec2(window_extent), surface_extent);

    should_close = !view_sys.tick(input_buffer, view, focus_view, canvas, loop);

    if (view_sys.cursor() != cursor)
    {
      window_sys->set_cursor(cursor);
    }

    auto input_info = view_sys.text_input();

    if (input_info.is_some() && !text_input_mode)
    {
      window_sys->start_text_input(window, input_info.value());
      text_input_mode = true;
    }
    else if (input_info.is_none() && text_input_mode)
    {
      window_sys->end_text_input(window);
      text_input_mode = false;
    }

    canvas.end_recording();

    renderer.render_canvas(gpu_sys.fb_, canvas);
    gpu_sys.submit_frame(swapchain);

    auto const frame_end  = steady_clock::now();
    auto const frame_time = frame_end - frame_start;

    if (frame_time < min_frame_interval)
    {
      auto const sleep_dur = min_frame_interval - frame_time;
      std::this_thread::sleep_for(sleep_dur);
    }
  }

  trace("Ended Engine Run Loop");
}

Storage<Systems> systems_storage;

void hook_engine(Engine * instance)
{
  if (instance == nullptr)
  {
    ash::logger    = nullptr;
    ash::engine    = nullptr;
    ash::scheduler = nullptr;
    ash::sys       = nullptr;
    return;
  }

  ash::logger    = instance->logger;
  ash::engine    = instance;
  ash::scheduler = instance->scheduler;
  ash::sys = new (systems_storage.storage_) Systems{instance->get_systems()};
}

}    // namespace ash

ash::Engine * ::ash::engine = nullptr;
