#pragma once

// Do not re-arrange this struct without permission!
#if defined(__cplusplus)
namespace utl {
#endif

#ifdef __CUDACC__
#	define FLOAT4 float4
#	define FLOAT3 float3
#	define FLOAT2 float2
#else // engine
#	define FLOAT4 glm::vec4
#	define FLOAT3 glm::vec3
#	define FLOAT2 glm::vec2
#endif

struct Vertex {
	// 3 * 4 bytes
	FLOAT3 position;
	// 3 * 4 bytes
	FLOAT3 normal;
	// 4 * 4 bytes
	FLOAT4 tangents;
	// 3 * 4 bytes
	FLOAT3 bitangents;
	// 2 * 4 bytes
	FLOAT2 textCoord0;
};

struct VertexExtension {
	// 2 * 4 bytes
	FLOAT2 textCoord1;

	FLOAT4 color0;

	FLOAT4 weights;

	glm::u16vec4 joints;
};
#if defined(__cplusplus)
}
#endif
