#pragma once
#include "assets/util/DynamicDescriptorSet.h"

struct TextCompilerErrors;

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
		REFLECT_VAR(unlitFragMain, PropertyFlags::NoEdit, PropertyFlags::Multiline);

		REFLECT_VAR(passType, PropertyFlags::Hidden);
	}


	ArchetypeType passType{};

	std::string sharedFunctions{};
	std::string gbufferFragMain{};
	std::string gbufferVertMain{};
	std::string depthShader{};
	std::string unlitFragMain{};

	std::vector<uint32> gbufferFragBinary;
	std::vector<uint32> gbufferVertBinary;
	std::vector<uint32> depthFragBinary;
	std::vector<uint32> depthVertBinary;
	std::vector<uint32> unlitFragBinary;


	std::vector<PodHandle<MaterialInstance>> instances;

	// Active is the one used in gpu assets
	DynamicDescriptorSetLayout descriptorSetLayout;

	static void MakeGltfArchetypeInto(MaterialArchetype* mat);
	static void MakeDefaultInto(MaterialArchetype* mat);

protected:
	// Propagates the editable Descriptor Set Layout to active Layout
	void ChangeLayout(DynamicDescriptorSetLayout&& newLayout);

public:
	bool CompileAll(
		DynamicDescriptorSetLayout&& newLayout, shd::GeneratedShaderErrors& outErrors, bool outputToConsole = false);


	static PodEntry* MakeInstancePod(PodHandle<MaterialArchetype> archetype, const uri::Uri& path = "");
};
