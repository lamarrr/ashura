



layout(location = 0) in vec3 in_position;
layout(location = 0) out vec3 out_position;

layout(set = 0, binding = 0) uniform global_uniform_object{
    mat4 projection;
    mat4 view;
    mat4 model;
} global_ubo;

uniform (binding = 0, set = 1) vec3 color;


void main(){
    gl_Position = vec4()
}