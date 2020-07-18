#pragma once

struct Vertex {
	glm::vec3 position{};
	glm::vec3 normal{};
	glm::vec3 tangent{};
	glm::vec2 uv{};
};

// Transition note: (Former GeometryGroup)
// This "slot" now batches all vertices per material during importing.
// GeometrySlots now is a parallel array to the materials array. (ie: geometrySlots[2] material == materials[2])
struct GeometrySlot {
	std::vector<uint32> indices{};
	std::vector<Vertex> vertices{};
};

struct SkinnedVertex {
	glm::vec3 position{};
	glm::vec3 normal{};
	glm::vec3 tangent{};
	glm::vec2 uv{};
	glm::ivec4 joint{};
	glm::vec4 weight{};
};

struct SkinnedGeometrySlot {
	std::vector<uint32> indices{};
	std::vector<SkinnedVertex> vertices{};
};

struct Joint {
	static constexpr int32 c_rootParentJointIndex = -1;

	int32 parentJoint;
	glm::mat4 inverseBindMatrix;

	glm::vec3 translation;
	glm::quat rotation;
	glm::vec3 scale;


	int32 index;

	std::string name;

	[[nodiscard]] bool IsRoot() const { return parentJoint == c_rootParentJointIndex; }
};
