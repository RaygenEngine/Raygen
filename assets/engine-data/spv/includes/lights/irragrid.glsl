#ifndef irragrid_glsl
#define irragrid_glsl

#include "aabb.glsl"
#include "bsdf.glsl"
#include "fresnel.glsl"
#include "onb.glsl"
#include "random.glsl"
#include "sampling.glsl"
#include "surface.glsl"

vec3 SampleIrrad(Irragrid grid, samplerCubeArray irradianceSamplers, Surface surface, float x, float y, float z) 
{
	float c = 0;
	c += x;
	c += y * grid.width;
	c += z * grid.width * grid.height;
	int i = int(c) % grid.builtCount;

	vec3 irrPos = grid.firstPos + vec3(x * grid.distToAdjacent, y * grid.distToAdjacent, z * grid.distToAdjacent);

	Aabb aabb = createAabb(irrPos, grid.distToAdjacent);

	vec3 N = surface.basis.normal;
	vec3 reprojN = (surface.position - irrPos) + intersectionDistanceAabb(aabb, surface.position, N) * N;

	// SMATH: which normal
	vec3 kd = 1.0 - F_SchlickRoughness(surface.nov, surface.f0, surface.a);

	return kd * texture(irradianceSamplers, vec4(normalize(reprojN), i)).rgb
	//	 * saturate(dot(N, irrPos - surface.position));
	;
}

vec3 Irragrid_Contribution(Irragrid grid, samplerCubeArray irradianceSamplers, Surface surface)
{
	vec3 probeCount  = vec3(grid.width - 1, grid.height - 1, grid.depth - 1);
	vec3 size = probeCount * grid.distToAdjacent;
	
	vec3 uvw = (surface.position - grid.firstPos) / size;
	
	vec3 delim = 1 / probeCount; 
	
	if(any(greaterThan(uvw, 1 + delim)) || 
	   any(lessThan(uvw, -delim))) {
		return vec3(0); // WIP: matrix based volume
	}
	
	float attenfactor = 1.f;
	if(any(greaterThan(uvw, vec3(1))) || 
	   any(lessThan(uvw, vec3(0)))) {
	   // PERF: probably
	   Aabb aabb = createAabb(vec3(grid.firstPos), vec3(grid.firstPos + size));
	   Aabb aabb2 = createAabb(vec3(grid.firstPos - grid.distToAdjacent), 
	                           vec3(grid.firstPos + size + grid.distToAdjacent));
	   
	   float sdist = distanceFromOutside(aabb, surface.position);
	   float bdist = distanceFromInside(aabb2, surface.position);
	  
	   attenfactor = (bdist * bdist) / ((sdist + bdist) * (sdist + bdist));
	  
	   //return vec3(attenfactor);
	}

	// if you don't saturate it will wrap arround
	uvw = saturate(uvw);

	// SMATH: interpolation
	float su = uvw.x * probeCount.x;
	float sv = uvw.y * probeCount.y;
	float sw = uvw.z * probeCount.z;
	
	vec3 FTL = SampleIrrad(grid, irradianceSamplers, surface, floor(su), floor(sv), floor(sw));
	vec3 FTR = SampleIrrad(grid, irradianceSamplers, surface, ceil (su), floor(sv), floor(sw));
	vec3 FBL = SampleIrrad(grid, irradianceSamplers, surface, floor(su), ceil (sv), floor(sw));
	vec3 FBR = SampleIrrad(grid, irradianceSamplers, surface, ceil (su), ceil (sv), floor(sw));
								 
	vec3 BTL = SampleIrrad(grid, irradianceSamplers, surface, floor(su), floor(sv), ceil (sw));
	vec3 BTR = SampleIrrad(grid, irradianceSamplers, surface, ceil (su), floor(sv), ceil (sw));
	vec3 BBL = SampleIrrad(grid, irradianceSamplers, surface, floor(su), ceil (sv), ceil (sw));
	vec3 BBR = SampleIrrad(grid, irradianceSamplers, surface, ceil (su), ceil (sv), ceil (sw));

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
	
	return diffuseLight * LambertianDiffuse(surface.albedo) * attenfactor;
}

#endif
