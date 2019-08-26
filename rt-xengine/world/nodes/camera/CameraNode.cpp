#include "pch.h"

#include "world/nodes/camera/CameraNode.h"
#include "assets/other/xml/ParsingAux.h"

namespace World
{
	CameraNode::CameraNode(Node* parent)
		: Node(parent),
		  m_focalLength(1.f),
		  m_vFov(45.f), 
		  m_hFov(45.f),
		  m_near(0.2f),
		  m_far(1000.0f),
		  m_viewMatrix(),
		  m_projectionMatrix(),
		  m_viewProjectionMatrix()
	{
	}

	std::string CameraNode::ToString(bool verbose, uint depth) const
	{
		return std::string("    ") * depth + "|--camera " + Node::ToString(verbose, depth);
	}

	void CameraNode::GetTracingVariables(glm::vec3& u, glm::vec3& v, glm::vec3& w)
	{
		const auto tanVHalfFov = tan(glm::radians(m_vFov)*0.5f);
		const auto tanHHalfFov = tan(glm::radians(m_hFov)*0.5f);

		u = GetRight();
		v = GetUp();
		// TODO: check how is this affected by focal length, is it?
		w = GetFront();

		v *= tanVHalfFov * m_focalLength;
		u *= tanHHalfFov * m_focalLength;
	}

	void CameraNode::Update()
	{
		m_viewMatrix = glm::lookAt(GetWorldTranslation(), GetLookAt(), GetUp());
		m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
	}

	void CameraNode::WindowResize(int32 width, int32 height)
	{
		auto ar = static_cast<float>(width) / static_cast<float>(height);

		m_projectionMatrix = glm::perspective(glm::radians(m_vFov), ar, m_near, m_far);
		m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;

		m_hFov = glm::degrees(2 * atan(ar * tan(glm::radians(m_vFov) * 0.5f)));
	}

	bool CameraNode::LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData)
	{
		Node::LoadAttributesFromXML(xmlData);

		Assets::ReadFloatsAttribute(xmlData, "fov", m_vFov);
		Assets::ReadFloatsAttribute(xmlData, "focal_length", m_focalLength);
		Assets::ReadFloatsAttribute(xmlData, "near", m_near);
		Assets::ReadFloatsAttribute(xmlData, "far", m_far);

		m_viewMatrix = glm::lookAt(GetWorldTranslation(), GetLookAt(), GetUp());

		return true;
	}
}
