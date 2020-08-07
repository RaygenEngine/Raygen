#pragma once
#include "assets/AssetPod.h"
#include "editor/imgui/ImguiImpl.h"
#include "editor/imgui/ImguiUtil.h"
#include "reflection/ReflClass.h"
#include "reflection/ReflEnum.h"
#include "assets/AssetRegistry.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#define ETXT(Icon, Text) U8(Icon u8"  " u8##Text)


namespace EdColor {
inline const ImVec4 Success = ImVec4(0.3f, 0.9f, 0.3f, 1.0f);
inline const ImVec4& Green = Success;

inline const ImVec4 LightSuccess = ImVec4(0.3f, 0.6f, 0.3f, 1.0f);
inline const ImVec4& LightGreen = LightSuccess;


inline const ImVec4 Failure = ImGui::ColorConvertU32ToFloat4(0xA02040ff);
inline const ImVec4& Red = Failure;

inline const ImVec4 LightFailure = ImGui::ColorConvertU32ToFloat4(0x702030ff);
inline const ImVec4& LightRed = LightFailure;

inline const ImVec4 LightText = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
} // namespace EdColor

class Node;
struct ComponentMetaEntry;

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

bool IsItemDoubleClicked(ImGuiMouseButton button = ImGuiMouseButton_Left);

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

// Similar as typed pod drop but accepts *any* pod type. Usefull for generic pod drag & drops (eg renames, or spawns
// where you will deduce the type again from the entry)
PodEntry* AcceptGenericPodDrop(std::function<void(BasePodHandle, PodEntry*)> onDropped = {});


// Call inside an if(ImGui::BeginDragDrop ...)
template<typename T>
inline void EndDragDropSourceObject(T* payload, const char* payloadTag)
{
	ImGui::SetDragDropPayload(payloadTag, &payload, sizeof(T*));
	ImGui::EndDragDropSource();
}


template<typename T>
inline T* AcceptDropObject(const char* payloadTag)
{
	if (!ImGui::BeginDragDropTarget()) {
		return nullptr;
	}

	const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadTag);
	if (!payload) {
		ImGui::EndDragDropTarget();
		return nullptr;
	}

	assert(payload->DataSize == sizeof(T*));
	T* data = *reinterpret_cast<T**>(payload->Data);
	ImGui::EndDragDropTarget();
	return data;
}


// Call inside an if(ImGui::BeginDragDrop ...)
template<typename T>
inline void EndDragDropSourceByCopy(T& payload, const char* payloadTag)
{
	ImGui::SetDragDropPayload(payloadTag, &payload, sizeof(T));
	ImGui::EndDragDropSource();
}


template<typename T>
inline std::optional<T> AcceptDropByCopy(const char* payloadTag)
{
	if (!ImGui::BeginDragDropTarget()) {
		return {};
	}

	const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadTag);
	if (!payload) {
		ImGui::EndDragDropTarget();
		return {};
	}

	assert(payload->DataSize == sizeof(T));
	T data = *reinterpret_cast<T*>(payload->Data);
	ImGui::EndDragDropTarget();
	return data;
}


template<typename T>
bool EnumDropDown(const char* label, T& enumval)
{
	MetaEnumInst t = GenMetaEnum(enumval);
	auto enumMeta = t.GetEnum();

	// int32 currentItem = ;
	std::string str;
	str = t.GetValueStr();
	int32 currentValue = t.GetValue();

	bool edited = false;

	if (ImGui::BeginCombo(label, str.c_str())) // The second parameter is the label previewed before opening the combo.
	{
		for (auto& [enumStr, value] : enumMeta.GetStringsToValues()) {
			bool selected = (currentValue == value);
			if (ImGui::Selectable(enumStr.c_str(), &selected)) {
				t.SetValue(value);
				edited = true;
			}
			if (selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	return edited;
}


const ComponentMetaEntry* ComponentClassMenu();

} // namespace ImEd
