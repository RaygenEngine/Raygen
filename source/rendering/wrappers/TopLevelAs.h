#pragma once
#include "rendering/wrappers/AccelerationStructure.h"
#include "rendering/wrappers/Buffer.h"

struct SceneGeometry;
struct SceneSpotlight;
struct ScenePointlight;
struct SceneReflprobe;
struct SceneDirlight;
struct SceneIrradianceGrid;
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
	InFlightResources<vk::DescriptorSet> descSetPointlights;
	InFlightResources<vk::DescriptorSet> descSetSpotlights;
	InFlightResources<vk::DescriptorSet> descSetDirlights;
	InFlightResources<vk::DescriptorSet> descSetReflprobes;
	InFlightResources<vk::DescriptorSet> descSetIrragrids;

	void AddGeomGroup(const GpuGeometryGroup& group, const GpuMesh& mesh, const glm::mat4& transform);
	void WriteImages();
	void WritePointlights(const std::vector<ScenePointlight*>& pointlights);
	void WriteSpotlights(const std::vector<SceneSpotlight*>& spotlights);
	void WriteDirlights(const std::vector<SceneDirlight*>& dirlights);
	void WriteReflprobes(const std::vector<SceneReflprobe*>& reflprobes);
	void WriteIrragrids(const std::vector<SceneIrradianceGrid*>& irragrids);
	void WriteGeomGroups();

	int32 spotlightCount{ 0 };
	int32 pointlightCount{ 0 };
	int32 dirlightCount{ 0 };
	int32 reflprobeCount{ 0 };
	int32 irragridCount{ 0 };

	RBuffer spotlightsBuffer;
	RBuffer pointlightsBuffer;
	RBuffer dirlightsBuffer;
	RBuffer reflprobesBuffer;
	RBuffer irragridsBuffer;

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
