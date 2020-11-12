#version 460 
#extension GL_GOOGLE_include_directive: enable
#extension GL_EXT_nonuniform_qualifier : enable
#include "global.glsl"

#include "attachments.glsl"
#include "aabb.glsl"
#include "fresnel.glsl"
#include "surface.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout (location = 0) in vec2 uv;

// uniform

layout(set = 1, binding = 0) uniform UBO_Camera { Camera cam; };
layout(set = 2, binding = 0) uniform UBO_Irragrid { Irragrid grid; };
layout(set = 3, binding = 0) uniform samplerCube irradianceSampler[];

vec3 SampleIrrad(float x, float y, float z, vec3 fragPos, vec3 N, vec3 f0, float a) {
	float c = 0;
	c += x;
	c += y * grid.width;
	c += z * grid.width * grid.height;
	int i = int(c) % grid.builtCount;

	vec3 irrPos = grid.firstPos + vec3(x * grid.distToAdjacent, y * grid.distToAdjacent, z * grid.distToAdjacent);

	Aabb aabb = createAabb(irrPos, grid.distToAdjacent);

	vec3 reprojN = (fragPos - irrPos) + (fragPos + intersectionDistanceAabb(aabb, fragPos, N) * N);

	vec3 V = normalize(cam.position - fragPos);
	// SMATH: which normal
	vec3 kd = 1.0 - F_SchlickRoughness(saturate(dot(N, V)), f0, a);

	return kd * texture(irradianceSampler[nonuniformEXT(i)], normalize(reprojN)).rgb
	//	 * saturate(dot(N, irrPos - fragPos));
	;
}

void main( ) {

	Surface surface = surfaceFromGBuffer(
	    cam,
		g_DepthSampler,
		g_NormalSampler,
		g_AlbedoSampler,
		g_SpecularSampler,
		g_EmissiveSampler,
		uv
	);
	
	vec3 N = surface.basis.normal;

	vec3 probeCount  = vec3(grid.width - 1, grid.height - 1, grid.depth - 1);
	vec3 size = probeCount * grid.distToAdjacent;
	
	vec3 uvw = (surface.position - grid.firstPos) / size; 

	vec3 delim = 1.0 / size; 
	

	if(uvw.x > 1 + delim.x || 
	   uvw.y > 1 + delim.y || 
	   uvw.z > 1 + delim.z ||
	   uvw.x < -delim.x || 
	   uvw.y < -delim.y || 
	   uvw.z < -delim.z) {
		discard; // WIP: matrix based volume
	}
	
	uvw = saturate(uvw);

	// SMATH: interpolation
	float su = uvw.x * probeCount.x;
	float sv = uvw.y * probeCount.y;
	float sw = uvw.z * probeCount.z;
	
	vec3 FTL = SampleIrrad(floor(su), floor(sv), floor(sw), surface.position, N, surface.f0, surface.a);
	vec3 FTR = SampleIrrad(ceil (su), floor(sv), floor(sw), surface.position, N, surface.f0, surface.a);
	vec3 FBL = SampleIrrad(floor(su), ceil (sv), floor(sw), surface.position, N, surface.f0, surface.a);
	vec3 FBR = SampleIrrad(ceil (su), ceil (sv), floor(sw), surface.position, N, surface.f0, surface.a);
															
	vec3 BTL = SampleIrrad(floor(su), floor(sv), ceil (sw), surface.position, N, surface.f0, surface.a);
	vec3 BTR = SampleIrrad(ceil (su), floor(sv), ceil (sw), surface.position, N, surface.f0, surface.a);
	vec3 BBL = SampleIrrad(floor(su), ceil (sv), ceil (sw), surface.position, N, surface.f0, surface.a);
	vec3 BBR = SampleIrrad(ceil (su), ceil (sv), ceil (sw), surface.position, N, surface.f0, surface.a);

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

	outColor = vec4(diffuseLight * surface.albedo, 1.0);
}
