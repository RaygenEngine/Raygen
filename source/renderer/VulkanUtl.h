#include "asset/pods/SamplerPod.h"

#include <vulkan/vulkan.hpp>

inline vk::Filter GetFilter(TextureFiltering f)
{
	switch (f) {
		case TextureFiltering::Nearest:
		case TextureFiltering::NearestMipmapNearest:
		case TextureFiltering::NearestMipmapLinear: return vk::Filter::eNearest;
		case TextureFiltering::Linear:
		case TextureFiltering::LinearMipmapNearest:
		case TextureFiltering::LinearMipmapLinear: return vk::Filter::eLinear;
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

inline vk::SamplerMipmapMode GetMipmapFilter(TextureFiltering f)
{
	switch (f) {
		case TextureFiltering::Nearest:
		case TextureFiltering::NearestMipmapNearest:
		case TextureFiltering::LinearMipmapNearest: return vk::SamplerMipmapMode::eNearest;
		case TextureFiltering::Linear:
		case TextureFiltering::NearestMipmapLinear:
		case TextureFiltering::LinearMipmapLinear: return vk::SamplerMipmapMode::eLinear;
		default: return vk::SamplerMipmapMode::eLinear;
	}
}
