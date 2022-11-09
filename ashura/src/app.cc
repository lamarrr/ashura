#include "ashura/app.h"

#include "ashura/vulkan.h"
#include "ashura/window.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "stx/vec.h"

namespace asr {

// TODO(lamarrr): handle should_quit
void App::tick(std::chrono::nanoseconds interval) {
  /*
  auto frame_budget = frequency_to_period(present_refresh_rate_hz);
  auto begin = std::chrono::steady_clock::now();
  auto total_used = std::chrono::steady_clock::duration(0);

  // TODO(lamarrr): add actual tick
  BackingStoreDiff backing_store_diff = BackingStoreDiff::None;

  {
    // ASR_TRACE(trace_context, "Swapchain", "Pipeline Tick");
    backing_store_diff = pipeline->tick({});
  }

  // only try to present if the pipeline has new changes or window was
  // resized
  if (backing_store_diff != BackingStoreDiff::None || window_extent_changed) {
    // TODO(lamarrr): make presentation happen after recreation, for the first
    // iteration. and remove the created swapchain in the init method

    // only try to recreate swapchain if the present swapchain can't be used for
    // presentation
    if (window_extent_changed) {
      // ASR_TRACE(trace_context, "Swapchain", "Recreation");
      window->handle.handle->recreate_swapchain(vk_render_context);
    }

    // TODO(lamarrr): we don't need another backing store on the pipeline side
    WindowSwapchainDiff swapchain_diff =
        window->handle.handle->present_backing_store(
            pipeline->tile_cache.backing_store_cache.get_surface_ref());

    while (swapchain_diff != WindowSwapchainDiff::None) {
      {
        // ASR_TRACE(trace_context, "Swapchain", "Recreation");
        window->handle.handle->recreate_swapchain(vk_render_context);
      }

      {
        //   ASR_TRACE(trace_context, "Swapchain", "Presentation");
        swapchain_diff = window->handle.handle->present_backing_store(
            pipeline->tile_cache.backing_store_cache.get_surface_ref());
      }
    }

    // we need to update viewport in case it changed
    pipeline->viewport.resize(
        window->handle.handle->extent,
        pipeline->viewport.get_unresolved_widgets_allocation());

    // resize event could have been caused by a display or DPR-setting change.
    // we thus need to update the DPR accordingly
    pipeline->tile_cache.update_dpr(dpr_from_extents(
        window->handle.handle->extent, window->handle.handle->surface_extent));

    SDL_DisplayMode display_mode{};
    ASR_SDL_ENSURE(SDL_GetWindowDisplayMode(window->handle.handle->window,
                                            &display_mode) == 0,
                   "Unable to get window display mode");

    // TODO(lamarrr): log refresh rate
    present_refresh_rate_hz = static_cast<uint32_t>(display_mode.refresh_rate);

    window_extent_changed = false;

    auto render_end = std::chrono::steady_clock::now();

    total_used = render_end - begin;
    // we can save power on minimized and not have to update the buffer
  }

  // poll events to make the window not be marked as unresponsive.
  // we also poll events from SDL's event queue until there are none left.
  //
  // any missed event should be rolled over to the next tick()
  do {
  } while (window_api->poll_events());

  total_used = std::chrono::steady_clock::now() - begin;

  if (total_used < frame_budget) {
    std::this_thread::sleep_for(frame_budget - total_used);
  }

  pipeline->dispatch_events(
      window->handle.handle->event_queue.mouse_button_events,
      window->handle.handle->event_queue.window_events);

  window_extent_changed =
      any_eq(window->handle.handle->event_queue.window_events,
             WindowEvent::SizeChanged);

  if (any_eq(window->handle.handle->event_queue.window_events,
             WindowEvent::Close)) {
    std::exit(0);
  }

  window->handle.handle->event_queue.clear();
  */
}

}  // namespace asr
