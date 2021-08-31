#include "EdAttachmentDebuggerWindow.h"

#include "editor/EditorObject.h"
#include "engine/Events.h"
#include "rendering/Rendering.h"
#include "rendering/RendererBase.h"

namespace ed {

AttachmentDebuggerWindow::AttachmentDebuggerWindow(std::string_view name)
	: ed::UniqueWindow(name)
{
	Event::OnViewportUpdated.BindFlag(this, m_willInvalidateDescriptors);
}

void AttachmentDebuggerWindow::ImguiDraw()
{
	bool shouldShowDescriptors = !m_willInvalidateDescriptors.Access();

	auto showImage = [&, shouldShowDescriptors](const vl::RendererBase::AttachmentData& att) {
		bool& isOpen = isAttachmentOpen[att.name]; // find or insert

		ImGui::PushID(&att);
		ImGui::Checkbox(att.name, &isOpen);
		if (isOpen) {
			std::string name = fmt::format("Att {}", att.name);
			ImGui::SetNextWindowSize(ImVec2(att.extent.x / 4.f, att.extent.y / 4.f), ImGuiCond_FirstUseEver);
			if (ImGui::Begin(name.c_str(), &isOpen)) {
				auto descrSet = att.descSet;

				if (!descrSet) {
					ImGui::Text("Null handle");
					return;
				}
				auto ext = att.extent;

				ImVec2 size = { static_cast<float>(ext.x), static_cast<float>(ext.y) };

				auto windowSize = ImGui::GetCurrentWindow()->Size;
				windowSize.x -= 4;  // CHECK: Proper calculation for padding (to avoid scrollbar)
				windowSize.y -= 35; //

				if (size.x > windowSize.x) {
					float scaleFactor = windowSize.x / std::max(size.x, 1.f);

					size.x *= scaleFactor;
					size.y *= scaleFactor;
				}

				if (size.y > windowSize.y) {
					float scaleFactor = windowSize.y / std::max(size.y, 1.f);

					size.x *= scaleFactor;
					size.y *= scaleFactor;
				}

				if (shouldShowDescriptors) {
					ImGui::Image(descrSet, size);
				}
				else {
					ImGui::Image(ImGui::GetIO().Fonts->TexID, size, ImVec2(0, 0), ImVec2(0, 0));
				}
			}
			ImGui::End();
		}
		ImGui::PopID();
	};

	auto activeRenderer = Rendering::GetActiveRenderer();
	// TODO: GetMainScene()->GetDebugAttachments - or through renderer somehow

	for (const auto& debugAtt : activeRenderer->GetDebugAttachments()) {
		showImage(debugAtt);
	}
}

} // namespace ed
