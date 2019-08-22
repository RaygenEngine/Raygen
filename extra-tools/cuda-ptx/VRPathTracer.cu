#include "VRPathTracerAux.h"

#include "commonStructs.h"
#include "random.h"

#include "lambert_brdf.h"
#include "ggx_brdf.h"

#include <optixu/optixu_matrix_namespace.h>

#define FAR 300

struct PerRayData_radiance_VRPT
{
	float3 result;
	float3 radiance;
	float depth_value;
	float3 albedo;
	float3 normal;
	float3 throughput;
	float3 origin;
	float3 direction;
	unsigned int seed;
	int depth;
	int done;
};

// TODO: there is an issue (alignement) with the payload structures (possibly an Optix 6.0 bug?)
struct PerRayData_shadow_VRPT
{
	int in_shadow;
	float3 result;
	float3 radiance;
	float depth_value;
	float3 albedo;
	float3 normal;
	float3 throughput;
	float3 origin;
	float3 direction;
	unsigned int seed;
	int depth;
	int done;
};

rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );

rtDeclareVariable(unsigned int, radiance_ray_type, , );
rtDeclareVariable(unsigned int, shadow_ray_type, , );

rtDeclareVariable(float, scene_epsilon, , );

rtDeclareVariable(rtObject, top_object, , );
rtDeclareVariable(rtObject, top_shadower, , );


rtBuffer<float4, 2> output_buffer;
rtBuffer<float,  2> depth_buffer;
rtBuffer<float4, 2> input_albedo_buffer;
rtBuffer<float4, 2> input_normal_buffer;


rtDeclareVariable(unsigned int, max_spp, , );
rtDeclareVariable(unsigned int, max_bounces, , );
rtDeclareVariable(unsigned int, sqrt_max_spp, , );

rtDeclareVariable(int, enable_sampling_mask, , );


rtDeclareVariable(float, time_, , );
////////////////////////////////////////// RAY GENERATION PROGRAMS //////////////////////////////////////////

static __device__ __inline__ void ray_gen_eye(const float3& eye, const float3& eye_u, const float3& eye_v, const float3& eye_w, const Matrix3x3& normal_matrix, const uint2& buffer_index)
{
	size_t2 eye_screen = output_buffer.size();
	eye_screen.x = eye_screen.x / 2;

	float2 inv_screen = 1.0f / make_float2(eye_screen) * 2.f;
	float2 pixel = make_float2(launch_index) * inv_screen - 1.f;

	float2 center = make_float2(eye_screen.x / 2.f, eye_screen.y / 2.f);
	float2 point = make_float2(launch_index);
	float distance_from_center = length(point - center);

	uint samples_per_pixel = max_spp;

	if (enable_sampling_mask != 0)
	{

		float2 d = make_float2(0.f);

		float normalized = 1 - (distance_from_center / (eye_screen.x));
		samples_per_pixel = 1 + uint(normalized*(max_spp - 1) / 8);

		auto center_region = eye_screen.x / 2.5;

		if (distance_from_center <= center_region)
		{
			auto dist = (distance_from_center / (center_region));

			normalized = 1 - (dist * dist);

			samples_per_pixel += uint(normalized*(max_spp - 1));
		}

		if (samples_per_pixel == 0)
			return;

	}
	//right eye test
	//if (buffer_index.x > eye_screen.x)
	//{
	//	output_buffer[buffer_index] = make_float4(samples_per_pixel / float(max_spp));
	//	return;
	//}

	float2 jitter_scale = inv_screen / sqrtf(samples_per_pixel);

	float3 result = make_float3(0.0f);
	float3 albedo = make_float3(0.0f);
	float3 normal = make_float3(0.0f);
	float depth_value = 0.f;

	float spp = 0.f;	
	unsigned int seed = tea<16>(eye_screen.x*launch_index.y + launch_index.x, 124144);//fmodf(time_, 1241)
	//if (buffer_index.x > eye_screen.x)
	//{
	//	seed = tea<16>(eye_screen.y*buffer_index.y + buffer_index.x, 124144);//fmodf(time_, 1241)
	//}

	do
	{
		//float2 d = make_float2(0.f);

		//right eye test
		//if(buffer_index.x > eye_screen.x)
		//{
		//	d = pixel;

		//}
		//else
		//{
			unsigned int x = max_spp % sqrt_max_spp;
			unsigned int y = max_spp / sqrt_max_spp;
			float2 jitter = make_float2(x - rnd(seed), y - rnd(seed));
			float2 d = pixel + jitter * 0.0025;
		//}

		auto current_ray_origin = eye;
		auto current_ray_direction = normalize(d.x * eye_u + d.y * eye_v + eye_w);

		// Initialze per-ray data
		PerRayData_radiance_VRPT prd;
		prd.result = make_float3(0.f);
		prd.albedo = make_float3(0.f);
		prd.throughput = make_float3(1.f);
		prd.done = false;
		prd.seed = seed;
		prd.depth = 0;

		// Each iteration is a segment of the ray path.  The closest hit will
		// return new segments to be traced here.
		auto max_depth = max_bounces;

		do
		{
			optix::Ray ray = make_Ray(current_ray_origin, current_ray_direction, radiance_ray_type, scene_epsilon, FAR);
			rtTrace(top_object, ray, prd);

			// We have hit the background or a luminaire
			if (prd.done)
			{
				//if(prd.depth < 1)
				//	prd.result = make_float3(1.f);

				break;
			}

			// Russian roulette termination 
			if (prd.depth >= 5)
			{
				float pcont = fmaxf(prd.throughput);
				if (rnd(prd.seed) >= pcont)
					break;
				prd.throughput /= pcont;
			}

			prd.depth++;

			// Update ray data for the next path segment
			current_ray_origin = prd.origin;
			current_ray_direction = prd.direction;

		} while (--max_depth);

		result += prd.result;
		seed = prd.seed;

		albedo += prd.albedo;
		++spp;
		float3 normal_eyespace = (length(prd.normal) > 0.f) ? normalize(normal_matrix * prd.normal) : make_float3(0., 0., 1.);
		normal += normal_eyespace;

		depth_value = prd.depth_value;

	} while (--samples_per_pixel);
	result = result / spp;
	output_buffer[buffer_index]       = make_float4(clamp(result, 0.f, 1.f), 1.0f);
	input_albedo_buffer[buffer_index] = make_float4(albedo / spp, 1.0f);
	input_normal_buffer[buffer_index] = make_float4(normal / spp, 1.0f);

	depth_buffer[buffer_index] = depth_value / FAR;// log2(max(1e-6, 1.0 + depth_value)) * (2.0 / log2(FAR + 1.0)) - 1.0;
}


rtDeclareVariable(float3, eye_L, , );
rtDeclareVariable(float3, U_L, , );
rtDeclareVariable(float3, V_L, , );
rtDeclareVariable(float3, W_L, , );
rtDeclareVariable(Matrix3x3, normal_matrix_L, , );

// different eye raygens for future path reprojection work 
RT_PROGRAM void pathtrace_vr_left_eye()
{
	uint2 buffer_index = make_uint2(launch_index.x, launch_index.y);

	ray_gen_eye(eye_L, U_L, V_L, W_L, normal_matrix_L, buffer_index);
}

rtDeclareVariable(float3, eye_R, , );
rtDeclareVariable(float3, U_R, , );
rtDeclareVariable(float3, V_R, , );
rtDeclareVariable(float3, W_R, , );
rtDeclareVariable(Matrix3x3, normal_matrix_R, , );

// to access results from left eye use the left side of the output/albedo/normal buffer
RT_PROGRAM void pathtrace_vr_right_eye()
{
	uint2 buffer_index = make_uint2(launch_index.x + (output_buffer.size().x/2), launch_index.y);

	//output_buffer[buffer_index] = output_buffer[launch_index];
	//input_albedo_buffer[buffer_index] = input_albedo_buffer[launch_index];
	//input_normal_buffer[buffer_index] = input_normal_buffer[launch_index];

	ray_gen_eye(eye_R, U_R, V_R, W_R, normal_matrix_R, buffer_index);
}

////////////////////////////////////////// RAY GENERATION PROGRAMS //////////////////////////////////////////


////////////////////////////////////////// CLOSEST HIT DIFFUSE PROGRAM //////////////////////////////////////////

// Bindless mat texture IDs.
rtDeclareVariable(int, Albedo_mapId, , );
rtDeclareVariable(int, Emission_mapId, , );
rtDeclareVariable(int, SpecularParameters_mapId, , );
rtDeclareVariable(int, Bump_mapId, , );
rtDeclareVariable(int, Noise_mapId, , );

rtDeclareVariable(int, hasNormalMap, , );

rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(float3, texcoord, attribute texcoord, );

rtDeclareVariable(PerRayData_radiance_VRPT, current_prd, rtPayload, );

rtDeclareVariable(float, t_hit, rtIntersectionDistance, );

rtDeclareVariable(Core::BasicLight, light, ,);

rtDeclareVariable(float, forced_metal, , );
rtDeclareVariable(float, forced_roughness, , );
rtDeclareVariable(float, forced_reflectance, , );
rtDeclareVariable(float, light_intensity, , );

RT_PROGRAM void surface_shading()
{
	float3 world_shading_normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
	float3 world_geometric_normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, geometric_normal));

	float3 hit_point = ray.origin + t_hit * ray.direction;

	// shading normal
	float3 ns = faceforward(world_shading_normal, -ray.direction, world_geometric_normal);

	optix::Onb onb_local(ns);
	if (hasNormalMap != -1)
	{
		//float3 sample = make_float3(rtTex2D<float4>(Bump_mapId, texcoord.x, texcoord.y)) * 2 - 1;
		//onb_local.inverse_transform(sample);
		//ns = normalize(sample);
	}

	const float3 albedo = make_float3(rtTex2D<float4>(Albedo_mapId, texcoord.x, texcoord.y));
	// x : refle, y: roughness, z: metal
	const float4 specular_parameters = rtTex2D<float4>(SpecularParameters_mapId, texcoord.x, texcoord.y);

	if (current_prd.depth == 0)
	{
		current_prd.depth_value = abs(t_hit);

		current_prd.albedo = albedo;
		current_prd.normal = ns;
	}

	float reflectivity = forced_reflectance;//specular_parameters.x;
	float roughness = forced_roughness;// force  specular_parameters.y;
	float metallic = forced_metal;// specular_parameters.z;



	float3 diffuse_color = (1.0 - metallic) * albedo;
	float3 f0 = make_float3(0.16) * reflectivity * reflectivity * (1.0 - metallic) + (albedo * metallic);


	// incoming (surface space)
	float3 wi = world_to_surface(onb_local, -ray.direction);

	// DIRECT ILLUMINATION

	 //Calculate properties of light sample 
	const float  Ldist = length(light.pos - hit_point);
	const float3 L = normalize(light.pos - hit_point);
	const float  nDl = dot(ns, L);

	float3 luminance = make_float3(0.0f);

	//cast shadow ray
	if (nDl > 0.0f)
	{
		PerRayData_shadow_VRPT shadow_prd;
		shadow_prd.in_shadow = false;

		Ray shadow_ray = make_Ray(hit_point, L, shadow_ray_type, scene_epsilon, Ldist - scene_epsilon);
		rtTrace(top_shadower, shadow_ray, shadow_prd);

		if (!shadow_prd.in_shadow)
			luminance = light.color * light_intensity * nDl / (Ldist*Ldist);
	}

	// direct diffuse and specular
	float3 wl = world_to_surface(onb_local, L);

	float3 wh_d = normalize(wi + wl);

	float3 F_d = Fresnel_Schlick(max(0.0, dot(wi, wh_d)), f0);

	float3 fr_d = MicrofacetGGX_Evaluate(roughness, F_d, wh_d, wi, wl);

	float3 fd_d = diffuse_color / M_PIf;

	current_prd.result = current_prd.result + (luminance * current_prd.throughput *(fd_d + fr_d)); //lerp(fd_d, fr_d, (F_d.x+F_d.y+F_d.z)/3.f));


	// INDIRECT ILLUMINATION



	// reflect or transmit
	float3 F_p = Fresnel_Schlick(fabs(wi.y), f0);

	float2 sample = make_float2(rnd(current_prd.seed), rnd(current_prd.seed));
	// halfway (ss)
	float3 wh;
	// outgoing (ss)
	float3 wo;
	// pdf of outgoing 
	float pdf;
	MicrofacetDistribution_GGX_SampleHalfwayVector(wi, roughness, sample, wh, wo, pdf);

	float3 F_i = Fresnel_Schlick(max(0.0, dot(wi, wh)), f0);
	float k_s_i = (F_i.x + F_i.y + F_i.z) / 3.f;

	if(rnd(current_prd.seed) > k_s_i)
	{
		float z1 = rnd(current_prd.seed);
		float z2 = rnd(current_prd.seed);
		float3 p;
		cosine_sample_hemisphere(z1, z2, p);
		onb_local.inverse_transform(p);
		current_prd.direction = p;

		current_prd.throughput *= diffuse_color;
	}
	else
	{

		if (roughness <= 0.01)
		{
			float3 wo = optix::reflect(-wi, make_float3(0,1,0));

			// next path segment
			current_prd.origin = hit_point;
			current_prd.direction = surface_to_world(onb_local, wo);

			return;
		}

		//float2 sample = make_float2(rnd(current_prd.seed), rnd(current_prd.seed));
		//// halfway (ss)
		//float3 wh;
		//// outgoing (ss)
		//float3 wo;
		//// pdf of outgoing 
		//float pdf;
		//MicrofacetDistribution_GGX_SampleHalfwayVector(wi, roughness, sample, wh, wo, pdf);

		//float3 F_i = Fresnel_Schlick(max(0.0, dot(wi, wh)), f0);
		//float k_s_i = (F_i.x + F_i.y + F_i.z) / 3.f;

		float3 fr_i = MicrofacetGGX_Evaluate(roughness, F_i, wh, wi, wo) / pdf;

		current_prd.throughput *= fr_i*fabs(wo.y);
		//current_prd.throughput *= fabs(dot(ns, current_prd.direction));

		current_prd.direction = surface_to_world(onb_local, wo);
	}
	//// next path segment
	current_prd.origin = hit_point;
}

////////////////////////////////////////// CLOSEST HIT DIFFUSE PROGRAM //////////////////////////////////////////


////////////////////////////////////////// MISS PROGRAM //////////////////////////////////////////

rtDeclareVariable(float3, bg_color, , );

// Bindless sky texture id.
rtDeclareVariable(int, sky_mapId, , );

RT_PROGRAM void miss()
{
	//current_prd.radiance = optix::make_float3(optix::rtTexCubemap<float4>(sky_mapId, ray.direction.x, ray.direction.y, ray.direction.z));
	//current_prd.radiance = bg_color;

	float theta = atan2f(ray.direction.x, ray.direction.z);
	float phi = M_PIf * 0.5f - acosf(ray.direction.y);
	float u = (theta + M_PIf) * (0.5f * M_1_PIf);
	float v = 0.5f * (1.0f + sin(phi));

	current_prd.result = current_prd.result + (current_prd.throughput * make_float3(rtTex2D<float4>(sky_mapId, u, v)));
	current_prd.done = true;

	// TODO: Find out what the albedo buffer should really have. For now just set to black for misses.
	if (current_prd.depth == 0)
	{
		current_prd.depth_value = FAR;

		current_prd.albedo = make_float3(0, 0, 0);
		current_prd.normal = make_float3(0, 0, 0);
	}
}

////////////////////////////////////////// MISS PROGRAM //////////////////////////////////////////


////////////////////////////////////////// EXCEPTION PROGRAM //////////////////////////////////////////

rtDeclareVariable(float3, bad_color, , );

RT_PROGRAM void exception()
{
	output_buffer[launch_index] = make_float4(bad_color, 1.f);
}

////////////////////////////////////////// EXCEPTION PROGRAM //////////////////////////////////////////


////////////////////////////////////////// ANY HIT SHADOW PROGRAM //////////////////////////////////////////

rtDeclareVariable(PerRayData_shadow_VRPT, current_prd_shadow, rtPayload, );

RT_PROGRAM void any_hit_shadow()
{
	// this material is opaque, so it fully attenuates all shadow rays
	current_prd_shadow.in_shadow = 1;
	rtTerminateRay();
}

////////////////////////////////////////// ANY HIT SHADOW PROGRAM //////////////////////////////////////////