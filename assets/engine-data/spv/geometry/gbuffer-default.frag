#version 450
#extension GL_GOOGLE_include_directive: enable
#include "global.glsl"

// out

layout (location = 0) out vec4 gNormal;
// rgb: diffuse color, a: opacity
layout (location = 1) out vec4 gDiffuseColor;
// r: f0, a: a (roughness^2)
layout (location = 2) out vec4 gSpecularColor;
// rgb: emissive, a: occlusion
layout (location = 3) out vec4 gEmissive;

// in

layout(location=0) in Data
{ 
	vec2 uv;
	mat3 TBN;
	vec3 fragPos;
};

// uniforms

void main() {
	// CHECK: adjust
	vec3 normal = normalize(vec3(0.5, 0.5, 1.0) * 2.0 - 1.0);
	gNormal = vec4(normalize(TBN * normal), 1.0);
	gDiffuseColor = vec4(0.3, 0.3, 0.3, 1.0);
	gSpecularColor = vec4(vec3(0.17), 0.25);
	gEmissive = vec4(0.0, 0.0, 0.0, 1.0);
}                                                                                        

