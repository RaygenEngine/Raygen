#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_ray_query: enable

#include "global.h"
#include "rtshared.h"
#include "bsdf.h"
#include "hammersley.h"
#include "sampling.h"
#include "shading-space.h"

struct Attr{
	vec2 x;
};

hitAttributeEXT vec2 baryCoord;

// Texture samplers are 3 samplers per gl_InstanceID: 
// colorSampler, normalSampler, metallicRoughnessSampler
// eg: normal sampler of instance id: 3 would be:
//     (id * 3) + 1 = 10

#define GEOM_GROUPS 25

layout(set = 1, binding = 0) uniform accelerationStructureEXT topLevelAs;

layout(set = 3, binding = 0) uniform sampler2D textureSamplers[GEOM_GROUPS * 3];
layout(std430, set = 3, binding = 1) readonly buffer Vertices{ Vertex v[]; } vertices;
layout(std430, set = 3, binding = 2) readonly buffer Indicies{ uint i[]; } indices;
layout(std430, set = 3, binding = 3) readonly buffer IndexOffsets { uint offset[]; } indexOffsets;
layout(std430, set = 3, binding = 4) readonly buffer PrimitveOffsets { uint offset[]; } primOffsets;

//HitAttributeKHR vec2 attribs;

layout(location = 0) rayPayloadInEXT hitPayload inPrd;
layout(location = 1) rayPayloadEXT hitPayload prd;

layout(push_constant) uniform Constants
{
    int frame;
    int depth;
    int samples;
};


OldVertex fromVertex(Vertex p) {
	OldVertex vtx;
	
	vtx.position = vec3(p.posX, p.posY, p.posZ);
	vtx.normal = vec3(p.nrmX, p.nrmY, p.nrmZ);
	vtx.tangent = vec3(p.tngX, p.tngY, p.tngZ);
	
	vtx.uv = vec2(p.u, p.v);
	
	return vtx;
}

void RRTerminateOrTraceRay(vec3 nextOrigin, vec3 nextDirection, vec3 throughput)
{
	
	// RR termination
	if(inPrd.depth >= 1){

		// TODO: check cumulative throughput
		float p_spawn = max(throughput.x, max(throughput.y, throughput.z));

		if(rnd(prd.seed) >= p_spawn){
			return; 
		}

		throughput /= p_spawn;
	}

	prd.radiance = vec3(0);
	prd.depth = inPrd.depth + 1;
    prd.seed = inPrd.seed;

	if(prd.depth > depth)
	{
		return;
	}

    uint  rayFlags = gl_RayFlagsOpaqueEXT;
    float tMin     = 0.001;
    float tMax     = 10000.0;

	// trace ray
	traceRayEXT(topLevelAs,     // acceleration structure
				rayFlags,       // rayFlags
				0xFF,           // cullMask
				0,              // sbtRecordOffset
				0,              // sbtRecordStride
				0,              // missIndex
				nextOrigin,     // ray origin
				tMin,           // ray min range
				nextDirection,  // ray direction
				tMax,           // ray max range
				1               // payload (location = 0)
	);

	inPrd.radiance += throughput * prd.radiance;
}

void main() {

	int matId = gl_InstanceID % 25;

 
	const uint indOffset = indexOffsets.offset[matId];
	const uint primOffset = primOffsets.offset[matId];
	
	// Indices of the triangle
	ivec3 ind = ivec3(indices.i[3 * (gl_PrimitiveID + primOffset)],   //
			indices.i[3 * (gl_PrimitiveID + primOffset) + 1],   //
			indices.i[3 * (gl_PrimitiveID + primOffset) + 2]) + ivec3(indOffset);  //


	const vec3 barycentrics = vec3(1.0 - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
	
	
	// Vertex of the triangle
	OldVertex v0 = fromVertex(vertices.v[ind.x]);
	OldVertex v1 = fromVertex(vertices.v[ind.y]);
	OldVertex v2 = fromVertex(vertices.v[ind.z]);


	vec2 uv = v0.uv * barycentrics.x + v1.uv * barycentrics.y + v2.uv * barycentrics.z;
		
	// Computing the normal at hit position
	vec3 normal = v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z;
	vec3 tangent = v0.tangent * barycentrics.x + v1.tangent * barycentrics.y + v2.tangent * barycentrics.z;

	vec3 Tg = normalize(tangent);
   	vec3 Ng = normalize(normal);

	// Gram-Schmidt process + cross product
	// re-orthogonalize T with respect to N
	Tg = normalize(Tg - dot(Tg, Ng) * Ng);
	// then retrieve perpendicular vector B with the cross product of T and N
	vec3 Bg = cross(Ng, Tg);

	mat3 TBN = mat3(Tg, Bg, Ng); 

	// TODO: Transforming the normal to world space
	// normal = normalize(vec3(scnDesc.i[gl_InstanceID].transfoIT * vec4(normal, 0.0)));


	// Computing the coordinates of the hit position
	vec3 worldPos = v0.position * barycentrics.x + v1.position * barycentrics.y + v2.position * barycentrics.z;
	// TODO: Transforming the position to world space

	// sample material textures
	vec4 sampledBaseColor = texture(textureSamplers[matId * 3], uv);
	vec4 sampledNormal = texture(textureSamplers[matId * 3 + 1], uv);
	vec4 sampledMetallicRoughness = texture(textureSamplers[matId * 3 + 2], uv); 

	
	// final material values
	vec3 albedo = sampledBaseColor.rgb;
	float metallic = sampledMetallicRoughness.b;
	float roughness = sampledMetallicRoughness.g;
	float reflectance = 0.f;


    // remapping

	// diffuseColor = (1.0 - metallic) * albedo;
    vec3 diffuseColor = (1.0 - metallic) * albedo;
    // f0 = 0.16 * reflectance * reflectance * (1.0 - metallic) + albedo * metallic;
	vec3 f0 = vec3(0.16 * reflectance * reflectance * (1.0 - metallic)) + albedo * metallic;
    // a = roughness roughness;
	float a = roughness * roughness;

	vec3 hitPoint = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;

	// TODO: use shading normal
	vec3 N = Ng;//normalize(TBN * (sampledNormal.rgb* 2.0 - 1.0));
	vec3 V = normalize(gl_WorldRayOriginEXT - hitPoint);

	vec3 wo = normalize(toSurface(Ng, Tg, Bg, V));

	// DIRECT

	inPrd.radiance = vec3(0);

	// for each light
	if(inPrd.depth != 0)
	{
		vec3 lightPos = vec3(7,2,0);
		vec3 lightColor = vec3(0.98823529411764705882352941176471, 0.83137254901960784313725490196078, 0.25098039215686274509803921568627);
		float lightIntensity = 30;
		
		vec3 wi = normalize(toSurface(Ng, Tg, Bg, lightPos - hitPoint));
		
		// attenuation
		float dist = length(lightPos - hitPoint);
		float attenuation = 1.0 / (dist * dist); // TODO: fix with coefs
		
		float cosTheta_ = cosTheta(wi);

		// if you are gonna trace this ray 
		// check dot(N,L) > 0 
		float shadow = 0.f; //...
		{ // PERF : dont use query here of course
			// float tMin      = 0.01f;
			// float tMax      = distance(lightPos, hitPoint);
		
			// // Initializes a ray query object but does not start traversal
			// rayQueryEXT rayQuery;
			// rayQueryInitializeEXT(rayQuery, topLevelAs, gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, hitPoint, tMin,
			// 				L, tMax);
		
			// // Start traversal: return false if traversal is complete
			// while(rayQueryProceedEXT(rayQuery)) {
			// }
			
			// // Returns type of committed (true) intersection
			// if(rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionNoneEXT) {
			// // Got an intersection == Shadow
			// shadow = 1.0;
			// }
		}

		// sample shadowmap for shadow
		vec3 Li = (1.0 - shadow) * lightColor * lightIntensity * attenuation; 
	

		// to get final diffuse and specular both those terms are multiplied by Li * NoL
		vec3 brdf_d = LambertianReflection(wo, wi, diffuseColor);
		vec3 brdf_r = MicrofacetReflection(wo, wi, a, f0);

		// so to simplify (faster math)
		// throughput = (brdf_d + brdf_r) * NoL
		// incoming radiance = Li;
		vec3 finalContribution = (brdf_d + brdf_r) * Li * cosTheta_;

		inPrd.radiance += finalContribution;
	}

	// INDIRECT

	// TRANSMISSION
	{
		vec2 u = vec2(rnd(inPrd.seed), rnd(inPrd.seed));
		vec3 wi = cosineSampleHemisphere2(u);

		// If wo is in the opposite hemisphere, then wi must be flipped to lie in the same hemisphere as wo.
		if (wo.z < 0) 
			wi.z *= -1;

		float cosTheta_ = cosTheta(wi);

		if(cosTheta_ > 0)
		{
			float pdf = cosineHemispherePdf(cosTheta_);
		
			vec3 throughput = LambertianReflection(wo, wi, diffuseColor) * cosTheta_ / pdf;

			// RR termination
			RRTerminateOrTraceRay(hitPoint, toWorld(Ng, Tg, Bg, wi), throughput);
		}
	}
	
	// REFLECTION
	{
		// sample new direction wi and its pdfs based on distribution of the GGX BRDF
		vec2 Xi = vec2(rnd(inPrd.seed), rnd(inPrd.seed)); 
		vec3 wh  = importanceSampleGGX2(Xi, a);

		// TODO: use reflect
		vec3 wi  = normalize(2.0 * dot(wo, wh) * wh - wo);

		// If wo is in the opposite hemisphere, then wi must be flipped to lie in the same hemisphere as wo.
		if (wo.z < 0) 
			wi.z *= -1;
	
		float cosTheta_ = cosTheta(wi);

		if(cosTheta_ > 0)
		{
			vec3 brdf_r = MicrofacetReflection(wo, wi, a, f0);

			float pdf = (D_TrowbridgeReitzDistribution(wh, a) * cosTheta(wh) / (4 * dot(wh, wo))) + 0.0001;

			vec3 throughput = brdf_r * cosTheta_ / pdf;

			RRTerminateOrTraceRay(hitPoint, toWorld(Ng, Tg, Bg, wi), throughput);
		}
	}
	
}

















