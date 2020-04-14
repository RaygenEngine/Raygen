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

// WIP: we should have a dirty per frace
void Scene_::UploadDirty()
{
	for (auto cam : cameras.elements) {
		if (cam->isDirty[vl::Renderer_::currentFrame]) {
			cam->UploadUbo(vl::Renderer_::currentFrame);
			cam->isDirty[vl::Renderer_::currentFrame] = false;
		}
	}

	for (auto sl : spotlights.elements) {
		if (sl->isDirty[vl::Renderer_::currentFrame]) {
			sl->UploadUbo(vl::Renderer_::currentFrame);
			sl->isDirty[vl::Renderer_::currentFrame] = false;
		}
	}

	for (auto rp : reflProbs.elements) {
		if (rp->isDirty[vl::Renderer_::currentFrame]) {
			rp->UploadUbo(vl::Renderer_::currentFrame);
			rp->isDirty[vl::Renderer_::currentFrame] = false;
		}
	}
}
