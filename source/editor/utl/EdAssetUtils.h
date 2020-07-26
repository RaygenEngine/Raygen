#pragma once
#include "assets/PodEntry.h"

namespace ed::asset {

void MaybeHoverTooltip(PodEntry* entry);
void MaybeHoverTooltip(BasePodHandle handle);


void MaybeHoverTooltipForced(bool showTooltip, PodEntry* entry);
void MaybeHoverTooltipForced(bool showTooltip, BasePodHandle handle);


void OpenForEdit(BasePodHandle handle);
void OpenForEdit(PodEntry* entry);

} // namespace ed::asset
