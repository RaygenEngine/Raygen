#include "pch.h"
#include "SpotlightPass.h"

#include "assets/Assets.h"
#include "engine/Engine.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/Device.h"
#include "rendering/Renderer.h"
#include "rendering/scene/Scene.h"
#include "rendering/Layouts.h"

namespace vl {
void SpotlightPass::MakePipeline() {}

void SpotlightPass::RecordCmd(vk::CommandBuffer* cmdBuffer, const vk::Viewport& viewport, const vk::Rect2D& scissor) {}
} // namespace vl
