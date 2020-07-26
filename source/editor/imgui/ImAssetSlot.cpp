#include "pch.h"
#include "ImAssetSlot.h"

namespace ImEd {
void AssetSlotPopup::Clear()
{
	searchFilter.Clear();
	results.clear();
	searchType = mti::GetTypeId<void>();
}

void AssetSlotPopup::UpdateSearch()
{
	// PERF: filter and store by type
	// PERF: can search subresults when a letter is added instead of a new search
	auto& allPods = AssetHandlerManager::Z_GetPods();

	const bool useType = searchType != mti::GetTypeId<void>();
	results.clear();


	for (auto& pod : allPods) {
		if (useType && searchType != pod->type)
			[[likely]] { continue; }

		if (searchFilter.PassFilter(pod->name.c_str())) {
			results.push_back(pod.get());
		}
	}
}


bool AssetSlotPopup::Draw(BasePodHandle& handle, bool stealFocus)
{
	if (stealFocus) {
		ImGui::SetKeyboardFocusHere();
	}
	if (searchFilter.Draw(U8(FA_SEARCH u8"##AssetSlotSearch"))) {
		UpdateSearch();
	}


	bool changed{ false };
	for (auto& entry : results) {
		if (ImGui::Selectable(entry->path.c_str())) {
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
