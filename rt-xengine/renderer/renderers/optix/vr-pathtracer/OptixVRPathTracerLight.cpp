#include "pch.h"

#include "OptixVRPathTracerLight.h"

namespace Renderer::Optix
{
	OptixVRPathTracerLight::OptixVRPathTracerLight(OptixVRPathTracerRenderer* renderer, World::LightNode* node)
		: TypedNodeObserver<OptixVRPathTracerRenderer, World::LightNode>(renderer, node),
		light()
	{
		OptixVRPathTracerLight::UpdateFromNode();
	}

	void OptixVRPathTracerLight::UpdateFromNode()
	{
		light.casts_shadow = 1;
		light.color = m_node->GetColor();
		light.pos = m_node->GetWorldTranslation();

		GetRenderer()->GetOptixContext()["light"]->setUserData(sizeof(light), &light);
	}
}
