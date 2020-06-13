#include "pch.h"
#include "Material.h"

#include "assets/util/SpirvCompiler.h"
#include "assets/util/SpirvReflector.h"
#include "assets/util/shadergen/ShaderGen.h"
#include "assets/PodEditor.h"
#include "reflection/ReflectionTools.h"

namespace {
void RerouteShaderErrors(shd::GeneratedShaderErrors& errors)
{
	TextCompilerErrors sharedErrors;
	TextCompilerErrors inOutErrors;
	TextCompilerErrors descSetErrors;
	for (auto& [name, errors] : errors.editorErrors) {
		for (auto& error : errors.errors) {
			int32 div = error.first / shd::c_errorLineModifier;
			int32 mod = error.first % shd::c_errorLineModifier;
			switch (div) {
				case 1:
					inOutErrors.errors.insert({ mod, error.second });
					LOG_ERROR("InOut Shader compilation Error: {}", error.second); // CHECK: Temp hack:
					break;
				case 2:
					descSetErrors.errors.insert({ mod, error.second });
					LOG_ERROR("Descriptor Set Shader compilation Error: {}", error.second); // CHECK: Temp hack:
					break;
				case 3: sharedErrors.errors.insert({ mod, error.second }); break;
			}
		}
	}

	errors.editorErrors.insert({ "Shared", std::move(sharedErrors) });
}
} // namespace


void MaterialArchetype::ChangeLayout(DynamicDescriptorSetLayout&& newLayout)
{
	for (auto& instance : instances) {
		PodEditor editor(instance);
		editor->descriptorSet.SwapLayout(descriptorSetLayout, newLayout);
	}

	descriptorSetLayout = std::move(newLayout);
}

bool MaterialArchetype::CompileAll(
	DynamicDescriptorSetLayout&& newLayout, shd::GeneratedShaderErrors& outErrors, bool outputToConsole)
{
	std::string descSetCode = shd::GenerateDescriptorSetCode(newLayout, newLayout.uboName);

	std::string depth = shd::GenerateDepthShader(descSetCode, sharedFunctions, depthShader);

	if (outputToConsole) {
		LOG_REPORT("DEPTH SHADER: === \n{}", depth);
	}

	outErrors.editorErrors.clear();
	auto depthErrors = &outErrors.editorErrors.insert({ "Depth", {} }).first->second;
	auto depthBin = ShaderCompiler::Compile(depth, ShaderStageType::Fragment, depthErrors);

	if (!depthBin.size()) {
		RerouteShaderErrors(outErrors);
		return false;
	}

	std::string fragCode = shd::GenerateGBufferFrag(descSetCode, sharedFunctions, gbufferFragMain);

	if (outputToConsole) {
		LOG_REPORT("GBUFFER FRAG SHADER: === \n{}", fragCode);
	}


	auto fragErrors = &outErrors.editorErrors.insert({ "Fragment", {} }).first->second;
	auto fragBin = ShaderCompiler::Compile(fragCode, ShaderStageType::Fragment, fragErrors);

	if (!fragBin.size()) {
		RerouteShaderErrors(outErrors);
		return false;
	}
	// All stages compiled, swap layout and store the new binaries
	RerouteShaderErrors(outErrors);


	ChangeLayout(std::move(newLayout));

	gbufferFragBinary.swap(fragBin);
	depthBinary.swap(depthBin);

	return true;
}

void DynamicDescriptorSet::SwapLayout(
	const DynamicDescriptorSetLayout& oldLayout, const DynamicDescriptorSetLayout& newLayout)
{
	if (uboData.size() < oldLayout.SizeOfUbo()) {
		uboData.resize(newLayout.SizeOfUbo());
		samplers2d.resize(newLayout.samplers2d.size());
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
