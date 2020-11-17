#include "StaticPipes.h"

#include "rendering/passes/bake/IrradianceMapCalculation.h"
#include "rendering/passes/bake/PrefilteredMapCalculation.h"
#include "rendering/passes/direct/DirlightBlend.h"
#include "rendering/passes/direct/PointlightBlend.h"
#include "rendering/passes/direct/SpotlightBlend.h"
#include "rendering/passes/gi/AoBlend.h"
#include "rendering/passes/gi/AoSubpass.h"
#include "rendering/passes/gi/IrradianceGridBlend.h"
#include "rendering/passes/gi/ReflprobeBlend.h"
#include "rendering/passes/unlit/UnlitBillboardPass.h"
#include "rendering/passes/unlit/UnlitVolumePass.h"

namespace vl {
void StaticPipes::InternalInitRegistered()
{
	StaticPipes::Init<DirlightBlend>();
	StaticPipes::Init<PointlightBlend>();
	StaticPipes::Init<SpotlightBlend>();
	StaticPipes::Init<ReflprobeBlend>();
	StaticPipes::Init<IrradianceGridBlend>();
	StaticPipes::Init<IrradianceMapCalculation>();
	StaticPipes::Init<PrefilteredMapCalculation>();
	StaticPipes::Init<UnlitVolumePass>();
	StaticPipes::Init<UnlitBillboardPass>();
	StaticPipes::Init<AoSubpass>();
	StaticPipes::Init<AoBlend>();
}
} // namespace vl
