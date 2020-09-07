#pragma once
#include "rendering/wrappers/Buffer.h"
#include "rendering/wrappers/AccelerationStructure.h"

struct SceneGeometry;
struct SceneSpotlight;
struct Scene;


namespace vl {


struct AsInstance {
	uint64 blas;

	int32 instanceId{ 0xFF }; // gl_InstanceID
	int32 materialId{ 0 };    // aka Shader Binding Table Offset

	vk::GeometryInstanceFlagsKHR flags{ vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable }; // WIP:
	glm::mat4 transform{ glm::identity<glm::mat4>() };
};

struct RtSceneDescriptor {
	InFlightResources<vk::DescriptorSet> descSet;
	InFlightResources<vk::DescriptorSet> descSetSpotlights;

	void AddGeomGroup(const GpuGeometryGroup& group, const GpuMesh& mesh, const glm::mat4& transform);
	void WriteImages();
	void WriteSpotlights(const std::vector<SceneSpotlight*>& spotlights);
	void WriteGeomGroups();

	int32 spotlightCount{ 0 };
	RBuffer spotlightsBuffer;
	RBuffer geomGroupsBuffer;
};

struct TopLevelAs : RAccelerationStructure {
	TopLevelAs() = default;
	TopLevelAs(const std::vector<SceneGeometry*>& geoms, Scene* scene);


	void AddAsInstance(AsInstance&& instance);

	void Clear();
	void Build();


	// WIP: Get this out of here
	RtSceneDescriptor sceneDesc;

private:
	RBuffer instanceBuffer;

	std::vector<vk::AccelerationStructureInstanceKHR> instances;
};
} // namespace vl
