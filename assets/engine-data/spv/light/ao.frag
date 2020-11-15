#version 460
#extension GL_GOOGLE_include_directive: enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_ray_query: require

#include "global.glsl"

#include "attachments.glsl"
#include "random.glsl"
#include "sampling.glsl"
#include "surface.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) in vec2 uv;

// uniform

layout(set = 1, binding = 0) uniform UBO_Camera { Camera cam; };
layout(set = 2, binding = 0) uniform accelerationStructureEXT topLevelAs;

float VisibilityOfRay(vec3 origin, vec3 direction, float tMin, float tMax) {

	// Initializes a ray query object but does not start traversal
	rayQueryEXT rayQuery;
	rayQueryInitializeEXT(rayQuery, 
							topLevelAs, 
							gl_RayFlagsTerminateOnFirstHitEXT, 
							0xFF, 
							origin, 
							tMin,
							direction, 
							tMax);

	// Start traversal: return false if traversal is complete
	while(rayQueryProceedEXT(rayQuery)) {
	}
      
	// Returns type of committed (true) intersection
	if(rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionNoneEXT) {
		// Got an intersection == Shadow
		return 0.0;
	}

	return 1.0;
}

void main()
{
	Surface surface = surfaceFromGBuffer(
	    cam,
		g_DepthSampler,
		g_NormalSampler,
		g_AlbedoSampler,
		g_SpecularSampler,
		g_EmissiveSampler,
		uv
	);


	float occlusion = 0;
	#define samples 1
	for(uint smpl = 0; smpl < samples; ++smpl){

		// WIP: seed
		uint seed = tea16(uint(surface.uv.y * 2160 * 4096 + surface.uv.x * 4096), samples + smpl);

		vec2 u = rand2(seed); 
		float m = rand(seed); 
		vec3 L = uniformSampleHemisphere(u) * m; // sample random direction, random magnitude (for screen space)

		outOnbSpace(surface.basis, L);

		float tMax = 1.0;
		vec3 samplePos = surface.position + tMax * L;

		vec4 clipPos = cam.viewProj * vec4(samplePos, 1.0); // clip space
		vec3 ndc = clipPos.xyz / clipPos.w; // NDC
		ndc.xy = ndc.xy * 0.5 + 0.5; // 0 to 1
		float d = texture(g_DepthSampler, ndc.xy).r;
		float sampleDepth = reconstructWorldPosition(d, ndc.xy, cam.viewProjInv).r;

		float radius = tMax;
		//float rangeCheck = smoothstep(0.0, 1.0, radius / abs(surface.position.z - sampleDepth));

		bool screenSpace =  ndc.x < 0 || ndc.x > 1 || ndc.y < 0 || ndc.y > 1;
		occlusion += screenSpace ? (sampleDepth >= samplePos.z + .025 ? 1.0 : 0.0) 
		                         : VisibilityOfRay(surface.position, L, 0.01, tMax);
	}

	// object occlusion
	occlusion /= samples;
	occlusion *= surface.occlusion;

	outColor = vec4(occlusion);
}                               




















