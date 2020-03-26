#pragma once
#include "assets/AssetManager.h"
#include "renderer/asset/GpuAssetHandle.h"
#include "universe/nodes/geometry/GeometryNode.h"


struct SceneGeometry {
	glm::mat4 transform;
	GpuHandle<ModelPod> model;

	GeometryNode* node;
};

struct SceneCamera {
	glm::mat4 proj{};
	glm::mat4 view{};
	glm::mat4 viewProj{};
};

struct SceneSpotlight {
	struct Ubo {
		glm::vec4 position;
		glm::vec4 forward;

		// Lightmap
		glm::mat4 viewProj{};
		glm::vec4 color{};

		float intensity{};

		float near_{};
		float far_{};

		// angle
		float outerAper{};
		// inner
		float innerAper{};
	} ubo;
	bool isDirty{ true };
};

template<typename T>
concept CSceneElem
	= std::is_same_v<SceneGeometry, T> || std::is_same_v<SceneCamera, T> || std::is_same_v<SceneSpotlight, T>;