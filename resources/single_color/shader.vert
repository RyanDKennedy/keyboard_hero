#version 450

layout (location = 0) in vec3 a_in_position;

layout (location = 0) out vec3 v_pos;

layout (set = 0, binding = 0) uniform Camera
{
    mat4 vp;
} u_camera;

layout (set = 2, binding = 0) uniform MeshInfo
{
    mat4 model;
} u_mesh_info;
    

void main()
{
    v_pos = a_in_position;
    gl_Position = u_camera.vp * u_mesh_info.model * vec4(a_in_position.xyz, 1.0);
}
