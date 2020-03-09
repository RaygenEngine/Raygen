#pragma once

#include "renderer/LogicalDevice.h"
#include "asset/pods/ModelPod.h"
#include "world/nodes/geometry/GeometryNode.h"
#include "renderer/Material.h"

#include "vulkan/vulkan.hpp"


// TODO: From https://vulkan-tutorial.com/en/Vertex_buffers/Index_buffer
// store multiple buffers, like the vertex and index buffer, into a single VkBuffer and use offsets in commands like
// vkCmdBindVertexBuffers. The advantage is that your data is more cache friendly in that case, because it's closer
// together. It is even possible to reuse the same chunk of memory for multiple resources if they are not used
// during the same render operations, provided that their data is refreshed, of course. This is known as aliasing
// and some Vulkan functions have explicit flags to specify that you want to do this.

// PERF: batching
struct GPUGeometryGroup {

	vk::UniqueBuffer vertexBuffer;
	vk::UniqueDeviceMemory vertexBufferMemory;

	vk::UniqueBuffer indexBuffer;
	vk::UniqueDeviceMemory indexBufferMemory;

	std::unique_ptr<Material> material;

	// one for each swapchain image
	// TODO: check
	// https://stackoverflow.com/questions/36772607/vulkan-texture-rendering-on-multiple-meshes this
	vk::DescriptorSet descriptorSet;

	uint32 indexCount{ 0u };
};

struct Model {
	std::vector<GPUGeometryGroup> geometryGroups;

	Model(PodHandle<ModelPod> podHandle);

	GeometryNode* m_node;
};
