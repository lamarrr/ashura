#include "ashura/app.h"

#include "ashura/vulkan.h"
#include "ashura/window.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "stx/vec.h"

namespace ash {

void App::tick(std::chrono::nanoseconds interval) { engine.tick(interval); }

}  // namespace ash
