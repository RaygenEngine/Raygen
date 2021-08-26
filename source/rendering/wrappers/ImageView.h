#pragma once

#include "rendering/VkCoreIncludes.h"

#include <optional>

// NOTE: this is an ImageViewer wrapper that needs lots of work
namespace vl {
struct RBuffer;

struct RImage {

	vk::Format format{};
	vk::Extent3D extent{};

	vk::ImageAspectFlags aspectMask{};
	vk::SampleCountFlags samples{};
	vk::ImageCreateFlags flags{};

	uint32 arrayLayers{ 0u };
	uint32 mipLevels{ 0u };

	bool isDepth{ false };

	std::string name{};

	RImage() = default;

	// clang-format off
	RImage(vk::ImageType imageType, const std::string& name, vk::Extent3D extent, vk::Format format, uint32 mipLevels, uint32 arrayLayers, 
		vk::ImageLayout finalLayout = vk::ImageLayout::eUndefined,
		vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage,
		vk::ImageCreateFlags flags = {},
		vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined,
		vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1,
		vk::SharingMode sharingMode = vk::SharingMode::eExclusive,
		vk::ImageTiling tiling = vk::ImageTiling::eOptimal);
	// clang-format on

	RImage(RImage const&) = delete;
	RImage(RImage&&) = default;
	RImage& operator=(RImage const&) = delete;
	RImage& operator=(RImage&&) = default;
	virtual ~RImage() = default;

	void CopyBufferToImage(const RBuffer& buffers);
	void CopyImageToBuffer(const RBuffer& buffers);

	// Blocking transition to layout
	void BlockingTransitionToLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
	// Pipeline stages are explicit
	void BlockingTransitionToLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
		vk::PipelineStageFlags sourceStage, vk::PipelineStageFlags destStage);

	// Pipeline stages are chosen based on image layouts
	void TransitionToLayout(vk::CommandBuffer cmdBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const;
	// Pipeline stages are explicit
	void TransitionToLayout(vk::CommandBuffer cmdBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
		vk::PipelineStageFlags sourceStage, vk::PipelineStageFlags destStage) const;

	void GenerateMipmapsAndTransitionEach(vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

	[[nodiscard]] vk::Image handle() const { return uHandle.get(); }
	[[nodiscard]] vk::ImageView view() const { return uView.get(); }
	[[nodiscard]] vk::DeviceMemory memory() const { return uMemory.get(); }

	[[nodiscard]] vk::DescriptorSet GetDebugDescriptor();

protected:
	vk::UniqueImage uHandle;
	vk::UniqueImageView uView;
	vk::UniqueDeviceMemory uMemory;

	std::optional<vk::DescriptorSet> debugDescriptorSet;

	[[nodiscard]] vk::ImageMemoryBarrier CreateTransitionBarrier(vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
		uint32 baseMipLevel = 0u, uint32 baseArrayLevel = 0u) const;
};

struct RImage2D : RImage {
	RImage2D() = default;
	RImage2D(const std::string& name, vk::Extent2D extent, vk::Format format,
		vk::ImageLayout finalLayout = vk::ImageLayout::eUndefined, uint32 mipLevels = 1,
		vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage,
		vk::MemoryPropertyFlags memoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal)
		: RImage(vk::ImageType::e2D, name, vk::Extent3D{ extent, 1u }, format, mipLevels, 1u, finalLayout, memoryFlags,
			usageFlags){};
};

struct RCubemap : RImage {
	RCubemap() = default;
	RCubemap(const std::string& name, uint32 res, vk::Format format,
		vk::ImageLayout finalLayout = vk::ImageLayout::eUndefined, uint32 mipLevels = 1u,
		vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage,
		vk::MemoryPropertyFlags memoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal)
		: RImage(vk::ImageType::e2D, name, vk::Extent3D{ res, res, 1u }, format, mipLevels, 6u, finalLayout,
			memoryFlags, usageFlags, vk::ImageCreateFlagBits::eCubeCompatible){};

	RCubemap(const std::string& name, uint32 res, vk::Format format, uint32 mipLevels,
		vk::ImageLayout finalLayout = vk::ImageLayout::eUndefined,
		vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage,
		vk::MemoryPropertyFlags memoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal)
		: RImage(vk::ImageType::e2D, name, vk::Extent3D{ res, res, 1u }, format, mipLevels, 6u, finalLayout,
			memoryFlags, usageFlags, vk::ImageCreateFlagBits::eCubeCompatible){};

	void CopyBuffer(const RBuffer& buffers, size_t pixelSize, uint32 mipCount);

	std::vector<vk::UniqueImageView> GetFaceViews(uint32 atMip = 0u) const;
	std::vector<vk::UniqueImageView> GetMipViews() const;
	vk::UniqueImageView GetMipView(uint32 atMip = 0u) const;
	vk::UniqueImageView GetFaceArrayView(uint32 atMip = 0u) const;
};

struct RCubemapArray : RImage {
	RCubemapArray() = default;
	RCubemapArray(const std::string& name, uint32 res, vk::Format format, uint32 arrayCount,
		vk::ImageLayout finalLayout = vk::ImageLayout::eUndefined, uint32 mipLevels = 1u,
		vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage,
		vk::MemoryPropertyFlags memoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal)
		: RImage(vk::ImageType::e2D, name, vk::Extent3D{ res, res, 1u }, format, mipLevels, arrayCount * 6u,
			finalLayout, memoryFlags, usageFlags, vk::ImageCreateFlagBits::eCubeCompatible){};

	std::vector<vk::UniqueImageView> GetFaceViews(uint32 atArrayIndex, uint32 atMip = 0u) const;
	vk::UniqueImageView GetCubemapView(uint32 atArrayIndex, uint32 atMip = 0u) const;
};

} // namespace vl
