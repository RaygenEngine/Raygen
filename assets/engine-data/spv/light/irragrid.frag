#version 460 
#extension GL_GOOGLE_include_directive: enable

#include "global.glsl"

#include "fragment.glsl"
#include "attachments.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

layout (location = 0) in vec2 uv;

// uniform

layout(push_constant) uniform PC {
	vec3 firstPos;
	float pad;
	float distToAdjacent;
	float blendProportion;
	int width;
	int height;
	int depth;
};

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


layout(set = 2, binding = 0) uniform samplerCube irradianceSampler[1024];

vec3 SampleIrrad(float x, float y, float z, vec3 N) {

	float i = 0;
	i += x;
	i += y * 16;
	i += z * 16 * 16;
	return texture(irradianceSampler[int(i)], N).rgb;
}

void main( ) {
	//discard;

	float depth1 = texture(g_DepthSampler, uv).r;


	if(depth1 == 1.0) {
		// TODO: discard here like in spotlights
		vec3 V = normalize(reconstructWorldPosition(depth1, uv, cam.viewProjInv) - cam.position);
		
		outColor = vec4(SampleIrrad(4, 0, 1, V).xyz, 1);
		return;
	}

	// PERF:
	Fragment frag = getFragmentFromGBuffer(
		depth1,
		cam.viewProjInv,
		g_NormalSampler,
		g_AlbedoSampler,
		g_SpecularSampler,
		g_EmissiveSampler,
		uv);
	
	vec3 N = frag.normal;

	vec3 probeCount  = vec3(15, 15, 3);
	vec3 size = probeCount * distToAdjacent;
	
	vec3 uvw = (frag.position - firstPos) / size; 

	vec3 delim = 0.5 / size; 
	


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
	
	vec3 FTL = SampleIrrad(floor(su), floor(sv), floor(sw), N);
	vec3 FTR = SampleIrrad(ceil (su), floor(sv), floor(sw), N);
	vec3 FBL = SampleIrrad(floor(su), ceil (sv), floor(sw), N);
	vec3 FBR = SampleIrrad(ceil (su), ceil (sv), floor(sw), N);

	vec3 BTL = SampleIrrad(floor(su), floor(sv), ceil (sw), N);
	vec3 BTR = SampleIrrad(ceil (su), floor(sv), ceil (sw), N);
	vec3 BBL = SampleIrrad(floor(su), ceil (sv), ceil (sw), N);
	vec3 BBR = SampleIrrad(ceil (su), ceil (sv), ceil (sw), N);

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


















