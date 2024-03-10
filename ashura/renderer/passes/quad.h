
// TODO(lamarrr): rename to custom shaded object
// pipelines are compiled at startup time
struct QuadProperties
{
  int pipeline, mesh, descriptor_set, first_instance, num_instances,
      first_vertex, vertex_offset, vertex_buffer, index_buffer, uniform_data;
};

// pipeline bundle (pipeline + vs shader + fs shader)
// mvp