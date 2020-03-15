#pragma once
#include "asset/importers/PodImporter.h"


struct ShaderImporter : public PodImporter<ImagePod> {
	ShaderImporter(std::string_view name)
		: PodImporter<ImagePod>(
			{
				"vert",
				"tesc",
				"tese",
				"geom",
				"frag",
				"comp",
				"rgen",
				"rint",
				"rahit",
				"rchit",
				"rmiss",
				"rcall",
				"mesh",
				"task",
			},
			name)
	{
	}

	BasePodHandle Import(const fs::path& path) override;
};

/*
vert
tesc
tese
geom
frag
comp

rgen
rint
rahit
rchit
rmiss
rcall
mesh
task

//

Vertex
TessellationControl
TessellationEvaluation
Geometry
Fragment
Compute

RaygenNV
AnyHitNV
ClosestHitNV
MissNV
IntersectionNV
CallableNV
TaskNV
MeshNV

struct ShaderImporter : public PodImporter<ImagePod> {
	ShaderImporter(std::string_view name)
		: ShaderImporter(, name)
	{
	}

	BasePodHandle Import(const fs::path& path) override;
};

*/
