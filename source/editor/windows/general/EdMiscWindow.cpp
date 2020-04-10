#include "pch.h"
#include "EdMiscWindow.h"

#include "editor/DataStrings.h"
#include "editor/EditorObject.h"
#include "editor/imgui/ImEd.h"
#include "editor/imgui/ImguiUtil.h"
#include "rendering/renderer/Renderer.h"

#include <imgui/imgui.h>
namespace ed {
void HelpWindow::OnDraw(const char* title, bool* open)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);
	ImGui::SetNextWindowPos(ImVec2(650.f, 100.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(0, 720.f), ImGuiCond_Always);

	if (ImGui::Begin(title, open)) {
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 33.0f);
		ImGui::Text(txt_help);
		ImGui::Text("");
	}

	ImGui::End();
	ImGui::PopStyleVar();
}

void AboutWindow::ImguiDraw()
{
	ImGui::Text("Raygen: v0.1");
	ImEd::HSpace(220.f);
	ImGui::PushTextWrapPos(ImGui::GetFontSize() * 33.0f);
	ImGui::Text(txt_about);
	ImGui::Text("");
}

void ImGuiDemoWindow::OnDraw(const char* title, bool* open)
{
	ImGui::ShowDemoWindow(open);
}


GBufferDebugWindow::GBufferDebugWindow(std::string_view name)
	: ed::UniqueWindow(name)
{
	Event::OnViewportUpdated.BindFlag(this, m_willInvalidateDescriptors);
}

void GBufferDebugWindow::ImguiDraw()
{
	auto gbuff = vl::Renderer->GetGBuffer();
	if (!gbuff) {
		return;
	}

	bool shouldShowDescriptors = !m_willInvalidateDescriptors.Access();

	ImGui::DragInt2("Preview Size", &m_imgSize.x, 1.f, 0, 4096);


	auto showAttachment = [&, shouldShowDescriptors](vl::ImageAttachment* att) {
		if (ImGui::CollapsingHeader(att->GetName().c_str())) {
			auto descrSet = att->GetDebugDescriptor();

			if (!descrSet) {
				ImGui::Text("Null handle");
				return;
			}

			ImVec2 size = { static_cast<float>(m_imgSize.x), static_cast<float>(m_imgSize.y) };

			if (shouldShowDescriptors) {
				ImGui::Image(descrSet, size);
			}
			else {
				ImGui::Image(ImGui::GetIO().Fonts->TexID, size, ImVec2(0, 0), ImVec2(0, 0));
			}
		}
	};

	for (uint32 i = 0; i < vl::GCount; ++i) {
		showAttachment((*gbuff)[i]);
	}
}
} // namespace ed
