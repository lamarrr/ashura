#include <tuple>
#include <utility>

#include "vlk/scheduler.h"

namespace vlk {
namespace sched {

template <typename Input, typename Func, typename FirstOutput,
          typename... Outputs>
std::tuple<Future<FirstOutput>, Future<Outputs>...> fork(
    Future<Input>&& input, Func&& task, TaskPriority priority,
    TaskTraceInfo trace_info);

}
}  // namespace vlk
