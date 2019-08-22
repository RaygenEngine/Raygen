#include "pch.h"

#include "OptixVRPathTracerHead.h"

#include "OptixVRPathTracerRenderer.h"

namespace Renderer::Optix
{
	OptixVRPathTracerHead::OptixVRPathTracerHead(OptixVRPathTracerRenderer* renderer, World::OculusHeadNode* node)
		: TypedNodeObserver<OptixVRPathTracerRenderer, World::OculusHeadNode>(renderer, node), fovHalfTan()
	{
	}

	void OptixVRPathTracerHead::UpdateFromNode()
	{
		// LEFT EYE

		auto* camL = m_node->GetEye(ET_LEFT);

		auto uL = camL->GetRight();
		auto vL = camL->GetUp();
		auto wL = camL->GetFront();

		auto eyeL = camL->GetWorldTranslation();

		vL *= fovHalfTan.y * camL->GetFocalLength();
		uL *= fovHalfTan.x * camL->GetFocalLength();

		GetRenderer()->GetOptixContext()["eye_L"]->setFloat(eyeL.x, eyeL.y, eyeL.z);
		GetRenderer()->GetOptixContext()["U_L"]->setFloat(uL.x, uL.y, uL.z);
		GetRenderer()->GetOptixContext()["V_L"]->setFloat(vL.x, vL.y, vL.z);
		GetRenderer()->GetOptixContext()["W_L"]->setFloat(wL.x, wL.y, wL.z);

		// TODO dont use lookat
		auto matL = glm::mat3(glm::inverse(glm::lookAt(eyeL, camL->GetLookAt(), camL->GetUp())));
		GetRenderer()->GetOptixContext()["normal_matrix_L"]->setMatrix3x3fv(false, &matL[0][0]);

		// RIGHT EYE

		auto* camR = m_node->GetEye(ET_RIGHT);

		auto uR = camR->GetRight();
		auto vR = camR->GetUp();
		auto wR = camR->GetFront();

		auto eyeR = camR->GetWorldTranslation();

		vR *= fovHalfTan.y * camR->GetFocalLength();
		uR *= fovHalfTan.x * camR->GetFocalLength();
		
		GetRenderer()->GetOptixContext()["eye_R"]->setFloat(eyeR.x, eyeR.y, eyeR.z);
		GetRenderer()->GetOptixContext()["U_R"]->setFloat(uR.x, uR.y, uR.z);
		GetRenderer()->GetOptixContext()["V_R"]->setFloat(vR.x, vR.y, vR.z);
		GetRenderer()->GetOptixContext()["W_R"]->setFloat(wR.x, wR.y, wR.z);

		// TODO dont use lookat
		auto matR = glm::mat3(glm::inverse(glm::lookAt(eyeR, camR->GetLookAt(), camR->GetUp())));
		GetRenderer()->GetOptixContext()["normal_matrix_R"]->setMatrix3x3fv(false, &matR[0][0]);
	}

	void OptixVRPathTracerHead::UpdateFromVisual(RenderTarget* target)
	{
		fovHalfTan = { target->glOculusSingleTextureSymmetricalFov.xfovHalfTan, target->glOculusSingleTextureSymmetricalFov.yfovHalfTan };

		UpdateFromNode();
	}
}
