#version 460
#extension GL_GOOGLE_include_directive: enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_ray_query: require

#include "global.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform


void main()
{
	vec2 center = vec2(1.);
	vec2 d = (uv - center);
	d *= d;

	outColor = d.x + d.y < 0.5 ? 
	vec4(0.4, 0.4, 0.5, 1.0) : vec4(0);
	
}                               




















