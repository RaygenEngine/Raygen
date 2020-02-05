#pragma once

#include "renderer/renderers/vulkan/Device.h"
#include "asset/pods/ModelPod.h"

#include "vulkan/vulkan.hpp"

namespace vlkn {

class Image {


public:
	Model(Device* device, PodHandle<ModelPod> handle);

	const std::vector<GeometryGroup>& GetGeometryGroups() const { return m_geometryGroups; }
};
} // namespace vlkn
