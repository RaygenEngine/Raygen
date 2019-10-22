#include "pch/pch.h"

#include "world/nodes/vr/OVRNode.h"
#include "system/Engine.h"
#include "world/World.h"
#include "world/NodeFactory.h"
#include "core/MathAux.h"

#include <ovr/OVR_CAPI.h>
#include <ovr/Extras/OVR_Math.h>
#include <gtx/matrix_decompose.inl>

void OVRNode::RecalculateEyesMatrices()
{
	// WIP
	for (auto i = 0; i < 2; ++i) {

		const auto ori = m_eyes[i].pose.Orientation;
		const auto pos = m_eyes[i].pose.Position;
		auto localMatrix
			= math::TransformMatrixFromTOS({ 1.f, 1.f, 1.f }, { ori.w, ori.x, ori.y, ori.z }, { pos.x, pos.y, pos.z });
		auto worldMatrix = GetWorldMatrix() * localMatrix;
		// PERF:
		glm::vec3 skew, scale, translation;
		glm::vec4 persp;
		glm::quat orientation;
		glm::decompose(worldMatrix, scale, orientation, translation, skew, persp);

		auto forward = orientation * glm::vec3(0.f, 0.f, -1.f);
		auto up = orientation * glm::vec3(0.f, 1.f, 0.f);

		m_eyes[i].viewMatrix = glm::lookAt(translation, forward, up);
		m_eyes[i].viewProjection = m_eyes[i].projectionMatrix * m_eyes[i].viewMatrix;

		m_eyes[i].worldPos = translation;
	}
}

OVRNode::OVRNode()
{
	ovrInitParams initParams = { ovrInit_RequestVersion | ovrInit_FocusAware, OVR_MINOR_VERSION, NULL, 0, 0 };
	ovrResult result = ovr_Initialize(&initParams);

	CLOG_ABORT(OVR_FAILURE(result), "Failed to initialize oculus");

	ovrGraphicsLuid luid;
	result = ovr_Create(&m_session, &luid);

	CLOG_ABORT(OVR_FAILURE(result), "Failed to initialize oculus session");


	auto hmd = ovr_GetHmdDesc(m_session);
	// note: ovrProjection_ClipRangeOpenGL because of world conventions
	auto proj0 = ovrMatrix4f_Projection(hmd.DefaultEyeFov[0], 0.01f, 10000.0f, ovrProjection_ClipRangeOpenGL);
	auto proj1 = ovrMatrix4f_Projection(hmd.DefaultEyeFov[1], 0.01f, 10000.0f, ovrProjection_ClipRangeOpenGL);
	m_eyes[0].projectionMatrix = reinterpret_cast<glm::mat4&>(proj0);
	m_eyes[1].projectionMatrix = reinterpret_cast<glm::mat4&>(proj1);

	m_eyes[0].fov = hmd.DefaultEyeFov[0];
	m_eyes[1].fov = hmd.DefaultEyeFov[1];

	// WIP
	const auto eyeSizeLeft = ovr_GetFovTextureSize(m_session, ovrEye_Left, hmd.DefaultEyeFov[ovrEye_Left], 1.0f);
	const auto eyeSizeRight = ovr_GetFovTextureSize(m_session, ovrEye_Right, hmd.DefaultEyeFov[ovrEye_Right], 1.0f);

	m_eyes[0].width = eyeSizeLeft.w + eyeSizeRight.w;
	m_eyes[0].height = glm::max(eyeSizeLeft.h, eyeSizeRight.h);
	m_eyes[1].width = eyeSizeLeft.w + eyeSizeRight.w;
	m_eyes[1].height = glm::max(eyeSizeLeft.h, eyeSizeRight.h);
}

OVRNode::~OVRNode()
{
	ovr_Destroy(m_session);
	ovr_Shutdown();
}

void OVRNode::Update(float deltaTime)
{
	if (!enabled)
		return;

	ovrSessionStatus sessionStatus;
	ovr_GetSessionStatus(m_session, &sessionStatus);

	visible = sessionStatus.IsVisible;

	if (sessionStatus.ShouldRecenter) {
		ovr_RecenterTrackingOrigin(m_session);
	}

	if (sessionStatus.IsVisible) {
		CLOG_ABORT(!OVR_SUCCESS(ovr_WaitToBeginFrame(m_session, m_frameIndex)), "Oculus Wait to begin frame failure");

		// Ask the API for the times when this frame is expected to be displayed.
		const double frameTiming = ovr_GetPredictedDisplayTime(m_session, m_frameIndex);

		// Get the corresponding predicted pose state.
		const auto trackingState = ovr_GetTrackingState(m_session, frameTiming, ovrTrue);


		ovrEyeRenderDesc eyeRenderDesc[2];
		eyeRenderDesc[0] = ovr_GetRenderDesc(m_session, ovrEye_Left, m_eyes[0].fov);
		eyeRenderDesc[1] = ovr_GetRenderDesc(m_session, ovrEye_Right, m_eyes[1].fov);

		// Starting with version 1.17, HmdToEyeOffset has been renamed to HmdToEyeTargetPose
		// using the type ovrPosef which contains a Position and Orientation, effectively giving eye poses
		// six degrees of freedom. This means that each eye’s render frustum can now be rotated away from
		// the HMDs orientation, in addition to being translated by the SDK.Because of this, the eye frustums
		// axes are no longer guaranteed to be parallel to each other or to the HMD’s orientation axes.
		ovrPosef hmdToEyePose[2] = { eyeRenderDesc[0].HmdToEyePose, eyeRenderDesc[1].HmdToEyePose };

		ovrPosef eyePoses[2];
		ovr_CalcEyePoses(trackingState.HeadPose.ThePose, hmdToEyePose, eyePoses);

		const auto ori = trackingState.HeadPose.ThePose.Orientation;
		SetLocalOrientation({ ori.w, ori.x, ori.y, ori.z });

		const auto pos = trackingState.HeadPose.ThePose.Position;
		SetLocalTranslation({ pos.x, pos.y, pos.z });

		m_eyes[0].pose = eyePoses[0];
		m_eyes[1].pose = eyePoses[1];
	}
}

void OVRNode::EditLayerDetails(ovrLayerEyeFovDepth& ld) const
{
	ovrTimewarpProjectionDesc posTimewarpProjectionDesc = {};

	OVR::Matrix4f proj = ovrMatrix4f_Projection(m_eyes[0].fov, 0.2f, 300.f, ovrProjection_None);
	posTimewarpProjectionDesc = ovrTimewarpProjectionDesc_FromProjection(proj, ovrProjection_None);

	ld.ProjectionDesc = posTimewarpProjectionDesc;
	ld.SensorSampleTime = ovr_GetTimeInSeconds(); // m_trackingState.HeadPose.TimeInSeconds;

	ld.Viewport[0] = { 0, 0, m_eyes[0].width / 2, m_eyes[0].height };
	ld.Fov[0] = m_eyes[0].fov;
	ld.RenderPose[0] = m_eyes[0].pose;
	ld.Viewport[1] = { m_eyes[1].width / 2, 0, m_eyes[1].width / 2, m_eyes[1].height };
	ld.Fov[1] = m_eyes[1].fov;
	ld.RenderPose[1] = m_eyes[1].pose;
}

void OVRNode::EditLayerDetails(ovrLayerEyeFov& ld) const
{
	// WIP
	ld.SensorSampleTime = ovr_GetTimeInSeconds();

	ld.Viewport[0] = { 0, 0, m_eyes[0].width / 2, m_eyes[0].height };
	ld.Fov[0] = m_eyes[0].fov;
	ld.RenderPose[0] = m_eyes[0].pose;
	ld.Viewport[1] = { m_eyes[1].width / 2, 0, m_eyes[1].width / 2, m_eyes[1].height };
	ld.Fov[1] = m_eyes[1].fov;
	ld.RenderPose[1] = m_eyes[1].pose;
}

void OVRNode::DirtyUpdate(DirtyFlagset flags)
{
	if (flags[DF::TRS]) {
		RecalculateEyesMatrices();
	}
}
