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










