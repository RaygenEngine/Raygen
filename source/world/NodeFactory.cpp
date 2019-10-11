#include "pch/pch.h"

#include "world/NodeFactory.h"
#include "world/nodes/sky/SkyCubeNode.h"
#include "world/nodes/sky/SkyHDRNode.h"
#include "world/nodes/camera/WindowCameraNode.h"
#include "world/nodes/MetaNodeTranslation.h"
#include "world/World.h"
#include "world/nodes/user/freeform/FreeformUserNode.h"

void NodeFactory::RegisterNodes()
{
	RegisterNodeList<CameraNode, WindowCameraNode, GeometryNode, DirectionalLightNode, PunctualLightNode, SpotLightNode,
		SkyCubeNode, SkyHDRNode, FreeformUserNode, TransformNode>();
}

Node* NodeFactory::NewNodeFromType(const std::string& type)
{
	auto it = m_nodeEntries.find(std::string(FilterNodeName(type)));
	if (it != m_nodeEntries.end()) {
		return it->second.newInstance();
	}

	LOG_ASSERT("Failed to find node registration for: {}", type);
	return nullptr;
}

void NodeFactory::LoadChildrenXML(const tinyxml2::XMLElement* xmlData, Node* parent)
{
	for (auto* xmdChildElement = xmlData->FirstChildElement(); xmdChildElement != nullptr;
		 xmdChildElement = xmdChildElement->NextSiblingElement()) {
		const std::string type = xmdChildElement->Name();

		Node* created = NewNodeFromType(type);
		Engine::GetWorld()->RegisterNode(created, parent);

		if (created) {
			bool loaded = created->LoadFromXML(xmdChildElement);
			CLOG_ERROR(!loaded, "Failed to load a node with type: {0}", type);
		}

		CLOG_ERROR(!created, "Failed to create a child with type: {0}", type);
	}
}
