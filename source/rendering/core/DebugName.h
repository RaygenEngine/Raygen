#pragma once

#include <source_location>

#include <vulkan/vulkan.hpp>


#define COMMAND_SCOPE(cmdBuffer, name) // const rvk::CmdScopedLabel MACRO_PASTE(z_scope_t, __LINE__){ cmdBuffer, name };
#define COMMAND_SCOPE_AUTO(cmdBuffer)                                                                                  \
	//	const rvk::CmdScopedLabel MACRO_PASTE(z_scope_t, __LINE__){ cmdBuffer,                                             \
//		std::string(typeid(*this).name()) + "::" + std::source_location::current().function_name() };


#define DEBUG_NAME(handle, name) rvk::registerDebugName(handle, name)
#define DEBUG_NAME_AUTO(handle)  DEBUG_NAME(handle, #handle)

namespace vl {
struct RBuffer;
}

namespace rvk {

struct CmdScopedLabel {
	vk::CommandBuffer cmdBuffer;

	CmdScopedLabel(vk::CommandBuffer cmdBuffer, const std::string& name)
		: cmdBuffer(cmdBuffer)
	{
		cmdBuffer.beginDebugUtilsLabelEXT({ name.c_str(), { 1.f, 1.f, 1.f, 1.f } });
	}

	~CmdScopedLabel() { cmdBuffer.endDebugUtilsLabelEXT(); }
};

void setDebugUtilsObjectName(vk::DebugUtilsObjectNameInfoEXT&&);

template<typename T, typename = void>
struct HasCType : std::false_type {
};

template<typename T>
struct HasCType<T, std::void_t<typename T::CType>> : std::true_type {
};

template<typename T>
void registerDebugName(const T& handle, const std::string& name)
{
	vk::DebugUtilsObjectNameInfoEXT debugNameInfo{};

	if constexpr (HasCType<T>::value) {
		debugNameInfo
			.setObjectType(handle.objectType) //
			.setObjectHandle(reinterpret_cast<uint64>(T::CType(handle)));
	}
	else {
		debugNameInfo
			.setObjectType(handle->objectType) //
			.setObjectHandle(reinterpret_cast<uint64>(T::element_type::CType(handle.get())));
	}

	debugNameInfo.setPObjectName(name.c_str());
	setDebugUtilsObjectName(std::move(debugNameInfo));
}

template<>
void registerDebugName<vl::RBuffer>(const vl::RBuffer& buffer, const std::string& name);

} // namespace rvk
