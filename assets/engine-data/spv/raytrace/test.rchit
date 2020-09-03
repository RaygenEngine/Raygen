#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_ray_query: enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "global.h"
#include "rt-global.h"
#include "random.h"
#include "sampling.h"
#include "shading-space.h"
#include "bsdf.h"
#include "onb.h"

struct Attr{
	vec2 x;
};

hitAttributeEXT vec2 baryCoord;

// Texture samplers are 3 samplers per gl_InstanceID: 
// colorSampler, normalSampler, metallicRoughnessSampler
// eg: normal sampler of instance id: 3 would be:
//     (id * 3) + 1 = 10

#define GEOM_GROUPS 25
     
layout(set = 3, binding = 0) uniform accelerationStructureEXT topLevelAs;

layout(set = 4, binding = 0) uniform sampler2D textureSamplers[GEOM_GROUPS * 3];
layout(std430, set = 4, binding = 1) readonly buffer Vertices{ Vertex v[]; } vertices;
layout(std430, set = 4, binding = 2) readonly buffer Indicies{ uint i[]; } indices;
layout(std430, set = 4, binding = 3) readonly buffer IndexOffsets { uint offset[]; } indexOffsets;
layout(std430, set = 4, binding = 4) readonly buffer PrimitveOffsets { uint offset[]; } primOffsets;
layout(std430, set = 4, binding = 5) readonly buffer Spotlights{ Spotlight light[]; } spotlights;
layout(set = 4, binding = 6) uniform sampler2DShadow spotlightShadowmap[16];
//HitAttributeKHR vec2 attribs;

layout(location = 0) rayPayloadInEXT hitPayload inPrd;
layout(location = 1) rayPayloadEXT hitPayload prd;


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

		if(rand(prd.seed) >= p_spawn){
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
	

	vec3 facen = normalize(cross(v1.position - v0.position, v2.position - v0.position));


	vec3 Tg = normalize(tangent);
   	vec3 Ng = normalize(normal);

	// Gram-Schmidt process + cross product
	// re-orthogonalize T with respect to N
	Tg = normalize(Tg - dot(Tg, Ng) * Ng);
	// then retrieve perpendicular vector B with the cross product of T and N
	vec3 Bg = cross(Ng, Tg);

	mat3 TBN = mat3(Tg, Bg, Ng);

	// sample material textures
	vec4 sampledBaseColor = texture(textureSamplers[matId * 3], uv);
	vec4 sampledNormal = texture(textureSamplers[matId * 3 + 1], uv);
	vec4 sampledMetallicRoughness = texture(textureSamplers[matId * 3 + 2], uv); 

	vec3 Ns = normalize(TBN * (sampledNormal.rgb * 2.0 - 1.0));

	Onb shadingOrthoBasis = branchlessOnb(Ns);


	// final material values
	vec3 albedo = sampledBaseColor.rgb;
	float metallic = sampledMetallicRoughness.b;
	float roughness = sampledMetallicRoughness.g;
	float reflectance = 0.5f;

    // remapping

	// diffuseColor = (1.0 - metallic) * albedo;
    vec3 diffuseColor = (1.0 - metallic) * albedo;
    // f0 = 0.16 * reflectance * reflectance * (1.0 - metallic) + albedo * metallic;
	vec3 f0 =  vec3(0.16 * reflectance * reflectance * (1.0 - metallic)) + albedo * metallic;
    // a = roughness roughness;
	float a = roughness * roughness;

	vec3 hitPoint = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;

	// LMATH: unit?
	vec3 wo = -gl_WorldRayDirectionEXT;

	vec3 Ng_s = Ng;
	if(wo.z < 0){
		if(dot(wo, facen) < 0){	
			//inPrd.radiance = vec3(1000.f,0,1000.f); TODO
			return; // backface culling
		}
		Ng_s = facen;
	}

	toOnbSpace(shadingOrthoBasis, wo);
	toOnbSpace(shadingOrthoBasis, Ng_s);

	// DIRECT

	inPrd.radiance = vec3(0);
	// for each light
	for(int i = 0; i < spotlightCount; ++i) 
	{
		Spotlight light = spotlights.light[i];
		vec3 lightPos = light.position;

		
		vec3 wi = normalize(lightPos - hitPoint);
		toOnbSpace(shadingOrthoBasis, wi);

		bool reflect = dot(Ng_s, wi) * dot(Ng_s, wo) > 0;
		
		float cosTheta = CosTheta(wi);

		if (reflect && cosTheta > 0)
		{
			vec3 lightColor = light.color;
			float dist = length(light.position - hitPoint);
			float attenuation = 1.0 / (light.constantTerm + light.linearTerm * dist + 
		  			     light.quadraticTerm * (dist * dist));
			float lightIntensity = light.intensity;
			
			// spot effect (soft edges)
			float theta = dot(L, -light.direction);
		    float epsilon = (light.innerCutOff - light.outerCutOff);
		    float spotEffect = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
			

			
			vec3 lightFragColor = lightColor * lightIntensity * attenuation * spotEffect;

			// Only sample the shadowmap if this fragment is lit
			if (lightFragColor.x + lightFragColor.y + lightFragColor.z > 0.001) {
				float shadow = 0.f; //...
	
				vec4 fragPosLightSpace = light.viewProj * vec4(hitPoint, 1.0);
			 	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

				projCoords = vec3(projCoords.xy * 0.5 + 0.5, projCoords.z - light.maxShadowBias);
				float bias = light.maxShadowBias;
			
			    shadow = 1.0 - texture(spotlightShadowmap[i], projCoords);

				vec3 Li = (1.0 - shadow) * lightFragColor; 
		
				// to get final diffuse and specular both those terms are multiplied by Li * NoL
				vec3 brdf_d = LambertianReflection(diffuseColor);
				vec3 brdf_r = MicrofacetReflection(wo, wi, a, a, f0);
	
				// so to simplify (faster math)
				// throughput = (brdf_d + brdf_r) * NoL
				// incoming radiance = Li;
				vec3 finalContribution = (brdf_d + brdf_r) * Li * cosTheta;
	
				inPrd.radiance += finalContribution;
			}
		}
	}

	// INDIRECT

	// TRANSMISSION

	// Diffuse 'reflection'
	{
		vec2 u = rand2(inPrd.seed);
		vec3 wi = cosineSampleHemisphere(u);

		bool reflect = dot(Ng_s, wi) * dot(Ng_s, wo) > 0;

		float cosTheta = CosTheta(wi);

		if(reflect && cosTheta > 0)
		{
			float pdf = cosTheta * INV_PI;
		
			vec3 throughput = LambertianReflection(diffuseColor) * cosTheta / pdf;

			outOnbSpace(shadingOrthoBasis, wi);
			RRTerminateOrTraceRay(hitPoint, wi, throughput);
		}
	}
	
	// REFLECTION
	{
		// sample new direction wi and its pdfs based on distribution of the GGX BRDF
		vec2 u = rand2(inPrd.seed);
		vec3 wh  = TrowbridgeReitzDistribution_Sample_wh(wo, u, a, a);

		vec3 wi  = reflect(wo, wh);

		bool reflect = dot(wi, Ng_s) * dot(wo, Ng_s) > 0;
	
		float cosTheta = CosTheta(wi);

		if(reflect && cosTheta > 0)
		{
			vec3 brdf_r = MicrofacetReflection(wo, wi, a, a, f0);

			float pdf = TrowbridgeReitzSamplePdf(wo, wh, a, a) / (4 * dot(wo, wh));

			vec3 throughput = brdf_r * cosTheta  / pdf;

			outOnbSpace(shadingOrthoBasis, wi);
			RRTerminateOrTraceRay(hitPoint, wi, throughput);
		}
	}
}
