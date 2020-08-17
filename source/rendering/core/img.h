#pragma once
#include "assets/shared/ImageShared.h"
#include "assets/shared/TextureShared.h"

// WIP: rename header

namespace rvk {

inline vk::Filter getTextureFilter(TextureFiltering f)
{
	switch (f) {
		case TextureFiltering::Nearest: return vk::Filter::eNearest;
		case TextureFiltering::Linear: return vk::Filter::eLinear;
		default: LOG_ABORT("Unsupported");
	}
}

inline vk::SamplerAddressMode getWrapping(TextureWrapping w)
{
	switch (w) {
		case TextureWrapping::ClampToEdge: return vk::SamplerAddressMode::eClampToEdge;
		case TextureWrapping::MirroredRepeat: return vk::SamplerAddressMode::eMirroredRepeat;
		case TextureWrapping::Repeat: return vk::SamplerAddressMode::eRepeat;
		default: LOG_ABORT("Unsupported");
	}
}

inline vk::SamplerMipmapMode getMipmapFilter(MipmapFiltering f)
{
	switch (f) {
		case MipmapFiltering::Nearest: return vk::SamplerMipmapMode::eNearest;
		case MipmapFiltering::Linear: return vk::SamplerMipmapMode::eLinear;
		case MipmapFiltering::NoMipmap: LOG_ABORT("Programmer error");
		default: LOG_ABORT("Unsupported");
	}
}


inline vk::Format getFormat(ImageFormat format)
{
	switch (format) {
		case ImageFormat::Hdr: return vk::Format::eR32G32B32A32Sfloat; break;
		case ImageFormat::Srgb: return vk::Format::eR8G8B8A8Srgb; break;
		case ImageFormat::Unorm: return vk::Format::eR8G8B8A8Unorm; break;
		default: LOG_ABORT("Unsupported");
	}
}

inline vk::ImageAspectFlags getAspectMask(vk::ImageUsageFlags usage, vk::Format format)
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

inline bool isDepthFormat(vk::Format format)
{
	switch (format) {
		case vk::Format::eX8D24UnormPack32:
		case vk::Format::eD24UnormS8Uint:
		case vk::Format::eD32Sfloat:
		case vk::Format::eD32SfloatS8Uint:
		case vk::Format::eD16Unorm:
		case vk::Format::eD16UnormS8Uint: return true;
		default: return false;
	}
}


inline vk::AccessFlags accessFlagsForImageLayout(vk::ImageLayout layout)
{
	switch (layout) {
		case vk::ImageLayout::ePreinitialized: return vk::AccessFlagBits::eHostWrite;
		case vk::ImageLayout::eTransferDstOptimal: return vk::AccessFlagBits::eTransferWrite;
		case vk::ImageLayout::eTransferSrcOptimal: return vk::AccessFlagBits::eTransferRead;
		case vk::ImageLayout::eColorAttachmentOptimal: return vk::AccessFlagBits::eColorAttachmentWrite;
		case vk::ImageLayout::eDepthStencilAttachmentOptimal: return vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		case vk::ImageLayout::eShaderReadOnlyOptimal: return vk::AccessFlagBits::eShaderRead;
		default: return {};
	}
}

inline vk::PipelineStageFlags pipelineStageForLayout(vk::ImageLayout layout)
{
	switch (layout) {
		case vk::ImageLayout::eTransferDstOptimal:
		case vk::ImageLayout::eTransferSrcOptimal: return vk::PipelineStageFlagBits::eTransfer;
		case vk::ImageLayout::eColorAttachmentOptimal: return vk::PipelineStageFlagBits::eColorAttachmentOutput;
		case vk::ImageLayout::eDepthStencilAttachmentOptimal: return vk::PipelineStageFlagBits::eEarlyFragmentTests;
		case vk::ImageLayout::eShaderReadOnlyOptimal: return vk::PipelineStageFlagBits::eFragmentShader;
		case vk::ImageLayout::ePreinitialized: return vk::PipelineStageFlagBits::eHost;
		case vk::ImageLayout::eUndefined: return vk::PipelineStageFlagBits::eTopOfPipe;
		default: return vk::PipelineStageFlagBits::eBottomOfPipe;
	}
}

// WIP: move function
inline vk::PipelineStageFlags accessMaskPipelineStageFlags(vk::AccessFlags accessMask,
	vk::PipelineStageFlags supportedShaderBits
	= vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eTessellationControlShader
	  | vk::PipelineStageFlagBits::eTessellationEvaluationShader | vk::PipelineStageFlagBits::eFragmentShader
	  | vk::PipelineStageFlagBits::eGeometryShader | vk::PipelineStageFlagBits::eComputeShader
	  | vk::PipelineStageFlagBits::eRayTracingShaderKHR // WIP: which shaders default
)
{
	if (!accessMask) {
		return vk::PipelineStageFlagBits::eTopOfPipe;
	}


	static constexpr std::array accessMasks = {
		vk::AccessFlagBits::eIndirectCommandRead,
		vk::AccessFlagBits::eIndexRead,
		vk::AccessFlagBits::eVertexAttributeRead,
		vk::AccessFlagBits::eUniformRead,
		vk::AccessFlagBits::eInputAttachmentRead,
		vk::AccessFlagBits::eShaderRead,
		vk::AccessFlagBits::eShaderWrite,
		vk::AccessFlagBits::eColorAttachmentRead,
		vk::AccessFlagBits::eColorAttachmentReadNoncoherentEXT,
		vk::AccessFlagBits::eColorAttachmentWrite,
		vk::AccessFlagBits::eDepthStencilAttachmentRead,
		vk::AccessFlagBits::eDepthStencilAttachmentWrite,
		vk::AccessFlagBits::eTransferRead,
		vk::AccessFlagBits::eTransferWrite,
		vk::AccessFlagBits::eHostRead,
		vk::AccessFlagBits::eHostWrite,
		vk::AccessFlagBits::eMemoryRead,
		vk::AccessFlagBits::eMemoryWrite,
		vk::AccessFlagBits::eAccelerationStructureReadKHR,
		vk::AccessFlagBits::eAccelerationStructureWriteKHR,
	};

	static const std::array<vk::PipelineStageFlags, 20> pipeStages = {

		vk::PipelineStageFlagBits::eDrawIndirect,
		vk::PipelineStageFlagBits::eVertexInput,
		vk::PipelineStageFlagBits::eVertexInput,
		supportedShaderBits,
		vk::PipelineStageFlagBits::eFragmentShader,
		supportedShaderBits,
		supportedShaderBits,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eEarlyFragmentTests
			| vk::PipelineStageFlagBits::eLateFragmentTests, // CHECK: maybe not optimized
		vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eHost,
		vk::PipelineStageFlagBits::eHost,
		vk::PipelineStageFlags{ 0 },
		vk::PipelineStageFlags{ 0 },
		vk::PipelineStageFlagBits::eRayTracingShaderKHR | supportedShaderBits
			| vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
		vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
	};


	vk::PipelineStageFlags pipelineStages{};
	for (uint32_t i = 0; i < accessMasks.size(); ++i) {
		if (accessMasks[i] & accessMask) {
			pipelineStages |= pipeStages[i];
		}
	}

	CLOG_ABORT(!pipelineStages, "Programmer error");

	return pipelineStages;
}

} // namespace rvk
