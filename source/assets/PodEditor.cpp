#include "PodEditor.h"

#include "assets/AssetRegistry.h"
#include "assets/ShaderRegistry.h"

// TODO: this is probably an asset registry function but its here to avoid nasty header includes
void PodEditorBase::CommitUpdate(size_t uid, AssetUpdateInfo&& info)
{
	auto entry = AssetRegistry::GetEntry(uid);

	entry->MarkSave();

	if (entry->IsA<ShaderStage>() || entry->IsA<ShaderHeader>()) {
		ShaderRegistry::OnEdited({ entry->uid });
	}
	AssetRegistry::RequestGpuUpdateFor(uid, std::move(info));
}
