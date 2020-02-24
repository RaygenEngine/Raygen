#pragma once

#include <vulkan/vulkan.hpp>


template<typename T>
class VkObjectWrapper {

protected:
	T m_vkHandle;

public:
	T* operator->() { return &m_vkHandle; }
	T get() const { return m_vkHandle; }
};

template<typename T>
class VkUniqueObjectWrapper {

protected:
	T m_vkHandle;

public:
	VkUniqueObjectWrapper() = default;
	VkUniqueObjectWrapper(const VkUniqueObjectWrapper&) = delete;
	VkUniqueObjectWrapper& operator=(const VkUniqueObjectWrapper&) = delete;
	VkUniqueObjectWrapper(VkUniqueObjectWrapper&&) = delete;
	VkUniqueObjectWrapper& operator=(VkUniqueObjectWrapper&&) = delete;

	typename T::element_type* operator->() { return &m_vkHandle.get(); }
	typename T::element_type get() const { return m_vkHandle.get(); }
};
