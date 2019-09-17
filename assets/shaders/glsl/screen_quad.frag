#version 460 core

out vec4 out_color;

in vec2 UV;

uniform sampler2D outTexture;

void main()
{
	out_color = texture(outTexture, UV);
}