#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : enable

#include "global.h"
#include "rtshared.h"
#include "bsdf.h"
#include "hammersley.h"

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

vec3 randomCosineWeightedHemispherePoint(vec2 rand, vec3 n, vec3 t, vec3 b) {
  float r = rand.x * 0.5 + 0.5; // [-1..1) -> [0..1)
  float angle = (rand.y + 1.0) * PI; // [-1..1] -> [0..2*PI)
  float sr = sqrt(r);
  vec2 p = vec2(sr * cos(angle), sr * sin(angle));
  /*
   * Unproject disk point up onto hemisphere:
   * 1.0 == sqrt(x*x + y*y + z*z) -> z = sqrt(1.0 - x*x - y*y)
   */
  vec3 ph = vec3(p.xy, sqrt(1.0 - p*p));
  /*
   * Compute some arbitrary tangent space for orienting
   * our hemisphere 'ph' around the normal. We use the camera's up vector
   * to have some fix reference vector over the whole screen.
   */

  /* Make our hemisphere orient around the normal. */
  return t * ph.x + b * ph.y + n * ph.z;
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
	float reflectance = 0.5;


    // remapping

	// diffuseColor = (1.0 - metallic) * albedo;
    vec3 diffuseColor = (1.0 - metallic) * albedo;
    // f0 = 0.16 * reflectance * reflectance * (1.0 - metallic) + albedo * metallic;
	vec3 f0 = vec3(0.16 * reflectance * reflectance * (1.0 - metallic)) + albedo * metallic;
    // a = roughness roughness;
	float a = roughness * roughness;

	vec3 hitPoint = prd.origin + gl_HitTEXT * prd.direction;

	vec3 N = normalize(TBN * (sampledNormal.rgb* 2.0 - 1.0));
	vec3 V = normalize(prd.origin - hitPoint);

	float NoV = abs(dot(N, V)) + 1e-5; 

	//if(prd.depth == 0)
	//{
	//	prd.debug = N;
	//	prd.debugDone = true;
	//	return;
	//}

	// Direct Light

	// for each light
	{
		vec3 lightPos = vec3(0,35,0);
		vec3 lightColor = vec3(1,1,1);
		float lightIntensity = 3;

		//vec3 N = normal;
		//vec3 V = normalize(prd.origin - hitPoint);
		vec3 L = normalize(lightPos - hitPoint); 
		
		// attenuation
		float dist = length(lightPos - hitPoint);
		float attenuation = 1.0; // TODO: fix with coefs
		
		float NoL = saturate(dot(N, L));

		// if you are gonna trace this ray 
		// check dot(N,L) > 0 
		float shadow = 0.f; //...

		// sample shadowmap for shadow
		vec3 Li = (1.0 - shadow) * lightColor * lightIntensity * attenuation; 
	
		vec3 H = normalize(V + L);


		float NoH = saturate(dot(N, H));
		float LoH = saturate(dot(L, H));

		// to get final diffuse and specular both those terms are multiplied by Li * NoL
		vec3 brdf_d = Fd_Burley(NoV, NoL, LoH, diffuseColor, a);
		vec3 brdf_r = Fr_CookTorranceGGX(NoV, NoL, NoH, LoH, f0, a);

		// so to simplify (faster math)
		vec3 finalContribution = (brdf_d + brdf_r) * Li * NoL;

		prd.radiance += prd.throughput * finalContribution;
	}
	// Indirect Light

	// NEXT: something is wrong here
		vec3 F = F_Schlick(NoV, f0);

		// incoming light (wi)
		vec3 L = vec3(0);

		

		// TRANSMISSION
		if(rnd(prd.seed) > ((F.x + F.y + F.z) / 3))
		{
			// SCATTERING Depends on albedo and transparency

				// DIFFUSE "REFLECTION"

					//Sample a cosine weighted hemisphere for a new direction wi with pdfd

					vec2 Xi = hammersley(prd.seed, 1024);
					vec3 p = randomCosineWeightedHemispherePoint(Xi, Ng, Tg, Bg); // uses geometric surface
					L = p - hitPoint;

					vec3 H = normalize(V + L);

					float LoH = saturate(dot(L, H));
					float NoL = saturate(dot(N, L));

					vec3 brdf_d = Fd_Burley(NoV, NoL, LoH, diffuseColor, a);

					float pdf_d = NoL / PI;

					prd.throughput *= brdf_d * NoL / pdf_d; // try to simplify based on brdf
					
				// TODO: SUBSURFACE SCATTERING else if

				// TODO: SPECULAR TRANSMISSION (i.e refraction) else if

			// TODO: ABSORPTION
				// prd.done = true;
		}
		
		// REFLECTION
		else
		{
			// sample new direction wi and its pdfs based on distribution of the GGX BRDF

			vec2 Xi = hammersley(prd.seed, 1024);
       	 	vec3 H  = importanceSampleGGX(Xi, a, N);

			// TODO: use reflect
			L  = normalize(2.0 * dot(V, H) * H - V);
      
			float NoL = saturate(dot(N, L));

			//if(NoL > 0.0) PERF
			//{
			float NoH = saturate(dot(N, H));        
			float HoV = saturate(dot(H, V)); 
			float LoH = saturate(dot(L, H));        

			float D = D_GGX(NoH, a); 
			float pdf = (D * NoH / (4 * HoV)) + 0.0001;
			
			//}
			vec3 brdf_r = Fr_CookTorranceGGX(NoV, NoL, NoH, LoH, f0, a);

			prd.throughput = brdf_r * NoL / pdf;
		}

		prd.origin = hitPoint;
		prd.direction = L;
	
}












