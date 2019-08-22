#ifndef OPTIXMESH_H
#define OPTIXMESH_H

#include "OptixMaterial.h"

namespace Renderer::Optix
{
	struct OptixMeshGeometryGroup
	{
		optix::GeometryTriangles group;
		std::shared_ptr<OptixMaterial> material;
	};
	// Could be intergrated in OptixModel, however this is cleaner and more extendible
	// A mesh can contain multiple groups (based on materials)
	class OptixMesh : public OptixAsset
	{
		std::vector<OptixMeshGeometryGroup> m_groups;

		std::shared_ptr<OptixProgram> m_triGeomVarProgram;
		std::shared_ptr<Assets::StringFile> m_triGeomVarPtx;

	public:
		OptixMesh(OptixRendererBase* renderer);
		~OptixMesh() = default;

		bool Load(Assets::XMesh* data, uint32 closestHitRayType, Assets::StringFile* closestHitProgramSource, std::string closestHitProgramName, 
			uint32 anyHitRayType, Assets::StringFile* anyHitProgramSource, std::string anyHitProgramName);

		const std::vector<OptixMeshGeometryGroup>& GetOptixMeshGeometryGroups() const { return m_groups; }

		void ToString(std::ostream& os) const override { os << "asset-type: OptixMesh, name: " << m_associatedDescription; }
	};

}

#endif // OPTIXMESH_H
