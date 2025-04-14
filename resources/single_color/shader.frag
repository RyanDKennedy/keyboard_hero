#version 450

layout (location = 0) out vec4 out_color;

layout (location = 0) in vec3 v_pos;

layout (set = 1, binding = 0) uniform Material
{
    vec3 diffuse;
} u_material;

void main()
{
    out_color = vec4(u_material.diffuse.xyz, 1.0);
}
