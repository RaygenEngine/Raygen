#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_EXT_ray_query: require

#include "global.glsl"
#include "raytrace/pt/pt.glsl"

#include "random.glsl"
#include "sampling.glsl"
#include "bsdf.glsl"
#include "onb.glsl"
#include "surface.glsl"
#include "lights/pointlight.glsl"

hitAttributeEXT vec2 baryCoord;
layout(location = 0) rayPayloadInEXT hitPayload prd;


struct Vertex
{
	float posX;
	float posY;
	float posZ;
	float nrmX;
	float nrmY;
	float nrmZ;
	float tngX;
	float tngY;
	float tngZ;
	float u;
	float v;
};

struct OldVertex
{
	vec3 position;
	vec3 normal;
	vec3 tangent;
	vec2 uv;
};

layout(set = 1, binding = 0) uniform accelerationStructureEXT topLevelAs;
#ifndef rt_indirect_glsl
#define rt_indirect_glsl

#include "surface.glsl"

// META:
// Expects pre declared variable prd before the inclusion of the file

// Handle just radiance here (no throughput). 
// This function should be able to  be used without throughput (eg: a debug ray directly from the camera that invokes hit shaders)
vec3 RadianceOfRay(vec3 nextOrigin, vec3 nextDirection) {
	prd.radiance = vec3(0);

    // PERF: any ideas?
    if(any(isnan(nextDirection)) || any(isinf(nextDirection))){
       return vec3(0.0);
    }

	prd.depth += 1;

    uint  rayFlags = gl_RayFlagsOpaqueEXT | gl_RayFlagsCullFrontFacingTrianglesEXT;
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
				0               // payload (location = 0)
	);
	
    prd.depth -= 1;
	return prd.radiance;
}

// Handle Termination and throughput in this function, handle radiance in the above
vec3 TraceNext(vec3 thisRayThroughput, vec3 L_shadingSpace, Surface surface) {

    vec3 prevCumulativeThroughput = prd.accumThroughput;
    // RR termination
    // TODO: Use perceived luminace of throughput for termination (instead of max)

	float p_spawn = max(max(thisRayThroughput * prevCumulativeThroughput), 0.1);
	if(rand(prd.seed) > p_spawn) {
		return vec3(0); 
	}

    thisRayThroughput /= p_spawn; 
    
    // Update accum throughput for the remaining of the recursion
    prd.accumThroughput = prevCumulativeThroughput * thisRayThroughput;

    outOnbSpace(surface.basis, L_shadingSpace); // NOTE: not shading space after the function, make onb return values for better names
    vec3 result = thisRayThroughput * RadianceOfRay(surface.position, L_shadingSpace); 

    // Restore accum throughput
    prd.accumThroughput = prevCumulativeThroughput;
    return result;
}

//#define RNG_BRDF_X(bounce)                (4 + 4 + 9 * bounce)
//#define RNG_BRDF_Y(bounce)                (4 + 5 + 9 * bounce)
//
//float get_rng(uint idx, uint rng_seed)
//{
//	//uvec3 p = uvec3(rng_seed, rng_seed >> 10, rng_seed >> 20);
//	//p.z = (p.z + idx);
//	//p &= uvec3(BLUE_NOISE_RES - 1, BLUE_NOISE_RES - 1, NUM_BLUE_NOISE_TEX - 1); // array impl for temporal (z)
//
//	//return min(texelFetch(TEX_BLUE_NOISE, ivec3(p), 0).r, 0.9999999999999);
//	//return fract(vec2(get_rng_uint(idx)) / vec2(0xffffffffu));
//	uvec2 p = uvec2(rng_seed, rng_seed >> 10);
//	p &= uvec2(470 - 1);
//	
//	return texture(blueNoiseSampler, vec2(p)).r;
//}
 

vec3 TraceIndirect(Surface surface) {
    vec3 radiance = vec3(0.f);

    vec3 V = surface.wo;
    vec3 albedo = surface.albedo;
    vec3 f0 =  surface.f0;
    float a = surface.a;

    float NoV = max(Ndot(V), BIAS);

    float p_specular = 0.5;

    // Glossy reflection
    if(rand(prd.seed) > p_specular)
    {
        // Ideally specular
        vec2 u = rand2(prd.seed);
        vec3 H = importanceSampleGGX(u, a);

        vec3 L = reflect(-V, H);

        float NoL = max(Ndot(L), BIAS); 

        float NoH = max(Ndot(H), BIAS); 
        float LoH = max(dot(L, H), BIAS);

        
        float pdf = D_GGX(NoH, a) * NoH /  (4.0 * LoH);
        pdf = max(pdf, BIAS); // CHECK: pbr-book stops tracing if pdf == 0

        if(a < SPEC_THRESHOLD){
            L = reflect(-V);
            H = normalize(V + L);
            NoL = max(Ndot(L), BIAS); 
           
            NoH = max(Ndot(H), BIAS); 
            LoH = max(dot(L, H), BIAS);
            pdf = 1.0;
        }   

        vec3 ks = F_Schlick(LoH, f0);

		addSurfaceIncomingLightDirectionInBasis(surface, L);
        vec3 brdf_r = SpecularTerm(surface, ks) / p_specular;

        radiance += TraceNext(brdf_r * NoL / pdf, L, surface);
    }

    // Diffuse reflection
    else
    {
        vec2 u = rand2(prd.seed); 
        vec3 L = cosineSampleHemisphere(u);

        float NoL = max(Ndot(L), BIAS); 

        vec3 H = normalize(V + L);
        float LoH = max(dot(L, H), BIAS);

        float pdf = NoL * INV_PI;

        vec3 kd = 1 - F_Schlick(LoH, f0);
				addSurfaceIncomingLightDirectionInBasis(surface, L);
        vec3 brdf_d = DiffuseTerm(surface, kd) / (1 - p_specular);




        radiance += TraceNext(brdf_d * NoL / pdf, L, surface);
    }

    return radiance;
}

#else
#error "RT Indirect glsl header should have no reason to be included twice (it declares descriptors). Check if what you are trying to do is correct."
#endif


struct samplerRef {
	int index;
};

struct GltfMat {
	// factors
	vec4 baseColorFactor;
	vec4 emissiveFactor;
	float metallicFactor;
	float roughnessFactor;
	float normalScale;
	float occlusionStrength;

	// alpha mask
	float alphaCutoff;
	int mask;

	samplerRef baseColor;
	samplerRef metallicRough;
	samplerRef occlusion;
	samplerRef normal;
	samplerRef emissive;
};

layout(buffer_reference, std430) buffer Vertices { Vertex v[]; };
layout(buffer_reference, std430) buffer Indicies { uint i[]; };
layout(buffer_reference, std430) buffer Material { GltfMat m; };


struct GeometryGroup {
	Vertices vtxBuffer;
	Indicies indBuffer;
	Material materialUbo;

	uint indexOffset;
	uint primOffset;

	mat4 transform;
	mat4 invTransform;
};

layout(set = 2, binding = 0, std430) readonly buffer GeometryGroups { GeometryGroup g[]; } geomGroups;
layout(set = 2, binding = 1) uniform sampler2D textureSamplers[];

layout(set = 3, binding = 0, std430) readonly buffer Pointlights { Pointlight light[]; } pointlights;

vec4 texture(samplerRef s, vec2 uv) {
	return texture(textureSamplers[nonuniformEXT(s.index)], uv);
}

OldVertex fromVertex(Vertex p) {
	OldVertex vtx;
	
	vtx.position = vec3(p.posX, p.posY, p.posZ);
	vtx.normal = vec3(p.nrmX, p.nrmY, p.nrmZ);
	vtx.tangent = vec3(p.tngX, p.tngY, p.tngZ);
	
	vtx.uv = vec2(p.u, p.v);
	
	return vtx;
}

Surface surfaceFromGeometryGroup(
    GeometryGroup gg)
{
	const uint indOffset = gg.indexOffset;
	const uint primOffset = gg.primOffset;
	
	// Indices of the triangle
	ivec3 ind = ivec3(gg.indBuffer.i[3 * (gl_PrimitiveID + primOffset)],   //
					  gg.indBuffer.i[3 * (gl_PrimitiveID + primOffset) + 1],   //
					  gg.indBuffer.i[3 * (gl_PrimitiveID + primOffset) + 2]) + ivec3(indOffset);  //


	const vec3 barycentrics = vec3(1.0 - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
	
	// Vertex of the triangle
	OldVertex v0 = fromVertex(gg.vtxBuffer.v[ind.x]);
	OldVertex v1 = fromVertex(gg.vtxBuffer.v[ind.y]);
	OldVertex v2 = fromVertex(gg.vtxBuffer.v[ind.z]);

	vec2 uv = v0.uv * barycentrics.x + v1.uv * barycentrics.y + v2.uv * barycentrics.z;
		
	// Computing the normal at hit position
	vec3 normal = v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z;
	vec3 tangent = v0.tangent * barycentrics.x + v1.tangent * barycentrics.y + v2.tangent * barycentrics.z;
	
	normal =  normalize(vec3(gg.invTransform * vec4(normal , 0.0)));
	tangent = normalize(vec3(gg.invTransform * vec4(tangent, 0.0)));

	vec3 Tg = normalize(tangent);
   	vec3 Ng = normalize(normal);
	// Gram-Schmidt process + cross product
	// re-orthogonalize T with respect to N
	Tg = normalize(Tg - dot(Tg, Ng) * Ng);
	// then retrieve perpendicular vector B with the cross product of T and N
	vec3 Bg = cross(Ng, Tg);
	mat3 TBN = mat3(Tg, Bg, Ng);

	GltfMat mat = gg.materialUbo.m;

	// sample material textures
	vec4 sampledBaseColor = texture(mat.baseColor, uv) * mat.baseColorFactor; // 
	vec4 sampledNormal = texture(mat.normal, uv) * mat.normalScale; // 
	vec4 sampledMetallicRoughness = texture(mat.metallicRough, uv); //
	vec4 sampledEmissive = texture(mat.emissive, uv) * mat.emissiveFactor;
	vec3 ns = normalize(TBN * (sampledNormal.rgb * 2.0 - 1.0));
	vec3 baseColor = sampledBaseColor.rgb;
	float metallic = sampledMetallicRoughness.b * mat.metallicFactor;
	float roughness = sampledMetallicRoughness.g * mat.roughnessFactor;
	float reflectance = 0.5;

	Surface surface;

	surface.position = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
	surface.basis = branchlessOnb(ns);

	vec3 V = -gl_WorldRayDirectionEXT;
	addSurfaceOutgoingLightDirection(surface, V);

	surface.albedo = (1.0 - metallic) * baseColor;
    surface.opacity = sampledBaseColor.a;

	surface.f0 = vec3(0.16 * reflectance * reflectance * (1.0 - metallic)) + baseColor * metallic;
	surface.a = roughness * roughness;

    surface.emissive = sampledEmissive.rgb;
    surface.occlusion = sampledEmissive.a;

    return surface;
}

void main() {
	
	int matId = gl_InstanceID;

	GeometryGroup gg = geomGroups.g[nonuniformEXT(matId)];

	Surface surface = surfaceFromGeometryGroup(gg);

	vec3 radiance = vec3(0);
	// DIRECT
	{
		if (sum(surface.emissive) > BIAS) {
			prd.radiance = surface.emissive.xyz;
			return;
		}

		// for each light
		for(int i = 0; i < pointlightCount; ++i) {
			Pointlight pl = pointlights.light[i];
			radiance +=  Pointlight_Contribution(topLevelAs, pl, surface);
		}
	}

	if(prd.depth > bounces){
		prd.radiance = radiance;
		return;
	}

	radiance += TraceIndirect(surface);
	prd.radiance = radiance;
}










