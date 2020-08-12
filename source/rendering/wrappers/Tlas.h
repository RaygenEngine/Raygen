#pragma once

namespace vl {

struct AsInstance {
	glm::mat4 transform{};
	int32 id{ -1 }; // gl_InstanceID
	int32 blasId{ -1 };
	int32 hitGroupId{ 0 }; // WIP: for now use the same hit group for all objects
	vk::GeometryInstanceFlagsKHR flags{};
};

struct Tlas {
	std::vector<AsInstance> instance;
};
} // namespace vl
