#version 460 core

out vec4 out_color;

uniform vec2 invTextureSize;

layout(binding=0) uniform sampler2D outTexture;

void main()
{             
	vec2 textUV = gl_FragCoord.st * invTextureSize;

	out_color = texture(outTexture, textUV);
}