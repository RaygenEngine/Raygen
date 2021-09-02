#extension GL_EXT_ray_query: require

#include "global-descset.glsl"
#include "intersection.glsl"
#include "surface.glsl"

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 uv;

layout(push_constant) uniform PC
{
	int quadlightCount;
};

layout(set = 1, binding = 0, std430) readonly buffer Quadlights { Quadlight light[]; } quadlights;
layout(set = 2, binding = 0) uniform accelerationStructureEXT topLevelAs;

bool VisibilityOfRay(vec3 origin, vec3 direction, float tMin, float tMax) {

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
		return false;
	}

	return true;
}

vec3 Quadlight_SpecularContribution(Quadlight ql, Surface surface)
{
	if(surface.a < SPEC_THRESHOLD) {
		return vec3(0);
	}

	vec3 V = getIncomingDir(surface);

	vec3 L = reflect(-V, surface.basis.normal);

	// we need to rotate the reflection ray to force intersection with quad's plane
	float t;
	if (!RayPlaneIntersection(surface.position, L, ql.center, ql.normal, t)) { 
		//return vec3(0);
		vec3 perp_r_n = L - dot(L, ql.normal) * ql.normal;
		vec3 pointOnPlane = ql.center + perp_r_n * INF; // SMATH: something big
		L = normalize(pointOnPlane - surface.position);
	}

	vec3 p = surface.position + t * L; // intersection point with rect's plane 

	// if point isn't in rectangle, choose the closest that is
	if(!PointInsideRectangle(p, ql.center, ql.normal, ql.right, ql.up, ql.width, ql.height)) {
		p = PointRectangleNearestPoint(p, ql.center, ql.normal, ql.right, ql.up, ql.width, ql.height);
	}

	L = normalize(p - surface.position);

	float cosTheta_o = dot(ql.normal, -L);

	if (cosTheta_o < BIAS) {
		return vec3(0);
	}

	addOutgoingDir(surface, L);
	if(isOutgoingDirPassingThrough(surface)) { 
		return vec3(0);
	}

	float dist = distance(p, surface.position);

	//if(!VisibilityOfRay(surface.position, L, 0.001, dist)) {
	//	return vec3(0);
	//}

	return ql.color * ql.intensity * microfacetBRDF(surface);
}

vec3 Quadlight_DiffuseContribution(Quadlight ql, Surface surface) 
{
	return ql.color * ql.intensity * diffuseBRDF(surface);
}

float AmbientInfoBlurredOcclusion()
{
	float Offsets[4] = float[]( -1.5, -0.5, 0.5, 1.5 );

    float color = texture(ambientLightSampler, uv).a;

    for (int i = 0 ; i < 4 ; i++) {
        for (int j = 0 ; j < 4 ; j++) {
            vec2 tc = uv;
            tc.x = uv.x + Offsets[j] / textureSize(ambientLightSampler, 0).x;
            tc.y = uv.y + Offsets[i] / textureSize(ambientLightSampler, 0).y;
            color += texture(ambientLightSampler, tc).a;
        }
    }

    color /= 16.0;

    return color;
}

void main()
{
	float depth = texture(g_DepthSampler, uv).r;

	vec4 ambientInfo = texture(ambientLightSampler, uv);


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
    vec3 mirror = texture(_reserved2_, uv).rgb;

	// ...

    vec3 arealights = vec3(0);

	// WIP: max 3 quadlights atm
    for (int i = 0; i < quadlightCount; ++i) {
		if(i > 2) break;
		Quadlight ql = quadlights.light[i];

		vec3 L = normalize(ql.center - surface.position);
		addOutgoingDir(surface, L);

		//arealights += ql.color * ql.intensity * explicitBRDF(surface) * arealightShadowing[i];

		vec3 ks = interfaceFresnel(surface);
		vec3 kd = (1.0 - ks) * surface.opacity;

		arealights += kd * Quadlight_DiffuseContribution(ql, surface) * arealightShadowing[i];
		arealights += ks * Quadlight_SpecularContribution(ql, surface) * arealightShadowing[i];
    }

	vec3 final =  directLight + (indirectLight *  AmbientInfoBlurredOcclusion()) + surface.emissive + mirror + arealights;

	outColor = vec4(final, 1.0);
}
