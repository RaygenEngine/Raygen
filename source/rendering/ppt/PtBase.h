#pragma once

#include <vulkan/vulkan.hpp>

#include "reflection/GenMacros.h"

namespace vl {
class PtBase {
public:
	// Initialize the pipline layout on constructor
	PtBase() = default;

	// Make the pipeline here, will be called on shader recompiles on child classes (WIP)
	virtual void MakePipeline(){};

	virtual void Draw(vk::CommandBuffer cmdBuffer, uint32 frameIndex) = 0;


	PtBase(const PtBase&) = delete;
	PtBase(PtBase&&) = default;
	PtBase& operator=(const PtBase&) = delete;
	PtBase& operator=(PtBase&&) = default;

	virtual ~PtBase() = default;

	// TODO: Reflected settings struct
};


// TODO: Proper concept
template<typename T>
concept CPostTech = requires
{
	std::is_base_of_v<PtBase, T>;
};
} // namespace vl
