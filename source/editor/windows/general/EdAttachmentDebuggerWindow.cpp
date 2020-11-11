#include "EdAttachmentDebuggerWindow.h"

#include "editor/EditorObject.h"
#include "rendering/Renderer.h"
#include "rendering/scene/SceneSpotlight.h"
#include "engine/Events.h"
#include "rendering/Layer.h"


namespace ed {

AttachmentDebuggerWindow::AttachmentDebuggerWindow(std::string_view name)
	: ed::UniqueWindow(name)
{
	Event::OnViewportUpdated.BindFlag(this, m_willInvalidateDescriptors);
}

void AttachmentDebuggerWindow::ImguiDraw()
{
	bool shouldShowDescriptors = !m_willInvalidateDescriptors.Access();

	auto showImage = [&, shouldShowDescriptors](vl::RImage& att) {
		bool& isOpen = isAttachmentOpen[att.name]; // find or insert

		ImGui::PushID(&att);
		ImGui::Checkbox(att.name.c_str(), &isOpen);
		if (isOpen) {
			std::string name = fmt::format("Att {}", att.name);
			ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_FirstUseEver);
			if (ImGui::Begin(name.c_str(), &isOpen)) {
				auto descrSet = att.GetDebugDescriptor();

				if (!descrSet) {
					ImGui::Text("Null handle");
					return;
				}
				auto ext = att.extent;

				ImVec2 size = { static_cast<float>(ext.width), static_cast<float>(ext.height) };

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

	auto showFramebuffer = [&, shouldShowDescriptors](vl::RFramebuffer& fb) {
		for (auto& att : fb.ownedAttachments) {
			showImage(att);
		}
	};

	auto& gbufferFramebuffer = vl::Renderer->m_gbufferInst.at(0).framebuffer;
	auto& directPassFramebuffer = vl::Renderer->m_directLightPass.at(0).framebuffer;
	auto& indirectPassFramebuffer = vl::Renderer->m_indirectLightPass.at(0).framebuffer;
	auto& ptPassFramebuffer = vl::Renderer->m_ptPass.at(0).framebuffer;

	showFramebuffer(gbufferFramebuffer);
	showFramebuffer(directPassFramebuffer);
	showFramebuffer(indirectPassFramebuffer);
	showImage(vl::Renderer->m_indirectSpecPass.m_result.at(0));
	showFramebuffer(ptPassFramebuffer);

	for (auto sl : vl::Layer->mainScene->Get<SceneSpotlight>()) {
		showImage(sl->shadowmap.at(0).framebuffer.ownedAttachments.at(0));
	}
}

} // namespace ed
