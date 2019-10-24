#version 460 core

out vec4 out_color;

uniform float gamma;

// those are the uvs on the quad
in vec2 quad_uv;

// to pass those use 
// shader->SendTexture(<host code varname>, <bindingLoc>);
// or -//- SendCubeTexture -//-
layout(binding=0) uniform sampler2D lightsColorSampler; 
//layout(binding=1) uniform sampler2D depthSampler;
//layout(binding=2) uniform sampler2D positionsSampler;
//layout(binding=3) uniform sampler2D normalsSampler;
// ...

void main()
{
	vec3 color = texture(lightsColorSampler, quad_uv).rgb;
	
    color = pow(color, vec3(1.0/gamma));
	
	out_color = vec4(color, 1);
}