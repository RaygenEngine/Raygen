#pragma once

#include <glm/glm.hpp>
#include <array>


struct VertexData {
	glm::vec3 position{};
	glm::vec3 normal{};
	glm::vec3 tangent{};
	glm::vec3 bitangent{};
	glm::vec2 textCoord0{};
	glm::vec2 textCoord1{};
};

struct Box {
	glm::vec3 min{};
	glm::vec3 max{};
};

// plane equation
// ax + by + cz + d = 0
struct Plane {
	float a, b, c, d;
};

struct Frustum {
	enum
	{
		TOP = 0,
		BOTTOM,
		LEFT,
		RIGHT,
		NEAR_,
		FAR_
	};

	std::array<Plane, 6> planes;
};
