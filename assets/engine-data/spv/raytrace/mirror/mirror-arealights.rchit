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

#include "global-descset.glsl"

#include "aabb.glsl"
#include "bsdf.glsl"
#include "lights/quadlight.glsl"
#include "onb.glsl"
#include "random.glsl"
#include "sampling.glsl"
#include "surface.glsl"

struct hitPayload
{
	vec3 radiance;
	int depth;
};

layout(push_constant) uniform PC
{
	int depth;
	int pointlightCount;
	int spotlightCount;
	int dirlightCount;
	int irragridCount;
	int quadlightCount;
};

hitAttributeEXT vec2 baryCoord;
layout(location = 0) rayPayloadInEXT hitPayload prd;

layout(set = 8, binding = 0, std430) readonly buffer Quadlights { Quadlight light[]; } quadlights;

void main() {

	int quadId = gl_InstanceCustomIndexEXT;

	Quadlight ql = quadlights.light[nonuniformEXT(quadId)];

	vec3 hitpoint = gl_WorldRayOriginEXT + gl_HitTEXT * gl_WorldRayDirectionEXT;
	
	float dist = distance(hitpoint, gl_WorldRayOriginEXT);
	float attenuation = 1.0 / (ql.constantTerm + ql.linearTerm * dist + 
  			     ql.quadraticTerm * (dist * dist));

	float pdf_area = 1.0 / (ql.width * ql.height);
	const float mirrorPdf = 1.0;

	vec3 Le = ql.color * ql.intensity * attenuation / (pdf_area + mirrorPdf); 

	prd.radiance = Le;
}
