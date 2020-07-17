#pragma once
#include "rendering/Layouts.h"

namespace vl {
class RBuffer;
}

// SceneStructs that upload a Ubo when dirty
struct SceneStruct {
	size_t uboSize;

	std::array<vk::DescriptorSet, 3> descSets;
	std::array<UniquePtr<vl::RBuffer>, 3> buffers;

	std::array<bool, 3> isDirty{ true, true, true };


	SceneStruct(size_t uboSize);

	void UploadDataToUbo(uint32 curFrame, void* data, size_t size);
};

#define SCENE_STRUCT(Child)                                                                                            \
	Child()                                                                                                            \
		: SceneStruct(sizeof(decltype(ubo)))                                                                           \
	{                                                                                                                  \
	}                                                                                                                  \
	void UploadUbo(uint32 curFrame) { UploadDataToUbo(curFrame, &ubo, sizeof(decltype(ubo))); }
