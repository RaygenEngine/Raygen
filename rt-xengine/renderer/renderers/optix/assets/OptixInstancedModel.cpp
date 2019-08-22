#include "pch.h"
#include "OptixInstancedModel.h"


namespace Renderer::Optix
{
	OptixInstancedModel::OptixInstancedModel(OptixRendererBase* renderer)
		: OptixModel(renderer)
	{
	}

	bool OptixInstancedModel::Load(Assets::XModel* data, uint32 closestHitRayType, Assets::StringFile* closestHitProgramSource,
		std::string closestHitProgramName, uint32 anyHitRayType, Assets::StringFile* anyHitProgramSource,
		std::string anyHitProgramName, World::TriangleModelInstancedGeometryNode* nodeInstancer)
	{
		// if base model is not loaded 
		if (!OptixModel::Load(data, closestHitRayType, closestHitProgramSource, closestHitProgramName, anyHitRayType,
			anyHitProgramSource, anyHitProgramName))
			return false;

		// create instancing transforms
		for (auto& instance : nodeInstancer->GetInstanceGroup())
		{
			auto tran = GetOptixContext()->createTransform();

			// instancing
			tran->setChild(m_handle);

			auto mat = instance.second.worldMatrix;
			tran->setMatrix(true, reinterpret_cast<float*>(&mat), nullptr);
			m_transforms.push_back(tran);
		}

		return true;
	}
}
