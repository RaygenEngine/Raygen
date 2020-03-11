#include "pch.h"
#include "renderer/wrapper/GBuffer.h"

#include "renderer/wrapper/Device.h"

GBuffer::GBuffer(uint32 width, uint32 height)
{
	position = std::make_unique<Attachment>(width, height, vk::Format::eR8G8B8A8Snorm,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);

	normal = std::make_unique<Attachment>(width, height, vk::Format::eR8G8B8A8Snorm,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);

	albedo = std::make_unique<Attachment>(width, height, vk::Format::eR8G8B8A8Snorm,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);

	specular = std::make_unique<Attachment>(width, height, vk::Format::eR8G8B8A8Snorm,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);

	emissive = std::make_unique<Attachment>(width, height, vk::Format::eR8G8B8A8Snorm,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);

	vk::Format depthFormat = Device->pd->FindDepthFormat();

	depth = std::make_unique<Attachment>(
		width, height, depthFormat, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);
}

void GBuffer::TransitionForShaderRead()
{
	position->image->TransitionToLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	normal->image->TransitionToLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	albedo->image->TransitionToLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	specular->image->TransitionToLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	emissive->image->TransitionToLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	depth->image->TransitionToLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
}

void GBuffer::TransitionForAttachmentWrite()
{
	position->image->TransitionToLayout(vk::ImageLayout::eColorAttachmentOptimal);
	normal->image->TransitionToLayout(vk::ImageLayout::eColorAttachmentOptimal);
	albedo->image->TransitionToLayout(vk::ImageLayout::eColorAttachmentOptimal);
	specular->image->TransitionToLayout(vk::ImageLayout::eColorAttachmentOptimal);
	emissive->image->TransitionToLayout(vk::ImageLayout::eColorAttachmentOptimal);
	depth->image->TransitionToLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
}

std::array<vk::ImageView, 6> GBuffer::GetViewsArray()
{
	return { position->view.get(), normal->view.get(), albedo->view.get(), specular->view.get(), emissive->view.get(),
		depth->view.get() };
}
