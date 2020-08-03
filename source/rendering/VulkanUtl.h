#pragma once
#include "assets/shared/ImageShared.h"
#include "assets/shared/TextureShared.h"

namespace vl {

inline vk::Filter GetTextureFilter(TextureFiltering f)
{
	switch (f) {
		case TextureFiltering::Nearest: return vk::Filter::eNearest;
		case TextureFiltering::Linear: return vk::Filter::eLinear;
		default: LOG_ABORT("Unsupported");
	}
}

inline vk::SamplerAddressMode GetWrapping(TextureWrapping w)
{
	switch (w) {
		case TextureWrapping::ClampToEdge: return vk::SamplerAddressMode::eClampToEdge;
		case TextureWrapping::MirroredRepeat: return vk::SamplerAddressMode::eMirroredRepeat;
		case TextureWrapping::Repeat: return vk::SamplerAddressMode::eRepeat;
		default: LOG_ABORT("Unsupported");
	}
}

inline vk::SamplerMipmapMode GetMipmapFilter(MipmapFiltering f)
{
	switch (f) {
		case MipmapFiltering::Nearest: return vk::SamplerMipmapMode::eNearest;
		case MipmapFiltering::Linear: return vk::SamplerMipmapMode::eLinear;
		case MipmapFiltering::NoMipmap: LOG_ABORT("Programmer error");
		default: LOG_ABORT("Unsupported");
	}
}


inline vk::Format GetFormat(ImageFormat format)
{
	switch (format) {
	case ImageFormat::Hdr: return vk::Format::eR32G32B32A32Sfloat; break;
	case ImageFormat::Srgb: return vk::Format::eR8G8B8A8Srgb; break;
	case ImageFormat::Unorm: return vk::Format::eR8G8B8A8Unorm; break;
	default: LOG_ABORT("Unsupported");
	}
}

inline bool IsDepthFormat(vk::Format format)
{

	switch (format) {
	case vk::Format::eD32Sfloat:
	case vk::Format::eD32SfloatS8Uint:
	case vk::Format::eD24UnormS8Uint: return true;
	default: return false;
	}
}

// TODO: this should be explicit to the code
inline vk::AccessFlags GetAccessMask(vk::ImageLayout imL)
{
	switch (imL) {
		case vk::ImageLayout::eUndefined: return vk::AccessFlags{ 0u };
		case vk::ImageLayout::eColorAttachmentOptimal: return vk::AccessFlagBits::eColorAttachmentWrite;
		case vk::ImageLayout::eShaderReadOnlyOptimal: return vk::AccessFlagBits::eShaderRead;
		case vk::ImageLayout::eTransferDstOptimal: return vk::AccessFlagBits::eTransferWrite;
		case vk::ImageLayout::eTransferSrcOptimal: return vk::AccessFlagBits::eTransferRead;
		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			return vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		default: LOG_ABORT("Unsupported");
	}
}

// TODO: this should be explicit to the code
inline vk::PipelineStageFlags GetPipelineStage(vk::ImageLayout imL)
{
	switch (imL) {
		case vk::ImageLayout::eUndefined: return vk::PipelineStageFlagBits::eTopOfPipe;
		case vk::ImageLayout::eColorAttachmentOptimal: return vk::PipelineStageFlagBits::eColorAttachmentOutput;
		case vk::ImageLayout::eShaderReadOnlyOptimal: return vk::PipelineStageFlagBits::eFragmentShader;
		case vk::ImageLayout::eTransferSrcOptimal:
		case vk::ImageLayout::eTransferDstOptimal: return vk::PipelineStageFlagBits::eTransfer;
		case vk::ImageLayout::eDepthStencilAttachmentOptimal: return vk::PipelineStageFlagBits::eEarlyFragmentTests;
		default: LOG_ABORT("Unsupported");
	}
}

// TODO: this should be explicit to the code
inline vk::ImageAspectFlags GetAspectMask(vk::ImageUsageFlags usage, vk::Format format)
{
	auto aspectMask = vk::ImageAspectFlagBits::eColor;

	if (usage & vk::ImageUsageFlagBits::eDepthStencilAttachment) {
		aspectMask = vk::ImageAspectFlagBits::eDepth;

		// if has stencil component
		if (format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint) {
			return aspectMask | vk::ImageAspectFlagBits::eStencil;
		}
	}

	return aspectMask;
}

// TODO: this should be explicit to the code
inline vk::ImageAspectFlags GetAspectMask(const vk::ImageCreateInfo& ici)
{
	return GetAspectMask(ici.usage, ici.format);
}

} // namespace vl
