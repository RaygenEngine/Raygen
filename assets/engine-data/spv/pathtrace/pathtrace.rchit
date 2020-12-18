#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_EXT_ray_query: require

// TODO:
#define RAY
#include "global.glsl"

#include "pathtrace/lights.glsl"
#include "surface.glsl"

// Values before being filled
struct hitPayload
{
	vec3 radiance; // previous radiance

	vec3 origin; // origin and dir of THIS ray
	vec3 direction;

	vec3 attenuation;  // attenuation of THIS ray
	float nol;
	float sampleWeight;

	int hitType; // previous hit type
	uint seed;
};

layout(push_constant) uniform PC
{
	int bounces;
	int frame;
	int pointlightCount;
	int spotlightCount;
	int dirlightCount;
	int quadlightCount;
};

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
	mat4 invTransform;

	Vertices vtxBuffer;
	Indicies indBuffer;
	Material materialUbo;

	uint indexOffset;
	uint primOffset;

	mat4 transform;

};

OldVertex fromVertex(Vertex p) {
	OldVertex vtx;
	
	vtx.position = vec3(p.posX, p.posY, p.posZ);
	vtx.normal = vec3(p.nrmX, p.nrmY, p.nrmZ);
	vtx.tangent = vec3(p.tngX, p.tngY, p.tngZ);
	
	vtx.uv = vec2(p.u, p.v);
	
	return vtx;
}

layout(set = 2, binding = 0) uniform accelerationStructureEXT topLevelAs;
layout(set = 3, binding = 0, std430) readonly buffer GeometryGroups { GeometryGroup g[]; } geomGroups;
layout(set = 3, binding = 1) uniform sampler2D textureSamplers[];
layout(set = 4, binding = 0, std430) readonly buffer Pointlights { Pointlight light[]; } pointlights;
layout(set = 5, binding = 0, std430) readonly buffer Spotlights { Spotlight light[]; } spotlights;
layout(set = 6, binding = 0, std430) readonly buffer Dirlights { Dirlight light[]; } dirlights;
layout(set = 7, binding = 0, std430) readonly buffer Quadlights { Quadlight light[]; } quadlights;

vec4 texture(samplerRef s, vec2 uv) {
	return texture(textureSamplers[nonuniformEXT(s.index)], uv);
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

	vec3 V = normalize(-gl_WorldRayDirectionEXT);
    surface.v = normalize(toOnbSpace(surface.basis, V));
    surface.nov = max(Ndot(surface.v), BIAS);

	surface.albedo = (1.0 - metallic) * baseColor;
    surface.opacity = sampledBaseColor.a;

	surface.f0 = vec3(0.16 * reflectance * reflectance * (1.0 - metallic)) + baseColor * metallic;
	surface.a = roughness * roughness;

    surface.emissive = sampledEmissive.rgb;
    surface.occlusion = sampledEmissive.a;

    return surface;
}

void main() {
	
	int matId = gl_InstanceCustomIndexEXT;

	GeometryGroup gg = geomGroups.g[nonuniformEXT(matId)];

	Surface surface = surfaceFromGeometryGroup(gg);

	vec3 radiance = vec3(0.f);

	// DIRECT 
	{
		int totalLights = pointlightCount + quadlightCount + spotlightCount + dirlightCount;
		float u = rand(prd.seed);
		int i = int(floor(u * totalLights));
		float p_selectLight = 1.0 / float(totalLights); // pick one of the lights

#define pIndex i
#define qIndex pIndex - pointlightCount
#define sIndex qIndex - quadlightCount
#define dIndex sIndex - spotlightCount

		// PERF:
		// pointlights // WIP: aperture of effect
		if(pIndex < pointlightCount) {
		    Pointlight pl = pointlights.light[pIndex];

			radiance += Pointlight_LightSample(topLevelAs, pl, surface, prd.seed) / p_selectLight; 
		}

		// quadlights WIP: area - sphere
		else if (qIndex < quadlightCount) {
			Quadlight ql = quadlights.light[qIndex];
			// direct light sample, brdf sample from hit shader and MIS
			radiance += Quadlight_LightSample(topLevelAs, ql, surface, prd.seed) / p_selectLight; 
		}

		// spotlights WIP: area - hemisphere
		else if (sIndex < spotlightCount) {
			Spotlight sl = spotlights.light[sIndex];

			radiance += Spotlight_LightSample(topLevelAs, sl, surface, prd.seed) / p_selectLight;  
		}

		// dirlights WIP: area - disk for each position
		else if (dIndex < dirlightCount) {
			Dirlight dl = dirlights.light[dIndex];

			radiance += Dirlight_LightSample(topLevelAs, dl, surface, prd.seed) / p_selectLight;  
		}

		if(any(greaterThan(surface.emissive, vec3(BIAS)))) {
			radiance += surface.emissive;
		}
	}

	prd.radiance = radiance;

	// INDIRECT - next step
	{
		// mirror H = N 
		float LoH = max(Ndot(surface.v), BIAS);

		if(surface.a >= SPEC_THRESHOLD) {
			vec2 u = rand2(prd.seed);
			vec3 H = importanceSampleGGX(u, surface.a);

			surface.l =  reflect(-surface.v, H);
			LoH = max(dot(surface.v, H), BIAS); 
		}

		// Use a information
		vec3 ks = F_SchlickRoughness(LoH, surface.f0, surface.a);

		float p_specular = sum(ks) / 3.f;

		// diffuse
		if(rand(prd.seed) > p_specular) {
			prd.attenuation = SampleDiffuseDirection(surface, prd.seed, prd.sampleWeight);
			prd.sampleWeight /= (1 - p_specular);
			prd.hitType = 1;
		}

		// reflection
		else {

			// mirror
			if(surface.a < SPEC_THRESHOLD){
				prd.attenuation = SampleMirrorDirection(surface);
				prd.sampleWeight = 1.f;
				prd.hitType = 4;// mirror
			}

			// glossy
			else {
				cacheSurfaceDots(surface);

				float pdf = importanceSamplePdf(surface.a, surface.noh, surface.loh);
				if(pdf < BIAS) {
					prd.hitType = 3; // end this path
					return;
				}
    
				prd.attenuation = ks * SpecularTerm(surface);
				prd.sampleWeight = 1.f / pdf;
				prd.hitType = 1;
			}

			prd.sampleWeight /= p_specular;
		}

//		float p_specular = 0.5;
//
//		// diffuse
//		if(rand(prd.seed) > p_specular) {
//			prd.attenuation = SampleDiffuseDirection(surface, prd.seed, prd.sampleWeight);
//			prd.sampleWeight /= (1 - p_specular);
//		}
//
//		// specular
//		else {
//			prd.attenuation = SampleSpecularDirection(surface, prd.seed, prd.sampleWeight);
//			prd.sampleWeight /= p_specular;
//		}

		prd.nol = surface.nol;	
		prd.origin = surface.position;
		prd.direction = surfaceIncidentLightDir(surface);
	}
}
