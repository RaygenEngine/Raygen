#pragma once
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
	RImage(vk::ImageType imageType, vk::Extent3D extent, uint32 mipLevels, uint32 arrayLayers, vk::Format format,
		vk::ImageTiling tiling, vk::ImageLayout initialLayout, vk::ImageUsageFlags usage,
		vk::SampleCountFlagBits samples, vk::SharingMode sharingMode, vk::ImageCreateFlags flags,
		vk::MemoryPropertyFlags properties, vk::ImageViewType viewType, const std::string& name = "unnamed");

	RImage(RImage const&) = delete;
	RImage(RImage&&) = default;
	RImage& operator=(RImage const&) = delete;
	RImage& operator=(RImage&&) = default;

	void CopyBufferToImage(const RBuffer& buffer);
	void CopyImageToBuffer(const RBuffer& buffer);

	// Affects all mips and array elements
	vk::ImageMemoryBarrier CreateTransitionBarrier(
		vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32 baseMipLevel = 0u, uint32 baseArrayLevel = 0u);

	// Blocking transition to layout
	void BlockingTransitionToLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
	void TransitionToLayout(vk::CommandBuffer* cmdBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
	// Current layout must be read optimal
	void TransitionForWrite(vk::CommandBuffer* cmdBuffer);
	// Current layout must be write optimal
	void TransitionForRead(vk::CommandBuffer* cmdBuffer);

	void GenerateMipmapsAndTransitionEach(vk::ImageLayout oldLayout, vk::ImageLayout finalLayout);

	[[nodiscard]] operator vk::Image() const noexcept { return image.get(); }
	[[nodiscard]] operator vk::ImageView() const noexcept { return view.get(); }

	virtual ~RImage() = default;

	[[nodiscard]] vk::DescriptorSet GetDebugDescriptor();

protected:
	vk::UniqueImage image;
	vk::UniqueDeviceMemory memory;

	vk::UniqueImageView view;

	std::optional<vk::DescriptorSet> m_debugDescriptorSet;
};

struct RImage2D : RImage {
	RImage2D() = default;
	RImage2D(uint32 width, uint32 height, uint32 mipLevels, vk::Format format, vk::ImageTiling tiling,
		vk::ImageLayout initalLayout, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties,
		const std::string& name = "unnamed_image2d")
		: RImage(vk::ImageType::e2D, { width, height, 1u }, mipLevels, 1u, format, tiling, initalLayout, usage,
			vk::SampleCountFlagBits::e1, vk::SharingMode::eExclusive, {}, properties, vk::ImageViewType::e2D, name){};
};

struct RCubemap : RImage {
	RCubemap() = default;
	RCubemap(uint32 dims, uint32 mipCount, vk::Format format, vk::ImageTiling tiling, vk::ImageLayout initalLayout,
		vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, const std::string& name = "unnamed_cubemap")
		: RImage(vk::ImageType::e2D, { dims, dims, 1u }, mipCount, 6u, format, tiling, initalLayout, usage,
			vk::SampleCountFlagBits::e1, vk::SharingMode::eExclusive, vk::ImageCreateFlagBits::eCubeCompatible,
			properties, vk::ImageViewType::eCube, name){};

	void RCubemap::CopyBuffer(const RBuffer& buffer, size_t pixelSize, uint32 mipCount);
};

struct RImageAttachment : RImage {
	RImageAttachment() = default;
	RImageAttachment(uint32 width, uint32 height, vk::Format format, vk::ImageTiling tiling,
		vk::ImageLayout initalLayout, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties,
		const std::string& name = "unnamed_attachment")
		: RImage(vk::ImageType::e2D, { width, height, 1u }, 1u, 1u, format, tiling, initalLayout, usage,
			vk::SampleCountFlagBits::e1, vk::SharingMode::eExclusive, {}, properties, vk::ImageViewType::e2D, name){};
};

} // namespace vl
