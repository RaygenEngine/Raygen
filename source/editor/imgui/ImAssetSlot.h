#pragma once

#include "assets/PodHandle.h"
#include "assets/AssetRegistry.h"
#include "editor/imgui/ImguiUtil.h"
#include "editor/imgui/ImEd.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>


namespace ImEd {


struct AssetSlotPopup {
	ImGuiTextFilter searchFilter;
	std::vector<PodEntry*> results;
	mti::TypeId searchType;


	void Clear();


	template<CONC(CAssetPod) T>
	void Open()
	{
		Clear();
		searchType = mti::GetTypeId<T>();
		UpdateSearch();
	}

	bool Draw(BasePodHandle& handle);

protected:
	void UpdateSearch();
};


// Returns true when the handle is changed
template<CONC(CAssetPod) T>
bool AssetSlot(const std::string& name, PodHandle<T>& handle)
{
	static AssetSlotPopup searchPopup;

	auto entry = AssetHandlerManager::GetEntry(handle);

	// ImGui::TextUnformatted(U8(T::StaticClass().GetIcon()));
	// ImGui::SameLine();

	auto id = ImGui::GetID(name.c_str());


	bool wasChanged = false;
	if (ImGui::BeginCombo(name.c_str(), entry->path.c_str(), ImGuiComboFlags_HeightLarge)) {
		if (ImGui::IsWindowAppearing()) {
			searchPopup.Open<T>();
		}
		if (searchPopup.Draw(handle)) {
			searchPopup.Clear();
			ImGui::CloseCurrentPopup();
			wasChanged = true;
		}
		ImGui::EndCombo();
	}

	if (auto entry = ImEd::AcceptTypedPodDrop<T>(); entry) {
		handle = entry->GetHandleAs<T>();
		wasChanged = true;
	}

	return wasChanged;
}
} // namespace ImEd
