#pragma once

#include "asset/util/ParsingAux.h"
#include "world/World.h"
#include "world/nodes/geometry/InstancedGeometryNode.h"
#include "system/Engine.h"

//inline InstancedGeometryNode* LoadInstancingMatrixMetaNode(Node* parent, const tinyxml2::XMLElement* xmlElement)
//{
//	LOG_INFO("Loading instancing matrix meta node");
//
//	uint32 matWidth = 1u;
//	xmlElement->QueryUnsignedAttribute("matrix_width", &matWidth);
//
//	uint32 matHeight = 1u;
//	xmlElement->QueryUnsignedAttribute("matrix_height", &matHeight);
//
//	auto interval = 5.f;
//	ParsingAux::ReadFloatsAttribute<float>(xmlElement, "interval", interval);
//
//	glm::vec3 translation{ 0.f, 0.f, 0.f };
//	ParsingAux::ReadFloatsAttribute(xmlElement, "translation", translation);
//	glm::vec3 eulerPYR{ 0.f, 0.f, 0.f };
//	ParsingAux::ReadFloatsAttribute(xmlElement, "euler_pyr", eulerPYR);
//	auto orientation = glm::quat(glm::radians(eulerPYR));
//	glm::vec3 scale{ 1.f, 1.f, 1.f };
//	ParsingAux::ReadFloatsAttribute(xmlElement, "scale", scale);
//
//	std::string name = {};
//	ParsingAux::ReadStringAttribute(xmlElement, "name", name);
//	std::string file = {};
//	ParsingAux::ReadStringAttribute(xmlElement, "file", file);
//
//	// create trimesh_geometry_instanced node
//	tinyxml2::XMLElement* triGeomInstancedNode = const_cast<tinyxml2::XMLElement*>(xmlElement)->GetDocument()->NewElement("trimesh_geometry_instanced");
//	triGeomInstancedNode->SetAttribute("name", name.c_str());
//	triGeomInstancedNode->SetAttribute("file", file.c_str());
//	
//	// create instances
//	for (auto i = 0u; i < matHeight; ++i)
//	{
//		for (auto j = 0u; j < matWidth; ++j)
//		{
//			tinyxml2::XMLElement* instance = const_cast<tinyxml2::XMLElement*>(xmlElement)->GetDocument()->NewElement("instance");
//
//			const auto instanceTranslation = translation + interval * glm::vec3(j, i, 0);
//
//			instance->SetAttribute("name", (name + "$instance$" + std::to_string(j + i * matHeight)).c_str());
//			instance->SetAttribute("translation", (std::to_string(instanceTranslation.x) + ", " + std::to_string(instanceTranslation.y) + ", " + std::to_string(instanceTranslation.z)).c_str());
//			instance->SetAttribute("euler_pyr", (std::to_string(eulerPYR.x) + ", " + std::to_string(eulerPYR.y) + ", " + std::to_string(eulerPYR.z)).c_str());
//			instance->SetAttribute("scale", (std::to_string(scale.x) + ", " + std::to_string(scale.y) + ", " + std::to_string(scale.z)).c_str());
//
//				
//			triGeomInstancedNode->InsertEndChild(instance);
//		}
//	}
//
//	return Engine::GetWorld()->LoadNode<InstancedGeometryNode>(parent, triGeomInstancedNode);
//}
