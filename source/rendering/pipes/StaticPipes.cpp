#include "StaticPipes.h"

#include "rendering/pipes/AmbientPipe.h"
#include "rendering/pipes/ArealightsPipe.h"
#include "rendering/pipes/BillboardPipe.h"
#include "rendering/pipes/CubemapConvolutionPipe.h"
#include "rendering/pipes/CubemapPrefilterPipe.h"
#include "rendering/pipes/DirlightPipe.h"
#include "rendering/pipes/IrragridPipe.h"
#include "rendering/pipes/MirrorPipe.h"
#include "rendering/pipes/NaivePathtracePipe.h"
#include "rendering/pipes/PointlightPipe.h"
#include "rendering/pipes/ReflprobePipe.h"
#include "rendering/pipes/SelectionStencilPipe.h"
#include "rendering/pipes/SpotlightPipe.h"
#include "rendering/pipes/StochasticPathtracePipe.h"
#include "rendering/pipes/VolumePipe.h"
#include "rendering/pipes/AccumulationPipe.h"
#include "rendering/pipes/SvgfMomentsPipe.h"
#include "rendering/pipes/SvgfAtrousPipe.h"
#include "rendering/pipes/SvgfModulatePipe.h"


namespace vl {
void StaticPipes::InternalInitRegistered()
{
	StaticPipes::Init<AmbientPipe>();
	StaticPipes::Init<BillboardPipe>();
	StaticPipes::Init<CubemapConvolutionPipe>();
	StaticPipes::Init<DirlightPipe>();
	StaticPipes::Init<IrragridPipe>();
	StaticPipes::Init<MirrorPipe>();
	StaticPipes::Init<PointlightPipe>();
	StaticPipes::Init<CubemapPrefilterPipe>();
	StaticPipes::Init<ReflprobePipe>();
	StaticPipes::Init<SelectionStencilPipe>();
	StaticPipes::Init<SpotlightPipe>();
	StaticPipes::Init<VolumeLinesPipe>();
	StaticPipes::Init<VolumeTrianglesPipe>();
	StaticPipes::Init<ArealightsPipe>();
	StaticPipes::Init<StochasticPathtracePipe>();
	StaticPipes::Init<NaivePathtracePipe>();
	StaticPipes::Init<AccumulationPipe>();
	StaticPipes::Init<SvgfMomentsPipe>();
	StaticPipes::Init<SvgfAtrousPipe>();
	StaticPipes::Init<SvgfModulatePipe>();
}
} // namespace vl
