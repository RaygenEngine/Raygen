#pragma once
#include "rendering/scene/SceneStructs.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"

struct Spotlight_Ubo {
	glm::vec4 position{};
	glm::vec4 front{};

	// Lightmap
	glm::mat4 viewProj{};
	glm::vec4 color{};

	float intensity{};

	float _near{};
	float _far{};

	float outerCutOff{};
	float innerCutOff{};

	float constantTerm{};
	float linearTerm{};
	float quadraticTerm{};

	float maxShadowBias{};
	int32 samples{};
	float sampleInvSpread{};
	int32 hasShadow{};
};

struct SceneSpotlight : SceneStruct {
	SCENE_STRUCT(SceneSpotlight);
	Spotlight_Ubo ubo;

	InFlightResources<vl::RenderingPassInstance> shadowmapPass;
	InFlightResources<vk::DescriptorSet> shadowmapDescSet;

	vk::Sampler depthSampler;

	std::string name;
	void MaybeResizeShadowmap(uint32 width, uint32 height);

	bool isDynamic;
};
