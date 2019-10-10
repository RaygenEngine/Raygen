#include "pch/pch.h"

#include "world/NodeFactory.h"
#include "world/nodes/sky/SkyCubeNode.h"
#include "world/nodes/sky/SkyHDRNode.h"
#include "world/nodes/camera/WindowCameraNode.h"
#include "world/nodes/MetaNodeTranslation.h"
#include "world/World.h"
#include "world/nodes/user/freeform/FreeformUserNode.h"

bool NodeFactory::LoadChildrenXML(const tinyxml2::XMLElement* xmlData, Node* parent)
{
	for (auto* xmdChildElement = xmlData->FirstChildElement(); xmdChildElement != nullptr;
		 xmdChildElement = xmdChildElement->NextSiblingElement()) {
		const std::string type = xmdChildElement->Name();

		Node* created = LoadNodeFromType(type, parent);

		if (created) {
			bool loaded = created->LoadFromXML(xmdChildElement);
			CLOG_ERROR(!loaded, "Failed to load a node with type: {0}", type);
		}

		CLOG_ERROR(!created, "Failed to create a child with type: {0}", type);
	}

	return parent->PostChildrenLoaded();
}

Node* NodeFactory::LoadNodeFromType(const std::string& type, Node* parent)
{
	World* world = Engine::GetWorld();
	if (type == "freeform_user") {
		return world->CreateNode<FreeformUserNode>(parent);
	}
	if (type == "camera") {
		return world->CreateNode<CameraNode>(parent);
	}
	if (type == "window_camera") {
		return world->CreateNode<WindowCameraNode>(parent);
	}
	if (type == "punctual_light") {
		return world->CreateNode<PunctualLightNode>(parent);
	}
	if (type == "directional_light") {
		return world->CreateNode<DirectionalLightNode>(parent);
	}
	if (type == "spot_light") {
		return world->CreateNode<SpotLightNode>(parent);
	}
	if (type == "geometry") {
		return world->CreateNode<GeometryNode>(parent);
	}
	if (type == "transform") {
		return world->CreateNode<TransformNode>(parent);
	}
	if (type == "sky_cube") {
		return world->CreateNode<SkyCubeNode>(parent);
	}
	if (type == "sky_hdr") {
		return world->CreateNode<SkyHDRNode>(parent);
	}

	return nullptr;
}
