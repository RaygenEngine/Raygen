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
					LOG_ERROR("InOut Shader compilation Error: {}", error.second);
					break;
				case 2:
					descSetErrors.errors.insert({ mod, error.second });
					LOG_ERROR("Descriptor Set Shader compilation Error: {}", error.second);
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
	mat->gbufferFragBinary = ShaderCompiler::Compile("engine-data/spv/geometry/gbuffer-gltf.frag");
	mat->depthFragBinary = ShaderCompiler::Compile("engine-data/spv/geometry/depthmap-gltf.frag");

	CLOG_ABORT(mat->gbufferFragBinary.size() == 0 || mat->depthFragBinary.size() == 0,
		"Failed to compile gltf archetype shader code.");

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

	cl.AddProperty<int>("baseColorSampler", PropertyFlags::Transient);
	cl.AddProperty<int>("metallicRoughnessSampler", PropertyFlags::Transient);
	cl.AddProperty<int>("occlusionSampler", PropertyFlags::Transient);
	cl.AddProperty<int>("normalSampler", PropertyFlags::Transient);
	cl.AddProperty<int>("emissiveSampler", PropertyFlags::Transient);
}

void MaterialArchetype::MakeDefaultInto(MaterialArchetype* mat)
{
	mat->gbufferFragBinary = ShaderCompiler::Compile("engine-data/spv/geometry/gbuffer-default.frag");
	mat->depthFragBinary = ShaderCompiler::Compile("engine-data/spv/geometry/depthmap-default.frag");
	CLOG_ABORT(mat->gbufferFragBinary.size() == 0 || mat->depthFragBinary.size() == 0,
		"Failed to compile defualt shader code. (engine-data/spv/geometry/)");
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
	outErrors.editorErrors.clear();


	auto generateShader
		= [&](const std::string& mainCode, decltype(&shd::GenerateDepthFrag) generatorFunction,
			  const std::string& editorName, ShaderStageType stage, std::string* descSetCodeOpt = nullptr) {
			  std::string code;
			  if (descSetCodeOpt) {
				  code = (*generatorFunction)(*descSetCodeOpt, sharedFunctions, mainCode);
			  }
			  else {
				  code = (*generatorFunction)(descSetCode, sharedFunctions, mainCode);
			  }


			  if (outputToConsole) {
				  LOG_REPORT("{} SHADER: === \n{}", editorName, code);
			  }

			  auto errors = &outErrors.editorErrors.insert({ editorName, {} }).first->second;
			  auto shaderBinary = ShaderCompiler::Compile(code, stage, errors);

			  if (!shaderBinary.size()) {
				  RerouteShaderErrors(outErrors);
				  return shaderBinary;
			  }
			  return shaderBinary;
		  };


	auto depthFragBin = generateShader(depthShader, &shd::GenerateDepthFrag, "Depth", ShaderStageType::Fragment);
	auto gbufferFragBin
		= generateShader(gbufferFragMain, &shd::GenerateGbufferFrag, "Fragment", ShaderStageType::Fragment);

	if (depthFragBin.size() == 0 || gbufferFragBin.size() == 0) {
		return false;
	}


	auto gbufferVertBin = generateShader(gbufferVertMain, &shd::GenerateGbufferVert, "Vertex", ShaderStageType::Vertex);
	auto depthVertBin = generateShader(gbufferVertMain, &shd::GenerateDepthVert, "Vertex", ShaderStageType::Vertex);

	if (unlitFragMain.size() > 2) {
		auto unlitFragBin = generateShader(unlitFragMain, &shd::GenerateUnlitFrag, "Unlit", ShaderStageType::Fragment);
		unlitFragBinary.swap(unlitFragBin);
	}


	// WIP:
	if (raytracingMain.size() > 2) {
		std::string dscSet = shd::GenerateRtStructCode(newLayout);
		auto rtBin
			= generateShader(raytracingMain, &shd::GenerateRtCallable, "Raytrace", ShaderStageType::Callable, &dscSet);
		if (!rtBin.empty()) {
			raytracingBinary.swap(rtBin);
		}
	}
	else {
		raytracingBinary.clear();
	}


	RerouteShaderErrors(outErrors);

	ChangeLayout(std::move(newLayout));

	gbufferFragBinary.swap(gbufferFragBin);
	depthFragBinary.swap(depthFragBin);

	gbufferVertBinary.swap(gbufferVertBin);
	depthVertBinary.swap(depthVertBin);


	return true;
}

PodEntry* MaterialArchetype::MakeInstancePod(PodHandle<MaterialArchetype> archetype, const uri::Uri& path)
{
	auto actualPath = path.empty() ? AssetRegistry::GetPodUri(archetype) + " Inst" : path;

	auto [entry, pod] = AssetRegistry::CreateEntry<MaterialInstance>(actualPath);
	MaterialInstance::SetArchetype(entry->GetHandleAs<MaterialInstance>(), archetype);
	return entry;
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
		if (prop.HasFlags(PropertyFlags::Transient)) {
			continue;
		}
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
