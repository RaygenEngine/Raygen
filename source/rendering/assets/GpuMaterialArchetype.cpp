#include "GpuMaterialArchetype.h"

#include "assets/pods/MaterialArchetype.h"
#include "rendering/Layouts.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/pipes/geometry/DepthmapPipe.h"
#include "rendering/pipes/geometry/GBufferPipe.h"
#include "rendering/pipes/geometry/UnlitPipe.h"

using namespace vl;

namespace {
vk::UniqueShaderModule CreateShaderModule(const std::vector<uint32_t>& code)
{
	vk::ShaderModuleCreateInfo createInfo{};
	createInfo
		.setCodeSize(code.size() * 4) //
		.setPCode(code.data());
	return Device->createShaderModuleUnique(createInfo);
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
		.setSetLayouts(layouts) //
		.setPushConstantRanges(pushConstantRange);

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

	if (arch->gbufferFragBinary.empty()) {
		arch = PodHandle<MaterialArchetype>().Lock();
		LOG_WARN("GBufferFragBinary is empty on Gpu Update Archetype. Using default archetype.");
	}

	{
		auto createDescLayout = [&]() {
			descLayout = {};
			if (arch->descriptorSetLayout.SizeOfUbo() != 0) {
				descLayout.AddBinding(vk::DescriptorType::eUniformBuffer,
					vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex);
			}

			for (uint32 i = 0; i < arch->descriptorSetLayout.samplers2d.size(); ++i) {
				descLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler,
					vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex);
			}
			descLayout.Generate();
		};

		if (!descLayout.HasBeenGenerated()) {
			createDescLayout();
		}
		else if (descLayout.GetBindings().size() != arch->descriptorSetLayout.samplers2d.size() + 1) {
			createDescLayout();
		}
	}

	std::vector descLayouts{
		descLayout.handle(),
		Layouts->singleUboDescLayout.handle(),
	};

	depth = CreatePassInfoOptional<DepthmapPipe>(
		"engine-data/spv/geometry/depth_map.shader", arch->depthVertBinary, arch->depthFragBinary, descLayouts);

	gbuffer = CreatePassInfoOptional<GbufferPipe>(
		"engine-data/spv/geometry/gbuffer.shader", arch->gbufferVertBinary, arch->gbufferFragBinary, descLayouts);


	std::vector descLayoutsAnim{
		descLayout.handle(),
		Layouts->singleUboDescLayout.handle(),
		Layouts->jointsDescLayout.handle(),
	};

	// GBufferAnim Pass
	{
		size_t pushConstantSize = GbufferPipe::GetPushConstantSize();
		GpuMaterialArchetype::PassInfo info;
		info.shaderModules.emplace_back(CreateShaderModule(arch->gbufferFragBinary));
		info.shaderStages = CreateShaderStages("engine-data/spv/geometry/gbuffer-anim.shader", *info.shaderModules[0]);
		info.pipelineLayout = CreatePipelineLayout(pushConstantSize, descLayoutsAnim);
		info.pipeline = GbufferPipe::CreateAnimPipeline(*info.pipelineLayout, info.shaderStages);

		gbufferAnimated = std::move(info);
	}

	// DepthAnim Pass
	{
		size_t pushConstantSize = DepthmapPipe::GetPushConstantSize();

		GpuMaterialArchetype::PassInfo info;
		info.shaderModules.emplace_back(CreateShaderModule(arch->depthFragBinary));
		info.shaderStages = CreateShaderStages("engine-data/spv/geometry/depthmap-anim.shader", *info.shaderModules[0]);
		info.pipelineLayout = CreatePipelineLayout(pushConstantSize, descLayoutsAnim);
		info.pipeline = DepthmapPipe::CreateAnimPipeline(*info.pipelineLayout, info.shaderStages);

		depthAnimated = std::move(info);
	}


	// Unlit Pass
	{
		isUnlit = arch->unlitFragBinary.size() > 0;

		if (isUnlit) {
			size_t pushConstantSize = UnlitPipe::GetPushConstantSize();

			GpuMaterialArchetype::PassInfo info;
			info.shaderModules.emplace_back(CreateShaderModule(arch->unlitFragBinary));
			info.shaderStages = CreateShaderStages("engine-data/spv/geometry/gbuffer.shader", *info.shaderModules[0]);
			info.pipelineLayout = CreatePipelineLayout(pushConstantSize, descLayouts);

			info.pipeline = UnlitPipe::CreatePipeline(*info.pipelineLayout, info.shaderStages);

			unlit = std::move(info);
		}
	}
}
