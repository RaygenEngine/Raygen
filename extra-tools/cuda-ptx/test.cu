#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "test.h"

using namespace optix;

////////////////////////////////////////// RAY GENERATION PROGRAM //////////////////////////////////////////

rtDeclareVariable(PerRayData_result, prd, rtPayload, );
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(unsigned int, result_ray_type, , );
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(rtObject, top_object, , );

rtDeclareVariable(float3, eye, , );
rtDeclareVariable(float3, U, , );
rtDeclareVariable(float3, V, , );
rtDeclareVariable(float3, W, , );
rtBuffer<uchar4, 2> output_buffer;

RT_PROGRAM void pinhole_camera()
{
	size_t2 screen = output_buffer.size();
	float2 d = make_float2(launch_index) / make_float2(screen) * 2.f - 1.f;

	float3 ray_origin = eye;
	float3 ray_direction = normalize(d.x * U + d.y * V + W);
	optix::Ray ray(ray_origin, ray_direction, result_ray_type, scene_epsilon);
	PerRayData_result prd;

	rtTrace(top_object, ray, prd);
	output_buffer[launch_index] = make_color(prd.result);
}

////////////////////////////////////////// MISS PROGRAM //////////////////////////////////////////

rtDeclareVariable(float3, bg_color, , );
// Bindless sky texture id.
rtDeclareVariable(int, sky_mapId, , );

RT_PROGRAM void miss()
{
	//prd_radiance.result = optix::make_float3(optix::rtTexCubemap<float4>(sky_mapId, ray.direction.x, ray.direction.y, ray.direction.z));
	//prd_radiance.result = bg_color;

	float theta = atan2f(ray.direction.x, ray.direction.z);
	float phi = M_PIf * 0.5f - acosf(ray.direction.y);
	float u = (theta + M_PIf) * (0.5f * M_1_PIf);
	float v = 0.5f * (1.0f + sin(phi));
	prd.result = make_float3(rtTex2D<float4>(sky_mapId, u, v));
}

////////////////////////////////////////// EXCEPTION PROGRAM //////////////////////////////////////////

rtDeclareVariable(float3, bad_color, , );

RT_PROGRAM void exception()
{
	output_buffer[launch_index] = make_color(bad_color);
}

////////////////////////////////////////// CLOSEST HIT PROGRAM //////////////////////////////////////////

// from intersection
rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(float3, texcoord, attribute texcoord, );

// Bindless mat texture IDs.
rtDeclareVariable(int, Albedo_mapId, , ); 
rtDeclareVariable(int, Emission_mapId, , );
rtDeclareVariable(int, SpecularParameters_mapId, , );
rtDeclareVariable(int, Bump_mapId, , );
rtDeclareVariable(int, Noise_mapId, , );

rtDeclareVariable(int, hasNormalMap, , );

rtDeclareVariable(int, mode, , );

// very basic unoptimized surface test shader
RT_PROGRAM void closest_hit()
{
	float3 world_shading_normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
	float3 world_geometric_normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, geometric_normal));
	float3 ffnormal = faceforward(world_shading_normal, -ray.direction, world_geometric_normal);

	const float4 albedo = rtTex2D<float4>(Albedo_mapId, texcoord.x, texcoord.y);
	const float4 emission = rtTex2D<float4>(Emission_mapId, texcoord.x, texcoord.y);
	const float4 specular_parameters = rtTex2D<float4>(SpecularParameters_mapId, texcoord.x, texcoord.y);
	const float4 bump = rtTex2D<float4>(Bump_mapId, texcoord.x, texcoord.y);
	const float noise = rtTex2D<float>(Noise_mapId, texcoord.x, texcoord.y);
	
	switch (mode)
	{
		case 0: // albedo
			prd.result = make_float3(albedo);
			break;

		case 1: // emission
			prd.result = make_float3(emission);
			break;

		case 2: // reflectivity
			prd.result = make_float3(specular_parameters.x, specular_parameters.x, specular_parameters.x);
			break;

		case 3: // roughness
			prd.result = make_float3(specular_parameters.y, specular_parameters.y, specular_parameters.y);
			break;

		case 4: // metallic
			prd.result = make_float3(specular_parameters.z, specular_parameters.z, specular_parameters.z);
			break;

		case 5: // world normal
			prd.result = ffnormal;
			break;

		case 6: // normal map
			prd.result = make_float3(bump.x, bump.y, bump.z);
			break;

		case 7: // final normal
			if (hasNormalMap)
			{
				optix::Onb onb(ffnormal);
				float3 sample = make_float3(bump.x, bump.y, bump.z) * 2 - 1;
				onb.inverse_transform(sample);
				ffnormal = normalize(sample);
			}
			prd.result = ffnormal;
			break;

		case 8: // uv
			prd.result = make_float3(texcoord.x, texcoord.y, 0);
			break;

		case 9: // height
			prd.result = make_float3(bump.w, bump.w, bump.w);
			break;

		case 10: // translucency
			prd.result = make_float3(specular_parameters.w, specular_parameters.w, specular_parameters.w);
			break;

		case 11: // ambient occlusion
			prd.result = make_float3(emission.w, emission.w, emission.w);
			break;

		case 12: // opacity
			prd.result = make_float3(albedo.w, albedo.w, albedo.w);
			break;

		case 13: // noise
			prd.result = make_float3(noise, noise, noise);
			break;
	}
}

////////////////////////////////////////// ANY HIT PROGRAM //////////////////////////////////////////

RT_PROGRAM void any_hit()
{

}