#pragma once

// SceneStructs that upload a Ubo when dirty
struct SceneStruct {
	size_t uboSize;

	SceneStruct(size_t uboSize);
};

#define SCENE_STRUCT(Child)                                                                                            \
	Child()                                                                                                            \
		: SceneStruct(sizeof(decltype(ubo)))                                                                           \
	{                                                                                                                  \
	}\
