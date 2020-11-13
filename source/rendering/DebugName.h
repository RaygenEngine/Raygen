#pragma once

#define DEBUG_NAME(handle, name) detail::RegisterDebugName(handle, name);
#define DEBUG_NAME_AUTO(handle)  DEBUG_NAME(handle, #handle)

void SetDebugUtilsObjectName(vk::DebugUtilsObjectNameInfoEXT&&);

namespace detail {
template<typename T, typename = void>
struct HasCType : std::false_type {
};

template<typename T>
struct HasCType<T, std::void_t<typename T::CType>> : std::true_type {
};

template<typename T>
void RegisterDebugName(T handle, const std::string& name)
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
	SetDebugUtilsObjectName(std::move(debugNameInfo));
}


} // namespace detail
