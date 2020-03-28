#include "pch.h"
#include "ImEd.h"

#include "assets/PodEntry.h"
#include "assets/AssetRegistry.h"

namespace ImEd {
int InputTextCallback(ImGuiInputTextCallbackData* data);

struct InputTextCallback_UserData {
	std::string* Str;
	ImGuiInputTextCallback ChainCallback;
	void* ChainCallbackUserData;
};


bool Button(const char* label, const ImVec2& size)
{
	bool result;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
	result = ImGui::Button(label, size);
	ImGui::PopStyleVar();
	return result;
}

void SetNextItemPerc(float perc)
{
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * perc);
}

bool BeginMenuBar()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.f, 7.f)); // On edit update imextras.h c_MenuPaddingY
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.f, 7.f));
	if (!ImGui::BeginMenuBar()) {
		ImGui::PopStyleVar(2);
		return false;
	}
	return true;
}

bool BeginMenu(const char* label, bool enabled)
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

void EndMenuBar()
{
	ImGui::EndMenuBar();
	ImGui::PopStyleVar(2);
}


void EndMenu()
{
	ImGui::PopStyleVar(2);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, -3.f));
	ImGui::Spacing();
	ImGui::PopStyleVar();
	ImGui::EndMenu();
}

void HSpace(float space)
{
	ImGui::SameLine();
	ImGui::Dummy(ImVec2(space, 0.f));
}


int InputTextCallback(ImGuiInputTextCallbackData* data)
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

bool InputTextSized(const char* label, std::string* str, ImVec2 size, ImGuiInputTextFlags flags,
	ImGuiInputTextCallback callback, void* user_data)
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

void CreateTypedPodDrag(PodEntry* entry, ImGuiDragDropFlags flags)
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

void CreateTypedPodDrag(BasePodHandle handle, ImGuiDragDropFlags flags)
{
	auto entry = AssetHandlerManager::GetEntry(handle);
	CreateTypedPodDrag(entry, flags);
}

int AssetNameFilter::FilterImGuiLetters(ImGuiInputTextCallbackData* data)
{
	// CHECK: Define and use in asset manager somewhere
	if (data->EventChar < 256 && !strchr("./<>:\\|?*~#", (char)data->EventChar)) {
		return 0;
	}
	return 1;
}


void HelpTooltip(const char* tooltip)
{
	ImEd::HSpace(1.f);
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 0.95f));
	ImGui::TextUnformatted("\xc2\xb0"); // help symbol: aka U+00b0
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
		if (tooltip[0] == '\n') {
			tooltip++;
		}
		ImGui::TextUnformatted(tooltip);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}
void HelpTooltipInline(const char* tooltip)
{
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
		if (tooltip[0] == '\n') {
			tooltip++;
		}
		ImGui::TextUnformatted(tooltip);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void CollapsingHeaderHelpTooltip(const char* tooltip)
{
	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 12.f);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.6f, 0.95f));
	ImGui::TextUnformatted("?");
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
		if (tooltip[0] == '\n') {
			tooltip++;
		}
		ImGui::TextUnformatted(tooltip);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}


} // namespace ImEd
