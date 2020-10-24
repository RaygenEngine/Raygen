#pragma once
#include "rendering/scene/SceneStructs.h"

namespace vl {
class AmbientBaker;
}
struct Reflprobe_UBO {
	glm::vec4 position{};
	float innerRadius;
	float outerRadius;
};

struct SceneReflprobe : SceneStruct {
	// SCENE_STRUCT(SceneReflprobe);

	SceneReflprobe();
	~SceneReflprobe();


	Reflprobe_UBO ubo;

	vl::AmbientBaker* ab{ nullptr };

	vk::DescriptorSet reflDescSet;
	// vl::GpuHandle<EnvironmentMap> envmap;

	void Build();
};
