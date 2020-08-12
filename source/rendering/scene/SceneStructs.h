#pragma once
#include "rendering/Layouts.h"
#include "rendering/wrappers/RBuffer.h"

// SceneStructs that upload a Ubo when dirty
struct SceneStruct {
	size_t uboSize;

	FrameArray<vk::DescriptorSet> descSet;
	FrameArray<vl::RBuffer> buffer;

	FrameArray<bool> isDirty{ true };

	SceneStruct(size_t uboSize);

	void UploadDataToUbo(uint32 curFrame, void* data, size_t size);
};

#define SCENE_STRUCT(Child)                                                                                            \
	Child()                                                                                                            \
		: SceneStruct(sizeof(decltype(ubo)))                                                                           \
	{                                                                                                                  \
	}                                                                                                                  \
	void UploadUbo(uint32 curFrame) { UploadDataToUbo(curFrame, &ubo, sizeof(decltype(ubo))); }
