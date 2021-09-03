#ifndef radiance_glsl
#define radiance_glsl

#include "aabb.glsl"
#include "intersection.glsl"
#include "random.glsl"
#include "shading-math.glsl"
#include "surface.glsl"

float ShadowCalculationSmooth(sampler2DShadow shadowmap, mat4 lightMatrix, vec3 fragPos, float maxBias, int samples, float invSpread)
{
	const vec2 poissonDisk16[16] = vec2[](
		vec2(-0.94201624, -0.39906216),
		vec2(0.94558609, -0.76890725),
		vec2(-0.094184101, -0.92938870),
		vec2(0.34495938, 0.29387760),
		vec2(-0.91588581, 0.45771432),
		vec2(-0.81544232, -0.87912464),
		vec2(-0.38277543, 0.27676845),
		vec2(0.97484398, 0.75648379),
		vec2(0.44323325, -0.97511554),
		vec2(0.53742981, -0.47373420),
		vec2(-0.26496911, -0.41893023),
		vec2(0.79197514, 0.19090188),
		vec2(-0.24188840, 0.99706507),
		vec2(-0.81409955, 0.91437590),
		vec2(0.19984126, 0.78641367),
		vec2(0.14383161, -0.14100790)
	);


	vec4 fragPosLightSpace = lightMatrix * vec4(fragPos, 1.0);
 	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
 	
	float bias = clamp(0.005, 0, maxBias);
	
	projCoords = vec3(projCoords.xy, projCoords.z - bias);

	float shadow = 0;
	// Stratified Poisson Sampling
	// PERF: samples here prevent loop unrolling
	for (int i = 0; i < samples; ++i)
	{
		int index = int(16.0*rand(vec4(fragPos, i)))%16;
		
		vec3 jitteredProjCoords = vec3((projCoords.xy + poissonDisk16[index] / invSpread) * 0.5 + 0.5, projCoords.z);
	
		// Hardware implemented PCF on sample
		shadow += 1.0 - texture(shadowmap, jitteredProjCoords);
	}
		
    return shadow / samples;
} 

float ShadowCalculation(sampler2DShadow shadowmap, mat4 lightMatrix, vec3 fragPos, float bias)
{
	vec4 fragPosLightSpace = lightMatrix * vec4(fragPos, 1.0);
 	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
 	
	projCoords = vec3(projCoords.xy * 0.5 + 0.5, projCoords.z - bias);

    return 1.0 - texture(shadowmap, projCoords);
} 

vec3 Dirlight_Contribution(Dirlight dl, sampler2DShadow shadowmap, Surface surface, float shadow)
{
	vec3 L = normalize(-dl.front); 

	vec3 Li = (1.0 - shadow) * dl.color * dl.intensity; 
	return Li * explicitBRDFcosTheta(surface, L);
}

vec3 Dirlight_EstimateDirect(Dirlight dl, sampler2DShadow shadowmap, Surface surface)
{
	float shadow = dl.hasShadow * ShadowCalculation(shadowmap, dl.viewProj, surface.position, dl.maxShadowBias);
	return Dirlight_Contribution(dl, shadowmap, surface, shadow);
}

vec3 Dirlight_EstimateDirectSmooth(Dirlight dl, sampler2DShadow shadowmap, Surface surface)
{
	float shadow = dl.hasShadow * ShadowCalculationSmooth(shadowmap, dl.viewProj, surface.position, dl.maxShadowBias, dl.samples, dl.sampleInvSpread);
	return Dirlight_Contribution(dl, shadowmap, surface, shadow);
}

vec3 Spotlight_Contribution(Spotlight sl, sampler2DShadow shadowmap, Surface surface, float shadow)
{
	vec3 L = normalize(sl.position - surface.position);

	float dist = distance(sl.position, surface.position);
	float attenuation = 1.0 / (sl.constantTerm + sl.linearTerm * dist + 
  			     sl.quadraticTerm * (dist * dist));

	// spot effect (soft edges)
	float theta = dot(L, -sl.front);
    float epsilon = (sl.innerCutOff - sl.outerCutOff);
    float spotEffect = clamp((theta - sl.outerCutOff) / epsilon, 0.0, 1.0);

	vec3 Li = (1.0 - shadow) * sl.color * sl.intensity * attenuation * spotEffect; 
	return Li * explicitBRDFcosTheta(surface, L);
}

vec3 Spotlight_EstimateDirect(Spotlight sl, sampler2DShadow shadowmap, Surface surface)
{
	float shadow = sl.hasShadow * ShadowCalculation(shadowmap, sl.viewProj, surface.position, sl.maxShadowBias);
	return Spotlight_Contribution(sl, shadowmap, surface, shadow);
}

vec3 Spotlight_EstimateDirectSmooth(Spotlight sl, sampler2DShadow shadowmap, Surface surface)
{
	float shadow = sl.hasShadow * ShadowCalculationSmooth(shadowmap, sl.viewProj, surface.position, sl.maxShadowBias, sl.samples, sl.sampleInvSpread);
	return Spotlight_Contribution(sl, shadowmap, surface, shadow);
}

vec3 Reflprobe_EstimateDirect(Reflprobe rp, sampler2D brdfLutSampler, samplerCube irradianceSampler, samplerCube prefilteredSampler, Surface surface)
{	   // TODO: indirect specular lightblend with mirror // TODO use sphere volume like point lights
	if(surface.a < SPEC_THRESHOLD || distance(rp.position, surface.position) > rp.radius) {
		return vec3(0);
	}

	vec3 R = outOnbSpace(surface.basis, reflect(-surface.i));
	vec3 reprojR = (surface.position - rp.position) + RaySphereIntersection(surface.position, R, rp.position, rp.radius) *  R;

	float lod = sqrt(surface.a) * rp.lodCount; 
	
	float NoV = absNdot(surface.i);

	vec3 brdfLut = texture(std_BrdfLut, vec2(NoV, surface.a)).rgb;

	vec3 ks = F_SchlickRoughness(NoV, surface.f0, surface.a);
	vec3 kd = (1.0 - ks) * surface.opacity;

	vec3 diffuseLight = texture(irradianceSampler, surface.basis.normal).rgb;
	vec3 specularLight = textureLod(prefilteredSampler, reprojR, lod).rgb;

	vec3 diffuse = diffuseLight * diffuseBRDF(surface) * rp.irradianceFactor;
	vec3 specular = specularLight * (surface.f0 * brdfLut.x + brdfLut.y);

	return ks * specular + kd * diffuse;
}

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
	vec3 kd = 1.0 - F_SchlickRoughness(absNdot(surface.i), surface.f0, surface.a);

	return kd * texture(irradianceSamplers, vec4(normalize(reprojN), i)).rgb
	//	 * saturate(dot(N, irrPos - surface.position));
	;
}

vec3 Irragrid_EstimateDirect(Irragrid grid, samplerCubeArray irradianceSamplers, Surface surface)
{
	vec3 probeCount  = vec3(grid.width - 1, grid.height - 1, grid.depth - 1);
	vec3 size = probeCount * grid.distToAdjacent;
	
	vec3 uvw = (surface.position - grid.firstPos) / size;
	
	vec3 delim = 1 / probeCount; 
	
	if(any(greaterThan(uvw, 1 + delim)) || 
	   any(lessThan(uvw, -delim))) {
		return vec3(0); // PERF: matrix based volume
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
	
	return diffuseLight * diffuseBRDF(surface) * attenfactor;
}

// Representative point method
vec3 Arealight_EstimateDirectNoLightAttenuation(Quadlight ql, Surface surface)
{
	vec3 V = getIncomingDir(surface);

	vec3 L = reflect(-V, surface.basis.normal);

	// we need to rotate the reflection ray to force intersection with quad's plane
	float t;
	if (!RayPlaneIntersection(surface.position, L, ql.center, ql.normal, t)) {
		vec3 perp_r_n = L - dot(L, ql.normal) * ql.normal;
		vec3 pointOnPlane = ql.center + perp_r_n * INF;
		L = normalize(pointOnPlane - surface.position);
	}

	vec3 p = surface.position + t * L; // intersection point with rect's plane 

	// if point isn't in rectangle, choose the closest that is
	if (!PointInsideRectangle(p, ql.center, ql.normal, ql.right, ql.up, ql.width, ql.height)) {
		p = PointRectangleNearestPoint(p, ql.center, ql.normal, ql.right, ql.up, ql.width, ql.height);
	}

	L = normalize(p - surface.position);

	addOutgoingDir(surface, L);
	if (isOutgoingDirPassingThrough(surface)) {
		return vec3(0);
	}

	vec3 Li = ql.color * ql.intensity; // missing smooth shadow and attenuation, i.e. arealightShadowing factor

	return Li * explicitBRDF(surface);
}

// Representative point method
vec3 Arealight_EstimateDirectHackLightAttenuation(Quadlight ql, Surface surface)
{
	vec3 V = getIncomingDir(surface);

	vec3 L = reflect(-V, surface.basis.normal);

	// we need to rotate the reflection ray to force intersection with quad's plane
	float t;
	if (!RayPlaneIntersection(surface.position, L, ql.center, ql.normal, t)) {
		vec3 perp_r_n = L - dot(L, ql.normal) * ql.normal;
		vec3 pointOnPlane = ql.center + perp_r_n * INF;
		L = normalize(pointOnPlane - surface.position);
	}

	vec3 p = surface.position + t * L; // intersection point with rect's plane 

	// if point isn't in rectangle, choose the closest that is
	if (!PointInsideRectangle(p, ql.center, ql.normal, ql.right, ql.up, ql.width, ql.height)) {
		p = PointRectangleNearestPoint(p, ql.center, ql.normal, ql.right, ql.up, ql.width, ql.height);
	}

	L = normalize(p - surface.position);

	float cosTheta_o = dot(ql.normal, -L);

	if (cosTheta_o < BIAS) {
		return vec3(0);
	}

	addOutgoingDir(surface, L);
	if (isOutgoingDirPassingThrough(surface)) {
		return vec3(0);
	}

	float dist = distance(p, surface.position);

	float attenuation = 1.0 / (ql.constantTerm + ql.linearTerm * dist +
		ql.quadraticTerm * (dist * dist));

	vec3 Li = ql.color * ql.intensity * attenuation; // missing smooth shadow and attenuation, i.e. arealightShadowing factor

	return Li * explicitBRDFcosTheta(surface);
}

#endif
