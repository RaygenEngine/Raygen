#include "AmbientBaker.h"

#include "rendering/Layouts.h"
#include "rendering/util/WriteDescriptorSets.h"

namespace vl {
AmbientBaker::AmbientBaker(SceneReflprobe* rp)
	: m_ptCubemap(rp)
	, m_irrCalc(rp)
	, m_prefCalc(rp)
{
}

void AmbientBaker::RecordPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	m_ptCubemap.RecordPass(cmdBuffer, sceneDesc, m_surroundingEnv.extent.width);

	m_surroundingEnv.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR, vk::PipelineStageFlagBits::eFragmentShader);

	m_irrCalc.RecordPass(cmdBuffer, m_surroundingEnvSamplerDescSet, m_surroundingEnv.extent.width);
	m_prefCalc.RecordPass(cmdBuffer, m_surroundingEnvSamplerDescSet, m_surroundingEnv.extent.width);

	m_irradiance.TransitionToLayout(cmdBuffer, vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eFragmentShader);

	m_prefiltered.TransitionToLayout(cmdBuffer, vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eFragmentShader);
}

void AmbientBaker::Resize(uint32 resolution)
{
	m_surroundingEnv = RCubemap(resolution, 1u, vk::Format::eR32G32B32A32Sfloat, vk::ImageTiling::eOptimal,
		vk::ImageLayout::eUndefined, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal, fmt::format("SurrCube: WIP:reflprobenamehere"));
	// 1.undefined

	// CHECK: indirect_result at Renderer ???
	{
		m_surroundingEnv.BlockingTransitionToLayout(vk::ImageLayout::eUndefined,
			vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eTopOfPipe,
			vk::PipelineStageFlagBits::eFragmentShader);
		// 1.undefined -> shaderRead

		m_surroundingEnvSamplerDescSet = Layouts->cubemapLayout.AllocDescriptorSet();

		rvk::writeDescriptorImages(m_surroundingEnvSamplerDescSet, 0u, { m_surroundingEnv.view() });
	}
	{
		m_surroundingEnv.BlockingTransitionToLayout(vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
			vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eRayTracingShaderKHR);
		// 1.shaderRead -> general

		m_surroundingEnvStorageDescSet = Layouts->singleStorageImage.AllocDescriptorSet();

		rvk::writeDescriptorImages(m_surroundingEnvStorageDescSet, 0u, { m_surroundingEnv.view() }, nullptr,
			vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);
	}

	m_irradiance = RCubemap(resolution, 1u, vk::Format::eR32G32B32A32Sfloat, vk::ImageTiling::eOptimal,
		vk::ImageLayout::eUndefined,
		vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
		vk::MemoryPropertyFlagBits::eDeviceLocal, fmt::format("IrrCube: WIP:reflprobenamehere"));
	// 2.undefined

	m_irradiance.BlockingTransitionToLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
	// 2.undefined -> colorAttachment

	m_prefiltered = RCubemap(resolution, 6u, vk::Format::eR32G32B32A32Sfloat, vk::ImageTiling::eOptimal,
		vk::ImageLayout::eUndefined,
		vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
		vk::MemoryPropertyFlagBits::eDeviceLocal, fmt::format("PreCube: WIP:reflprobenamehere"));
	// 3.undefined

	m_prefiltered.BlockingTransitionToLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
	// 3.undefined -> colorAttachment

	m_ptCubemap.Resize(m_surroundingEnv, resolution);
	m_irrCalc.Resize(m_irradiance, resolution);
	m_prefCalc.Resize(m_prefiltered, resolution);
}

} // namespace vl
