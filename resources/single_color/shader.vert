#version 450

layout (location = 0) in vec3 a_in_position;

layout (location = 0) out vec3 v_pos;

layout (set = 0, binding = 0) uniform Camera
{
    mat4 vp;
} u_camera;


void main()
{
    v_pos = a_in_position;
    gl_Position = u_camera.vp * vec4(a_in_position.xyz, 1.0);
}
