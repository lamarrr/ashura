#pragma once

#include "ashura/primitives.h"

namespace ash
{

struct FrameStats
{
  nanoseconds gpu_time{0};
  nanoseconds cpu_time{0};
  nanoseconds gpu_sync_time{0};
  u64         input_assembly_vertices     = 0;
  u64         input_assembly_primitives   = 0;
  u64         vertex_shader_invocations   = 0;
  u64         fragment_shader_invocations = 0;
  u64         compute_shader_invocations  = 0;
  u64         task_shader_invocations     = 0;
  u64         mesh_shader_invocations     = 0;
};

}        // namespace ash
