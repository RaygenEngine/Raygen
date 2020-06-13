#include "pch.h"
#include "PostprocessPass.h"

namespace {
struct PushConstant {
	glm::mat4 mvp;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
PostprocessPass::PostprocessPass() {}

void PostprocessPass::RecordCmd() {}
} // namespace vl
