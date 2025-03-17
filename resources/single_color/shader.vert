#version 450 core
#extension GL_EXT_scalar_block_layout : enable

layout (location = 0) in vec2 a_in_position;

layout (location = 0) out vec3 v_frag_color;

layout (set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} u_ubo;

void main()
{
    gl_Position = u_ubo.proj * u_ubo.view * u_ubo.model * vec4(a_in_position, 0.0, 1.0);
    v_frag_color = vec3(1.0, 0.0, 0.0);
}
