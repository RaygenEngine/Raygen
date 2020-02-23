#include "pch/pch.h"

#include "editor/utl/EdAssetUtils.h"
#include "editor/imgui/ImEd.h"
#include "reflection/ReflEnum.h"
#include "reflection/ReflectionTools.h"

namespace ed {
namespace asset {
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
				"  Pref. Disk: {}\n"
				"Exp. On Save: {}\n"
				"Imp. On Load: {}\n",
				entry->path, entry->name, entry->type.name(), entry->ptr, entry->uid, entry->transient, //
				entry->metadata.originalImportLocation, magic_enum::enum_name(entry->metadata.preferedDiskType),
				entry->metadata.exportOnSave, entry->metadata.reimportOnLoad);

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
			ImEd::EndCodeFont();
		}
	}
} // namespace asset
} // namespace ed