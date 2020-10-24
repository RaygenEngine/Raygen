#include "SceneReflprobe.h"

#include "rendering/assets/GpuEnvironmentMap.h"
#include "rendering/offline/AmbientBaker.h"
#include "rendering/wrappers/CmdBuffer.h"
#include "rendering/util/WriteDescriptorSets.h"

#include "engine/Timer.h"


// WIP:
#include "rendering/Layer.h"
#include "rendering/scene/Scene.h"
#include "rendering/Device.h"

using namespace vl;

SceneReflprobe::SceneReflprobe()
	: SceneStruct(sizeof(decltype(ubo)))
{
	reflDescSet = Layouts->envmapLayout.AllocDescriptorSet();
	ab = new AmbientBaker(this);
	ab->Resize(256);

	reflDescSet = Layouts->envmapLayout.AllocDescriptorSet();

	ab->m_surroundingEnv.BlockingTransitionToLayout(vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR, vk::PipelineStageFlagBits::eFragmentShader);

	ab->m_irradiance.BlockingTransitionToLayout(vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eFragmentShader);

	ab->m_prefiltered.BlockingTransitionToLayout(vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eFragmentShader);

	rvk::writeDescriptorImages(reflDescSet, 0u,
		{
			ab->m_surroundingEnv.view(),
			ab->m_irradiance.view(),
			ab->m_prefiltered.view(),
		});
}

SceneReflprobe::~SceneReflprobe()
{
	delete ab;
}


void SceneReflprobe::Build()
{
	Device->waitIdle();
	TIMER_SCOPE("pt + irr + pref");

	ScopedOneTimeSubmitCmdBuffer<Graphics> cmdBuffer{};

	ab->Resize(256);
	SceneRenderDesc sceneDesc{ Layer->mainScene, 0, 0 };
	ab->RecordPass(cmdBuffer, sceneDesc);

	rvk::writeDescriptorImages(reflDescSet, 0u,
		{
			ab->m_surroundingEnv.view(),
			ab->m_irradiance.view(),
			ab->m_prefiltered.view(),
		});
}
