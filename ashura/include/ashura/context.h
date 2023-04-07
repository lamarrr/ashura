#pragma once

#include <map>
#include <string>
#include <string_view>

#include "SDL3/SDL.h"
#include "ashura/loggers.h"
#include "ashura/plugin.h"
#include "ashura/primitives.h"
#include "fmt/format.h"
#include "stx/option.h"
#include "stx/scheduler.h"

namespace ash
{

struct WindowManager;
struct ClipBoard;

enum class SystemTheme
{
  Unknown = SDL_SYSTEM_THEME_UNKNOWN,
  Light   = SDL_SYSTEM_THEME_LIGHT,
  Dark    = SDL_SYSTEM_THEME_DARK
};

// add window controller class
// not thread-safe! ensure all api calls occur on the main thread.
struct Context
{
  stx::Vec<Plugin *>  plugins{stx::os_allocator};
  stx::TaskScheduler *task_scheduler = nullptr;
  ClipBoard          *clipboard      = nullptr;
  SystemTheme         theme          = SystemTheme::Unknown;
  // TODO(lamarrr): expose current window here

  Context()
  {}

  STX_MAKE_PINNED(Context)

  ~Context()
  {
    for (Plugin *plugin : plugins)
    {
      ASH_LOG_INFO(Plugins, "Destroying plugin: {} (type: {})", plugin->get_name(), typeid(*plugin).name());
      delete plugin;
    }
  }

  void register_plugin(Plugin *plugin)
  {
    plugins.push_inplace(plugin).unwrap();
    ASH_LOG_INFO(Plugins, "Registered plugin: {} (type: {})", plugin->get_name(), typeid(*plugin).name());
  }

  template <typename T>
  stx::Option<T *> get_plugin(std::string_view name) const
  {
    stx::Span plugin = plugins.span().which([name](Plugin *plugin) { return plugin->get_name() == name; });
    if (!plugin.is_empty())
    {
      return stx::Some(plugin[0]->template as<T>());
    }
    else
    {
      return stx::None;
    }
  }

  // TODO(lamarrr): on_EXIT

  void tick(std::chrono::nanoseconds interval)
  {
    theme = AS(SystemTheme, SDL_GetSystemTheme());
  }
};

}        // namespace ash
