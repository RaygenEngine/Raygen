#ifndef EYE_H
#define EYE_H

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
#endif

	struct Eye
	{
		// 3 * 4 bytes
		FLOAT3 position;
		// 4 bytes
		float verticalFovHalfTan;
	};

#if defined(__cplusplus)
}
#endif

#endif // EYE_H
