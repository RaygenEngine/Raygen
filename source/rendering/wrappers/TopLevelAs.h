#pragma once
#include "rendering/wrappers/Buffer.h"
#include "rendering/wrappers/AccelerationStructure.h"

struct SceneGeometry;

namespace vl {


struct AsInstance {
	uint64 blas;

	int32 instanceId{ 0xFF }; // gl_InstanceID
	int32 materialId{ 0 };    // aka Shader Binding Table Offset

	vk::GeometryInstanceFlagsKHR flags{ vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable }; // WIP:
	glm::mat4 transform{ glm::identity<glm::mat4>() };
};

struct TopLevelAs : RAccelerationStructure {
	TopLevelAs() = default;
	TopLevelAs(const std::vector<SceneGeometry*>& geoms);


	void AddAsInstance(AsInstance&& instance);

	void Clear();
	void Build();


private:
	RBuffer instanceBuffer;

	std::vector<vk::AccelerationStructureInstanceKHR> instances;
};
} // namespace vl
