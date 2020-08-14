#pragma once

struct SceneGeometry;

namespace vl {


struct AsInstance {
	glm::mat4 transform{};
	int32 id{ -1 }; // gl_InstanceID
	vk::DeviceAddress blasAddress;
	int32 hitGroupId{ 0 }; // WIP: for now use the same hit group for all objects
	vk::GeometryInstanceFlagsKHR flags{};
};

struct TopLevelAs {
	TopLevelAs() = default;
	TopLevelAs(const std::vector<SceneGeometry*>& geoms);

private:
	vk::UniqueAccelerationStructureKHR handle;
	vk::UniqueDeviceMemory memory;

	std::vector<AsInstance> instances;
};
} // namespace vl
