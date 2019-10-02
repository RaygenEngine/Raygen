#include "pch.h"

#include "world/NodeFactory.h"
#include "world/nodes/sky/SkyCubeNode.h"
#include "world/nodes/sky/SkyHDRNode.h"
#include "world/nodes/MetaNodeTranslation.h"
#include "world/World.h"
#include "world/nodes/user/freeform/FreeformUserNode.h"

bool NodeFactory::LoadChildren(const tinyxml2::XMLElement* xmlData,
										Node* parent)
{
	for (auto* xmdChildElement = xmlData->FirstChildElement(); xmdChildElement != nullptr;
			xmdChildElement = xmdChildElement->NextSiblingElement())
	{
		const std::string type = xmdChildElement->Name();

		Node* created = LoadChildSpecificNode(type, parent, xmdChildElement);

		if (!created)
		{
			created = LoadNodeFromType(type, parent, xmdChildElement);
		}

		CLOG_ERROR(!created, "Failed to load a child with type: {0}", type);
	}

	return parent->PostChildrenLoaded();
}

Node* NodeFactory::LoadChildSpecificNode(const std::string& type, Node* parentNode, const tinyxml2::XMLElement* xmdChildElement)
{
	Node* custom = parentNode->LoadSpecificChild(xmdChildElement);
	return custom;
}

Node* NodeFactory::LoadNodeFromType(const std::string& type, Node* parent, const tinyxml2::XMLElement* xmdChildElement)
{
	World* world = Engine::GetWorld();
	if (type == "freeform_user")
	{
		return world->LoadNode<FreeformUserNode>(parent, xmdChildElement);
	}
	if (type == "camera")
	{
		return world->LoadNode<CameraNode>(parent, xmdChildElement);
	}
	if (type == "punctual_light")
	{
		return world->LoadNode<PunctualLightNode>(parent, xmdChildElement);
	}
	if (type == "directional_light")
	{
		return world->LoadNode<DirectionalLightNode>(parent, xmdChildElement);
	}
	if (type == "spot_light")
	{
		return world->LoadNode<SpotLightNode>(parent, xmdChildElement);
	}
	if (type == "geometry")
	{
		return world->LoadNode<GeometryNode>(parent, xmdChildElement);
	}
	if (type == "transform")
	{
		return world->LoadNode<TransformNode>(parent, xmdChildElement);
	}
	if (type == "oculus_user")
	{
		//return world->LoadNode<OculusUserNode>(parent, xmdChildElement);
	}
	if (type == "geometry_instanced_matrix")
	{
		return LoadInstancingMatrixMetaNode(parent, xmdChildElement);
	}
	if (type == "geometry_instanced")
	{
		return world->LoadNode<InstancedGeometryNode>(parent, xmdChildElement);
	}
	if (type == "sky_cube")
	{
		return world->LoadNode<SkyCubeNode>(parent, xmdChildElement);
	}
	if (type == "sky_hdr")
	{
		return world->LoadNode<SkyHDRNode>(parent, xmdChildElement);
	}

	return nullptr;
}

