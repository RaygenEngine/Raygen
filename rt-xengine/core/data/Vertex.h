#pragma once

// Do not re-arrange this struct without permission! 
#if defined(__cplusplus) 
namespace Core
{
#endif

#ifdef __CUDACC__
	#define FLOAT3 float3
	#define FLOAT2 float2
#else // engine
	#define FLOAT3 glm::vec3
	#define FLOAT2 glm::vec2
#endif

	struct Vertex
	{
		// 3 * 4 bytes
		FLOAT3 position;
		// 3 * 4 bytes
		FLOAT3 normal;
		// 2 * 4 bytes
		FLOAT2 uv;
	};

#if defined(__cplusplus)
}
#endif
