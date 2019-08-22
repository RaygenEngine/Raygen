#ifndef METANODETRANSLATION_H
#define METANODETRANSLATION_H

#include "world/World.h"

namespace World
{
	inline void LoadInstancingMatrixMetaNode(Node* parent, const tinyxml2::XMLElement* xmlElement)
	{
		RT_XENGINE_LOG_INFO("Loading instancing matrix meta node");

		uint32 matWidth = 1u;
		xmlElement->QueryUnsignedAttribute("matrix_width", &matWidth);

		uint32 matHeight = 1u;
		xmlElement->QueryUnsignedAttribute("matrix_height", &matHeight);

		auto interval = 5.f;
		Assets::ReadFloatsAttribute<float>(xmlElement, "interval", interval);

		glm::vec3 translation{ 0.f, 0.f, 0.f };
		Assets::ReadFloatsAttribute(xmlElement, "translation", translation);
		glm::vec3 eulerPYR{ 0.f, 0.f, 0.f };
		Assets::ReadFloatsAttribute(xmlElement, "euler_pyr", eulerPYR);
		auto orientation = glm::quat(glm::radians(eulerPYR));
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		Assets::ReadFloatsAttribute(xmlElement, "scale", scale);

		std::string name = {};
		Assets::ReadStringAttribute(xmlElement, "name", name);
		std::string file = {};
		Assets::ReadStringAttribute(xmlElement, "file", file);

		// create trimesh_geometry_instanced node
		tinyxml2::XMLElement* triGeomInstancedNode = const_cast<tinyxml2::XMLElement*>(xmlElement)->GetDocument()->NewElement("trimesh_geometry_instanced");
		triGeomInstancedNode->SetAttribute("name", name.c_str());
		triGeomInstancedNode->SetAttribute("file", file.c_str());

		// create instances
		for (auto i = 0u; i < matHeight; ++i)
		{
			for (auto j = 0u; j < matWidth; ++j)
			{
				tinyxml2::XMLElement* instance = const_cast<tinyxml2::XMLElement*>(xmlElement)->GetDocument()->NewElement("instance");

				const auto instanceTranslation = translation + interval * glm::vec3(j, i, 0);

				instance->SetAttribute("name", (name + "$instance$" + std::to_string(j + i * matHeight)).c_str());
				instance->SetAttribute("translation", (std::to_string(instanceTranslation.x) + ", " + std::to_string(instanceTranslation.y) + ", " + std::to_string(instanceTranslation.z)).c_str());
				instance->SetAttribute("euler_pyr", (std::to_string(eulerPYR.x) + ", " + std::to_string(eulerPYR.y) + ", " + std::to_string(eulerPYR.z)).c_str());
				instance->SetAttribute("scale", (std::to_string(scale.x) + ", " + std::to_string(scale.y) + ", " + std::to_string(scale.z)).c_str());

				
				triGeomInstancedNode->InsertEndChild(instance);
			}
		}

		parent->GetWorld()->LoadNode<TriangleModelInstancedGeometryNode>(parent, triGeomInstancedNode);
	}
}

#endif // METANODETRANSLATION_H