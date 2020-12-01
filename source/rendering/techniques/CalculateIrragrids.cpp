#include "CalculateIrragrids.h"

#include "rendering/pipes/CubemapArrayConvolutionPipe.h"
#include "rendering/pipes/PathtraceCubemapArrayPipe.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneIrragrid.h"

namespace vl {
void CalculateIrragrids::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const
{
	for (auto ig : sceneDesc->Get<SceneIrragrid>()) {
		if (ig->shouldBuild.Access()) [[unlikely]] {

			ig->environmentCubemaps.TransitionToLayout(cmdBuffer, vk::ImageLayout::eUndefined,
				vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eTopOfPipe,
				vk::PipelineStageFlagBits::eRayTracingShaderKHR);

			StaticPipes::Get<PathtraceCubemapArrayPipe>().Draw(cmdBuffer, sceneDesc, *ig);

			ig->environmentCubemaps.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
				vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eRayTracingShaderKHR,
				vk::PipelineStageFlagBits::eComputeShader);

			ig->irradianceCubemaps.TransitionToLayout(cmdBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
				vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader);

			// CHECK: use compute buffer
			StaticPipes::Get<CubemapArrayConvolutionPipe>().Draw(cmdBuffer, sceneDesc, *ig);

			ig->irradianceCubemaps.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
				vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eComputeShader,
				vk::PipelineStageFlagBits::eFragmentShader);
		}
	}
}
} // namespace vl
