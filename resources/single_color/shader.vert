#version 450

layout (location = 0) in vec2 a_in_position;

layout (set = 0, binding = 0) uniform FrameDataUniform {
    mat4 vp_matrix;
} u_frame_data;

void main()
{
    gl_Position = u_frame_data.vp_matrix * vec4(a_in_position, 0.0, 1.0);
}
