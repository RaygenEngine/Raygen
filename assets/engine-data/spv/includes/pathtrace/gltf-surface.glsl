#ifndef pt_surface_glsl
#define pt_surface_glsl
// META: set=3 is used for geometry groups

// TODO: this is a temporary boilerplate header for offset 0 rchit shaders because all of them currently use the GltfMaterial

#include "surface.glsl"

struct Vertex {
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

struct OldVertex {
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
	float metalnessFactor;
	float roughnessFactor;
	float normalScale;
	float occlusionStrength;
	float baseReflectivity;

	float alphaCutoff;
	int alphaMode;

	bool doubleSided;

	samplerRef baseColor;
	samplerRef metalnessRough;
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

OldVertex fromVertex(Vertex p) 
{
	OldVertex vtx;
	
	vtx.position = vec3(p.posX, p.posY, p.posZ);
	vtx.normal = vec3(p.nrmX, p.nrmY, p.nrmZ);
	vtx.tangent = vec3(p.tngX, p.tngY, p.tngZ);
	
	vtx.uv = vec2(p.u, p.v);
	
	return vtx;
}

hitAttributeEXT vec2 baryCoord;
layout(set = 3, binding = 0, std430) readonly buffer GeometryGroups { GeometryGroup g[]; } geomGroups;
layout(set = 3, binding = 1) uniform sampler2D textureSamplers[];

vec4 texture(samplerRef s, vec2 uv) 
{
	return texture(textureSamplers[nonuniformEXT(s.index)], uv);
}

bool surfaceIgnoreIntersectionTest() 
{
	int matId = gl_InstanceCustomIndexEXT;

	GeometryGroup gg = geomGroups.g[nonuniformEXT(matId)];

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
		
	GltfMat mat = gg.materialUbo.m;

	// sample material textures
	vec4 sampledBaseColor = texture(mat.baseColor, uv) * mat.baseColorFactor;
	
	float opacity = sampledBaseColor.a;

	return mat.alphaMode == ALPHA_MODE_MASK && opacity <= mat.alphaCutoff;
}

Surface surfaceFromGeometryGroup() 
{
	int matId = gl_InstanceCustomIndexEXT;

	GeometryGroup gg = geomGroups.g[nonuniformEXT(matId)];

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
	vec4 sampledMetalnessRoughness = texture(mat.metalnessRough, uv); //
	vec4 sampledEmissive = texture(mat.emissive, uv) * mat.emissiveFactor;
	vec3 N = normalize(TBN * (sampledNormal.rgb * 2.0 - 1.0));

	// use geometric normal if this one is broken for any reason
    if(isnan(N.x) || isnan(N.y) || isnan(N.z)){
    	N = Ng;
    }

	vec3 baseColor = sampledBaseColor.rgb;
	float metalness = sampledMetalnessRoughness.b * mat.metalnessFactor;
	float roughness = sampledMetalnessRoughness.g * mat.roughnessFactor;

	vec3 V = normalize(-gl_WorldRayDirectionEXT);

	Surface surface;


	// Material stuff
	surface.albedo = mix(baseColor, vec3(0.0), metalness);
    surface.opacity = mat.alphaMode == ALPHA_MODE_BLEND ? sampledBaseColor.a : 1.0f; // if alphaMode is OPAQUE, ignore opacity

	surface.f0 = mix(vec3(mat.baseReflectivity), baseColor, metalness);
	surface.a = roughness * roughness;

    surface.emissive = sampledEmissive.rgb;
    surface.occlusion = sampledEmissive.a;		
	
	// Geometric stuff
	float f0 = max(surface.f0);
	float sqrtf0 = sqrt(f0);
	float eta = -(f0 + 1 + 2 * sqrtf0) / (f0 - 1);

	// Backwards tracing:
	surface.eta_i = 1.0; // vacuum view ray
	surface.eta_o = eta; // material light ray
	
	// from the inside of object
	if(dot(V, Ng) < 0) { 
		Ng = -Ng;
		N = -N;
		
		surface.eta_o = 1.0; // vacuum light ray
		surface.eta_i = eta; // material view ray
	}
		
	surface.position = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;

	addInitialVectors(surface, Ng, N, V);

    return surface;
}

#endif
