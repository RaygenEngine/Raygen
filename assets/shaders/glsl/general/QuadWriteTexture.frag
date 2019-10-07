#version 460 core

out vec4 out_color;

in vec2 uv;

layout(binding=0) uniform sampler2D outTexture;

void main()
{             
	out_color = texture(outTexture, uv);
}