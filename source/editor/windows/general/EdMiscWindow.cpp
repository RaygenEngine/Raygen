#include "pch.h"
#include "EdMiscWindow.h"

#include "editor/DataStrings.h"
#include "editor/EditorObject.h"
#include "editor/imgui/ImEd.h"
#include "editor/imgui/ImguiUtil.h"
#include "rendering/Renderer.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneDirectionalLight.h"
#include "rendering/scene/SceneSpotlight.h"
#include "engine/Events.h"

// WIP:
#include "rendering/Layer.h"

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


GbufferDebugWindow::GbufferDebugWindow(std::string_view name)
	: ed::UniqueWindow(name)
{
	Event::OnViewportUpdated.BindFlag(this, m_willInvalidateDescriptors);
}

void GbufferDebugWindow::ImguiDraw()
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
	auto& rasterDirectPassFramebuffer = vl::Renderer->m_rasterDirectPass.at(0).framebuffer;
	auto& ptPassFramebuffer = vl::Renderer->m_ptPass.at(0).framebuffer;
	auto& rtPassProgImage = vl::Renderer->m_raytracingPass.m_progressiveResult;

	showFramebuffer(gbufferFramebuffer); // TODO: fix depth val errors
	showFramebuffer(rasterDirectPassFramebuffer);
	showFramebuffer(ptPassFramebuffer);
	showImage(rtPassProgImage); // TODO: fix layout errors

	showImage(vl::Renderer->m_raytracingPass.svgfPass.swappingImages[0]);
	showImage(vl::Renderer->m_raytracingPass.svgfPass.swappingImages[1]);

	for (auto sl : vl::Layer->mainScene->spotlights.elements) {
		if (sl) {
			showImage(sl->shadowmap.at(0).framebuffer.ownedAttachments.at(0));
		}
	}
}

void PodEntryEditorWindow::ImguiDraw()
{
	ImEd::Button("Drop an asset here.");
	ImEd::AcceptGenericPodDrop([&](auto, PodEntry* newEntry) { entry = newEntry; });

	if (entry) {
		ImGui::Checkbox("Reimport On Load", &entry->metadata.reimportOnLoad);
		ImGui::Checkbox("Export On Save", &entry->metadata.exportOnSave);
		ImGui::Checkbox("Transient", &entry->transient);
		ImGui::InputText("Import Path", &entry->metadata.originalImportLocation);
		ImGui::Text("Hash: %d", &entry->metadata.podTypeHash);
		ImGui::Text("===", &entry->metadata.podTypeHash);

		if (ImGui::Button("Mark Save")) {
			entry->MarkSave();
		}

		if (ImGui::Button("Reload From Disk")) {
			AssetHandlerManager::ReimportFromOriginal(entry);
		}
	}
}
} // namespace ed
