#pragma once

struct VertexData {
	glm::vec3 position{};
	glm::vec3 normal{};
	glm::vec4 tangent{};
	glm::vec3 bitangent{};
	glm::vec2 textCoord0{};
	glm::vec2 textCoord1{};
};

struct Box {
	glm::vec3 max{};
	glm::vec3 min{};
};
