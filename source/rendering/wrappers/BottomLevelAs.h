#pragma once

namespace vl {
struct GpuGeometryGroup;

struct BottomLevelAs {

	BottomLevelAs() = default;
	BottomLevelAs(size_t vertexStride, const std::vector<GpuGeometryGroup>& gggs,
		vk::BuildAccelerationStructureFlagsKHR buildFlags = {});

	[[nodiscard]] vk::DeviceAddress GetAddress() const noexcept;

private:
	vk::UniqueAccelerationStructureKHR handle;
	vk::UniqueDeviceMemory memory;
};
} // namespace vl
