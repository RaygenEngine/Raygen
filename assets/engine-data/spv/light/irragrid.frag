#version 460 
#extension GL_GOOGLE_include_directive: enable

#include "global.glsl"

#include "fragment.glsl"
#include "attachments.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout (location = 0) in vec2 uv;

// uniform

layout(push_constant) uniform PC {
	vec4 pos[6];
};

layout(set = 1, binding = 0) uniform UBO_Camera {
	vec3 position;
	float pad0;
	mat4 view;
	mat4 proj;
	mat4 viewProj;
	mat4 viewInv;
	mat4 projInv;
	mat4 viewProjInv;
} cam;


layout(set = 2, binding = 0) uniform samplerCube irradianceSampler[6];

void main( ) {
	float depth = texture(g_DepthSampler, uv).r;

	// PERF:
	Fragment frag = getFragmentFromGBuffer(
		depth,
		cam.viewProjInv,
		g_NormalSampler,
		g_AlbedoSampler,
		g_SpecularSampler,
		g_EmissiveSampler,
		uv);
	
	vec3 N = frag.normal;
	vec3 V = normalize(cam.position - frag.position);
	vec3 R = normalize(reflect(-V, N));

	vec3 diffuseLight = vec3(0);
	for(int i = 0; i < 6; ++i){

		float weight = saturate(1 / distance(frag.position, pos[i].xyz));

		diffuseLight += texture(irradianceSampler[i], N).rgb * weight;
	}

	vec3 diffuse = diffuseLight * frag.albedo;
	outColor = vec4(diffuse, 1.0);
}





