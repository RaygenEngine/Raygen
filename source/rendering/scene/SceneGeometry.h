#pragma once
#include "rendering/scene/SceneStructs.h"

// TODO: rename model stuff
struct SceneGeometry {
	glm::mat4 transform;
	vl::GpuHandle<Mesh> mesh;

	FrameArray<bool> isDirty{ true };
};

struct SceneAnimatedGeometry {
	glm::mat4 transform{};
	vl::GpuHandle<SkinnedMesh> mesh;
	PodHandle<SkinnedMesh> modelPod;

	std::vector<glm::mat4> jointMatrices;

	FrameArray<vk::DescriptorSet> descSet;
	FrameArray<vl::RBuffer> buffer;

	void UploadSsbo(uint32 curFrame);

	SceneAnimatedGeometry()
	{
		for (size_t i = 0; i < c_framesInFlight; ++i) {
			descSet[i] = vl::Layouts->jointsDescLayout.GetDescriptorSet();
		}
	}

	void ResizeJoints(uint32 curFrame);


	void MaybeResizeJoints(size_t newSize);

	FrameArray<bool> isDirty{ true };
	FrameArray<bool> isDirtyResize{ true };

private:
	size_t GetBufferSize() const;
};
