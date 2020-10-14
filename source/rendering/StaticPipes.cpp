#include "pch.h"
#include "StaticPipes.h"

#include "rendering/passes/lightblend/DirlightBlend.h"
#include "rendering/passes/lightblend/PointlightBlend.h"
#include "rendering/passes/lightblend/SpotlightBlend.h"
#include "rendering/passes/lightblend/ReflprobeBlend.h"

namespace vl {
void StaticPipes::InternalInitRegistered()
{
	StaticPipes::Init<DirlightBlend>();
	StaticPipes::Init<PointlightBlend>();
	StaticPipes::Init<SpotlightBlend>();
	StaticPipes::Init<ReflprobeBlend>();
}
} // namespace vl
