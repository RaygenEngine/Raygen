#pragma once
#include "rendering/scene/SceneStructs.h"

struct SceneGeometry {
	glm::mat4 transform;
	glm::mat4 prevTransform;

	vl::GpuHandle<Mesh> mesh;

	InFlightResources<bool> isDirty{ true };
};

struct SceneAnimatedGeometry {
	glm::mat4 transform{};
	vl::GpuHandle<SkinnedMesh> mesh;
	PodHandle<SkinnedMesh> meshPod;

	std::vector<glm::mat4> jointMatrices;

	InFlightResources<vk::DescriptorSet> descSet;
	InFlightResources<vl::RBuffer> buffer;

	void UploadSsbo(uint32 curFrame);

	SceneAnimatedGeometry()
	{
		for (size_t i = 0; i < c_framesInFlight; ++i) {
			descSet[i] = vl::Layouts->jointsDescLayout.AllocDescriptorSet();
		}
	}

	void ResizeJoints(uint32 curFrame);


	void MaybeResizeJoints(size_t newSize);

	InFlightResources<bool> isDirty{ true };
	InFlightResources<bool> isDirtyResize{ true };

private:
	size_t GetBufferSize() const;
};
