#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_query: require
#include "global.glsl"

#include "global-descset.glsl"
#include "intersection.glsl"
#include "surface.glsl"

// out

layout(location = 0) out vec4 outColor;

// in

layout(location = 0) in vec2 uv;

// uniform

layout(push_constant) uniform PC
{
	int quadlightCount;
};

layout(set = 1, binding = 0, std430) readonly buffer Quadlights { Quadlight light[]; } quadlights;
layout(set = 2, binding = 0) uniform accelerationStructureEXT topLevelAs;

float VisibilityOfRay(vec3 origin, vec3 direction, float tMin, float tMax) {

	// Initializes a ray query object but does not start traversal
	rayQueryEXT rayQuery;
	rayQueryInitializeEXT(rayQuery, 
							topLevelAs, 
							gl_RayFlagsTerminateOnFirstHitEXT, 
							0xFD, 
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

//vec4 AmbientInfoBlurredOcclusion()
//{
//	float Offsets[4] = float[]( -1.5, -0.5, 0.5, 1.5 );
//
//    vec4 color = texture(aoSampler, uv);
//
//    for (int i = 0 ; i < 4 ; i++) {
//        for (int j = 0 ; j < 4 ; j++) {
//            vec2 tc = uv;
//            tc.x = uv.x + Offsets[j] / textureSize(aoSampler, 0).x;
//            tc.y = uv.y + Offsets[i] / textureSize(aoSampler, 0).y;
//            color.a += texture(aoSampler, tc).a;
//        }
//    }
//
//    color.a /= 16.0;
//
//    return color;
//}

// WIP:
vec3 Quadlight_AfterContribution(Quadlight ql, Surface surface)
{
	if(surface.a < SPEC_THRESHOLD) {
		return vec3(0);
	}

	vec3 V = getIncomingDir(surface);

	vec3 L = reflect(-V, surface.basis.normal);

	// we need to rotate the reflection ray to force intersection with quad's plane
	float t;
	if (!RayPlaneIntersection(surface.position, L, ql.center, ql.normal, t)) { 
		vec3 perp_r_n = L - dot(L, ql.normal) * ql.normal;
		vec3 pointOnPlane = ql.center + perp_r_n * 100; // WIP: something big
		L = normalize(pointOnPlane - surface.position);
	}

	vec3 p = surface.position + t * L; // intersection point with rect's plane 

	// if point isn't in rectangle, choose the closest that is
	if(!PointInsideRectangle(p, ql.center, ql.normal, ql.right, ql.up, ql.width, ql.height)) {
		p = PointRectangleNearestPoint(p, ql.center, ql.normal, ql.right, ql.up, ql.width, ql.height);
	}

	L = normalize(p - surface.position);
	addOutgoingDir(surface, L);

	if(isOutgoingDirPassingThrough(surface)) { 
		return vec3(0);
	}

	return ql.color * ql.intensity * explicitBRDF(surface);// * VisibilityOfRay(surface.position, L, 0.001, distance(surface.position, p)); // we lose the L, and Li(p, L) data - but at least we got smooth shadows - yey
}

void main()
{
	float depth = texture(g_DepthSampler, uv).r;

	vec4 ambientInfo = texture(aoSampler, uv);


	if(depth == 1.0) {
		outColor = vec4(ambientInfo.rgb, 1.0);
		return;
	}

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

	vec3 directLight = texture(directLightSampler, uv).rgb;
	vec3 indirectLight = texture(indirectLightSampler, uv).rgb;
    vec4 arealightShadowing = texture(_reserved0_, uv);
    vec3 mirror = texture(mirrorSampler, uv).rgb;

	// ...

    vec3 arealights = vec3(0);

	// WIP: max 4 quadlights atm
    for (int i = 0; i < quadlightCount; ++i) {
		if(i > 3) break;
		Quadlight ql = quadlights.light[i];
		arealights += Quadlight_AfterContribution(ql, surface) * arealightShadowing[i];
    }

	vec3 final =  directLight + (indirectLight * ambientInfo.a) + surface.emissive + mirror + arealights;

	outColor = vec4(final, 1.0);
}
