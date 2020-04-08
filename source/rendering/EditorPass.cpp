#include "pch.h"
#include "EditorPass.h"

#include "editor/imgui/ImguiImpl.h"
#include "engine/Engine.h"
#include "rendering/renderer/Renderer.h"

namespace vl {
void EditorPass::RecordCmd(vk::CommandBuffer* cmdBuffer)
{
	// No pipeline required here, imgui uses its own
	ImguiImpl::RenderVulkan(cmdBuffer);
}
} // namespace vl
