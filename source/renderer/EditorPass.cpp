#include "pch.h"
#include "renderer/EditorPass.h"

#include "editor/imgui/ImguiImpl.h"
#include "engine/Engine.h"
#include "renderer/VulkanLayer.h"

void EditorPass::RecordCmd(vk::CommandBuffer* cmdBuffer)
{
	// No pipeline required here, imgui uses its own
	// ImguiImpl::RenderVulkan(cmdBuffer);
}
