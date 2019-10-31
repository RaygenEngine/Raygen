//#include "pch/pch.h"
//
//#include "world/nodes/vr/OVRNode.h"
//#include "system/Engine.h"
//#include "world/World.h"
//#include "world/NodeFactory.h"
//#include "core/MathAux.h"
//
//#include <ovr/OVR_CAPI.h>
//#include <ovr/Extras/OVR_Math.h>
//#include <ovr/Extras/OVR_StereoProjection.h>
//#include <glm/gtx/matrix_decompose.inl>
//
// void OVRNode::PrepareEyes()
//{
//	auto hmd = ovr_GetHmdDesc(m_session);
//
//	for (size_t i = 0; i < 2; i++) {
//		const auto f = hmd.DefaultEyeFov[i];
//
//		// TODO: this should be updated from the cameras
//		m_info.eyeFovs[i] = f;
//
//		m_eyes[i]->m_hFov = atan(f.RightTan) + atan(f.LeftTan);
//		m_eyes[i]->m_hFovOffset = atan(f.RightTan) - atan(f.LeftTan);
//
//		m_eyes[i]->m_vFov = atan(f.UpTan) + atan(f.DownTan);
//		m_eyes[i]->m_vFovOffset = atan(f.UpTan) - atan(f.DownTan);
//	}
//
//	const auto eyeSizeLeft = ovr_GetFovTextureSize(m_session, ovrEye_Left, hmd.DefaultEyeFov[ovrEye_Left], 1.0f);
//	const auto eyeSizeRight = ovr_GetFovTextureSize(m_session, ovrEye_Right, hmd.DefaultEyeFov[ovrEye_Right], 1.0f);
//
//	m_eyes[0]->m_viewportWidth = eyeSizeLeft.w;
//	m_eyes[0]->m_viewportHeight = eyeSizeLeft.h;
//
//	m_eyes[1]->m_viewportWidth = eyeSizeRight.w;
//	m_eyes[1]->m_viewportHeight = eyeSizeRight.h;
//
//	m_eyes[0]->RecalculateProjectionFov();
//	m_eyes[1]->RecalculateProjectionFov();
//
//	m_info.eyeViewports[0] = { 0, 0, m_eyes[0]->m_viewportWidth, m_eyes[0]->m_viewportHeight };
//	m_info.eyeViewports[1] = { 0, 0, m_eyes[1]->m_viewportWidth, m_eyes[1]->m_viewportHeight };
//}
//
// OVRNode::OVRNode()
//{
//	ovrInitParams initParams = { ovrInit_RequestVersion | ovrInit_FocusAware, OVR_MINOR_VERSION, NULL, 0, 0 };
//	auto res = ovr_Initialize(&initParams);
//
//	CLOG_ABORT(OVR_FAILURE(res), "Failed to initialize oculus");
//
//	ovrGraphicsLuid luid;
//	res = ovr_Create(&m_session, &luid);
//
//	CLOG_ABORT(OVR_FAILURE(res), "Failed to initialize oculus session");
//}
//
// OVRNode::~OVRNode()
//{
//	ovr_Destroy(m_session);
//	ovr_Shutdown();
//}
//
// void OVRNode::Update(float deltaTime)
//{
//	ovrSessionStatus sessionStatus;
//	ovr_GetSessionStatus(m_session, &sessionStatus);
//
//	m_info.visible = sessionStatus.IsVisible;
//
//	if (sessionStatus.ShouldRecenter) {
//		ovr_RecenterTrackingOrigin(m_session);
//	}
//
//	if (sessionStatus.IsVisible) {
//		// Ask the API for the times when this frame is expected to be displayed.
//		const double frameTiming = ovr_GetPredictedDisplayTime(m_session, m_info.frameIndex);
//
//		// Get the corresponding predicted pose state.
//		const auto trackingState = ovr_GetTrackingState(m_session, frameTiming, ovrTrue);
//		m_info.sensorSampleTime = trackingState.HeadPose.TimeInSeconds;
//
//
//		auto hmd = ovr_GetHmdDesc(m_session);
//		ovrEyeRenderDesc eyeRenderDesc[2];
//		eyeRenderDesc[0] = ovr_GetRenderDesc(m_session, ovrEye_Left, hmd.DefaultEyeFov[0]);
//		eyeRenderDesc[1] = ovr_GetRenderDesc(m_session, ovrEye_Right, hmd.DefaultEyeFov[1]);
//
//		// Starting with version 1.17, HmdToEyeOffset has been renamed to HmdToEyeTargetPose
//		// using the type ovrPosef which contains a Position and Orientation, effectively giving eye poses
//		// six degrees of freedom. This means that each eye’s render frustum can now be rotated away from
//		// the HMDs orientation, in addition to being translated by the SDK.Because of this, the eye frustums
//		// axes are no longer guaranteed to be parallel to each other or to the HMD’s orientation axes.
//		ovrPosef hmdToEyePose[2] = { eyeRenderDesc[0].HmdToEyePose, eyeRenderDesc[1].HmdToEyePose };
//
//		ovr_CalcEyePoses2(trackingState.HeadPose.ThePose, hmdToEyePose, m_info.eyePoses);
//
//		auto ori = trackingState.HeadPose.ThePose.Orientation;
//		SetNodeOrientationLCS(glm::quat(ori.w, ori.x, ori.y, ori.z));
//
//		auto pos = trackingState.HeadPose.ThePose.Position;
//		SetNodePositionLCS({ pos.x, pos.y, pos.z });
//
//		ori = hmdToEyePose[0].Orientation;
//		m_eyes[0]->SetNodeOrientationLCS(glm::quat(ori.w, ori.x, ori.y, ori.z));
//
//		pos = hmdToEyePose[0].Position;
//		m_eyes[0]->SetNodePositionLCS({ pos.x, pos.y, pos.z });
//
//		ori = hmdToEyePose[1].Orientation;
//		m_eyes[1]->SetNodeOrientationLCS(glm::quat(ori.w, ori.x, ori.y, ori.z));
//
//		pos = hmdToEyePose[1].Position;
//		m_eyes[1]->SetNodePositionLCS({ pos.x, pos.y, pos.z });
//
//		auto proj = glm::transpose(m_eyes[0]->GetProjectionMatrix());
//		m_info.proj = reinterpret_cast<OVR::Matrix4f&>(proj);
//	}
//}
//
// void OVRNode::DirtyUpdate(DirtyFlagset flags)
//{
//	Node::DirtyUpdate(flags);
//
//	if (flags[DF::Children]) {
//		m_eyes[0] = GetOrCreateChild<CameraNode>("leftEye");
//		m_eyes[1] = GetOrCreateChild<CameraNode>("rightEye");
//
//		PrepareEyes();
//	}
//}
