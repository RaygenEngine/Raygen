#version 460 
#extension GL_GOOGLE_include_directive: enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "global.glsl"

#include "fragment.glsl"
#include "attachments.glsl"
#include "aabb.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout (location = 0) in vec2 uv;

// uniform

layout(set = 1, binding = 0) uniform UBO_Camera {
	vec3 position;
	float pad0;
	mat4 view;
	mat4 proj;
	mat4 viewProj;
	mat4 viewInv;
	mat4 projInv;
	mat4 viewProjInv;
} cam;

layout(set = 2, binding = 0) uniform UBO_Irragrid {
	int width;
	int height;
	int depth;
	int builtCount;

	vec3 firstPos;
	float distToAdjacent;
} grid;

layout(set = 3, binding = 0) uniform samplerCube irradianceSampler[];

vec3 SampleIrrad(float x, float y, float z, vec3 fragPos, vec3 N) {
	float c = 0;
	c += x;
	c += y * grid.width;
	c += z * grid.width * grid.height;
	int i = int(c) % grid.builtCount;

	vec3 irrPos = grid.firstPos + vec3(x * grid.distToAdjacent, y * grid.distToAdjacent, z * grid.distToAdjacent);

	Aabb aabb = createAabb(irrPos, grid.distToAdjacent);

	vec3 reprojNormal = (fragPos - irrPos) + (fragPos + intersectionDistanceAabb(aabb, fragPos, N) * N);

	return texture(irradianceSampler[nonuniformEXT(i)], normalize(reprojNormal)).rgb
	//	 * saturate(dot(N, irrPos - fragPos));
	;
}

void main( ) {
	float depth = texture(g_DepthSampler, uv).r;


//	if(depth == 1.0) {
//		// TODO: discard here like in spotlights
//		vec3 V = normalize(reconstructWorldPosition(depth, uv, cam.viewProjInv) - cam.position);
//		
//		outColor = vec4(SampleIrrad2(15, 0, 1, V).xyz, 1);
//
//		return;
//	}

	// PERF:
	Fragment frag = getFragmentFromGBuffer(
		depth,
		cam.viewProjInv,
		g_NormalSampler,
		g_AlbedoSampler,
		g_SpecularSampler,
		g_EmissiveSampler,
		uv);
	
	vec3 N = frag.normal;

	vec3 probeCount  = vec3(grid.width - 1, grid.height - 1, grid.depth - 1);
	vec3 size = probeCount * grid.distToAdjacent;
	
	vec3 uvw = (frag.position - grid.firstPos) / size; 

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

	// SMATH:
	float su = uvw.x * probeCount.x;
	float sv = uvw.y * probeCount.y;
	float sw = uvw.z * probeCount.z;
	
	vec3 FTL = SampleIrrad(floor(su), floor(sv), floor(sw), frag.position, N);
	vec3 FTR = SampleIrrad(ceil (su), floor(sv), floor(sw), frag.position, N);
	vec3 FBL = SampleIrrad(floor(su), ceil (sv), floor(sw), frag.position, N);
	vec3 FBR = SampleIrrad(ceil (su), ceil (sv), floor(sw), frag.position, N);

	vec3 BTL = SampleIrrad(floor(su), floor(sv), ceil (sw), frag.position, N);
	vec3 BTR = SampleIrrad(ceil (su), floor(sv), ceil (sw), frag.position, N);
	vec3 BBL = SampleIrrad(floor(su), ceil (sv), ceil (sw), frag.position, N);
	vec3 BBR = SampleIrrad(ceil (su), ceil (sv), ceil (sw), frag.position, N);

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

	outColor = vec4(diffuseLight * frag.albedo, 1.0);
}



























