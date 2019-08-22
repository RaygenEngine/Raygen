#include "tutorial.h"


rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );

rtDeclareVariable(unsigned int, radiance_ray_type, , );

rtDeclareVariable(float, scene_epsilon, , );

rtDeclareVariable(rtObject, top_object, , );

//
// Pinhole camera implementation
//

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
	optix::Ray ray(ray_origin, ray_direction, radiance_ray_type, scene_epsilon);
	PerRayData_radiance prd;
	prd.importance = 1.f;
	prd.depth = 0;

	rtTrace(top_object, ray, prd);
	output_buffer[launch_index] = make_color(prd.result);
} 

//
// Returns solid color for miss rays
//

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
	prd_radiance.result = make_float3(rtTex2D<float4>(sky_mapId, u, v));
} 

//
// Set pixel to solid color upon failur
//

rtDeclareVariable(float3, bad_color, , );

RT_PROGRAM void exception()
{
	output_buffer[launch_index] = make_color(bad_color);
}
