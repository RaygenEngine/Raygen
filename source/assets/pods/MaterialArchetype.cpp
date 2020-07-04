#include "pch.h"
#include "MaterialArchetype.h"

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


void MaterialArchetype::MakeGltfArchetypeInto(MaterialArchetype* mat)
{
	mat->gbufferFragBinary = ShaderCompiler::Compile("engine-data/spv/shader-defaults/gbuffer-gltf.frag");
	mat->depthBinary = ShaderCompiler::Compile("engine-data/spv/shader-defaults/depthmap-gltf.frag");

	CLOG_ABORT(mat->gbufferFragBinary.size() == 0 || mat->depthBinary.size() == 0,
		"Failed to compile gltf archetype shader code. (engine-data/spv/shader-defaults/)");

	mat->descriptorSetLayout.samplers2d
		= { "baseColorSampler", "metallicRoughnessSampler", "occlusionSampler", "normalSampler", "emissiveSampler" };
	mat->descriptorSetLayout.uboName = "material";


	auto& cl = mat->descriptorSetLayout.uboClass;

	cl.AddProperty<glm::vec4>("baseColorFactor", PropertyFlags::Color);
	cl.AddProperty<glm::vec4>("emissiveFactor", PropertyFlags::Color);
	cl.AddProperty<float>("metallicFactor");
	cl.AddProperty<float>("roughnessFactor");
	cl.AddProperty<float>("normalScale");
	cl.AddProperty<float>("occlusionStrength");
	cl.AddProperty<float>("alphaCutoff");
	cl.AddProperty<int>("mask");
}

void MaterialArchetype::MakeDefaultInto(MaterialArchetype* mat)
{
	mat->gbufferFragBinary = ShaderCompiler::Compile("engine-data/spv/shader-defaults/gbuffer-default.frag");
	mat->depthBinary = ShaderCompiler::Compile("engine-data/spv/shader-defaults/depthmap-default.frag");
	CLOG_ABORT(mat->gbufferFragBinary.size() == 0 || mat->depthBinary.size() == 0,
		"Failed to compile defualt shader code. (engine-data/spv/shader-defaults/)");
}

PodHandle<MaterialArchetype> MaterialArchetype::GetGltfArchetype()
{
	return { GetDefaultGtlfArchetypeUid() };
}

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

	std::string fragCode = shd::GenerateGbufferFrag(descSetCode, sharedFunctions, gbufferFragMain);

	if (outputToConsole) {
		LOG_REPORT("gbuffer FRAG SHADER: === \n{}", fragCode);
	}


	auto fragErrors = &outErrors.editorErrors.insert({ "Fragment", {} }).first->second;
	auto fragBin = ShaderCompiler::Compile(fragCode, ShaderStageType::Fragment, fragErrors);

	if (!fragBin.size()) {
		RerouteShaderErrors(outErrors);
		return false;
	}


	std::string vertCode = shd::GenerateGbufferVert(descSetCode, sharedFunctions, gbufferVertMain);

	if (outputToConsole) {
		LOG_REPORT("gbuffer VERT SHADER: === \n{}", fragCode);
	}


	auto vertErrors = &outErrors.editorErrors.insert({ "Vertex", {} }).first->second;
	auto vertBin = ShaderCompiler::Compile(vertCode, ShaderStageType::Vertex, vertErrors);

	// All stages compiled, swap layout and store the new binaries
	RerouteShaderErrors(outErrors);


	ChangeLayout(std::move(newLayout));

	gbufferFragBinary.swap(fragBin);
	depthBinary.swap(depthBin);
	gbufferVertBinary.swap(vertBin);


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

std::stringstream DynamicDescriptorSetLayout::GetUniformText() const
{
	std::stringstream uboText;

	for (auto& prop : uboClass.GetProperties()) {
		if (prop.IsA<glm::vec4>()) {
			if (prop.HasFlags(PropertyFlags::Color)) {
				uboText << "col4 ";
			}
			else {
				uboText << "vec4 ";
			}
		}
		else {
			uboText << prop.GetType().name() << " ";
		}
		uboText << prop.GetName() << ";\n";
	}
	uboText << "\n";

	uboText << "ubo " << uboName << ";\n\n";

	for (auto& sampler : samplers2d) {
		uboText << "sampler2d " << sampler << ";\n";
	}
	return uboText;
}
