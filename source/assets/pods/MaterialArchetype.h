#pragma once
#include "assets/pods/Sampler.h"
#include "reflection/GenMacros.h"
#include "assets/util/SpirvCompiler.h"
#include "assets/PodHandle.h"


namespace shd {
struct GeneratedShaderErrors {
	std::unordered_map<std::string, TextCompilerErrors> editorErrors;
};
} // namespace shd


// Holds the representantion for a runtime generated descriptor set.
// This is ONLY the representation (layout) (ie the variables/samplers info) and not an actual descriptor set.
// Redesigned to be usable everywhere.
// Currently supports samplers2D and a single UBO struct with vec4, float, ints
// The struct includes a RuntimeClass object for the ubo  which uses the engine's underlying reflection system. This
// allows us to use refltools and other utilities that act on "ReflClass" on the ubo data.
struct DynamicDescriptorSetLayout {
	std::vector<std::string> samplers2d; // names of the samplers from binding = 1 to N
	RuntimeClass uboClass;
	std::string uboName{ "ubo" };

	[[nodiscard]] size_t SizeOfUbo() const // TODO: Test padding
	{
		return uboClass.GetSize();
	}

	template<typename Archive>
	void serialize(Archive& ar)
	{
		ar(samplers2d, uboName, uboClass);
	}

	std::stringstream GetUniformText() const;
};

struct DynamicDescriptorSet {
	std::vector<PodHandle<Image>> samplers2d;
	std::vector<byte> uboData;

	// Attempts to "preserve" as much data as possible
	void SwapLayout(const DynamicDescriptorSetLayout& oldLayout, const DynamicDescriptorSetLayout& newLayout);

	template<typename Archive>
	void serialize(Archive& ar)
	{
		ar(samplers2d, uboData);
	}
};


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
