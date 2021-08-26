#pragma once

#include <vulkan/vulkan.hpp>

#define CMDSCOPE_BEGIN(cmdBuffer, name) cmdBuffer.beginDebugUtilsLabelEXT({ name, { 1.f, 1.f, 1.f, 1.f } })
#define CMDSCOPE_END(cmdBuffer)         cmdBuffer.endDebugUtilsLabelEXT()

#define DEBUG_NAME(handle, name) rvk::registerDebugName(handle, name)
#define DEBUG_NAME_AUTO(handle)  DEBUG_NAME(handle, #handle)

namespace vl {
struct RBuffer;
}

namespace rvk {
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
