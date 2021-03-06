#include "BakeProbes.h"

#include "rendering/pipes/CubemapConvolutionPipe.h"
#include "rendering/pipes/CubemapPrefilterPipe.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/pipes/StochasticPathtracePipe.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneIrragrid.h"
#include "rendering/scene/SceneReflProbe.h"
#include "rendering/wrappers/CmdBuffer.h"

namespace {
struct UBO_viewer {
	glm::mat4 viewInv;
	glm::mat4 projInv;
	float offset;
};

inline glm::mat4 standardProjection()
{
	glm::mat4 projInverse = glm::inverse(glm::perspective(1.5708f, 1.f, 1.f, 25.f));
	projInverse[1][1] *= -1;

	return projInverse;
}

} // namespace

namespace vl {
void BakeProbes::RecordCmd(const SceneRenderDesc& sceneDesc)
{
	for (auto rp : sceneDesc->Get<SceneReflprobe>()) {
		if (rp->shouldBuild.Access()) [[unlikely]] {

			rp->environment.BlockingTransitionToLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
				vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

			BakeEnvironment(sceneDesc, rp->environment.GetFaceViews(), rp->ubo.position, rp->ptSamples, rp->ptBounces,
				rp->ubo.radius, rp->environment.extent);

			rp->environment.BlockingTransitionToLayout(vk::ImageLayout::eGeneral,
				vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eRayTracingShaderKHR,
				vk::PipelineStageFlagBits::eComputeShader);

			rp->irradiance.BlockingTransitionToLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
				vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader);

			BakeIrradiance(rp->irradiance.GetFaceViews(), rp->environmentSamplerDescSet, rp->irradiance.extent);

			rp->irradiance.BlockingTransitionToLayout(vk::ImageLayout::eGeneral,
				vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eComputeShader,
				vk::PipelineStageFlagBits::eFragmentShader);

			rp->prefiltered.BlockingTransitionToLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
				vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader);

			BakePrefiltered(*rp);

			rp->prefiltered.BlockingTransitionToLayout(vk::ImageLayout::eGeneral,
				vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eComputeShader,
				vk::PipelineStageFlagBits::eFragmentShader);
		}
	}

	for (auto ig : sceneDesc->Get<SceneIrragrid>()) {
		if (ig->shouldBuild.Access()) [[unlikely]] {

			ig->environmentCubemaps.BlockingTransitionToLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
				vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

			for (int32 x = 0; x < ig->ubo.width; ++x) {
				for (int32 y = 0; y < ig->ubo.height; ++y) {
					for (int32 z = 0; z < ig->ubo.depth; ++z) {

						auto pos = glm::vec3(ig->ubo.posAndDist) + glm::vec3(x, y, z) * ig->ubo.posAndDist.w;

						int i = 0;
						i += x;
						i += y * ig->ubo.width;
						i += z * ig->ubo.width * ig->ubo.height;

						BakeEnvironment(sceneDesc, ig->environmentCubemaps.GetFaceViews(i), pos, ig->ptSamples,
							ig->ptBounces, 0.0, ig->environmentCubemaps.extent);
					}
				}
			}

			ig->environmentCubemaps.BlockingTransitionToLayout(vk::ImageLayout::eGeneral,
				vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eRayTracingShaderKHR,
				vk::PipelineStageFlagBits::eFragmentShader);

			ig->irradianceCubemaps.BlockingTransitionToLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
				vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader);

			for (int32 x = 0; x < ig->ubo.width; ++x) {
				for (int32 y = 0; y < ig->ubo.height; ++y) {
					for (int32 z = 0; z < ig->ubo.depth; ++z) {

						auto pos = glm::vec3(ig->ubo.posAndDist) + glm::vec3(x, y, z) * ig->ubo.posAndDist.w;

						int i = 0;
						i += x;
						i += y * ig->ubo.width;
						i += z * ig->ubo.width * ig->ubo.height;

						vk::DescriptorSet environmentCubemapSamplerDescSet;
						environmentCubemapSamplerDescSet
							= Layouts->singleSamplerDescLayout.AllocDescriptorSet(); // WIP: doesn't release

						auto cubemapView = ig->environmentCubemaps.GetCubemapView(i);

						rvk::writeDescriptorImages(environmentCubemapSamplerDescSet, 0u, { *cubemapView });

						BakeIrradiance(ig->irradianceCubemaps.GetFaceViews(i), environmentCubemapSamplerDescSet,
							ig->irradianceCubemaps.extent);
					}
				}
			}


			ig->irradianceCubemaps.BlockingTransitionToLayout(vk::ImageLayout::eGeneral,
				vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eComputeShader,
				vk::PipelineStageFlagBits::eFragmentShader);
		}
	}
}

void BakeProbes::BakeEnvironment(const SceneRenderDesc& sceneDesc, const std::vector<vk::UniqueImageView>& faceViews,
	const glm::vec3& probePosition, int32 ptSamples, int32 ptBounces, float offset, const vk::Extent3D& extent)
{
	RBuffer viewer[6];

	struct UBO_viewer {
		glm::mat4 viewInv;
		glm::mat4 projInv;
		float offset;
	};

	vk::DescriptorSet faceDescSet[6];
	vk::DescriptorSet viewerDescSet[6];

	// clang-format off

	glm::mat4 viewInverses[6] = {
		glm::inverse(glm::lookAt(probePosition, probePosition + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0))), // right
		glm::inverse(glm::lookAt(probePosition, probePosition + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0))), // left
		glm::inverse(glm::lookAt(probePosition, probePosition + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0))), // up
		glm::inverse(glm::lookAt(probePosition, probePosition + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0))), // down
		glm::inverse(glm::lookAt(probePosition, probePosition + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 1.0, 0.0))), // front
		glm::inverse(glm::lookAt(probePosition, probePosition + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0))), // back
	};

	// clang-format on

	// TODO: estimate time to complete (roughly)
	// command buffers shouldn't be divided per face
	auto iterations = 1 + ptSamples / 128;
	auto samplesPerIteration = ptSamples / iterations;
	auto finalIterationSamples = samplesPerIteration + ptSamples % iterations;

	for (auto i = 0; i < 6; ++i) {
		faceDescSet[i] = Layouts->singleStorageImage.AllocDescriptorSet(); // WIP: doesn't release
		rvk::writeDescriptorImages(
			faceDescSet[i], 0u, { *faceViews[i] }, vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);

		viewerDescSet[i] = Layouts->singleUboDescLayout.AllocDescriptorSet(); // WIP: doesn't release

		auto uboSize = sizeof(UBO_viewer);

		viewer[i] = vl::RBuffer{ uboSize, vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

		rvk::writeDescriptorBuffer(viewerDescSet[i], 0u, viewer[i].handle());

		UBO_viewer data = {
			viewInverses[i],
			standardProjection(),
			offset,
		};

		viewer[i].UploadData(&data, uboSize);


		for (auto iter = 0; iter < iterations; ++iter) {

			ScopedOneTimeSubmitCmdBuffer<Graphics> cmdBuffer{};

			StaticPipes::Get<StochasticPathtracePipe>().Draw(cmdBuffer, sceneDesc, faceDescSet[i], viewerDescSet[i],
				extent, iter, iter != iterations - 1 ? samplesPerIteration : finalIterationSamples, ptBounces);
		}
	}
}

void BakeProbes::BakeIrradiance(const std::vector<vk::UniqueImageView>& faceViews,
	vk::DescriptorSet environmentSamplerDescSet, const vk::Extent3D& extent)
{
	vk::DescriptorSet faceDescSet[6];
	vk::DescriptorSet viewerDescSet[6];

	// clang-format off

	glm::mat4 viewInverses[6] = {
		glm::inverse(glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0))), // right
		glm::inverse(glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0))), // left
		glm::inverse(glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0))), // up
		glm::inverse(glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0))), // down
		glm::inverse(glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 1.0, 0.0))), // front
		glm::inverse(glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0))), // back
	};
	// clang-format on

	ScopedOneTimeSubmitCmdBuffer<Compute> cmdBuffer{};

	for (auto i = 0; i < 6; ++i) {

		faceDescSet[i] = Layouts->singleStorageImage.AllocDescriptorSet(); // WIP: doesn't release
		rvk::writeDescriptorImages(
			faceDescSet[i], 0u, { *faceViews[i] }, vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);


		StaticPipes::Get<CubemapConvolutionPipe>().Draw(
			cmdBuffer, faceDescSet[i], environmentSamplerDescSet, extent, viewInverses[i], standardProjection());
	}
}

void BakeProbes::BakePrefiltered(const SceneReflprobe& rp)
{
	auto projInv = standardProjection();

	// clang-format off
	glm::mat4 viewInverses[6] = {
		glm::inverse(glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0))), // right
		glm::inverse(glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0))), // left
		glm::inverse(glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0))), // up
		glm::inverse(glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0))), // down
		glm::inverse(glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 1.0, 0.0))), // front
		glm::inverse(glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0))), // back
	};
	// clang-format on

	for (auto mip = 0; mip < rp.prefiltered.mipLevels; ++mip) {
		// get mip's faces
		auto faceViews = rp.prefiltered.GetFaceViews(mip);

		vk::DescriptorSet faceDescSet[6];

		uint32 mipResolution = static_cast<uint32>(rp.prefiltered.extent.width * std::pow(0.5, mip));

		ScopedOneTimeSubmitCmdBuffer<Graphics> cmdBuffer{};

		// pigy bag roughness inside this matrix to avoid descriptor sets
		projInv[3][3] = float(mip) / float(rp.ubo.lodCount - 1);

		for (auto i = 0; i < 6; ++i) {
			faceDescSet[i] = Layouts->singleStorageImage.AllocDescriptorSet(); // WIP: doesn't release
			rvk::writeDescriptorImages(
				faceDescSet[i], 0u, { *faceViews[i] }, vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);


			StaticPipes::Get<CubemapPrefilterPipe>().Draw(cmdBuffer, faceDescSet[i], rp.environmentSamplerDescSet,
				{ mipResolution, mipResolution, 1 }, viewInverses[i], projInv);
		}
	}
}

} // namespace vl
