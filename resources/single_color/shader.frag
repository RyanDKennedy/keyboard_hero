#version 450

layout (location = 0) out vec4 out_color;

layout (binding = 0) uniform FrameDataUniform {
    vec3 color;
} u_frame_data;

void main()
{
    out_color = vec4(u_frame_data.color.rgb, 1.0);
}
