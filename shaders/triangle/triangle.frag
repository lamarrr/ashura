#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec4 ifragment_color;

layout(location = 0) out vec4 ofragment_color;

void main(){
    ofragment_color = vecifragment_color
}