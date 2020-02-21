#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

// ImGui wrapper for calls that use different styles. (bigger buttons etc)
// header file to allow inlining
namespace ImEd {
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


// TODO: Add cpp to this header
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


} // namespace ImEd
