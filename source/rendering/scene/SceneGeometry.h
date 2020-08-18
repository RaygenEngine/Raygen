#pragma once
#include "rendering/scene/SceneStructs.h"

// TODO: rename model stuff
struct SceneGeometry {
	glm::mat4 transform;
	vl::GpuHandle<Mesh> mesh;

	InFlightResource<bool> isDirty{ true };
};

struct SceneAnimatedGeometry {
	glm::mat4 transform{};
	vl::GpuHandle<SkinnedMesh> mesh;
	PodHandle<SkinnedMesh> modelPod;

	std::vector<glm::mat4> jointMatrices;

	InFlightResource<vk::DescriptorSet> descSets;
	InFlightResource<vl::RBuffer> buffers;

	void UploadSsbo(uint32 curFrame);

	SceneAnimatedGeometry()
	{
		for (size_t i = 0; i < c_framesInFlight; ++i) {
			descSets[i] = vl::Layouts->jointsDescLayout.GetDescriptorSet();
		}
	}

	void ResizeJoints(uint32 curFrame);


	void MaybeResizeJoints(size_t newSize);

	InFlightResource<bool> isDirty{ true };
	InFlightResource<bool> isDirtyResize{ true };

private:
	size_t GetBufferSize() const;
};
