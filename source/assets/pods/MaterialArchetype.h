#pragma once
#include "assets/PodHandle.h"
#include "assets/util/DynamicDescriptorSet.h"
#include "assets/util/SpirvCompiler.h"
#include "reflection/GenMacros.h"


namespace shd {
struct GeneratedShaderErrors {
	std::unordered_map<std::string, TextCompilerErrors> editorErrors;
};
} // namespace shd

// Generates a dynamic archetype for a material from a shader
struct MaterialArchetype : AssetPod {
	REFLECTED_POD(MaterialArchetype)
	{
		REFLECT_ICON(FA_ALIGN_CENTER);
		REFLECT_VAR(instances, PropertyFlags::NoEdit, PropertyFlags::NoCopy);
		REFLECT_VAR(sharedFunctions, PropertyFlags::NoEdit, PropertyFlags::Multiline);
		REFLECT_VAR(gbufferFragMain, PropertyFlags::NoEdit, PropertyFlags::Multiline);
		REFLECT_VAR(depthShader, PropertyFlags::NoEdit, PropertyFlags::Multiline);

		REFLECT_VAR(gbufferVertMain, PropertyFlags::NoEdit, PropertyFlags::Multiline);
	}


	std::string sharedFunctions{};
	std::string gbufferFragMain{};
	std::string gbufferVertMain{};
	std::string depthShader{};


	std::vector<uint32> gbufferFragBinary;
	std::vector<uint32> gbufferVertBinary;
	std::vector<uint32> depthFragBinary;
	std::vector<uint32> depthVertBinary;


	std::vector<PodHandle<MaterialInstance>> instances;

	// Active is the one used in gpu assets
	DynamicDescriptorSetLayout descriptorSetLayout;

	static void MakeGltfArchetypeInto(MaterialArchetype* mat);
	static void MakeDefaultInto(MaterialArchetype* mat);


	static PodHandle<MaterialArchetype> GetGltfArchetype();

protected:
	// Propagates the editable Descriptor Set Layout to active Layout
	void ChangeLayout(DynamicDescriptorSetLayout&& newLayout);

public:
	bool CompileAll(
		DynamicDescriptorSetLayout&& newLayout, shd::GeneratedShaderErrors& outErrors, bool outputToConsole = false);
};
