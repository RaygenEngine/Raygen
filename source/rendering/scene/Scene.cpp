#include "pch.h"
#include "Scene.h"

#include "rendering/Renderer.h"

void Scene_::DrainQueueForDestruction()
{
	ExecuteCreations();

	size_t begin = 0;
	size_t end = cmds.size();

	if (end <= 0) {
		return;
	}

	for (size_t i = 0; i < end; ++i) {
		for (auto& cmd : *cmds[i]) {
			cmd();
		}
	}

	// std::lock_guard<std::mutex> guard(cmdBuffersVectorMutex);
	cmds.erase(cmds.begin(), cmds.begin() + end);
}

Scene_::Scene_(size_t size)
	: size(size)
{
	EnqueueEndFrame();
}

vk::DescriptorSet Scene_::GetActiveCameraDescSet()
{
	return GetActiveCamera()->descSets[vl::Renderer_::currentFrame];
}

// TODO: we should have a dirty per frace
void Scene_::UploadDirty()
{
	for (auto cam : cameras.elements) {
		if (cam && cam->isDirty[vl::Renderer_::currentFrame]) {
			cam->UploadUbo(vl::Renderer_::currentFrame);
			cam->isDirty[vl::Renderer_::currentFrame] = false;
		}
	}

	for (auto sl : spotlights.elements) {
		if (sl && sl->isDirty[vl::Renderer_::currentFrame]) {
			sl->UploadUbo(vl::Renderer_::currentFrame);
			sl->isDirty[vl::Renderer_::currentFrame] = false;
		}
	}

	for (auto an : animatedGeometries.elements) {
		if (an && an->isDirty[vl::Renderer_::currentFrame]) {
			an->UploadUbo(vl::Renderer_::currentFrame);
			an->isDirty[vl::Renderer_::currentFrame] = false;
		}
	}

	// for (auto rp : reflProbs.elements) {
	//	if (rp && rp->isDirty[vl::Renderer_::currentFrame]) {
	//		rp->UploadUbo(vl::Renderer_::currentFrame);
	//		rp->isDirty[vl::Renderer_::currentFrame] = false;
	//	}
	//}
}

// WIP:
ConsoleFunction<> console_BuildAll{ "s.buildAll", []() { Scene->BuildAll(); }, "Builds all build-able scene nodes" };
