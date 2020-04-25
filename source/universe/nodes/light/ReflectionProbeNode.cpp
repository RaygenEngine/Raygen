#include "pch.h"
#include "ReflectionProbeNode.h"

#include "assets/AssetImporterManager.h"

ReflectionProbeNode::ReflectionProbeNode()
{
	sceneUid = Scene->EnqueueCreateCmd<SceneReflectionProbe>();
}

ReflectionProbeNode::~ReflectionProbeNode()
{
	Scene->EnqueueDestroyCmd<SceneReflectionProbe>(sceneUid);
}


void ReflectionProbeNode::DirtyUpdate(DirtyFlagset flags)
{
	Node::DirtyUpdate(flags);
	if (flags[DF::SkyTexture]) {

		if (m_skybox.Lock()->width > 0) {
			Enqueue([&](SceneReflectionProbe& rp) { rp.UploadCubemap(m_skybox); });
		}
	}

	if (flags[DF::AmbientTerm]) {
		Enqueue([&](SceneReflectionProbe& rp) { rp.ubo.color = glm::vec4(m_ambientTerm, 1.f); });
	}
}

void ReflectionProbeNode::OnBuild()
{
	auto editablePod = ImporterManager->CreateGeneratedPod(m_skybox, false);
	SetDirty(DF::SkyTexture);

	for (auto& img : editablePod.BeginEdit()->faces) {
		auto edImg = ImporterManager->CreateGeneratedPod(img, false);
		Image* pod = edImg.BeginEdit();

		static int32 x = 0;
		if (x++ % 2 == 0) {
			pod->data[0] = 0xFF;
			pod->data[1] = 0x0;
			pod->data[2] = 0xFF;
		}
		else {
			pod->data[0] = 0x0;
			pod->data[1] = 0xFF;
			pod->data[2] = 0x99;
		}
	}
}
