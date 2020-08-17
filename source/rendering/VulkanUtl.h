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

inline bool IsDepthFormat(vk::Format format)
{

	switch (format) {
		case vk::Format::eD32Sfloat:
		case vk::Format::eD32SfloatS8Uint:
		case vk::Format::eD24UnormS8Uint: return true;
		default: return false;
	}
}

inline vk::AccessFlags GetAccessMask(vk::ImageLayout imL)
{
	switch (imL) {
		case vk::ImageLayout::eUndefined: return vk::AccessFlags{ 0u };
		case vk::ImageLayout::eGeneral: return vk::AccessFlags{ 0u };
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

template<typename T>
inline bool Contains(const std::vector<char const*>& data, std::vector<T> const& searchData)
{
	// return true if all T are listed in the properties
	return std::all_of(data.begin(), data.end(), [&properties](char const* name) {
		return std::find_if(properties.begin(), properties.end(), [&name](vk::LayerProperties const& property) {
			return strcmp(property.layerName, name) == 0;
		}) != properties.end();
	});
}

inline vk::PipelineStageFlags AccessMassPipelineStageFlags(vk::AccessFlags accessMask, vk::PipelineStageFlags supportedShaderBits =
	vk::PipelineStageFlagBits::eVertexShader |
	vk::PipelineStageFlagBits::eTessellationControlShader |
	vk::PipelineStageFlagBits::eTessellationEvaluationShader |
	vk::PipelineStageFlagBits::eFragmentShader |
	vk::PipelineStageFlagBits::eGeometryShader |
	vk::PipelineStageFlagBits::eComputeShader |
	vk::PipelineStageFlagBits::eRayTracingShaderKHR // WIP: which shaders default
)
{
	if (!accessMask)
	{
		return vk::PipelineStageFlagBits::eTopOfPipe;
	}


	static constexpr std::array accessMasks = {
		vk::AccessFlags{vk::AccessFlagBits::eIndirectCommandRead},
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
		vk::AccessFlagBits::eAccelerationStructureWriteKHR
	};

	static const std::array pipeStages = {
	
		vk::PipelineStageFlags{vk::PipelineStageFlagBits::eDrawIndirect},
		vk::PipelineStageFlagBits::eVertexInput,
		vk::PipelineStageFlagBits::eVertexInput,
		supportedShaderBits,
		vk::PipelineStageFlagBits::eFragmentShader,
		supportedShaderBits,
		supportedShaderBits,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests, // CHECK: maybe not optimized
		vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eHost,
		vk::PipelineStageFlagBits::eHost,
		vk::PipelineStageFlags{0},
		vk::PipelineStageFlags{0},
		vk::PipelineStageFlagBits::eRayTracingShaderKHR | supportedShaderBits | vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
		vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR
	};


	vk::PipelineStageFlags pipelineStages{};
	for (uint32_t i = 0; i < accessMasks.size(); ++i)
	{
		if (accessMasks[i] & accessMask)
		{
			pipelineStages |= pipeStages[i];
		}
	}

	CLOG_ABORT(!pipelineStages, "Programmer error");

	return pipelineStages;
}

} // namespace vl
