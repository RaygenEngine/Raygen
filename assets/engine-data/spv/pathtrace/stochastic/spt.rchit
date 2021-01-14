#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference2 : enable

// TODO:
#define RAY
#include "global.glsl"

#include "pathtrace/lights.glsl"
#include "surface-path.glsl"
#include "surface.glsl"

struct hitPayload
{
	vec3 radiance; // previous radiance

	vec3 origin; // stuff of THIS ray
	vec3 direction;
	vec3 attenuation; 
	float weightedPdf;

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
	float baseReflectivity;

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

bool exits = false;
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
	vec3 N = normalize(TBN * (sampledNormal.rgb * 2.0 - 1.0));

	// use geometric normal if this one is broken for any reason
    if(isnan(N.x) || isnan(N.y) || isnan(N.z)){
    	N = Ng;
    }

	vec3 baseColor = sampledBaseColor.rgb;
	float metallic = sampledMetallicRoughness.b * mat.metallicFactor;
	float roughness = sampledMetallicRoughness.g * mat.roughnessFactor;

	vec3 V = normalize(-gl_WorldRayDirectionEXT);

	Surface surface;


	// Material stuff
	surface.albedo = mix(baseColor, vec3(0.0), metallic);
    surface.opacity = mat.mask != 1 ? sampledBaseColor.a : 1.0f; // if mask is discard mode, ignore opacity

	surface.f0 = mix(vec3(mat.baseReflectivity), baseColor, metallic);
	surface.a = roughness * roughness;

    surface.emissive = sampledEmissive.rgb;
    surface.occlusion = sampledEmissive.a;		
	
	// Geometric stuff
	float f0 = max(surface.f0);
	float sqrtf0 = sqrt(f0);
	float eta = -(f0 + 1 + 2 * sqrtf0) / (f0 - 1);

	// WIP: use of current medium, not vacuum (1.0)
	float etaI = 1.0; // vacuum (recursive)
	float etaT = eta; // material
		
	// if behind actual surface flip the normals
	if(dot(V, Ng) < 0) { 
	    Ng = -Ng;
		N = -N; 
		// swap
		float t = etaI; 
		etaI = etaT;
		etaT = t;
	}

	surface.eta = etaI / etaT;
		
	surface.position = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
	surface.basis = branchlessOnb(N);

	surface.ng = normalize(toOnbSpace(surface.basis, Ng));

    surface.v = normalize(toOnbSpace(surface.basis, V));
    surface.nov = absNdot(surface.v);

    return surface;
}

void main() {
	
	int matId = gl_InstanceCustomIndexEXT;

	GeometryGroup gg = geomGroups.g[nonuniformEXT(matId)];

	Surface surface = surfaceFromGeometryGroup(gg);

	vec3 radiance = vec3(0.f);
	bool isDiffusePath;
	float pathPdf, bsdfPdf;
	if(!SampleBSDF(surface, prd.attenuation, pathPdf, bsdfPdf, isDiffusePath, prd.seed)) {
		prd.attenuation = vec3(0);
		prd.weightedPdf = 1;
		prd.hitType = 4;
		return;
	}

	bool deltaPath = surface.a < SPEC_THRESHOLD && !isDiffusePath;

	// DIRECT 
	if(!exits && !deltaPath) {
		int totalLights = quadlightCount;
		float u = rand(prd.seed);
		int i = int(floor(u * totalLights));
		float pdf_pickLight = 1.0 / float(totalLights); // pick one of the lights

		Quadlight ql = quadlights.light[i];
		// direct light sample, brdf sample from hit shader and MIS
		radiance += Quadlight_Ldirect(topLevelAs, ql, surface, isDiffusePath, pdf_pickLight, prd.seed); 
	}

	radiance += surface.emissive; // works like naive approach
	prd.radiance = radiance;

	// INDIRECT - next step
	{
		// bsdf * nol 
		prd.attenuation = prd.attenuation * surface.nol / pathPdf;
		prd.weightedPdf =  bsdfPdf; // NOTE: this is why we split the pdfs
		prd.hitType = !deltaPath ? 1 : 2; // general or delta
		prd.origin = surface.position;
		prd.direction = surfaceIncidentLightDir(surface);	
	}
}
