#pragma once
#include "rendering/scene/SceneStructs.h"

// TODO: rename model stuff
struct SceneGeometry {
	glm::mat4 transform;
	vl::GpuHandle<Mesh> model;

	std::vector<bool> isDirty;
};

struct SceneAnimatedGeometry {
	glm::mat4 transform{};
	vl::GpuHandle<SkinnedMesh> model;
	PodHandle<SkinnedMesh> modelPod;

	std::vector<glm::mat4> jointMatrices;

	std::array<vk::DescriptorSet, 3> descSets;
	std::array<UniquePtr<vl::RBuffer>, 3> buffers;

	void UploadSsbo(uint32 curFrame);

	SceneAnimatedGeometry()
	{
		for (size_t i = 0; i < 3; ++i) {
			descSets[i] = vl::Layouts->jointsDescLayout.GetDescriptorSet();
		}
	}

	void ResizeJoints(uint32 curFrame);

	std::array<bool, 3> isDirty{ true, true, true };
	std::array<bool, 3> isDirtyResize{ true, true, true };

private:
	size_t GetBufferSize() const;
};
