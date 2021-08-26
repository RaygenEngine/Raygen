#include "DebugName.h"

#include "rendering/Device.h"
#include "rendering/wrappers/Buffer.h"

namespace rvk {
void setDebugUtilsObjectName(vk::DebugUtilsObjectNameInfoEXT&& info)
{
	vl::Device->setDebugUtilsObjectNameEXT(info);
}

template<>
void registerDebugName<vl::RBuffer>(const vl::RBuffer& buffer, const std::string& name)
{
	vk::DebugUtilsObjectNameInfoEXT debugNameInfo{};

	rvk::registerDebugName(buffer.handle(), name + " [buffer]");
	rvk::registerDebugName(buffer.memory(), name + " [memory]");
}

} // namespace rvk
