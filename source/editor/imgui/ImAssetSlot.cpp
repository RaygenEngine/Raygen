#include "ImAssetSlot.h"

namespace ImEd {
void AssetSlotPopup::Clear()
{
	searchFilter.Clear();
	results.clear();
	searchType = mti::GetTypeId<void>();
}

void AssetSlotPopup::HoverIndex(int32 newIndex)
{
	if (results.empty()) {
		hoveredIndex = -1;
		hoveredEntry = nullptr;
		return;
	}
	hoveredIndex = std::clamp(newIndex, 0, static_cast<int32>(results.size() - 1));
	hoveredEntry = results[hoveredIndex];
}

void AssetSlotPopup::UpdateSearch()
{
	// PERF: filter and store by type
	// PERF: can search subresults when a letter is added instead of a new search
	auto& allPods = AssetRegistry::Z_GetPods();

	const bool useType = searchType != mti::GetTypeId<void>();
	results.clear();


	// Determine if we found the previously hovered item on the new search.
	// We want to keep the selection if we find it or select the first result if it is no longer in the results.
	int32 prevHoveredNewIndex = -1;

	for (auto& pod : allPods) {
		if (useType && searchType != pod->type)
			[[likely]] { continue; }

		if (searchFilter.PassFilter(pod->name.c_str())) {
			results.push_back(pod.get());
			if (pod.get() == hoveredEntry) {
				prevHoveredNewIndex = static_cast<int32>(results.size()) - 1;
			}
		}
	}

	HoverIndex(std::max(0, prevHoveredNewIndex));
}


bool AssetSlotPopup::Draw(BasePodHandle& handle, bool stealFocus)
{
	if (stealFocus) {
		ImGui::SetKeyboardFocusHere();
	}
	if (searchFilter.Draw(U8(FA_SEARCH u8"##AssetSlotSearch"))) {
		UpdateSearch();
	}

	if (ImGui::IsKeyPressedMap(ImGuiKey_UpArrow)) {
		HoverIndex(hoveredIndex - 1);
	}
	if (ImGui::IsKeyPressedMap(ImGuiKey_DownArrow)) {
		HoverIndex(hoveredIndex + 1);
	}

	bool selectHovered = ImGui::IsKeyPressedMap(ImGuiKey_Enter) || ImGui::IsKeyPressedMap(ImGuiKey_KeyPadEnter);


	bool changed{ false };
	for (auto& entry : results) {
		if (ImGui::Selectable(entry->path.c_str(), entry == hoveredEntry) || (selectHovered && entry == hoveredEntry)) {
			changed = true;
			handle = { entry->uid };
		}
	}

	if (changed) {
		Clear();
	}
	return changed;
}

} // namespace ImEd
