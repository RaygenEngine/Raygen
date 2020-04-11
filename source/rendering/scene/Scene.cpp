#include "pch.h"
#include "Scene.h"

#include "rendering/Renderer.h"

Scene_::Scene_(size_t size)
	: size(size)
{
	EnqueueEndFrame();
}

vk::DescriptorSet Scene_::GetActiveCameraDescSet()
{
	return GetActiveCamera()->descSets[vl::Renderer_::currentFrame];
}

vk::DescriptorSet Scene_::GetActiveSpotlightDescSet() const
{
	return spotlights.elements.at(0)->descSets[vl::Renderer_::currentFrame];
}

// WIP: we should have a dirty per frace
void Scene_::UploadDirty()
{
	for (auto cam : cameras.elements) {
		if (cam->isDirty[vl::Renderer_::currentFrame]) {
			cam->Upload();
			cam->isDirty[vl::Renderer_::currentFrame] = false;
		}
	}

	for (auto sl : spotlights.elements) {
		if (sl->isDirty[vl::Renderer_::currentFrame]) {
			sl->Upload();
			sl->isDirty[vl::Renderer_::currentFrame] = false;
		}
	}
}
