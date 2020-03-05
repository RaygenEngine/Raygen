#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 uv;

void main() 
{
	// 2, 1, 0
    uv = vec2(((2 - gl_VertexIndex) << 1) & 2, (2 - gl_VertexIndex) & 2);
    gl_Position = vec4(uv * 2.0f - 1.0f, 0.0f, 1.0f);
}