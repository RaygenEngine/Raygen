#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_EXT_ray_query: require

#include "global.glsl"
#include "raytrace/rtspec/rtspec.glsl"

#include "sampling.glsl"
#include "bsdf.glsl"
#include "onb.glsl"
#include "attachments.glsl"
#include "aabb.glsl"
#include "random.glsl"
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




OldVertex fromVertex(Vertex p) {
	OldVertex vtx;
	
	vtx.position = vec3(p.posX, p.posY, p.posZ);
	vtx.normal = vec3(p.nrmX, p.nrmY, p.nrmZ);
	vtx.tangent = vec3(p.tngX, p.tngY, p.tngZ);
	
	vtx.uv = vec2(p.u, p.v);
	
	return vtx;
}



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


layout(set = 3, binding = 0) uniform accelerationStructureEXT topLevelAs;
layout(set = 4, binding = 0, std430) readonly buffer GeometryGroups { GeometryGroup g[]; } geomGroups;
layout(set = 4, binding = 1) uniform sampler2D textureSamplers[];
layout(set = 5, binding = 0, std430) readonly buffer Pointlights { Pointlight light[]; } pointlights;
layout(set = 6, binding = 0) uniform UBO_Irragrid { Irragrid grid; };
layout(set = 7, binding = 0) uniform samplerCube irradianceSampler[];




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

vec3 RadianceOfRay(vec3 nextOrigin, vec3 nextDirection) {
	prd.radiance = vec3(0);
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


vec3 SampleIrrad(float x, float y, float z, vec3 fragPos, vec3 N, vec3 f0, float a) {
	float c = 0;
	c += x;
	c += y * grid.width;
	c += z * grid.width * grid.height;
	int i = int(c) % grid.builtCount;

	vec3 irrPos = grid.firstPos + vec3(x * grid.distToAdjacent, y * grid.distToAdjacent, z * grid.distToAdjacent);

	Aabb aabb = createAabb(irrPos, grid.distToAdjacent);
	
	
	vec3 V = normalize(gl_WorldRayOriginEXT - fragPos);
	vec3 kd = 1.0 - F_SchlickRoughness(saturate(dot(N, V)), f0, a);
	
	vec3 reprojNormal = (fragPos - irrPos) + (fragPos + intersectionDistanceAabb(aabb, fragPos, N) * N);

	return kd * texture(irradianceSampler[nonuniformEXT(i)], normalize(reprojNormal)).rgb
	//	 * saturate(dot(N, irrPos - fragPos));
	;
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

	// INDIRECT Diffuse 
    {
		vec3 probeCount  = vec3(grid.width - 1, grid.height - 1, grid.depth - 1);
		vec3 size = probeCount * grid.distToAdjacent;
	
		vec3 uvw = (surface.position - grid.firstPos) / size; 

		vec3 delim = 1.0 / size; 

		if(!(uvw.x > 1 + delim.x || 
		   uvw.y > 1 + delim.y || 
		   uvw.z > 1 + delim.z ||
		   uvw.x < -delim.x || 
		   uvw.y < -delim.y || 
		   uvw.z < -delim.z)) {

			uvw = saturate(uvw);

			float su = uvw.x * probeCount.x;
			float sv = uvw.y * probeCount.y;
			float sw = uvw.z * probeCount.z;
	
			vec3 FTL = SampleIrrad(floor(su), floor(sv), floor(sw), surface.position, surface.basis.normal, surface.f0, surface.a);
			vec3 FTR = SampleIrrad(ceil (su), floor(sv), floor(sw), surface.position, surface.basis.normal, surface.f0, surface.a);
			vec3 FBL = SampleIrrad(floor(su), ceil (sv), floor(sw), surface.position, surface.basis.normal, surface.f0, surface.a);
			vec3 FBR = SampleIrrad(ceil (su), ceil (sv), floor(sw), surface.position, surface.basis.normal, surface.f0, surface.a);
																					  
			vec3 BTL = SampleIrrad(floor(su), floor(sv), ceil (sw), surface.position, surface.basis.normal, surface.f0, surface.a);
			vec3 BTR = SampleIrrad(ceil (su), floor(sv), ceil (sw), surface.position, surface.basis.normal, surface.f0, surface.a);
			vec3 BBL = SampleIrrad(floor(su), ceil (sv), ceil (sw), surface.position, surface.basis.normal, surface.f0, surface.a);
			vec3 BBR = SampleIrrad(ceil (su), ceil (sv), ceil (sw), surface.position, surface.basis.normal, surface.f0, surface.a);

			float rightPercent = fract(su);
			float bottomPercent = fract(sv);
			float backPercent = fract(sw);

			vec3 topInterpolF  = mix(FTL, FTR, rightPercent);
			vec3 botInterpolF  = mix(FBL, FBR, rightPercent);
			vec3 topInterpolB  = mix(BTL, BTR, rightPercent);
			vec3 botInterpolB  = mix(BBL, BBR, rightPercent);
	
			vec3 frontInt = mix(topInterpolF, botInterpolF, bottomPercent);
			vec3 backInt  = mix(topInterpolB, botInterpolB, bottomPercent);	

			vec3 diffuseLight = mix(frontInt, backInt, backPercent);

			radiance += diffuseLight * surface.albedo;
		}
    }

	if(prd.depth > depth){
		prd.radiance = radiance;
		return;
	}

	// INDIRECT Specular
	{
		vec3 L;
		vec3 brdf_NoL_pdf = SampleSpecularDirection(surface, prd.seed);

		outOnbSpace(surface.basis, surface.wi);
		radiance += RadianceOfRay(surface.position, L) * brdf_NoL_pdf;
	}

	prd.radiance = radiance;
}













