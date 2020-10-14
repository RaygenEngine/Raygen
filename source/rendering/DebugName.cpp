#include "rendering/Device.h"

void SetDebugUtilsObjectName(vk::DebugUtilsObjectNameInfoEXT&& info)
{
	vl::Device->setDebugUtilsObjectNameEXT(info);
}
