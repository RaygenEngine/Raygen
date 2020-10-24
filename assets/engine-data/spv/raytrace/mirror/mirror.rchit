#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_EXT_ray_query: require

#include "global.glsl"
#include "raytrace/mirror/mirror.glsl"

#include "sampling.glsl"
#include "bsdf.glsl"
#include "onb.glsl"
#include "attachments.glsl"

hitAttributeEXT vec2 baryCoord;
layout(location = 0) rayPayloadInEXT hitPayload prd;

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

layout(set = 6, binding = 0, std430) readonly buffer Reflprobes { Reflprobe reflprobe[]; } reflprobes;
layout(set = 6, binding = 1) uniform samplerCube relfprobeTextures[];


vec3 RadianceOfRay2(vec3 nextOrigin, vec3 nextDirection) {
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

float ShadowRayQuery(vec3 lightPos, vec3 fragPos){ 
	vec3  L = normalize(lightPos - fragPos); 
	vec3  origin    = fragPos;
	vec3  direction = L;  // vector to light
	float tMin      = 0.01f;
	float tMax      = distance(fragPos, lightPos);

	// Initializes a ray query object but does not start traversal
	rayQueryEXT rayQuery;
	rayQueryInitializeEXT(rayQuery, topLevelAs, gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, origin, tMin,
                      direction, tMax);

	// Start traversal: return false if traversal is complete
	while(rayQueryProceedEXT(rayQuery)) {
	}
      
	// Returns type of committed (true) intersection
	if(rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionNoneEXT) {
	  // Got an intersection == Shadow
	  return 1.0;
	}
	return 0.0;
}

void main() {
	
	int matId = gl_InstanceID;

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
	vec4 sampledBaseColor = texture(mat.baseColor, uv) * mat.baseColorFactor;
	vec4 sampledNormal = texture(mat.normal, uv) * mat.normalScale;
	vec4 sampledMetallicRoughness = texture(mat.metallicRough, uv);
	vec4 sampledEmissive = texture(mat.emissive, uv) * mat.emissiveFactor;
	
	vec3 Ns = normalize(TBN * (sampledNormal.rgb * 2.0 - 1.0));

	// final material values
	vec3 baseColor = sampledBaseColor.rgb;
	float metallic = sampledMetallicRoughness.b * mat.metallicFactor;
	float roughness = sampledMetallicRoughness.g * mat.roughnessFactor;
	float reflectance = 0.5;

    // remapping
	FragBrdfInfo brdfInfo;
    brdfInfo.albedo = (1.0 - metallic) * baseColor;
	brdfInfo.f0 =  vec3(0.16 * reflectance * reflectance * (1.0 - metallic)) + baseColor * metallic;
	brdfInfo.a = roughness * roughness;

	vec3 hitPoint = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;

	FsSpaceInfo fragSpace = GetFragSpace_World(Ns, hitPoint, gl_WorldRayOriginEXT);

	vec3 radiance = vec3(0);
	// OLD DIRECT
	{
		// TODO: Refactor
		vec3 V = -gl_WorldRayDirectionEXT;
		float a = brdfInfo.a;

		Onb shadingOrthoBasis = branchlessOnb(Ns);
		toOnbSpace(shadingOrthoBasis, V);

		// same hemisphere
		float NoV = max(Ndot(V), BIAS);

		if (sum(sampledEmissive.xyz) > BIAS) {
			prd.radiance = sampledEmissive.xyz;
			return;
		}
		
		// DIRECT

		// for each light
		for(int i = 0; i < pointlightCount; ++i) {
			Pointlight light = pointlights.light[i];
			vec3 lightPos = light.position;

			vec3 L = normalize(lightPos - hitPoint);
			toOnbSpace(shadingOrthoBasis, L);

			float NoL = max(Ndot(L), BIAS);

			vec3 lightColor = light.color;
			float dist = length(light.position - hitPoint);
			float attenuation = 1.0 / (light.constantTerm + light.linearTerm * dist + 
						light.quadraticTerm * (dist * dist));
			float lightIntensity = light.intensity;
			
			
			vec3 lightFragColor = lightColor * lightIntensity * attenuation;

			// Only sample the shadowmap if this fragment is lit
			if (sum(lightFragColor) > 0.001) {
				float shadow = ShadowRayQuery(lightPos, hitPoint);

				vec3 Li = (1.0 - shadow) * lightFragColor; 
		
				vec3 H = normalize(V + L);
				float NoH = max(Ndot(H), BIAS); 
				float LoH = max(dot(L, H), BIAS);

				vec3 finalContribution = DirectLightBRDF(NoL, NoV, NoH, LoH, brdfInfo.a, brdfInfo.albedo, brdfInfo.f0)  * Li * NoL;

				radiance += finalContribution;
			}
		}
	}

	if(prd.depth > mirrorDepth){
		prd.radiance = radiance;
		return;
	}

	vec3 V = -gl_WorldRayDirectionEXT;
	vec3 N = Ns;
	float NoV = max(dot(N, V), BIAS);

	vec3 brdfLut = (texture(std_BrdfLut, vec2(NoV, brdfInfo.a))).rgb;

	// next mirror event
	if(brdfInfo.a < SPEC_THRESHOLD){
		// diffuse
		for(int i = 0; i < reflprobeCount; ++i){
			Reflprobe reflprobe = reflprobes.reflprobe[i];

			vec3 diffuseLight =  texture(relfprobeTextures[nonuniformEXT(i)], N).rgb; // irradiance

			vec3 diffuse = diffuseLight * brdfInfo.albedo;

			radiance += diffuse;
		}

		// specular
		vec3 L = normalize(reflect(-V, N));
		vec3 specularLight = RadianceOfRay2(hitPoint, L);

		vec3 specular = specularLight * (brdfInfo.f0 * brdfLut.x + brdfLut.y);

		radiance += specular;
	}
	// sample appropriate reflprobe
	else{
		for(int i = 0; i < reflprobeCount; ++i){
			Reflprobe reflprobe = reflprobes.reflprobe[i];

			vec3 R = normalize(reflect(-V, N));

			float NoV = abs(dot(N, V)) + 1e-5;
	
			const float MAX_REFLECTION_LOD = 4.0;
			// SMATH: which roughness should go here
			float lod = (brdfInfo.a * MAX_REFLECTION_LOD); 
			
			// SMATH: math of those
			vec3 diffuseLight =  texture(relfprobeTextures[nonuniformEXT(i)], N).rgb; // irradiance
			vec3 specularLight = textureLod(relfprobeTextures[nonuniformEXT(i+1)], R, lod).rgb; // prefiltered

			vec3 diffuse = diffuseLight * brdfInfo.albedo;
			vec3 specular = specularLight * (brdfInfo.f0 * brdfLut.x + brdfLut.y);

			vec3 iblContribution = diffuse + specular;

			radiance += iblContribution;
		}
	}

	prd.radiance = radiance;
}










