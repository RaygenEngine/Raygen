#include "pch/pch.h"

#include "renderer/EditorPass.h"
#include "renderer/VulkanLayer.h"
#include "editor/imgui/ImguiImpl.h"
#include "system/Engine.h"

void EditorPass::RecordCmd(vk::CommandBuffer* cmdBuffer)
{
	// No pipeline required here, imgui uses its own
	ImguiImpl::RenderVulkan(cmdBuffer);
}
