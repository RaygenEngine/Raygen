#include "CalculateReflprobes.h"

#include "rendering/pipes/CubemapConvolutionPipe.h"
#include "rendering/pipes/PathtraceCubemapPipe.h"
#include "rendering/pipes/PrefilteredConvolutionPipe.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneReflProbe.h"

namespace vl {
void CalculateReflprobes::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	for (auto rp : sceneDesc->Get<SceneReflprobe>()) {
		if (rp->shouldBuild.Access()) [[unlikely]] {
			rp->environment.TransitionToLayout(cmdBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
				vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

			StaticPipes::Get<PathtraceCubemapPipe>().Draw(cmdBuffer, sceneDesc, *rp);

			rp->environment.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
				vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eRayTracingShaderKHR,
				vk::PipelineStageFlagBits::eComputeShader);

			rp->irradiance.TransitionToLayout(cmdBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
				vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader);

			// CHECK: use compute buffer
			StaticPipes::Get<CubemapConvolutionPipe>().Draw(cmdBuffer, sceneDesc, *rp);

			rp->irradiance.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
				vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eComputeShader,
				vk::PipelineStageFlagBits::eFragmentShader);

			rp->prefiltered.TransitionToLayout(cmdBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
				vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader);

			StaticPipes::Get<PrefilteredConvolutionPipe>().Draw(cmdBuffer, sceneDesc, *rp);

			rp->prefiltered.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
				vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eComputeShader,
				vk::PipelineStageFlagBits::eFragmentShader);
		}
	}
}
} // namespace vl
