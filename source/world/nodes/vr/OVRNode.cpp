#include "pch/pch.h"

#include "world/nodes/vr/OVRNode.h"
#include "system/Engine.h"
#include "world/World.h"
#include "world/NodeFactory.h"

#include <ovr/OVR_CAPI.h>
#include <ovr/Extras/OVR_Math.h>

OVRNode::OVRNode()
{
	ovrResult result = ovr_Initialize(nullptr);
	if (OVR_FAILURE(result)) {
		return;
	}

	ovrGraphicsLuid luid;
	result = ovr_Create(&m_session, &luid);
	if (OVR_FAILURE(result)) {
		ovr_Shutdown();
		return;
	}

	m_hmd = ovr_GetHmdDesc(m_session);
	// WIP
	// const auto eyeSizeLeft = ovr_GetFovTextureSize(m_session, ovrEye_Left, m_hmd.DefaultEyeFov[ovrEye_Left], 1.0f);
	// const auto eyeSizeRight = ovr_GetFovTextureSize(m_session, ovrEye_Right,
	// m_hmd.DefaultEyeFov[ovrEye_Right], 1.0f);

	// OVR::Sizei bufferSize;
	// bufferSize.w = eyeSizeLeft.w + eyeSizeRight.w;
	// bufferSize.h = glm::max(eyeSizeLeft.h, eyeSizeRight.h);

	// WIP
	auto world = Engine::GetWorld();

	for (auto i = 0; i < 2; i++) {
		m_eyes[i] = world->GetNodeFactory()->NewNode<CameraNode>();
		// WIP set camera fovs and viewport width/height
		// and set local translation offset based on IPD
		// and name

		world->RegisterNode(m_eyes[i], this);
	}
}

OVRNode::~OVRNode()
{
	ovr_Destroy(m_session);
	ovr_Shutdown();
}

void OVRNode::Update(float deltaTime)
{
	ovrSessionStatus sessionStatus;
	ovr_GetSessionStatus(m_session, &sessionStatus);

	if (sessionStatus.ShouldRecenter) {
		ovr_RecenterTrackingOrigin(m_session);
	}

	if (sessionStatus.IsVisible) {
		ovr_WaitToBeginFrame(m_session, m_frameIndex);

		// Ask the API for the times when this frame is expected to be displayed.
		const double frameTiming = ovr_GetPredictedDisplayTime(m_session, m_frameIndex);

		// Get the corresponding predicted pose state.
		const auto trackingState = ovr_GetTrackingState(m_session, frameTiming, ovrTrue);


		ovrEyeRenderDesc eyeRenderDesc[2];
		eyeRenderDesc[0] = ovr_GetRenderDesc(m_session, ovrEye_Left, m_hmd.DefaultEyeFov[0]);
		eyeRenderDesc[1] = ovr_GetRenderDesc(m_session, ovrEye_Right, m_hmd.DefaultEyeFov[1]);

		// Starting with version 1.17, HmdToEyeOffset has been renamed to HmdToEyeTargetPose
		// using the type ovrPosef which contains a Position and Orientation, effectively giving eye poses
		// six degrees of freedom. This means that each eye’s render frustum can now be rotated away from
		// the HMDs orientation, in addition to being translated by the SDK.Because of this, the eye frustums
		// axes are no longer guaranteed to be parallel to each other or to the HMD’s orientation axes.
		ovrPosef hmdToEyePose[2] = { eyeRenderDesc[0].HmdToEyePose, eyeRenderDesc[1].HmdToEyePose };

		ovr_CalcEyePoses(trackingState.HeadPose.ThePose, hmdToEyePose, m_eyePoses);

		const auto ori = trackingState.HeadPose.ThePose.Orientation;
		SetLocalOrientation({ ori.w, ori.x, ori.y, ori.z });

		const auto pos = trackingState.HeadPose.ThePose.Position;
		SetLocalTranslation({ pos.x, pos.y, pos.z });
	}
}

void OVRNode::EditLayerDetails(ovrLayerEyeFovDepth& ld)
{
	//ovrTimewarpProjectionDesc posTimewarpProjectionDesc = {};

	//OVR::Matrix4f proj = ovrMatrix4f_Projection({ m_eyeFovs[ET_RIGHT].UpTan, m_eyeFovs[ET_RIGHT].DownTan,
	//												m_eyeFovs[ET_RIGHT].LeftTan, m_eyeFovs[ET_RIGHT].RightTan },
	//	0.2f, 300.f, ovrProjection_None);
	//posTimewarpProjectionDesc = ovrTimewarpProjectionDesc_FromProjection(proj, ovrProjection_None);

	// ld.ProjectionDesc = posTimewarpProjectionDesc;
	ld.SensorSampleTime = 1; // m_trackingState.HeadPose.TimeInSeconds;

	ld.Viewport[0] = { 0, 0, m_eyes[0]->GetWidth() / 2, m_eyes[0]->GetHeight() };
	ld.Fov[0] = m_hmd.DefaultEyeFov[0];
	ld.RenderPose[0] = m_eyePoses[0];
	ld.Viewport[1] = { m_eyes[1]->GetWidth() / 2, 0, m_eyes[1]->GetWidth() / 2, m_eyes[1]->GetHeight() };
	ld.Fov[1] = m_hmd.DefaultEyeFov[1];
	ld.RenderPose[1] = m_eyePoses[1];
}
