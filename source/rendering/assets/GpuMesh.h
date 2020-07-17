#pragma once
#include "assets/pods/Mesh.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/wrappers/RBuffer.h"
#include "universe/nodes/geometry/GeometryNode.h"

#include <vulkan/vulkan.hpp>

namespace vl {
class RBuffer;
}

// TODO: GPU ASSETS From https://vulkan-tutorial.com/en/Vertex_buffers/Index_buffer
// store multiple buffers, like the vertex and index buffer, into a single VkBuffer and use offsets in cmds like
// vkCmdBindVertexBuffers. The advantage is that your data is more m_cache friendly in that case, because it's closer
// together. It is even possible to reuse the same chunk of memory for multiple resources if they are not used
// during the same render operations, provided that their data is refreshed, of course. This is known as aliasing
// and some Vulkan functions have explicit flags to specify that you want to do this.

// PERF: batching
struct GpuGeometryGroup {
	uint32 indexCount{ 0u };
	vl::GpuHandle<MaterialInstance> material;

	UniquePtr<vl::RBuffer> vertexBuffer;
	UniquePtr<vl::RBuffer> indexBuffer;
};

struct Mesh::Gpu : public vl::GpuAssetTemplate<Mesh> {
	std::vector<GpuGeometryGroup> geometryGroups;


	Mesh::Gpu(PodHandle<Mesh> podHandle);

	void Update(const AssetUpdateInfo& info) override final;
};
