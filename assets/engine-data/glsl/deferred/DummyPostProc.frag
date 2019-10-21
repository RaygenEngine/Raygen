#version 460 core

out vec4 out_color;

// those are the uvs on the quad
in vec2 quad_uv;

// see how we pass invTextureSize and pass any uniform likewise
uniform vec2 invTextureSize;

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
	// with these you can sample the input textures
	// so that they match the quad (i.e. if the sampler is different resolution
	// than the quad use those uvs to match them)
	vec2 text_uv = gl_FragCoord.st * invTextureSize;
	
	vec3 color = texture(lightsColorSampler, text_uv).rgb;
	
	out_color = vec4(vec3(1,1,1) - color, 1);
}