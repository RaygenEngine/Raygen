#ifndef EYE_H
#define EYE_H

// Do not re-arrange this struct without permission! 
#if defined(__cplusplus) && !defined(__CUDACC__)
namespace Core
{
	#include <GLM/vec3.hpp>
	#include <GLM/vec2.hpp>

	#define FLOAT3 glm::vec3
	#define FLOAT2 glm::vec2

#endif

#ifdef __CUDACC__
#define FLOAT3 float3
#define FLOAT2 float2
#endif

	struct Eye
	{
		// 3 * 4 bytes
		FLOAT3 position;
		// 3 * 4 bytes
		FLOAT3 up;
		// 3 * 4 bytes
		FLOAT3 right;
		// 3 * 4 bytes
		FLOAT3 front;
		// 2 * 4 bytes
		FLOAT2 filmPlaneViewVectorIntersectionScreenSpaceCoords;

		// 4 bytes
		float verticalFovHalfTan;

		// 4 bytes
		float focalLength;
	};

#if defined(__cplusplus) && !defined(__CUDACC__)
}
#endif 

#endif // EYE_H
