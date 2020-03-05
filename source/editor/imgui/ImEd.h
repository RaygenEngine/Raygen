#pragma once
#include "editor/imgui/ImguiUtil.h"
#include "editor/imgui/ImguiImpl.h"
#include "asset/AssetManager.h"
#include "reflection/ReflClass.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#define ETXT(Icon, Text) U8(Icon u8"  " u8##Text)

// ImGui wrapper for calls that use different styles. (bigger buttons etc)
// header file to allow inlining
namespace ImEd {

struct AssetNameFilter {
	static int FilterImGuiLetters(ImGuiInputTextCallbackData* data)
	{
		// TODO: Define and use in asset manager somewhere
		if (data->EventChar < 256 && !strchr("./<>:\\|?*~#", (char)data->EventChar)) {
			return 0;
		}
		return 1;
	}
};

inline bool Button(const char* label, const ImVec2& size = ImVec2(0.f, 0.f))
{
	bool result;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
	result = ImGui::Button(label, size);
	ImGui::PopStyleVar();
	return result;
}

inline void SetNextItemPerc(float perc)
{
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * perc);
}

inline bool BeginMenuBar()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.f, 7.f)); // On edit update imextras.h c_MenuPaddingY
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.f, 7.f));
	if (!ImGui::BeginMenuBar()) {
		ImGui::PopStyleVar(2);
		return false;
	}
	return true;
}

inline void EndMenuBar()
{
	ImGui::EndMenuBar();
	ImGui::PopStyleVar(2);
}

inline bool BeginMenu(const char* label, bool enabled = true)
{
	bool open = ImGui::BeginMenu(label, enabled);
	if (open) {
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 3.f));
		ImGui::Spacing();
		ImGui::PopStyleVar();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.f, 7.f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5.f, 7.f));
		return true;
	}
	return false;
}

inline void EndMenu()
{
	ImGui::PopStyleVar(2);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, -3.f));
	ImGui::Spacing();
	ImGui::PopStyleVar();
	ImGui::EndMenu();
}

inline void HSpace(float space = 6.f)
{
	ImGui::SameLine();
	ImGui::Dummy(ImVec2(space, 0.f));
}


// NEXT: Add cpp to this header
struct InputTextCallback_UserData {
	std::string* Str;
	ImGuiInputTextCallback ChainCallback;
	void* ChainCallbackUserData;
};

inline int InputTextCallback(ImGuiInputTextCallbackData* data)
{
	InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)data->UserData;
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
		// Resize string callback
		// If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back
		// to what we want.
		std::string* str = user_data->Str;
		IM_ASSERT(data->Buf == str->c_str());
		str->resize(data->BufTextLen);
		data->Buf = (char*)str->c_str();
	}
	else if (user_data->ChainCallback) {
		// Forward to user callback, if any
		data->UserData = user_data->ChainCallbackUserData;
		return user_data->ChainCallback(data);
	}
	return 0;
}

inline bool InputTextSized(const char* label, std::string* str, ImVec2 size, ImGuiInputTextFlags flags = 0,
	ImGuiInputTextCallback callback = NULL, void* user_data = NULL)
{
	IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
	flags |= ImGuiInputTextFlags_CallbackResize;

	InputTextCallback_UserData cb_user_data;
	cb_user_data.Str = str;
	cb_user_data.ChainCallback = callback;
	cb_user_data.ChainCallbackUserData = user_data;

	return ImGui::InputTextEx(label, NULL, const_cast<char*>(str->c_str()), static_cast<int>(str->capacity()) + 1, size,
		flags, InputTextCallback, &cb_user_data);
}

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

inline void BeginCodeFont()
{
	ImGui::PushFont(ImguiImpl::s_CodeFont);
}

inline void EndCodeFont()
{
	ImGui::PopFont();
}

inline void CreateTypedPodDrag(PodEntry* entry, ImGuiDragDropFlags flags = 0)
{
	if (ImGui::BeginDragDropSource(flags)) {
		ImGui::PushFont(ImguiImpl::s_AssetIconFont);
		ImGui::TextUnformatted(U8(entry->GetClass()->GetIcon()));
		ImGui::PopFont();
		ImGui::TextUnformatted(std::string(entry->GetName()).c_str());

		std::string payloadTag = "POD_UID_" + std::to_string(entry->type.hash());
		ImGui::SetDragDropPayload(payloadTag.c_str(), &entry, sizeof(PodEntry*));
		ImGui::EndDragDropSource();
	}
}

inline void CreateTypedPodDrag(BasePodHandle handle, ImGuiDragDropFlags flags = 0)
{
	auto entry = AssetHandlerManager::GetEntry(handle);
	CreateTypedPodDrag(entry, flags);
}

struct NoFunc {
	void operator()(BasePodHandle, PodEntry*) {}
};

template<CONC(CAssetPod) T, typename Callback = NoFunc>
inline PodEntry* AcceptTypedPodDrop(Callback onDropped = {})
{
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
