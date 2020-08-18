#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : enable

#include "global.h"
#include "rtshared.h"

struct Attr{
	vec2 x;
};

hitAttributeEXT vec2 baryCoord;

// Texture samplers are 3 samplers per gl_InstanceID: 
// colorSampler, normalSampler, metallicRoughnessSampler
// eg: normal sampler of instance id: 3 would be:
//     (id * 3) + 1 = 10

#define GEOM_GROUPS 25

layout(set = 3, binding = 0) uniform sampler2D textureSamplers[GEOM_GROUPS * 3];
layout(std430, set = 3, binding = 1) readonly buffer Vertices{ Vertex v[]; } vertices;
layout(std430, set = 3, binding = 2) readonly buffer Indicies{ uint i[]; } indices;
layout(std430, set = 3, binding = 3) readonly buffer IndexOffsets { uint offset[]; } indexOffsets;
layout(std430, set = 3, binding = 4) readonly buffer PrimitveOffsets { uint offset[]; } primOffsets;

//layout(set = 1, binding = 0) uniform accelerationStructureEXT topLevelAs;

//HitAttributeKHR vec2 attribs;

layout(location = 0) rayPayloadInEXT hitPayload prd;


OldVertex fromVertex(Vertex p) {
	OldVertex vtx;
	
	vtx.position = vec3(p.posX, p.posY, p.posZ);
	vtx.normal = vec3(p.nrmX, p.nrmY, p.nrmZ);
	vtx.tangent = vec3(p.tngX, p.tngY, p.tngZ);
	
	vtx.uv = vec2(p.u, p.v);
	
	return vtx;
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
	// TODO: Transforming the normal to world space
	// normal = normalize(vec3(scnDesc.i[gl_InstanceID].transfoIT * vec4(normal, 0.0)));


	// Computing the coordinates of the hit position
	vec3 worldPos = v0.position * barycentrics.x + v1.position * barycentrics.y + v2.position * barycentrics.z;
	// TODO: Transforming the position to world space


	vec3 color = texture(textureSamplers[matId * 3], uv).xyz;
	vec3 normalTxt = texture(textureSamplers[matId * 3 + 1], uv).xyz;
	float metallic = texture(textureSamplers[matId * 3 + 2], uv).y;
	float roughness = texture(textureSamplers[matId * 3 + 2], uv).z;


	prd.hitValue = vec4(vec3(color), 1);	
}











