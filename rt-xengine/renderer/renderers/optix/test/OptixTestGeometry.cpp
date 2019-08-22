#include "pch.h"

#include "OptixTestGeometry.h"

namespace Renderer::Optix
{
	OptixTestGeometry::OptixTestGeometry(OptixTestRenderer* renderer, World::TriangleModelGeometryNode* node)
		: TypedNodeObserver<OptixTestRenderer, World::TriangleModelGeometryNode>(renderer, node)
	{
		auto testPtx = GetRenderer()->GetDiskAssetManager()->LoadFileAsset<Assets::StringFile>("test.ptx");

		optixModel = GetRenderer()->RequestOptixModel(node->GetModel(), RT_RESULT, testPtx.get(),
			"closest_hit", RT_RESULT, testPtx.get(), "any_hit");

		transform = GetRenderer()->GetOptixContext()->createTransform();
		transform->setChild(optixModel->GetOptixHandle());

		OptixTestGeometry::UpdateFromNode();
	}

	void OptixTestGeometry::UpdateFromNode()
	{
		auto mat = m_node->GetWorldMatrix();
		transform->setMatrix(true, reinterpret_cast<float*>(&mat), nullptr);
	}
}
