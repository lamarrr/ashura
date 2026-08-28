/// SPDX-License-Identifier: MIT
#include "ashura/engine/engine.hpp"
#include "ashura/engine/animation_system.hpp"
#include "ashura/engine/audio_system.hpp"
#include "ashura/engine/file_system.hpp"
#include "ashura/engine/font_system.hpp"
#include "ashura/engine/gpu_system.hpp"
#include "ashura/engine/image_system.hpp"
#include "ashura/engine/pipeline_system.hpp"
#include "ashura/engine/shader_system.hpp"
#include "ashura/engine/systems.hpp"
#include "ashura/engine/video_system.hpp"
#include "ashura/engine/view_system.hpp"
#include "ashura/engine/window_system.hpp"
#include "ashura/std/async.hpp"
#include "ashura/std/fs.hpp"
#include "ashura/std/log_sinks.hpp"
#include "ashura/std/sformat.hpp"
#include "ashura/std/trace.hpp"
#include "simdjson.h"
#include <filesystem>

namespace ash
{

Result<EngineCfg> EngineCfg::parse_json(Span<u8 const> json, Allocator allocator)
{
    ASH_TRACE_SCOPE;

    ThreadScratchScope scratch;

    EngineCfg out{.window{.title{allocator}},
                  .font_paths{allocator},
                  .image_paths{allocator},
                  .working_dir_path{allocator},
                  .pipeline_cache_path{allocator}};

    Vec<u8> json_copy{scratch};
    if (!(json_copy.reserve_extend(json.size() + simdjson::SIMDJSON_PADDING) &&
          json_copy.append(json)))
    {
        return Err{};
    }

    simdjson::ondemand::parser parser;

    auto error =
      parser.iterate(json_copy.data(), json_copy.size_bytes(), json_copy.capacity());

    if (error.error() != simdjson::SUCCESS)
    {
        return Err{};
    }

    auto & doc = error.value();

    // TODO: check valid schema
    auto cfg = doc.get_object().value();

    std::string_view version = cfg["version"].get_string().value();
    ASH_CHECK(version == "0.0.1", "");

    out.gpu.validation = cfg["gpu.validation"].get_bool().value();

    auto gpu_prefs = cfg["gpu.preferences"].get_array().value();
    ASH_CHECK(gpu_prefs.count_elements().value() <= 5, "");

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
            ASH_CHECK(false, "");
        }
    }

    out.gpu.hdr = cfg["gpu.hdr"].get_bool().value();
    out.gpu.buffering =
      (u32) clamp(cfg["gpu.buffering"].get_int64().value(), (i64) 1, (i64) 4);

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

    auto title = cfg["window.title"].get_string().value();
    out.window.title.append(span(title)).unwrap();
    out.window.resizable   = cfg["window.resizable"].get_bool().value();
    out.window.maximized   = cfg["window.maximized"].get_bool().value();
    out.window.full_screen = cfg["window.full_screen"].get_bool().value();
    out.window.borderless  = cfg["window.borderless"].get_bool().value();
    out.window.vsync       = cfg["window.vsync"].get_bool().value();
    out.window.width =
      (u32) clamp(cfg["window.width"].get_int64().value(), (i64) 0, (i64) U32_MAX);
    out.window.height =
      (u32) clamp(cfg["window.height"].get_int64().value(), (i64) 0, (i64) U32_MAX);

    out.font_height = clamp((u32) cfg["fonts.height"].get_int64().value(), 16U, 256U);

    std::string_view pipeline_cache_path =
      cfg["paths.pipeline_cache"].get_string().value();

    out.pipeline_cache_path.append(pipeline_cache_path).unwrap();

    std::string_view working_dir_path = cfg["paths.working_dir"].get_string().value();

    out.working_dir_path.append(working_dir_path).unwrap();

    return Ok{std::move(out)};
}

static void system_event_listener(Engine engine, SystemEvent const & event)
{
    event.match(
      [&](SystemTheme theme) {
          SystemState & s   = engine->state_;
          s.theme_.theme_   = theme;
          s.theme_.changed_ = true;
      },
      [](SystemEventType) {});
}

static void window_event_listener(IEngine::WindowEntry * win, WindowEvent const & event)
{
    auto & s = win->state_;

    event.match(
      [&](KeyEvent e) {
          switch (e.action)
          {
              case KeyAction::Press:
              {
                  s.key_.any_down_ = true;
                  s.key_.key_downs_.set_bit((usize) e.key_code);
                  s.key_.scan_downs_.set_bit((usize) e.scan_code);
                  s.key_.mod_downs_ |= e.modifiers;
              }
              break;
              case KeyAction::Release:
              {
                  s.key_.any_up_ = true;
                  s.key_.key_ups_.set_bit((usize) e.key_code);
                  s.key_.scan_ups_.set_bit((usize) e.scan_code);
                  s.key_.mod_ups_ |= e.modifiers;
              }
              break;
              default:
                  break;
          }
      },
      [&](MouseMotionEvent e) {
          s.mouse_.moved_       = true;
          s.mouse_.position_    = e.position;
          s.mouse_.translation_ = e.translation;
      },
      [&](MouseClickEvent e) {
          s.mouse_.num_clicks_[(u32) e.button] = e.clicks;
          s.mouse_.position_                   = e.position;
          switch (e.action)
          {
              case KeyAction::Press:
                  s.mouse_.downs_ |= static_cast<MouseButtons>(1U << (u32) e.button);
                  s.mouse_.any_down_ = true;
                  break;
              case KeyAction::Release:
                  s.mouse_.ups_ |= static_cast<MouseButtons>(1U << (u32) e.button);
                  s.mouse_.any_up_ = true;
                  break;
              default:
                  break;
          }
      },
      [&](MouseWheelEvent e) {
          s.mouse_.scrolled_          = true;
          s.mouse_.position_          = e.position;
          s.mouse_.wheel_translation_ = e.translation;
      },
      [&](TextInputEvent e) {
          s.key_.input_ = true;
          s.key_.text_  = vec::copy(s.key_.allocator_, e.text).unwrap();
      },
      [&](WindowEventType e) {
          switch (e)
          {
              case WindowEventType::Shown:
                  s.shown_ = true;
                  break;
              case WindowEventType::Hidden:
                  s.hidden_ = true;
                  break;
              case WindowEventType::Exposed:
                  s.exposed_ = true;
                  break;
              case WindowEventType::Moved:
                  s.moved_ = true;
                  break;
              case WindowEventType::Resized:
                  s.resized_ = true;
                  break;
              case WindowEventType::SurfaceResized:
                  s.surface_resized_ = true;
                  break;
              case WindowEventType::Minimized:
                  s.minimized_ = true;
                  break;
              case WindowEventType::Maximized:
                  s.maximized_ = true;
                  break;
              case WindowEventType::Restored:
                  s.restored_ = true;
                  break;
              case WindowEventType::MouseEnter:
                  s.mouse_.in_ = true;
                  break;
              case WindowEventType::MouseLeave:
                  s.mouse_.out_ = true;
                  break;
              case WindowEventType::KeyboardFocusIn:
                  s.key_.in_ = true;
                  break;
              case WindowEventType::KeyboardFocusOut:
                  s.key_.out_ = true;
                  break;
              case WindowEventType::CloseRequested:
                  s.close_requested_ = true;
                  break;
              case WindowEventType::Occluded:
                  s.occluded_ = true;
                  break;
              case WindowEventType::EnterFullScreen:
                  s.enter_fullscreen_ = true;
                  break;
              case WindowEventType::LeaveFullScreen:
                  s.leave_fullscreen_ = true;
                  break;
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
                        s.drop_.event_ = DropState::Event::Begin;
                        break;
                    case DropEventType::DropComplete:
                        s.drop_.event_ = DropState::Event::Complete;
                        break;
                    default:
                        break;
                }
            },
            [&](DropPositionEvent e) { s.mouse_.position_ = e.pos; },
            [&](DropFileEvent e) {
                s.drop_.data_ = DropState::DropFilePath{
                  .path = vec::copy(s.drop_.allocator_, e.path).unwrap()};
                s.drop_.event_ = DropState::Event::Data;
            },
            [&](DropTextEvent e) {
                s.drop_.data_ = DropState::DropText{
                  .text = vec::copy(s.drop_.allocator_, e.text).unwrap()};
                s.drop_.event_ = DropState::Event::Data;
            });
      });
}

IEngine::~IEngine()
{
    shutdown_();
}

Dyn<Engine> IEngine::create(Allocator allocator, EngineCfg const & cfg,
                            Callbacks callbacks)
{
    ASH_TRACE_SCOPE;

    trace("Initializing Engine Core Systems"_s);
    trace("Loading Graphics Pipeline Cache From {}"_s, cfg.pipeline_cache_path.view());

    std::filesystem::path path{
      std::string_view{cfg.pipeline_cache_path.data(), cfg.pipeline_cache_path.size()}
    };
    std::filesystem::create_directories(path.parent_path());
    Vec<u8> pipeline_cache{allocator};
    read_file(cfg.pipeline_cache_path, pipeline_cache, allocator)
      .match([](Void) {},
             [](IoErr err) {
                 ASH_CHECK(err == IoErr::InvalidFileOrDir, "Io Error Occured");
             });

    constexpr Str const dedicated_thread_names[] = {
      "Gpu Thread"_s,
      "Audio Thread"_s,
      "Video Thread"_s,
    };
    constexpr DedicatedThread                  gpu_thread   = DedicatedThread{0};
    [[maybe_unused]] constexpr DedicatedThread audio_thread = DedicatedThread{1};
    [[maybe_unused]] constexpr DedicatedThread video_thread = DedicatedThread{2};

    auto num_dedicated_threads =
      size32(dedicated_thread_names);    // gpu/render, audio, video,
    auto hardware_concurrency = std::thread::hardware_concurrency();
    auto min_worker_threads   = 1U;
    auto total_concurrency    = max(num_dedicated_threads + min_worker_threads +
                                      // main thread
                                      1,
                                    hardware_concurrency);
    auto num_worker_threads   = total_concurrency - (num_dedicated_threads + 1);

    Vec<SchedulerThreadInfo> dedicated_thread_infos{allocator};

    for (auto [i, name] : enumerate(dedicated_thread_names))
    {
        dedicated_thread_infos.push(SchedulerThreadInfo{.name = name}).unwrap();
    }

    Vec<StrVec>              worker_thread_names{allocator};
    Vec<SchedulerThreadInfo> worker_thread_infos{allocator};

    for (auto i : range(num_worker_threads))
    {
        worker_thread_names.push(sformat(allocator, "Worker Thread {}"_s, i).unwrap())
          .unwrap();
        worker_thread_infos
          .push(SchedulerThreadInfo{.name = worker_thread_names.last()})
          .unwrap();
    }

    initialize_scheduler(SchedulerInfo{.allocator         = allocator,
                                       .dedicated_threads = dedicated_thread_infos,
                                       .worker_threads    = worker_thread_infos,
                                       .main_thread_id = std::this_thread::get_id()});

    Dyn<FileSys> file_sys = dyn<IFileSys>(inplace, allocator, allocator).unwrap();

    ash::sys.file = file_sys.get();

    Dyn<WindowSys> window_sys = IWindowSys::create_SDL(allocator);

    ash::sys.win = window_sys.get();

    Dyn<gpu::Instance> gpu_instance =
      gpu::create_vulkan_instance(allocator, cfg.gpu.validation).unwrap();

    gpu::Device gpu_device =
      gpu_instance->create_device(allocator, cfg.gpu.preferences).unwrap();

    Dyn<GpuSys> gpu_sys = dyn<IGpuSys>(inplace, allocator).unwrap();

    ash::sys.gpu = gpu_sys.get();

    auto gpu_pref =
      GpuSysPreferences{.buffering             = cfg.gpu.buffering,
                        .cfg                   = GpuSysCfg{},
                        .color_formats         = ColorImage::SDR_FORMATS,
                        .depth_stencil_formats = DepthStencilImage::FORMATS};

    gpu_sys->init(allocator, gpu_device, pipeline_cache.view(), gpu_pref, gpu_thread);

    Dyn<ImageSys> image_sys =
      dyn<IImageSys>(inplace, allocator, allocator, gpu_sys.get(), file_sys.get())
        .unwrap();

    ash::sys.image = image_sys.get();

    Dyn<FontSys> font_sys =
      IFontSys::create(allocator, file_sys.get(), image_sys.get());

    auto font_fut = font_sys->init();

    {
        auto * plan = ash::sys.gpu->current_plan();
        plan->await();
        plan->reset();
        plan->begin();
        while (!font_fut())
        {
            scheduler().run_main_loop(10ms, 10ms);
        }
        plan->end();
        ash::sys.gpu->submit_frame();
    }

    ash::sys.font = font_sys.get();

    Dyn<ShaderSys> shader_sys =
      dyn<IShaderSys>(inplace, allocator, gpu_sys.get(), file_sys.get()).unwrap();

    auto shader_fut = shader_sys->init(allocator);

    {
        auto * plan = ash::sys.gpu->current_plan();
        plan->await();
        plan->reset();
        plan->begin();
        while (!shader_fut())
        {
            scheduler().run_main_loop(10ms, 10ms);
        }
        plan->end();
        ash::sys.gpu->submit_frame();
    }

    ash::sys.shader = shader_sys.get();

    Dyn<PipelineSys> pipeline_sys =
      dyn<IPipelineSys>(inplace, allocator, gpu_sys.get()).unwrap();

    ash::sys.pipeline = pipeline_sys.get();

    pipeline_sys->init(allocator);

    trace("All Core Systems Initialized"_s);

    Dyn<Engine> engine = dyn<IEngine>(inplace, allocator).unwrap();

    engine->allocator_ = allocator;
    engine->sys_       = Systems{
      .file     = std::move(file_sys),
      .gpu      = std::move(gpu_sys),
      .image    = std::move(image_sys),
      .font     = std::move(font_sys),
      .shader   = std::move(shader_sys),
      .win      = std::move(window_sys),
      .pipeline = std::move(pipeline_sys),
      .audio{nullptr, dyn_noop},
      .video{nullptr, dyn_noop},
      .animation{nullptr, dyn_noop}
    };
    engine->gpu_instance_ = std::move(gpu_instance);
    engine->gpu_device_   = std::move(gpu_device);
    engine->buffering_    = cfg.gpu.buffering;
    engine->state_        = SystemState{};
    engine->paths_        = Paths{
      .working_dir    = vec::copy(allocator, cfg.working_dir_path.view()).unwrap(),
      .pipeline_cache = vec::copy(allocator, cfg.pipeline_cache_path.view()).unwrap()};
    engine->callbacks_ = std::move(callbacks);

    hook_engine(engine.get());

    engine->sys_.win->listen({engine.get(), system_event_listener});
    trace("Creating Root Window"_s);

    engine->window_ = engine->attach_window_(cfg.window);

    return engine;
}

Dyn<IEngine::WindowEntry *> IEngine::attach_window_(EngineCfg::Window const & cfg)
{
    ASH_TRACE_SCOPE;

    auto entry = dyn<WindowEntry>(inplace, allocator_, *this, allocator_).unwrap();

    auto window = &sys_.win->create_window(gpu_instance_.get(), cfg.title).unwrap();

    entry->win_     = window;
    entry->surface_ = sys_.win->get_surface(window);

    ui::UserDataMap ui_data{allocator_};

    entry->view_sys_ =
      dyn<IViewSys>(inplace, allocator_, allocator_, std::move(ui_data)).unwrap();

    auto loop =
      dyn_lambda<WindowLoop>(allocator_, [&](Engine, ui::Scope const &) -> ui::View & {
          static ui::View view;
          return view;
      }).unwrap();

    entry->loop_ = std::move(loop);
    entry->present_mode_preference_ =
      cfg.vsync ? gpu::PresentMode::Fifo : gpu::PresentMode::Immediate;

    auto config_window = [&] {
        sys_.win->listen(window, {entry.get(), window_event_listener});

        if (cfg.maximized)
        {
            sys_.win->maximize(window);
        }
        else
        {
            sys_.win->set_extent(window, u32x2{cfg.width, cfg.height});
        }

        if (cfg.full_screen)
        {
            sys_.win->make_fullscreen(window);
        }
        else
        {
            sys_.win->make_windowed(window);
        }

        if (cfg.borderless)
        {
            sys_.win->make_borderless(window);
        }
        else
        {
            sys_.win->make_bordered(window);
        }

        if (cfg.resizable)
        {
            sys_.win->make_resizable(window);
        }
        else
        {
            sys_.win->make_unresizable(window);
        }

        sys_.win->set_cursor(Cursor::Default);
    };

    config_window();

    return entry;
}

void IEngine::poll_inputs_(time_point prev_frame_end, time_point frame_start)
{
    ASH_TRACE_SCOPE;

    auto timedelta = frame_start - prev_frame_end;

    state_.start_frame(frame_start, timedelta);

    {
        window_->state_.start_frame();
    }

    state_.theme_.theme_ = sys_.win->get_theme();
    sys_.win->poll_events();

    auto [mouse_btns, mouse_pos, mouse_window] = sys_.win->get_mouse_state();

    Bits<u64, NUM_KEY_CODES>  key_states;
    Bits<u64, NUM_SCAN_CODES> scan_states;

    auto [kb_mods, kb_window] =
      sys_.win->get_keyboard_state(scan_states.view(), key_states.view());

    {
        auto & s          = window_->state_;
        s.extent_         = sys_.win->get_extent(window_->win_);
        s.surface_extent_ = sys_.win->get_surface_extent(window_->win_);

        if (mouse_window.ptr() == window_->win_)
        {
            s.mouse_.focused_  = true;
            s.mouse_.position_ = mouse_pos - 0.5F * static_cast<f32x2>(s.extent_);
            s.mouse_.states_   = mouse_btns;
        }

        if (kb_window.ptr() == window_->win_)
        {
            s.key_.focused_     = true;
            s.key_.mod_states_  = kb_mods;
            s.key_.scan_states_ = scan_states;
            s.key_.key_states_  = key_states;
        }
    }
}

void IEngine::shutdown_()
{
    ASH_TRACE_SCOPE;

    trace("Shutting down engine"_s);
    callbacks_.pre_shutdown(this);

    gpu_device_->await_idle().unwrap();

    window_->swapchain_.match([&](gpu::ISwapchain & sc) { gpu_device_->uninit(&sc); },
                              []() {});
    sys_.win->uninit_window(window_->win_);

    sys_.pipeline->shutdown();
    sys_.win->shutdown();
    sys_.shader->shutdown();
    sys_.font->shutdown();
    sys_.image->shutdown();

    Vec<u8> pipeline_cache{allocator_};

    sys_.gpu->shutdown(pipeline_cache);

    if (!pipeline_cache.is_empty())
    {
        write_to_file(paths_.pipeline_cache, pipeline_cache, false, allocator_)
          .match(
            [&](Void) {
                trace("Saved pipeline cache to: {}"_s, paths_.pipeline_cache);
            },
            [&](IoErr err) {
                error("Error {} writing pipeline cache to {}"_s, err,
                      paths_.pipeline_cache);
            });
    }

    sys_.file->shutdown();
    // TODO: sys_.sched->shutdown();

    gpu_instance_->uninit(gpu_device_);

    callbacks_.post_shutdown(this);

    trace("Engine Uninitialized"_s);
}

Option<gpu::SwapchainInfo> IEngine::create_swapchain_info_(WindowEntry const & w)
{
    gpu::SurfaceCapabilities capabilities =
      gpu_device_->get_surface_capabilities(w.surface_).unwrap();
    ASH_CHECK(has_bits(capabilities.image_usage,
                       gpu::ImageUsage::TransferDst | gpu::ImageUsage::ColorAttachment),
              "");

    Vec<gpu::SurfaceFormat> formats{allocator_};
    gpu_device_->get_surface_formats(w.surface_, formats).unwrap();

    Vec<gpu::PresentMode> present_modes{allocator_};
    gpu_device_->get_surface_present_modes(w.surface_, present_modes).unwrap();

    u32x2 surface_extent = sys_.win->get_surface_extent(w.win_);
    if (surface_extent.any_zero())
    {
        return none;
    }

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
      w.present_mode_preference_, gpu::PresentMode::Immediate,
      gpu::PresentMode::Mailbox, gpu::PresentMode::Fifo, gpu::PresentMode::FifoRelaxed};

    bool               found_format = false;
    gpu::SurfaceFormat format;

    for (gpu::ColorSpace cp : preferred_color_spaces)
    {
        Span sel = find_if(formats.view(),
                           [&](gpu::SurfaceFormat a) { return a.color_space == cp; });
        if (!sel.is_empty())
        {
            found_format = true;
            format       = sel[0];
            break;
        }
    }

    ASH_CHECK(found_format, "");

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

    ASH_CHECK(found_present_mode, "");

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

    return gpu::SwapchainInfo{.label               = "Window Swapchain"_s,
                              .surface             = w.surface_,
                              .format              = format,
                              .usage               = gpu::ImageUsage::TransferDst |
                                                     gpu::ImageUsage::ColorAttachment,
                              .preferred_buffering = buffering_,
                              .present_mode        = present_mode,
                              .preferred_extent    = surface_extent,
                              .composite_alpha     = alpha};
}

void IEngine::prepare_render_targets_(GpuFramePlan plan)
{
    ASH_TRACE_SCOPE;

    auto required_framebuffer_extent = window_->state_.surface_extent_;

    plan->set_target(
      GpuFrameTargetInfo{.extent               = required_framebuffer_extent,
                         .color_format         = sys.gpu->color_format(),
                         .depth_stencil_format = sys.gpu->depth_stencil_format()});

    if (window_->swapchain_.is_none())
    {
        create_swapchain_info_(*window_).match([&](gpu::SwapchainInfo info) {
            window_->swapchain_ = *gpu_device_->create_swapchain(info).unwrap();
        });
    }
    else
    {
        // if swapchain extent is 0, defer creation until first resize event
        if ((window_->state_.resized_ || window_->state_.surface_resized_) &&
            !(window_->state_.extent_.any_zero() ||
              window_->state_.surface_extent_.any_zero()))
        {
            create_swapchain_info_(*window_).match([&](gpu::SwapchainInfo info) {
                plan->add_preframe_task([dev       = plan->device(),
                                         swapchain = &window_->swapchain_.v(),
                                         info](GpuFrame) {
                    dev->mark_swapchain_out_of_date(swapchain, info).unwrap();
                });
            });
        }
    }

    plan->add_preframe_task(
      [swapchain = &window_->swapchain_.v(), dev = plan->device()](GpuFrame) {
          dev->acquire_next(swapchain).unwrap();
      });
}

ViewSysState IEngine::record_frame_(GpuFramePlan plan)
{
    ASH_TRACE_SCOPE;

    auto & w = *window_;

    // TODO: framebuffer extent would be larger than surface extent since we
    // render multiple windows into one image
    w.canvas_.begin(
      gpu::Viewport{
        .offset{0, 0},
        .extent    = w.state_.surface_extent_.to<f32>(),
        .min_depth = 0,
        .max_depth = 1
    },
      w.state_.extent_.to<f32>(), w.state_.surface_extent_);

    auto state = w.view_sys_->tick(this, ui::InputScope{state_, w.state_}, &w.canvas_,
                                   w.loop_.get());

    w.canvas_.end();
    w.canvas_.execute(plan);
    w.canvas_.reset();

    if (w.state_.extent_.all_nonzero() && w.state_.surface_extent_.all_nonzero())
    {
        // TODO: swapchain would have been destroyed at start of frame if resize happens
        plan->add_pass(
          [swapchain = &w.swapchain_.v()](GpuFrame frame, gpu::CommandEncoder enc) {
              auto * dev   = frame->dev();
              auto   state = dev->get_swapchain_state(swapchain).unwrap();

              gpu::ImageCopy const copies[] = {
                {.src_layers = {.aspects   = gpu::ImageAspects::Color,
                                .mip_level = 0,
                                .array_layers{0, 1}},
                 .src_area   = {{0, 0, 0}, state.extent.append(1)},
                 .dst_layers = {.aspects   = gpu::ImageAspects::Color,
                                .mip_level = 0,
                                .array_layers{0, 1}}}
              };

              state.current_image.match([&](u32 i) {
                  auto & image = frame->get_scratch_images()[0];
                  enc->copy_image(image.color.image, state.images[i], copies);
                  enc->present(swapchain);
              });
          });
    }

    return state;
}

Option<TextInputInfo>
  IEngine::update_window_state_(ViewSysState const &          state,
                                Option<TextInputInfo> const & current_input_info)
{
    if (window_->state_.mouse_.focused_)
    {
        if (state.cursor != window_->cursor_)
        {
            sys_.win->set_cursor(state.cursor);
            window_->cursor_ = state.cursor;
        }
    }

    auto input_info = current_input_info;
    if (window_->state_.key_.focused_)
    {
        auto text = state.input_info;
        if (text != input_info)
        {
            sys_.win->set_text_input(window_->win_, text);
            input_info = text;
        }
    }

    return input_info;
}

Option<TextInputInfo> IEngine::frame_(time_point                    previous_frame_end,
                                      time_point                    current_frame_begin,
                                      Option<TextInputInfo> const & current_input_info)
{
    ASH_TRACE_SCOPE;

    poll_inputs_(previous_frame_end, current_frame_begin);
    auto * plan = sys_.gpu->current_plan();

    // TODO: pre-frame tasks were not executed before reset
    plan->await();
    plan->reset();
    plan->begin();

    prepare_render_targets_(plan);
    auto state = record_frame_(plan);

    auto input_info = update_window_state_(state, current_input_info);

    scheduler().run_main_loop(milliseconds{10}, nanoseconds{500});

    plan->end();
    sys_.gpu->submit_frame();

    // TODO: handle, running

    return input_info;
}

void IEngine::run(Dyn<WindowLoop> loop, nanoseconds timeout)
{
    ASH_TRACE_SCOPE;

    bool                  running            = true;
    Option<TextInputInfo> current_input_info = none;
    time_point            run_begin          = steady_clock::now();
    time_point            frame_begin        = run_begin;
    time_point            frame_end          = run_begin;

    // TODO: properly handle $running
    window_->loop_ = std::move(loop);

    do
    {
        frame_begin        = steady_clock::now();
        current_input_info = frame_(frame_end, frame_begin, current_input_info);
        frame_end          = steady_clock::now();
        frame_begin        = frame_end;
    } while ((frame_end - run_begin) < timeout && running &&
             !window_->state_.close_requested_);
}

void IEngine::run(ui::View & view, nanoseconds timeout)
{
    return run(
      dyn_lambda<WindowLoop>(
        allocator_, [&](Engine, ui::Scope const &) -> ui::View & { return view; })
        .unwrap(),
      timeout);
}

void hook_engine(Engine instance)
{
    ash::engine = instance;
}

}    // namespace ash

ash::Engine ash::engine = nullptr;
