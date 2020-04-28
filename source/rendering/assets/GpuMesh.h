#pragma once
#include "assets/pods/Mesh.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/assets/GpuMaterial.h"
#include "universe/nodes/geometry/GeometryNode.h"

#include <vulkan/vulkan.hpp>

// TODO: GPU ASSETS From https://vulkan-tutorial.com/en/Vertex_buffers/Index_buffer
// store multiple buffers, like the vertex and index buffer, into a single VkBuffer and use offsets in cmds like
// vkCmdBindVertexBuffers. The advantage is that your data is more m_cache friendly in that case, because it's closer
// together. It is even possible to reuse the same chunk of memory for multiple resources if they are not used
// during the same render operations, provided that their data is refreshed, of course. This is known as aliasing
// and some Vulkan functions have explicit flags to specify that you want to do this.

// PERF: batching
struct GpuGeometryGroup {

	UniquePtr<vl::RawBuffer> vertexBuffer;
	UniquePtr<vl::RawBuffer> indexBuffer;

	vl::GpuHandle<Material> material;

	uint32 indexCount{ 0u };
};

struct Mesh::Gpu : public vl::GpuAssetTemplate<Mesh> {
	std::vector<GpuGeometryGroup> geometryGroups;

	Mesh::Gpu(PodHandle<Mesh> podHandle);
};
