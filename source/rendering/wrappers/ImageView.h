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
		vk::MemoryPropertyFlags properties, vk::ImageViewType viewType, const std::string& name);

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

	vk::UniqueImageView GenerateNewView(vk::ImageUsageFlags usage, vk::ImageViewType viewType) const;

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
	RImage2D(uint32 width, uint32 height, uint32 mipLevels, vk::Format format, vk::ImageTiling tiling,
		vk::ImageLayout initalLayout, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties,
		const std::string& name)
		: RImage(vk::ImageType::e2D, { width, height, 1u }, mipLevels, 1u, format, tiling, initalLayout, usage,
			vk::SampleCountFlagBits::e1, vk::SharingMode::eExclusive, {}, properties, vk::ImageViewType::e2D, name){};

	// TODO: Refactor to constructor and cleanup old constructors
	// This will transition the image to final layout unless final layout is eUndefined
	static RImage2D Create(const std::string& name, vk::Extent2D extent, vk::Format format,
		vk::ImageLayout finalLayout = vk::ImageLayout::eUndefined,
		vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage,
		uint32 mipLevels = 1, vk::MemoryPropertyFlags memoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::ImageTiling tiling = vk::ImageTiling::eOptimal);
};

struct RCubemap : RImage {
	RCubemap() = default;
	RCubemap(uint32 dims, uint32 mipCount, vk::Format format, vk::ImageTiling tiling, vk::ImageLayout initalLayout,
		vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, const std::string& name)
		: RImage(vk::ImageType::e2D, { dims, dims, 1u }, mipCount, 6u, format, tiling, initalLayout, usage,
			vk::SampleCountFlagBits::e1, vk::SharingMode::eExclusive, vk::ImageCreateFlagBits::eCubeCompatible,
			properties, vk::ImageViewType::eCube, name){};

	void RCubemap::CopyBuffer(const RBuffer& buffers, size_t pixelSize, uint32 mipCount);

	std::vector<vk::UniqueImageView> GetFaceViews(uint32 atMip = 0u) const;
	std::vector<vk::UniqueImageView> GetMipViews() const;
	vk::UniqueImageView GetMipView(uint32 atMip = 0u) const;
	vk::UniqueImageView GetFaceArrayView(uint32 atMip = 0u) const;
};

struct RCubemapArray : RImage {
	RCubemapArray() = default;
	RCubemapArray(uint32 dims, uint32 mipCount, uint32 arraySize, vk::Format format, vk::ImageTiling tiling,
		vk::ImageLayout initalLayout, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties,
		const std::string& name)
		: RImage(vk::ImageType::e2D, { dims, dims, 1u }, mipCount, arraySize * 6u, format, tiling, initalLayout, usage,
			vk::SampleCountFlagBits::e1, vk::SharingMode::eExclusive, vk::ImageCreateFlagBits::eCubeCompatible,
			properties, vk::ImageViewType::eCubeArray, name){};

	std::vector<vk::UniqueImageView> GetFaceViews(uint32 atArrayIndex, uint32 atMip = 0u) const;
	vk::UniqueImageView GetCubemapView(uint32 atArrayIndex, uint32 atMip = 0u) const;
};

struct RImageAttachment : RImage {
	RImageAttachment() = default;
	RImageAttachment(uint32 width, uint32 height, vk::Format format, vk::ImageTiling tiling,
		vk::ImageLayout initalLayout, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties,
		const std::string& name)
		: RImage(vk::ImageType::e2D, { width, height, 1u }, 1u, 1u, format, tiling, initalLayout, usage,
			vk::SampleCountFlagBits::e1, vk::SharingMode::eExclusive, {}, properties, vk::ImageViewType::e2D, name){};

	RImageAttachment(RImageAttachment const&) = delete;
	RImageAttachment(RImageAttachment&&) = default;
	RImageAttachment& operator=(RImageAttachment const&) = delete;
	RImageAttachment& operator=(RImageAttachment&&) = default;
};

} // namespace vl
