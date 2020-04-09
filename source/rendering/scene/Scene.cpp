#include "pch.h"
#include "rendering/scene/Scene.h"

Scene_::Scene_(size_t size)
	: size(size)
{
	EnqueueEndFrame();

	// WIP/PERF: could have two seperate for each stage
	cameraDescLayout.AddBinding(
		vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
	cameraDescLayout.Generate();

	spotLightDescLayout.AddBinding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
	spotLightDescLayout.Generate();
}

void Scene_::Upload(uint32 i)
{
	cameras.Upload(i);
	spotlights.Upload(i);
}
