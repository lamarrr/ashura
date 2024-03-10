

struct QuadProperties
{
  int shader_fs, shader_vs;
  int first_instance, num_instances;
  int first_vertex;
  // uniform data
};

// pipeline bundle (pipeline + vs shader + fs shader)
// mvp