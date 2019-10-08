#pragma once

// Do not re-arrange this struct without permission!
#if defined(__cplusplus)
namespace utl {
#endif

#ifdef __CUDACC__
#	define FLOAT3 float3
#	define INT32  int
#else // engine
#	define FLOAT3 glm::vec3
#	define INT32  int32
#endif

struct BasicLight {
	// 3 * 4 bytes
	FLOAT3 pos;
	// 3 * 4 bytes
	FLOAT3 color;
	// 4 bytes
	INT32 casts_shadow;
	INT32 padding;
};
#if defined(__cplusplus)
}
#endif
