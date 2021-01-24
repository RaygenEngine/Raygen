#version 460
#extension GL_GOOGLE_include_directive: enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_ray_query: require

#include "global.glsl"

#include "global-descset.glsl"
#include "surface.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout(location = 0) noperspective in vec2 uv;

// uniform

layout (input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput g_DepthInput;
layout (input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput g_SNormalInput;
layout (input_attachment_index = 2, set = 1, binding = 2) uniform subpassInput g_GNormalInput;
layout (input_attachment_index = 3, set = 1, binding = 3) uniform subpassInput g_AlbedoInput;
layout (input_attachment_index = 4, set = 1, binding = 4) uniform subpassInput g_SpecularInput;
layout (input_attachment_index = 5, set = 1, binding = 5) uniform subpassInput g_EmissiveInput;
layout (input_attachment_index = 6, set = 1, binding = 6) uniform subpassInput g_VelocityInput;
layout (input_attachment_index = 7, set = 1, binding = 7) uniform subpassInput g_UVDrawIndexInput;
layout(set = 2, binding = 0) uniform UBO_Pointlight { Pointlight pl; };
layout(set = 3, binding = 0) uniform accelerationStructureEXT topLevelAs;

// WIP: this can't handle alpha mask
float ShadowRayTest(accelerationStructureEXT topLevelAs, vec3 origin, vec3 direction, float tMin, float tMax)
{ 
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
		return 1.0;
	}
	return 0.0;
}

void main()
{
	float depth = subpassLoad(g_DepthInput).r;

	Surface surface = surfaceFromGBuffer(
	    cam,
		depth,
		g_SNormalInput,
		g_GNormalInput,
		g_AlbedoInput,
		g_SpecularInput,
		g_EmissiveInput,
		uv
	);

	vec3 L = normalize(pl.position - surface.position);

	addOutgoingDir(surface, L);

	if(isOutgoingDirPassingThrough(surface)) { 
		discard;
	}

	float dist = distance(pl.position, surface.position);
	float shadow = 1 - pl.hasShadow * ShadowRayTest(topLevelAs, surface.position, L, 0.001, dist);

	float attenuation = 1.0 / (pl.constantTerm + pl.linearTerm * dist + 
  			     pl.quadraticTerm * (dist * dist));

	vec3 Li = pl.color * pl.intensity * attenuation * shadow; 
	
	vec3 finalContribution = Li * explicitBRDFcosTheta(surface);

	outColor = vec4(finalContribution, 1);
}
