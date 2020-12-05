#include "StaticPipes.h"

#include "rendering/pipes/AmbientPipe.h"
#include "rendering/pipes/ArealightsPipe.h"
#include "rendering/pipes/BillboardPipe.h"
#include "rendering/pipes/CubemapArrayConvolutionPipe.h"
#include "rendering/pipes/CubemapConvolutionPipe.h"
#include "rendering/pipes/DirlightPipe.h"
#include "rendering/pipes/IrragridPipe.h"
#include "rendering/pipes/MirrorPipe.h"
#include "rendering/pipes/PathtraceCubemapArrayPipe.h"
#include "rendering/pipes/PathtraceCubemapPipe.h"
#include "rendering/pipes/PointlightPipe.h"
#include "rendering/pipes/PrefilteredConvolutionPipe.h"
#include "rendering/pipes/ReflprobePipe.h"
#include "rendering/pipes/SelectionStencilPipe.h"
#include "rendering/pipes/SpotlightPipe.h"
#include "rendering/pipes/VolumePipe.h"


namespace vl {
void StaticPipes::InternalInitRegistered()
{
	StaticPipes::Init<AmbientPipe>();
	StaticPipes::Init<BillboardPipe>();
	StaticPipes::Init<CubemapArrayConvolutionPipe>();
	StaticPipes::Init<CubemapConvolutionPipe>();
	StaticPipes::Init<DirlightPipe>();
	StaticPipes::Init<IrragridPipe>();
	StaticPipes::Init<MirrorPipe>();
	StaticPipes::Init<PathtraceCubemapArrayPipe>();
	StaticPipes::Init<PathtraceCubemapPipe>();
	StaticPipes::Init<PointlightPipe>();
	StaticPipes::Init<PrefilteredConvolutionPipe>();
	StaticPipes::Init<ReflprobePipe>();
	StaticPipes::Init<SelectionStencilPipe>();
	StaticPipes::Init<SpotlightPipe>();
	// StaticPipes::Init<VolumePointsPipe>();
	StaticPipes::Init<VolumeLinesPipe>();
	StaticPipes::Init<VolumeTrianglesPipe>();
	StaticPipes::Init<ArealightsPipe>();
}
} // namespace vl
