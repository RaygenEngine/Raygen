#include "pch.h"
#include "PodEditor.h"

#include "assets/AssetRegistry.h"

void PodEditorBase::CommitUpdate(size_t uid, AssetUpdateInfo&& info)
{
	AssetRegistry::GetEntry(uid)->MarkSave();
	AssetRegistry::RequestGpuUpdateFor(uid, std::move(info));
}
