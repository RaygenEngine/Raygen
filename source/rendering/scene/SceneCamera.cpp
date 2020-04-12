#include "pch.h"
#include "SceneCamera.h"

#include "rendering/Layouts.h"

SceneCamera::SceneCamera()
	: SceneStruct<Camera_Ubo>()
{
	for (uint32 i = 0; i < 3; ++i) {
		descSets[i] = vl::Layouts->cameraDescLayout.GetDescriptorSet();
	}
}
