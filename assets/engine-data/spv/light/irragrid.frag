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
	vec4 firstPos;
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

// TODO: AABB shader math
bool PointInAABB(vec3 pmin, vec3 pmax, vec3 point)
{
    if(point.x > pmin.x && point.x < pmax.x &&
       point.y > pmin.y && point.y < pmax.y &&
       point.z > pmin.z && point.z < pmax.z){
        return true;
    }

	return false;
}

vec2 IntersectAABB(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax) {
    vec3 tMin = (boxMin - rayOrigin) / rayDir;
    vec3 tMax = (boxMax - rayOrigin) / rayDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar);
};

float DistPointAABB(vec3 boxMin, vec3 boxMax, vec3 p) {
  float dx = max(max(boxMin.x - p.x, 0.0), p.x - boxMax.x);
  float dy = max(max(boxMin.y - p.y, 0.0), p.y - boxMax.y);
  float dz = max(max(boxMin.z - p.z, 0.0), p.z - boxMax.z);
  return sqrt(dx*dx + dy*dy + dz * dz);
}

vec3 SampleIrrad(float x, float y, float z, vec3 N) {

	float i = 0;
	i += x;
	i += y * 16;
	i += z * 16 * 16;
	return texture(irradianceSampler[int(i)], -N).rgb;
}

void main( ) {
	float depth1 = texture(g_DepthSampler, uv).r;

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
	vec3 V = normalize(cam.position - frag.position);
	vec3 R = normalize(reflect(-V, N));

	vec3 diffuseLight = vec3(0);

	float blendDist = blendProportion * distToAdjacent;
	

	ivec3 size  = ivec3(16, 16, 4);
	vec3 endPos = size * distToAdjacent;

	vec3 uvw = saturate((frag.position - firstPos.xyz) / (endPos - firstPos.xyz));



	float su = uvw.x * size.x + 0.5;
	float sv = uvw.y * size.y + 0.5;
	float sw = uvw.z * size.z + 0.5;

				
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

	vec3 final = mix(frontInt, backInt, backPercent);

	outColor = vec4(final  * frag.albedo, 1.0);
	
	return;

	for (int i = 0; i < 1024; ++i)  {
				
		int x = i % 16;
		int y = (i / 16) % 16;
		int z = i / (16 * 16);

		vec3 center = firstPos.xyz + vec3(x, y, z) * distToAdjacent;

		// Big AABB
		vec3 halfSize = 0.5 * vec3(distToAdjacent + blendDist) ;
		vec3 pminB = center - halfSize;
		vec3 pmaxB = center + halfSize;

		bool pointInBig = PointInAABB(pminB, pmaxB, frag.position);

		// Small AABB
		halfSize = 0.5 * vec3(distToAdjacent - blendDist) ;
		vec3 pminS = center - halfSize;
		vec3 pmaxS = center + halfSize;

		bool pointInSmall = PointInAABB(pminS, pmaxS, frag.position);

		//float d = distPointAABB(pminS, pmaxS, frag.position);
		//float weight = 1 - d / blendDist;
		//diffuseLight = vec3(weight);


		// inner sample, 100% contribution
		if(pointInSmall){
			diffuseLight = texture(irradianceSampler[i], N).rgb;
			break;
		}
		// not in small but in big, weighted contribution - blend 
		else if(pointInBig)	{
		
			float d = DistPointAABB(pminS, pmaxS, frag.position);
			float weight = 1 - saturate(d / blendDist);
			diffuseLight += texture(irradianceSampler[i], N).rgb * weight;

		}
	}

	outColor = vec4(diffuseLight  * frag.albedo, 1.0);
}












