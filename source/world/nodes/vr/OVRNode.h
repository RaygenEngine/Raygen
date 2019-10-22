#pragma once

#include "world/nodes/Node.h"
#include "world/nodes/camera/CameraNode.h"

#include <ovr/OVR_CAPI.h>
#include <Extras/OVR_Math.h>

class OVRNode : public Node {
	REFLECTED_NODE(OVRNode, Node) {}

	ovrSession m_session{};

	std::array<CameraNode*, 2> m_eyes{};

	struct RendererInfo {
		bool visible{ false };
		int64 frameIndex{ 0 };

		ovrPosef eyePoses[2]{};
		ovrFovPort eyeFovs[2]{};
		ovrRecti eyeViewports[2]{};

		OVR::Matrix4f proj{};
		double sensorSampleTime{};
	} m_info{};

	void PrepareEyes();

public:
	OVRNode();
	~OVRNode();

	void Update(float deltaTime) override;

	[[nodiscard]] ovrSession GetOVRSession() const { return m_session; }
	[[nodiscard]] CameraNode* GetEyeCamera(int32 index) { return m_eyes.at(index); }
	[[nodiscard]] RendererInfo& GetRendererInfo() { return m_info; }


	void IncrementFrameIndex() { ++m_info.frameIndex; }

	void DirtyUpdate(DirtyFlagset flags) override;
};
