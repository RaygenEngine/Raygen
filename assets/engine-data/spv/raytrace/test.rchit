#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable
#include "global.glsl"
#include "rt-global.glsl"

#include "random.glsl"
#include "sampling.glsl"
#include "bsdf.glsl"
#include "onb.glsl"

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

layout(location = 0) rayPayloadInEXT hitPayload inPrd;

OldVertex fromVertex(Vertex p) {
	OldVertex vtx;
	
	vtx.position = vec3(p.posX, p.posY, p.posZ);
	vtx.normal = vec3(p.nrmX, p.nrmY, p.nrmZ);
	vtx.tangent = vec3(p.tngX, p.tngY, p.tngZ);
	
	vtx.uv = vec2(p.u, p.v);
	
	return vtx;
}

vec3 Contribution(vec3 throughput, vec3 nextOrigin, vec3 nextDirection)
{
	// RR termination
	vec3 cumulThroughput = inPrd.throughput * throughput;

	float p_spawn = max(throughput.x, max(throughput.y, throughput.z));

	if(rand(inPrd.seed) > p_spawn){
		return vec3(0); 
	}
	throughput /= p_spawn;
	
	hitPayload prevRay = inPrd;

	inPrd.radiance = vec3(0);
	inPrd.throughput = cumulThroughput / p_spawn;
	inPrd.depth = inPrd.depth + 1;
    inPrd.seed = inPrd.seed;

    uint  rayFlags = gl_RayFlagsOpaqueEXT | gl_RayFlagsCullFrontFacingTrianglesEXT  ;
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
	prevRay.seed = inPrd.seed;
	vec3 result = throughput * inPrd.radiance;
	inPrd = prevRay;

	return result;
}

void main() 
{

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
	float a = max(0.001, roughness * roughness);

	vec3 hitPoint = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;

	// LMATH: unit?
	vec3 V = -gl_WorldRayDirectionEXT;

	toOnbSpace(shadingOrthoBasis, V);

	// same hemisphere

	float NoV = max(Ndot(V), BIAS);
	
	

	// DIRECT
	vec3 radiance = vec3(0);

	// for each light
	for(int i = 0; i < spotlightCount; ++i) 
	{
		Spotlight light = spotlights.light[i];
		vec3 lightPos = light.position;

		vec3 L = normalize(lightPos - hitPoint);
		toOnbSpace(shadingOrthoBasis, L);

		float NoL = max(Ndot(L), BIAS);

		vec3 lightColor = light.color;
		float dist = length(light.position - hitPoint);
		float attenuation = 1.0 / (light.constantTerm + light.linearTerm * dist + 
	  			     light.quadraticTerm * (dist * dist));
		float lightIntensity = light.intensity;
		
		vec3 ld = light.direction;
		toOnbSpace(shadingOrthoBasis, ld);

		// spot effect (soft edges)
		float theta = dot(L, -ld);
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
	
			vec3 H = normalize(V + L);
			float NoH = max(Ndot(H), BIAS); 
			float LoH = max(dot(L, H), BIAS);

			vec3 brdf_d = DisneyDiffuse(NoL, NoV, LoH, a, diffuseColor);
			vec3 brdf_r = SpecularTerm(NoL, NoV, NoH, LoH, a, f0);

			// so to simplify (faster math)
			// throughput = (brdf_d + brdf_r) * NoL
			// incoming radiance = Li;
			vec3 finalContribution = (brdf_d + brdf_r) * Li * NoL;

			radiance += finalContribution;
		}
	}
	
	uint totalSamples = convergeUntilFrame != 0 ? convergeUntilFrame : 1024u;
	

	if(inPrd.depth + 1 > depth){
		return;
	}

	// INDIRECT

	// TRANSMISSION

	// Diffuse 'reflection'

//	{
//        vec2 u = rand2(inPrd.seed); 
//        vec3 L = cosineSampleHemisphere(u);
//
//		float NoL = max(Ndot(L), BIAS); 
//
//        vec3 H = normalize(V + L);
//        float LoH = max(dot(L, H), BIAS);
//
//        float pdf = NoL * INV_PI;
//    
//        vec3 brdf_d = DisneyDiffuse(NoL, NoV, LoH, a, diffuseColor);
//
//	    outOnbSpace(shadingOrthoBasis, L);
//        radiance += Contribution(brdf_d * NoL / pdf, hitPoint, L);
//	}
//
//	// REFLECTION
//	{
//        vec2 u = rand2(inPrd.seed); 
//        vec3 H = importanceSampleGGX(u, a);
//
//        vec3 L = reflect(-V, H);
//
//        float NoL = max(Ndot(L), BIAS); 
//
//		float NoH = max(Ndot(H), BIAS); 
//		float LoH = max(dot(L, H), BIAS);
//
//        // SMATH:
//        float pdf = D_GGX(NoH, a) * NoH /  (4.0 * LoH);
//        pdf = max(Ndot(H), BIAS); 
//
//        vec3 brdf_r = SpecularTerm(NoL, NoV, NoH, LoH, a, f0);
//
//	    outOnbSpace(shadingOrthoBasis, L);
//        radiance += Contribution(brdf_r * NoL / pdf, hitPoint, L);
//	}
	

	inPrd.radiance = radiance;
}








