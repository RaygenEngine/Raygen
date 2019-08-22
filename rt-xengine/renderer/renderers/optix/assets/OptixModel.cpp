#include "pch.h"
#include "OptixModel.h"
#include "OptixMesh.h"
#include "renderer/renderers/optix/OptixUtil.h"

namespace Renderer::Optix
{
	OptixModel::OptixModel(OptixRendererBase* renderer)
		: OptixAsset(renderer)
	{
	}

	bool OptixModel::Load(Assets::XModel* data, uint32 closestHitRayType, Assets::StringFile* closestHitProgramSource,
		std::string closestHitProgramName, uint32 anyHitRayType, Assets::StringFile* anyHitProgramSource,
		std::string anyHitProgramName)
	{
		SetIdentificationFromAssociatedDiskAssetIdentification(data->GetLabel());

		// Create Geometry Group
		m_handle = GetOptixContext()->createGeometryGroup();

		// xmodel consists of one or more triangle meshes (xmeshes)
		auto acceleration = GetOptixContext()->createAcceleration(ACCEL(AS_TRBVH));

		// if this geom is static, then simply refiting is enough for the bvh
		if (data->GetType() == Assets::GT_STATIC)
			acceleration->setProperty(ACCEL_PROP(ASP_REFIT), "1");

		m_handle->setAcceleration(acceleration);

		for (auto* mesh : data->GetMeshes())
		{
			const auto optixMesh = GetRenderer()->RequestOptixMesh(mesh,
				closestHitRayType, closestHitProgramSource, closestHitProgramName,
				anyHitRayType, anyHitProgramSource, anyHitProgramName);

			m_meshes.push_back(optixMesh);

			// create a geom instance for each geometry group
			auto optixMeshGroups = optixMesh->GetOptixMeshGeometryGroups();

			for (const auto& omg : optixMeshGroups)
			{
				const optix::GeometryInstance gi = GetOptixContext()->createGeometryInstance(omg.group, omg.material->GetOptixHandle());

				m_handle->addChild(gi);
			}
		}

		return true;
	}
}
