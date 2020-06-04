#include "pch.h"
#include "PodEditor.h"

#include "assets/AssetRegistry.h"

void PodEditorBase::CommitUpdate(size_t uid, AssetUpdateInfo&& info)
{
	AssetHandlerManager::GetEntry(uid)->MarkSave();
	AssetHandlerManager::RequestGpuUpdateFor(uid, std::move(info));
}
