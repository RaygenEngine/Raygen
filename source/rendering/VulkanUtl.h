#pragma once
#include "assets/pods/Sampler.h"

#include <vulkan/vulkan.hpp>

namespace vl {
inline vk::Filter GetTextureFilter(TextureFiltering f)
{
	switch (f) {
		case TextureFiltering::Nearest: return vk::Filter::eNearest;
		case TextureFiltering::Linear: return vk::Filter::eLinear;
		default: return vk::Filter::eLinear;
	}
}

inline vk::SamplerAddressMode GetWrapping(TextureWrapping w)
{
	switch (w) {
		case TextureWrapping::ClampToEdge: return vk::SamplerAddressMode::eClampToEdge;
		case TextureWrapping::MirroredRepeat: return vk::SamplerAddressMode::eMirroredRepeat;
		case TextureWrapping::Repeat: return vk::SamplerAddressMode::eRepeat;
		default: return vk::SamplerAddressMode::eRepeat;
	}
}

inline vk::SamplerMipmapMode GetMipmapFilter(MipmapFiltering f)
{
	switch (f) {
		case MipmapFiltering::Nearest: return vk::SamplerMipmapMode::eNearest;
		case MipmapFiltering::Linear: return vk::SamplerMipmapMode::eLinear;
		case MipmapFiltering::NoMipmap: LOG_ABORT("programmer error");
		default: return vk::SamplerMipmapMode::eLinear;
	}
}

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

// CHECK: how should we get the aspect mask of this image?
inline vk::ImageAspectFlags GetAspectMask(const vk::ImageCreateInfo& ici)
{
	auto aspectMask = vk::ImageAspectFlagBits::eColor;

	if (ici.usage & vk::ImageUsageFlagBits::eDepthStencilAttachment) {
		aspectMask = vk::ImageAspectFlagBits::eDepth;

		// if has stencil component
		if (ici.format == vk::Format::eD32SfloatS8Uint || ici.format == vk::Format::eD24UnormS8Uint) {
			return aspectMask | vk::ImageAspectFlagBits::eStencil;
		}
	}
	return aspectMask;
}
} // namespace vl
