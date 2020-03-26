#pragma once
#include "assets/pods/ModelPod.h"
#include "renderer/asset/GpuAssetHandle.h"
#include "renderer/asset/Material.h"
#include "world/nodes/geometry/GeometryNode.h"

#include <vulkan/vulkan.hpp>

// TODO: GPU ASSETS From https://vulkan-tutorial.com/en/Vertex_buffers/Index_buffer
// store multiple buffers, like the vertex and index buffer, into a single VkBuffer and use offsets in commands like
// vkCmdBindVertexBuffers. The advantage is that your data is more cache friendly in that case, because it's closer
// together. It is even possible to reuse the same chunk of memory for multiple resources if they are not used
// during the same render operations, provided that their data is refreshed, of course. This is known as aliasing
// and some Vulkan functions have explicit flags to specify that you want to do this.

// PERF: batching
struct GPUGeometryGroup {

	UniquePtr<Buffer> vertexBuffer;
	UniquePtr<Buffer> indexBuffer;

	GpuHandle<MaterialPod> material;

	uint32 indexCount{ 0u };
};


DECLARE_GPU_ASSET(Model, ModelPod)
{
	std::vector<GPUGeometryGroup> geometryGroups;
	GpuAssetBaseTyped<ModelPod>(PodHandle<ModelPod> podHandle);
};
