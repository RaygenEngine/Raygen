#include "pch.h"
#include "GpuMaterialArchetype.h"

#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Renderer.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/passes/DepthmapPass.h"


using namespace vl;

namespace {
vk::UniqueShaderModule CreateShaderModule(const std::vector<uint32_t>& code)
{
	vk::ShaderModuleCreateInfo createInfo{};
	createInfo.setCodeSize(code.size() * 4).setPCode(code.data());
	return vl::Device->createShaderModuleUnique(createInfo);
}

std::vector<vk::PipelineShaderStageCreateInfo> CreateShaderStages(
	const char* prototypePath, vk::ShaderModule fragModule)
{
	GpuAsset<Shader>& originalShader = GpuAssetManager->CompileShader(prototypePath);
	vk::ShaderModule vert = originalShader.vert.Lock().module.get();

	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(2);

	shaderStages[0]
		.setStage(vk::ShaderStageFlagBits::eVertex) //
		.setModule(vert)
		.setPName("main");

	shaderStages[1]
		.setStage(vk::ShaderStageFlagBits::eFragment) //
		.setModule(fragModule)
		.setPName("main");

	return shaderStages;
}

vk::UniquePipelineLayout CreatePipelineLayout(size_t pcSize, const std::vector<vk::DescriptorSetLayout>& layouts)
{
	vk::PushConstantRange pushConstantRange{};
	pushConstantRange
		.setStageFlags(vk::ShaderStageFlagBits::eVertex) //
		.setSize(pcSize)
		.setOffset(0u);

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo
		.setSetLayoutCount(static_cast<uint32>(layouts.size())) //
		.setPSetLayouts(layouts.data())
		.setPushConstantRangeCount(1u)
		.setPPushConstantRanges(&pushConstantRange);

	return Device->createPipelineLayoutUnique(pipelineLayoutInfo);
}

} // namespace

MaterialArchetype::Gpu::Gpu(PodHandle<MaterialArchetype> podHandle)
	: GpuAssetTemplate(podHandle)
{
	Update({}); // NOTE: Virtual function call in constructor will not call subclasses overrides, thats why we
				// explicitly mark this function as final in the header
}

void MaterialArchetype::Gpu::Update(const AssetUpdateInfo& info)
{
	auto arch = podHandle.Lock();
	ClearDependencies();

	{
		auto createDescLayout = [&]() {
			descLayout = std::make_unique<RDescriptorLayout>();
			if (arch->descriptorSetLayout.SizeOfUbo() != 0) {
				descLayout->AddBinding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
			}

			for (uint32 i = 0; i < arch->descriptorSetLayout.samplers2d.size(); ++i) {
				descLayout->AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
			}
			descLayout->Generate();
		};

		if (!descLayout) {
			createDescLayout();
		}
		else if (descLayout->bindings.size() != arch->descriptorSetLayout.samplers2d.size() + 1) {
			createDescLayout();
		}
	}

	std::vector descLayouts = { descLayout->setLayout.get(), Layouts->singleUboDescLayout.setLayout.get() };

	auto createPassInfoFrag
		= [&](const char* originalShader, size_t pushConstantSize, const std::vector<uint32_t>& fragBinary) {
			  PassInfo info;
			  info.shaderModules.emplace_back(CreateShaderModule(fragBinary));

			  info.shaderStages = CreateShaderStages(originalShader, *info.shaderModules[0]);

			  info.pipelineLayout = CreatePipelineLayout(pushConstantSize, descLayouts);
			  return std::move(info);
		  };


	gbuffer = createPassInfoFrag(
		"engine-data/spv/gbuffer.shader", GbufferPass::GetPushConstantSize(), arch->gbufferFragBinary);

	depth = createPassInfoFrag(
		"engine-data/spv/depth_map.shader", DepthmapPass::GetPushConstantSize(), arch->depthBinary);


	// gbufferFragModule = CreateShaderModule(arch->gbufferFragBinary);
	// depthFragModule = CreateShaderModule(arch->depthBinary);

	// auto podPtr = podHandle.Lock();

	// gbufferShaderStages = CreateShaderStages("engine-data/spv/gbuffer.shader", *gbufferFragModule);
	// depthShaderStages = CreateShaderStages("engine-data/spv/depth_map.shader", *depthFragModule);


	// gbufferPipelineLayout = CreatePipelineLayout(GbufferPass::GetPushConstantSize(), descLayouts);
	// depthPipelineLayout = CreatePipelineLayout(DepthmapPass::GetPushConstantSize(), { descLayout->setLayout.get() });
}
