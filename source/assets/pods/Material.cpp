#include "pch.h"
#include "Material.h"

#include "assets/util/SpirvCompiler.h"
#include "assets/util/SpirvReflector.h"
#include "assets/util/shadergen/ShaderGen.h"
#include "assets/PodEditor.h"
#include "reflection/ReflectionTools.h"


void MaterialArchetype::ChangeLayout(DynamicDescriptorSetLayout&& newLayout)
{
	for (auto& instance : instances) {
		PodEditor editor(instance);
		editor->descriptorSet.SwapLayout(descriptorSetLayout, newLayout);
	}

	descriptorSetLayout = std::move(newLayout);
}

bool MaterialArchetype::GenerateGBufferFrag(const DynamicDescriptorSetLayout& forLayout, TextCompilerErrors* outErrors)
{
	auto binary = ShaderCompiler::Compile(
		shd::GenerateGBufferFrag(forLayout, gbufferFragMain), ShaderStageType::Fragment, outErrors);
	if (!binary.size()) {
		return false;
	}
	binary.swap(gbufferFragBinary);
	return true;
}

void DynamicDescriptorSet::SwapLayout(
	const DynamicDescriptorSetLayout& oldLayout, const DynamicDescriptorSetLayout& newLayout)
{
	if (uboData.size() < oldLayout.SizeOfUbo()) {
		uboData.clear();
		uboData.resize(newLayout.SizeOfUbo());
		return;
	}

	std::vector<byte> newData(newLayout.SizeOfUbo());
	// Attempt to match as many properties as possible, but don't really care about errors.
	// CHECK: Note that data here is zeroed out if not overwritten.
	// This is currently fine for all types supported in custom ubos.

	refltools::CopyClassToEx(uboData.data(), newData.data(), oldLayout.uboClass, newLayout.uboClass);
	uboData.swap(newData);

	samplers2d.resize(newLayout.samplers2d.size());
}
