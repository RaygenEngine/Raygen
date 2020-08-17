#pragma once
#include "rendering/assets/GpuAssetBase.h"
#include "rendering/wrappers/BottomLevelAs.h"
#include "rendering/wrappers/TopLevelAs.h"

#include "rendering/wrappers/Buffer.h"

struct AssetUpdateInfo;
struct Mesh;
template<typename PodTypeT>
struct PodHandle;


namespace vl {

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

	RBuffer vertexBuffer;
	RBuffer indexBuffer;

	BottomLevelAs blas;
};


struct GpuMesh : public GpuAssetTemplate<Mesh> {
	std::vector<GpuGeometryGroup> geometryGroups;

	TopLevelAs meshAs;


	GpuMesh(PodHandle<Mesh> podHandle);

	void Update(const AssetUpdateInfo& info) override final;
};
} // namespace vl
