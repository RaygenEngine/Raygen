#include "pch.h"
#include "NodeFactory.h"
#include "world/nodes/sky/SkyCubeNode.h"
#include "world/nodes/sky/SkyHDRNode.h"
#include "world/nodes/MetaNodeTranslation.h"
#include "World.h"

namespace World
{
	bool NodeFactory::LoadChildren(const tinyxml2::XMLElement* xmlData,
										  Node* parent)
	{
		for (auto* xmdChildElement = xmlData->FirstChildElement(); xmdChildElement != nullptr;
			 xmdChildElement = xmdChildElement->NextSiblingElement())
		{
			const std::string type = xmdChildElement->Name();
			Node* loaded = LoadNodeFromType(type, parent, xmdChildElement);

			RT_XENGINE_CLOG_ERROR(loaded, "Failed to load a child with type: {0}", type);
		}

		return parent->PostChildrenLoaded();
	}

	Node* NodeFactory::LoadNodeFromType(const std::string& type, Node* parent, const tinyxml2::XMLElement* xmdChildElement)
	{
		Node* custom = parent->LoadSpecificChild(xmdChildElement);
		if (custom)
		{
			return custom;
		}

		World* world = parent->GetWorld();
		if (type == "freeform_user")
		{
			return world->LoadNode<FreeformUserNode>(parent, xmdChildElement);
		}
		if (type == "camera")
		{
			return world->LoadNode<CameraNode>(parent, xmdChildElement);
		}
		if (type == "light")
		{
			return world->LoadNode<LightNode>(parent, xmdChildElement);
		}
		if (type == "trimesh_geometry")
		{
			return world->LoadNode<TriangleModelGeometryNode>(parent, xmdChildElement);
		}
		if (type == "baked_trimesh_geometry")
		{
			//node = LoadNode<BakedTriangleMeshGeometryNode>(this, xmdChildElement);
		}
		if (type == "transform")
		{
			return world->LoadNode<TransformNode>(parent, xmdChildElement);
		}
		if (type == "oculus_user")
		{
			return world->LoadNode<OculusUserNode>(parent, xmdChildElement);
		}
		if (type == "trimesh_geometry_instanced_matrix")
		{
			return LoadInstancingMatrixMetaNode(parent, xmdChildElement);
		}
		if (type == "trimesh_geometry_instanced")
		{
			return world->LoadNode<TriangleModelInstancedGeometryNode>(parent, xmdChildElement);
		}
		if (type == "sky_cube")
		{
			return world->LoadNode<SkyCubeNode>(parent, xmdChildElement);
		}
		if (type == "sky_hdr")
		{
			return world->LoadNode<SkyHDRNode>(parent, xmdChildElement);
		}

		RT_XENGINE_LOG_WARN("Node of type {0} missed all possible class types, defaulting class.", type);
		return world->LoadNode<Node>(parent, xmdChildElement);
	}
}