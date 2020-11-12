#version 460 
#extension GL_GOOGLE_include_directive: enable

#include "global.glsl"

#include "bsdf.glsl"
#include "surface.glsl"
#include "attachments.glsl"
#include "sky.glsl"

// out

layout(location = 0) out vec4 outColor;

// in 

// uniform

layout(push_constant) uniform PC {
	mat4 reflVolMatVP;
    vec4 reflPosition;
    int lodCount;
} push;

layout(set = 1, binding = 0) uniform UBO_Camera { Camera cam; };
layout(set = 2, binding = 0) uniform samplerCube skyboxSampler;
layout(set = 2, binding = 1) uniform samplerCube irradianceSampler;
layout(set = 2, binding = 2) uniform samplerCube prefilteredSampler;

void main( ) {

//	vec2 iuv = gl_FragCoord.xy;
//	ivec2 screenSize = textureSize(g_AlbedoSampler, 0);
//
//	vec2 uv = iuv / screenSize; 
//
//	float depth = texture(g_DepthSampler, uv).r;
//
//	// TODO: discard when skymesh is implemented
//	if(depth == 1.0) {
//		// TODO: discard here like in spotlights
//		vec3 V = normalize(reconstructWorldPosition(depth, uv, cam.viewProjInv) - cam.position);
//		
//		// PERF:
//		outColor = vec4(GetSkyColor(cam.position, V), 1.0);
//		outColor = sampleCubemapLH(skyboxSampler, V);
//		return;
//	}
//
//	// PERF:
//	Fragment frag = getFragmentFromGBuffer(
//		depth,
//		cam.viewProjInv,
//		g_NormalSampler,
//		g_AlbedoSampler,
//		g_SpecularSampler,
//		g_EmissiveSampler,
//		uv);
//	
//	
//	vec3 N = frag.normal;
//	vec3 V = normalize(cam.position - frag.position);
//	vec3 R = normalize(reflect(-V, N));
//
//    float NoV = abs(dot(N, V)) + 1e-5;
//	
//	// CHECK: roughness / a differences
//	float lod = frag.a * push.lodCount; 
//	
//	vec3 brdfLut = (texture(std_BrdfLut, vec2(NoV, frag.a))).rgb;
//
//	vec3 ks = F_SchlickRoughness(saturate(dot(N, V)), frag.f0, frag.a);
//	vec3 kd = 1.0 - ks;
//
//	vec3 diffuseLight = texture(irradianceSampler, N).rgb;
//	vec3 specularLight = textureLod(prefilteredSampler, R, lod).rgb;
//
//	vec3 diffuse = diffuseLight * frag.albedo;
//	vec3 specular = specularLight * (frag.f0 * brdfLut.x + brdfLut.y);
//
//	vec3 iblContribution = kd * diffuse + ks * specular;
//
//	outColor =  vec4(iblContribution, 1.0f);
}






