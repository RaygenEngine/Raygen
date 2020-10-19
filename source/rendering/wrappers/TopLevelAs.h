#pragma once
#include "rendering/wrappers/Buffer.h"
#include "rendering/wrappers/AccelerationStructure.h"

struct SceneGeometry;
struct SceneSpotlight;
struct ScenePointlight;
struct Scene;


namespace vl {


struct AsInstance {
	uint64 blas;

	int32 instanceId{ 0xFF }; // gl_InstanceID
	int32 materialId{ 0 };    // aka Shader Binding Table Offset

	vk::GeometryInstanceFlagsKHR flags{ vk::GeometryInstanceFlagBitsKHR::eTriangleFrontCounterclockwise };
	glm::mat4 transform{ glm::identity<glm::mat4>() };
};

struct RtSceneDescriptor {
	InFlightResources<vk::DescriptorSet> descSet;
	InFlightResources<vk::DescriptorSet> descSetSpotlights;
	InFlightResources<vk::DescriptorSet> descSetPointlights;

	void AddGeomGroup(const GpuGeometryGroup& group, const GpuMesh& mesh, const glm::mat4& transform);
	void WriteImages();
	void WriteSpotlights(const std::vector<SceneSpotlight*>& spotlights);
	void WritePointlights(const std::vector<ScenePointlight*>& pointlights);
	void WriteGeomGroups();

	int32 spotlightCount{ 0 };
	int32 pointlightCount{ 0 };
	RBuffer spotlightsBuffer;
	RBuffer pointlightsBuffer;
	RBuffer geomGroupsBuffer;
};

struct TopLevelAs : RAccelerationStructure {
	TopLevelAs() = default;
	TopLevelAs(const std::vector<SceneGeometry*>& geoms, Scene* scene);


	void AddAsInstance(AsInstance&& instance);

	void Clear();
	void Build();


	// TODO: Get this out of here
	RtSceneDescriptor sceneDesc;

private:
	RBuffer instanceBuffer;

	std::vector<vk::AccelerationStructureInstanceKHR> instances;
};
} // namespace vl
