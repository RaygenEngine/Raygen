#include "pch.h"

#include "OptixVRPathTracerGeometry.h"

namespace Renderer::Optix
{
	OptixVRPathTracerGeometry::OptixVRPathTracerGeometry(OptixVRPathTracerRenderer* renderer, World::TriangleModelGeometryNode* node)
		: TypedNodeObserver<OptixVRPathTracerRenderer, World::TriangleModelGeometryNode>(renderer, node)
	{
		auto vrPathTracerPtx = GetRenderer()->GetDiskAssetManager()->LoadFileAsset<Assets::StringFile>("VRPathTracer.ptx");

		optixModel = GetRenderer()->RequestOptixModel(node->GetModel(), RT_RADIANCE, vrPathTracerPtx.get(),
			"surface_shading", RT_SHADOW, vrPathTracerPtx.get(), "any_hit_shadow");

		transform = GetRenderer()->GetOptixContext()->createTransform();
		transform->setChild(optixModel->GetOptixHandle());

		OptixVRPathTracerGeometry::UpdateFromNode();
	}

	void OptixVRPathTracerGeometry::UpdateFromNode()
	{
		auto mat = m_node->GetWorldMatrix();
		transform->setMatrix(true, reinterpret_cast<float*>(&mat), nullptr);
	}
}
