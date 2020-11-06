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
	vec4 pos[6];
	float distToAdjacent;
	float blendProportion;
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


layout(set = 2, binding = 0) uniform samplerCube irradianceSampler[6];

bool PointInAABB(vec3 pmin, vec3 pmax, vec3 point)
{
    if(point.x > pmin.x && point.x < pmax.x &&
       point.y > pmin.y && point.y < pmax.y &&
       point.z > pmin.z && point.z < pmax.z){
        return true;
    }

	return false;
}

vec2 intersectAABB(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax) {
    vec3 tMin = (boxMin - rayOrigin) / rayDir;
    vec3 tMax = (boxMax - rayOrigin) / rayDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar);
};

void main( ) {
	float depth = texture(g_DepthSampler, uv).r;

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
	vec3 V = normalize(cam.position - frag.position);
	vec3 R = normalize(reflect(-V, N));

	vec3 diffuseLight = vec3(0);

	float blendDist = blendProportion * distToAdjacent;
	
	for(int i  = 0; i < 6; ++i){

		vec3 center = pos[i].xyz;

		// Big AABB
		vec3 halfSize = 0.5 * vec3(distToAdjacent + blendDist) ;
		vec3 pmin = center - halfSize;
		vec3 pmax = center + halfSize;

		bool pointInBig = PointInAABB(pmin, pmax, frag.position);

		// Small AABB
		halfSize = 0.5 * vec3(distToAdjacent - blendDist) ;
		pmin = center - halfSize;
		pmax = center + halfSize;

		bool pointInSmall = PointInAABB(pmin, pmax, frag.position);

		// inner sample, 100% contribution
		if(pointInSmall){
			diffuseLight += texture(irradianceSampler[i], N).rgb;
			break;
		}
		// not in small but in big, weighted contribution - blend 
		else if(pointInBig)	{
			
			vec3 rayDir = normalize(frag.position - center);

			vec2 tminMax = intersectAABB(center, rayDir, pmin, pmax);

			vec3 intr = center + rayDir * tminMax.y;

			float weight = saturate(1 - distance(intr, frag.position) / blendDist);
			diffuseLight += texture(irradianceSampler[i], N).rgb * weight;
		}
	}

	vec3 diffuse = diffuseLight * frag.albedo;
	outColor = vec4(diffuse, 1.0);
}






