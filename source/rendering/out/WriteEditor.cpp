#include "pch.h"
#include "WriteEditor.h"

#include "editor/imgui/ImguiImpl.h"

namespace vl {
void WriteEditor::RecordCmd(vk::CommandBuffer* cmdBuffer)
{
	// No pipeline required here, imgui uses its own
	ImguiImpl::RenderVulkan(cmdBuffer);
}
} // namespace vl
