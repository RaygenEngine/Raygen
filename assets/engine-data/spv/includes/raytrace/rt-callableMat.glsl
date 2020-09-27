#ifndef rt_callableMat_glsl
#define rt_callableMat_glsl

// META:
// Expects pre declared struct "Material" before the inclusion of the file

layout(buffer_reference, std430) buffer Vertices { Vertex v[]; };
layout(buffer_reference, std430) buffer Indicies { uint i[]; };
layout(buffer_reference, std430) buffer MaterialBufRef { Material m; };

struct CallableMatInOut
{
	int matid; // Material ID
	vec2 uv; // Incoming UV

	FragBrdfInfo brdfInfo;

	vec3 emissive;
	vec3 localNormal;
};

struct GeometryGroup {
	Vertices vtxBuffer;
	Indicies indBuffer;
	MaterialBufRef materialUbo;

	uint indexOffset;
	uint primOffset;

	mat4 transform;
	mat4 invTransform;

	int callableIndex; // callableIndex == -1 is used at the moment for gltf (mainly for debugging purposes | you can skip callable materials completely)
};

layout(set = 4, binding = 0, std430) readonly buffer GeometryGroups { GeometryGroup g[]; } geomGroups;
layout(set = 4, binding = 1) uniform sampler2D textureSamplers[];//

vec4 texture(sampler2DRef s, vec2 uv) {
	return texture(textureSamplers[nonuniformEXT(s.index)], uv);
}


#else
#error "RT CallableMat glsl header should have no reason to be included twice. Check if what you are trying to do is correct."
#endif
