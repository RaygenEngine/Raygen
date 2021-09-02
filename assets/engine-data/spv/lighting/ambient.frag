#extension GL_EXT_ray_query: require

#include "global-descset.glsl"

#include "random.glsl"
#include "sampling.glsl"
#include "sky.glsl"
#include "surface.glsl"

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 uv;

layout(push_constant) uniform PC {
	float bias;
	float strength;
	float radius;
	int samples;
};

layout(set = 1, binding = 0) uniform accelerationStructureEXT topLevelAs;

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
		return strength;
	}

	return 0.0;
}

void main()
{
	float depth = texture(g_DepthSampler, uv).r;

	Surface surface = surfaceFromGBuffer(
	    cam,
		depth,
	    g_SNormalSampler,
		g_GNormalSampler,
		g_AlbedoSampler,
		g_SpecularSampler,
		g_EmissiveSampler,
		uv
	);

	// CHECK: could gather an approximation of the diffused sky light on surfaces during far ao queries
	vec3 skyColor = vec3(0.0);
	if(depth == 1.0) {
	
		vec3 V = normalize(surface.position - cam.position);
	
		skyColor = GetSkyColor(cam.position, V);
	}

	// eye space
	vec3 N       = (cam.view * vec4(surface.basis.normal, 0.0)).xyz;
	vec3 center  = (cam.view * vec4(surface.position, 1.0)).xyz;

	Onb view = branchlessOnb(N.xyz);
	
	// WIP: get res from global desc set -> render scale also should be factored here
	uint seed = tea16(uint(uv.y * 2160 * 4096 + uv.x * 4096), samples);

	float occlusion = 0;
	for(uint smpl = 0; smpl < samples; ++smpl){
		vec2 u = rand2(seed);
		
		//vec2 u = hammersley(smpl, samples); 
		float m = rand(seed); 
		
		// sample random direction, random magnitude (for screen space)
		vec3 L = uniformSampleHemisphere(u) * m; 

		occlusion += VisibilityOfRay(surface.position, normalize(outOnbSpace(surface.basis, L)), bias, radius); // WIP: radius
	}

	// object occlusion
	occlusion = 1 - (occlusion / samples);
	occlusion *= surface.occlusion;

	outColor = vec4(skyColor, occlusion);
}
