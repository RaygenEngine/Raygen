#include "pch.h"
#include "editor/utl/EdAssetUtils.h"

#include "editor/imgui/ImEd.h"
#include "reflection/ReflectionTools.h"
#include "reflection/ReflEnum.h"
#include "renderer/asset/GpuAssetManager.h"
#include "renderer/asset/Image.h"

namespace ed::asset {

void MaybeHoverTooltip(PodEntry* entry)
{
	if (ImGui::IsItemHovered()) {

		std::string text = fmt::format(
			" Path: {}\n"
			" Name: {}\n"
			" Type: {}\n"
			"  Ptr: {}\n"
			"  UID: {}\n"
			"Trans: {}\n"
			"====================\n"
			"  Orig. Loc.: {}\n"
			"Exp. On Save: {}\n"
			"Imp. On Load: {}\n",
			entry->path, entry->name, entry->type.name(), entry->ptr, entry->uid, entry->transient, //
			entry->metadata.originalImportLocation, entry->metadata.exportOnSave, entry->metadata.reimportOnLoad);

		ImEd::BeginCodeFont();
		constexpr float scale = 0.8f;
		if (!entry->ptr) {
			ImUtil::TextTooltipUtil(text, scale);
			ImEd::EndCodeFont();
			return;
		}

		text += "\n";
		text += "\n";
		text += "\n";

		text += refltools::PropertiesToText(entry->ptr.get());

		ImUtil::TextTooltipUtil(text, scale);

		// WIP:
		// if (entry->type == mti::GetTypeId<SamplerPod>()) {
		//	ImGui::BeginTooltip();

		//	auto handle = GpuAssetManager.GetGpuHandle<SamplerPod>(entry->GetHandleAs<SamplerPod>());
		//	ImGui::Image(GpuAssetManager.LockHandle(handle).GetDebugDescriptor(), ImVec2(256, 256));
		//	ImGui::EndTooltip();
		//}

		ImEd::EndCodeFont();
	}
}

} // namespace ed::asset
