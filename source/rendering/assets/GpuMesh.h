#pragma once
#include "rendering/assets/GpuAssetBase.h"

namespace vl {
class RBuffer;


// PERF: GPU ASSETS From https://vulkan-tutorial.com/en/Vertex_buffers/Index_buffer
// store multiple buffers, like the vertex and index buffer, into a single VkBuffer and use offsets in cmds like
// vkCmdBindVertexBuffers. The advantage is that your data is more m_cache friendly in that case, because it's closer
// together. It is even possible to reuse the same chunk of memory for multiple resources if they are not used
// during the same render operations, provided that their data is refreshed, of course. This is known as aliasing
// and some Vulkan functions have explicit flags to specify that you want to do this.

struct GpuGeometryGroup {
	uint32 indexCount{ 0u };
	uint32 vertexCount{ 0u }; // PERF: remove
	GpuHandle<MaterialInstance> material;

	UniquePtr<RBuffer> vertexBuffer;
	UniquePtr<RBuffer> indexBuffer;
};


struct GpuMesh : public GpuAssetTemplate<Mesh> {
	std::vector<GpuGeometryGroup> geometryGroups;
	vk::UniqueAccelerationStructureKHR accelStruct;


	GpuMesh(PodHandle<Mesh> podHandle);

	void Update(const AssetUpdateInfo& info) override final;
};
} // namespace vl
