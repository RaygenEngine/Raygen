#version 450
#extension GL_GOOGLE_include_directive: enable
#include "global.h"

// out

layout (location = 0) out vec4 gNormal;
// rgb: base color, a: opacity
layout (location = 1) out vec4 gBaseColor;
// r: metallic, g: roughness, b: reflectance, a: occlusion
layout (location = 2) out vec4 gSurface;
layout (location = 3) out vec4 gEmissive;

// in

layout(location=0) in Data
{ 
	vec2 uv;
	mat3 TBN;
};

// uniforms

void main() {
	vec3 normal = normalize(vec3(0.5, 0.5, 1.0) * 2.0 - 1.0);

    gNormal = vec4(normalize(TBN * normal.rgb), 1.f);
	
    gBaseColor = vec4(0.3f, 0.3f, 0.3f, 1.f);
	
	gSurface = vec4(0.f, 0.5f, 0.5f, 0.f);
	
	gEmissive = vec4(0.f, 0.f, 0.f, 1.f);
}                                                                                        

