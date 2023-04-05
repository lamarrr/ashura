#pragma once

#include <map>
#include <string>
#include <string_view>

#include "ashura/plugin.h"
#include "ashura/primitives.h"
#include "fmt/format.h"
#include "spdlog/logger.h"
#include "stx/option.h"
#include "stx/scheduler.h"

namespace ash
{

struct WindowManager;
struct ClipBoard;

// add window controller class
// not thread-safe! ensure all api calls occur on the main thread.
struct Context
{
  stx::Vec<Plugin *>  plugins{stx::os_allocator};
  stx::TaskScheduler *task_scheduler = nullptr;
  ClipBoard          *clipboard      = nullptr;
  // TODO(lamarrr): expose current window here

  Context()
  {}

  STX_MAKE_PINNED(Context)

  ~Context()
  {
    for (Plugin *plugin : plugins)
    {
      delete plugin;
    }
  }

  void register_plugin(Plugin *plugin)
  {
    plugins.push_inplace(plugin).unwrap();
  }

  template <typename T>
  stx::Option<T *> get_plugin(std::string_view name) const
  {
    stx::Span plugin = plugins.span().which([name](Plugin *plugin) { return plugin->get_name() == name; });
    if (!plugin.is_empty())
    {
      return stx::Some(plugin[0]->as<T>());
    }
    else
    {
      return stx::None;
    }
  }
};

}        // namespace ash
