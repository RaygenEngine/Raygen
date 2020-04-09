#include "pch.h"
#include "GpuResources.h"

namespace vl {
GpuResources_::GpuResources_()
{
	imageDebugDescLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	imageDebugDescLayout.Generate();
}
} // namespace vl
