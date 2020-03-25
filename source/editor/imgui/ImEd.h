#pragma once
#include "asset/AssetManager.h"
#include "editor/imgui/ImguiImpl.h"
#include "editor/imgui/ImguiUtil.h"
#include "reflection/ReflClass.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#define ETXT(Icon, Text) U8(Icon u8"  " u8##Text)

class Node;
// ImGui wrapper for calls that use different styles. (bigger buttons etc)
namespace ImEd {

struct AssetNameFilter {
	static int FilterImGuiLetters(ImGuiInputTextCallbackData* data);
};

bool Button(const char* label, const ImVec2& size = ImVec2(0.f, 0.f));
void SetNextItemPerc(float perc);

bool BeginMenuBar();
void EndMenuBar();

bool BeginMenu(const char* label, bool enabled = true);
void EndMenu();

void HSpace(float space = 6.f);


bool InputTextSized(const char* label, std::string* str, ImVec2 size, ImGuiInputTextFlags flags = 0,
	ImGuiInputTextCallback callback = NULL, void* user_data = NULL);


inline void BeginCodeFont()
{
	ImGui::PushFont(ImguiImpl::s_CodeFont);
}

inline void EndCodeFont()
{
	ImGui::PopFont();
}

void CreateTypedPodDrag(PodEntry* entry, ImGuiDragDropFlags flags = 0);
void CreateTypedPodDrag(BasePodHandle handle, ImGuiDragDropFlags flags = 0);


void HelpTooltip(const char* tooltip);
void HelpTooltipInline(const char* tooltip);
void CollapsingHeaderHelpTooltip(const char* tooltip);

template<typename T>
inline void DisabledSection(bool disabled, const T& codeSection)
{
	if (disabled) {
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f);
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	}
	codeSection();

	if (disabled) {
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}
}

struct NoFunc {
	void operator()(BasePodHandle, PodEntry*) {}
};
template<CONC(CAssetPod) T, typename Callback = NoFunc>
inline PodEntry* AcceptTypedPodDrop(Callback onDropped = {})
{
	// TODO: This function probably contains a bug

	if (!ImGui::BeginDragDropTarget()) {
		return nullptr;
	}
	std::string payloadTag = "POD_UID_" + std::to_string(mti::GetHash<T>());
	const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadTag.c_str());
	if (!payload) {
		ImGui::EndDragDropTarget();
		return nullptr;
	}

	assert(payload->DataSize == sizeof(PodEntry*));
	PodEntry* entry = *reinterpret_cast<PodEntry**>(payload->Data);
	PodHandle<T> handle{ entry->uid };

	onDropped(handle, entry);
	ImGui::EndDragDropTarget();
	return entry;
}

} // namespace ImEd
