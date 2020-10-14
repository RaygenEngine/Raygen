#include "EdAssetUtils.h"

#include "editor/imgui/ImEd.h"
#include "editor/EditorObject.h"
#include "reflection/ReflectionTools.h"
#include "reflection/ReflEnum.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuImage.h"


namespace ed::asset {

void MaybeHoverTooltip(PodEntry* entry)
{
	if (ImGui::IsItemHovered()) {
		MaybeHoverTooltipForced(true, entry);
	}
}

void MaybeHoverTooltip(BasePodHandle handle)
{
	MaybeHoverTooltip(AssetRegistry::GetEntry(handle));
}

void MaybeHoverTooltipForced(bool showTooltip, BasePodHandle handle)
{
	MaybeHoverTooltipForced(showTooltip, AssetRegistry::GetEntry(handle));
}

void OpenForEdit(BasePodHandle handle)
{
	OpenForEdit(AssetRegistry::GetEntry(handle));
}

void OpenForEdit(PodEntry* entry)
{
	EditorObject->m_windowsComponent.OpenAsset(entry);
}

void MaybeHoverTooltipForced(bool showTooltip, PodEntry* entry)
{
	if (showTooltip) {
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

		if (entry->type == mti::GetTypeId<Image>()) {
			ImGui::BeginTooltip();
			ImGui::SetWindowFontScale(1.0);
			auto handle = vl::GpuAssetManager->GetGpuHandle<Image>(entry->GetHandleAs<Image>());
			ImGui::Image(handle.Lock().image.GetDebugDescriptor(), ImVec2(256, 256));
			ImGui::EndTooltip();
		}
		else if (entry->type == mti::GetTypeId<MaterialInstance>()) {
			ImGui::BeginTooltip();
			ImGui::SetWindowFontScale(1.0);
			auto inst = entry->GetHandleAs<MaterialInstance>().Lock();

			if (inst->descriptorSet.samplers2d.size() > 0) {
				auto& previewImg = vl::GpuAssetManager->GetGpuHandle(inst->descriptorSet.samplers2d[0]);
				ImGui::Image(previewImg.Lock().image.GetDebugDescriptor(), ImVec2(256, 256));
			}

			ImGui::EndTooltip();
		}

		ImEd::EndCodeFont();
	}
}
} // namespace ed::asset
