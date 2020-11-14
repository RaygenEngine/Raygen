#ifndef irragrid_glsl
#define irragrid_glsl

#include "aabb.glsl"
#include "bsdf.glsl"
#include "fresnel.glsl"
#include "onb.glsl"
#include "random.glsl"
#include "sampling.glsl"
#include "surface.glsl"

vec3 SampleIrrad(Irragrid grid, Surface surface, float x, float y, float z) 
{
	float c = 0;
	c += x;
	c += y * grid.width;
	c += z * grid.width * grid.height;
	int i = int(c) % grid.builtCount;

	vec3 irrPos = grid.firstPos + vec3(x * grid.distToAdjacent, y * grid.distToAdjacent, z * grid.distToAdjacent);

	Aabb aabb = createAabb(irrPos, grid.distToAdjacent);

	vec3 N = surface.basis.normal;
	vec3 reprojN = (surface.position - irrPos) + (surface.position + intersectionDistanceAabb(aabb, surface.position, N) * N);

	// SMATH: which normal
	vec3 kd = 1.0 - F_SchlickRoughness(surface.nov, surface.f0, surface.a);

	return kd * texture(irradianceSamplers[nonuniformEXT(i)], normalize(reprojN)).rgb
	//	 * saturate(dot(N, irrPos - surface.position));
	;
}

vec3 Irragrid_Contribution(Irragrid grid, Surface surface)
{
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
		return vec3(0); // WIP: matrix based volume
	}
	
	uvw = saturate(uvw);

	// SMATH: interpolation
	float su = uvw.x * probeCount.x;
	float sv = uvw.y * probeCount.y;
	float sw = uvw.z * probeCount.z;
	
	vec3 FTL = SampleIrrad(grid, surface, floor(su), floor(sv), floor(sw));
	vec3 FTR = SampleIrrad(grid, surface, ceil (su), floor(sv), floor(sw));
	vec3 FBL = SampleIrrad(grid, surface, floor(su), ceil (sv), floor(sw));
	vec3 FBR = SampleIrrad(grid, surface, ceil (su), ceil (sv), floor(sw));
	 				  
	vec3 BTL = SampleIrrad(grid, surface, floor(su), floor(sv), ceil (sw));
	vec3 BTR = SampleIrrad(grid, surface, ceil (su), floor(sv), ceil (sw));
	vec3 BBL = SampleIrrad(grid, surface, floor(su), ceil (sv), ceil (sw));
	vec3 BBR = SampleIrrad(grid, surface, ceil (su), ceil (sv), ceil (sw));

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

	return diffuseLight * surface.albedo;
}

#endif
