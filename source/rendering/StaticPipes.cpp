#include "StaticPipes.h"

#include "rendering/passes/direct/DirlightBlend.h"
#include "rendering/passes/direct/PointlightBlend.h"
#include "rendering/passes/direct/SpotlightBlend.h"
#include "rendering/passes/gi/AmbientBlend.h"
#include "rendering/passes/gi/IrragridBlend.h"
#include "rendering/passes/gi/ReflprobeBlend.h"
#include "rendering/passes/unlit/UnlitBillboardPass.h"
#include "rendering/passes/unlit/UnlitSelectionStencilPass.h"
#include "rendering/passes/unlit/UnlitVolumePass.h"

namespace vl {
void StaticPipes::InternalInitRegistered()
{
	StaticPipes::Init<DirlightBlend>();
	StaticPipes::Init<PointlightBlend>();
	StaticPipes::Init<SpotlightBlend>();
	StaticPipes::Init<ReflprobeBlend>();
	StaticPipes::Init<IrragridBlend>();
	StaticPipes::Init<UnlitVolumePass>();
	StaticPipes::Init<UnlitBillboardPass>();
	StaticPipes::Init<AmbientBlend>();
	StaticPipes::Init<UnlitSelectionStencilPass>();
}
} // namespace vl
