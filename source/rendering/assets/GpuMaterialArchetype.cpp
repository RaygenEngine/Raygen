#include "pch.h"
#include "GpuMaterialArchetype.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/passes/DepthmapPass.h"
#include "rendering/passes/GBufferPass.h"
#include "assets/pods/MaterialArchetype.h"

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
		.setSize(static_cast<uint32>(pcSize))
		.setOffset(0u);

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo
		.setSetLayoutCount(static_cast<uint32>(layouts.size())) //
		.setPSetLayouts(layouts.data())
		.setPushConstantRangeCount(1u)
		.setPPushConstantRanges(&pushConstantRange);

	return Device->createPipelineLayoutUnique(pipelineLayoutInfo);
}

template<typename Pass>
GpuMaterialArchetype::PassInfo CreatePassInfoFrag(const char* originalShader, const std::vector<uint32_t>& fragBinary,
	const std::vector<vk::DescriptorSetLayout>& descLayouts)
{
	size_t pushConstantSize = Pass::GetPushConstantSize();

	GpuMaterialArchetype::PassInfo info;
	info.shaderModules.emplace_back(CreateShaderModule(fragBinary));

	info.shaderStages = CreateShaderStages(originalShader, *info.shaderModules[0]);

	info.pipelineLayout = CreatePipelineLayout(pushConstantSize, descLayouts);

	info.pipeline = Pass::CreatePipeline(*info.pipelineLayout, info.shaderStages);
	return std::move(info);
};


template<typename Pass>
GpuMaterialArchetype::PassInfo CreatePassInfoOptional(const char* originalShader,
	const std::vector<uint32_t>& vertBinary, const std::vector<uint32_t>& fragBinary,
	const std::vector<vk::DescriptorSetLayout>& descLayouts)
{
	if (vertBinary.size() > 0) {
		size_t pushConstantSize = Pass::GetPushConstantSize();

		GpuMaterialArchetype::PassInfo info;
		info.shaderModules.emplace_back(CreateShaderModule(vertBinary));
		auto& frag = info.shaderModules.emplace_back(CreateShaderModule(fragBinary));


		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(2);

		shaderStages[0]
			.setStage(vk::ShaderStageFlagBits::eVertex) //
			.setModule(*info.shaderModules[0])
			.setPName("main");

		shaderStages[1]
			.setStage(vk::ShaderStageFlagBits::eFragment) //
			.setModule(*frag)
			.setPName("main");


		info.shaderStages = std::move(shaderStages);

		info.pipelineLayout = CreatePipelineLayout(pushConstantSize, descLayouts);

		info.pipeline = Pass::CreatePipeline(*info.pipelineLayout, info.shaderStages);
		return info;
	}
	else {
		return CreatePassInfoFrag<Pass>(originalShader, fragBinary, descLayouts);
	}
};

} // namespace

GpuMaterialArchetype::GpuMaterialArchetype(PodHandle<MaterialArchetype> podHandle)
	: GpuAssetTemplate(podHandle)
{
	Update({}); // NOTE: Virtual function call in constructor will not call subclasses overrides, thats why we
				// explicitly mark this function as final in the header
}

void GpuMaterialArchetype::Update(const AssetUpdateInfo& updateInfo)
{
	auto arch = podHandle.Lock();
	ClearDependencies();

	{
		auto createDescLayout = [&]() {
			descLayout = std::make_unique<RDescriptorLayout>();
			if (arch->descriptorSetLayout.SizeOfUbo() != 0) {
				descLayout->AddBinding(vk::DescriptorType::eUniformBuffer,
					vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex);
			}

			for (uint32 i = 0; i < arch->descriptorSetLayout.samplers2d.size(); ++i) {
				descLayout->AddBinding(vk::DescriptorType::eCombinedImageSampler,
					vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex);
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

	depth = CreatePassInfoOptional<DepthmapPass>(
		"engine-data/spv/geometry/depth_map.shader", arch->depthVertBinary, arch->depthFragBinary, descLayouts);

	gbuffer = CreatePassInfoOptional<GbufferPass>(
		"engine-data/spv/geometry/gbuffer.shader", arch->gbufferVertBinary, arch->gbufferFragBinary, descLayouts);


	// GBufferAnim Pass
	{
		size_t pushConstantSize = GbufferPass::GetPushConstantSize();
		GpuMaterialArchetype::PassInfo info;
		info.shaderModules.emplace_back(CreateShaderModule(arch->gbufferFragBinary));
		info.shaderStages = CreateShaderStages("engine-data/spv/geometry/gbuffer-anim.shader", *info.shaderModules[0]);
		info.pipelineLayout = CreatePipelineLayout(
			pushConstantSize, { descLayout->setLayout.get(), Layouts->singleUboDescLayout.setLayout.get(),
								  Layouts->jointsDescLayout.setLayout.get() });
		info.pipeline = GbufferPass::CreateAnimPipeline(*info.pipelineLayout, info.shaderStages);

		gbufferAnimated = std::move(info);
	}

	// DepthAnim Pass
	{
		size_t pushConstantSize = DepthmapPass::GetPushConstantSize();

		GpuMaterialArchetype::PassInfo info;
		info.shaderModules.emplace_back(CreateShaderModule(arch->depthFragBinary));
		info.shaderStages = CreateShaderStages("engine-data/spv/geometry/depthmap-anim.shader", *info.shaderModules[0]);
		info.pipelineLayout = CreatePipelineLayout(
			pushConstantSize, { descLayout->setLayout.get(), Layouts->singleUboDescLayout.setLayout.get(),
								  Layouts->jointsDescLayout.setLayout.get() });
		info.pipeline = DepthmapPass::CreateAnimPipeline(*info.pipelineLayout, info.shaderStages);

		depthAnimated = std::move(info);
	}
}
