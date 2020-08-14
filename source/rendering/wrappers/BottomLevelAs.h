#pragma once

namespace vl {
struct GpuGeometryGroup;

struct BottomLevelAs {

	BottomLevelAs() = default;
	BottomLevelAs(size_t vertexStride, const std::vector<GpuGeometryGroup>& gggs,
		vk::BuildAccelerationStructureFlagsKHR buildFlags = {});

	[[nodiscard]] vk::DeviceAddress GetAddress() const noexcept;
	vk::UniqueAccelerationStructureKHR handle;

private:
	vk::UniqueDeviceMemory memory;
};
} // namespace vl
