#pragma once
#include "reflection/ReflClass.h"

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

	[[nodiscard]] size_t SizeOfUbo() const // TODO: Fix custom ubo padding
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
	void load(Archive& ar)
	{
		ar(samplers2d, uboData);

		// NEXT: Extremelly bad way of deserializing back into image index
		using HandleType = int32;
		byte* revCurs = &uboData.back() + 1;
		constexpr int32 samplerHandleStride = sizeof(HandleType);

		revCurs -= samplerHandleStride;

		for (int32 i = static_cast<int32>(samplers2d.size()) - 1; i >= 0; --i) {
			*reinterpret_cast<HandleType*>(revCurs) = static_cast<HandleType>(samplers2d[i].uid);
			revCurs -= samplerHandleStride;
		}
	}

	template<typename Archive>
	void save(Archive& ar) const
	{
		ar(samplers2d, uboData);
	}
};

enum class ArchetypeType
{
	Lit,
	Unlit,
};
