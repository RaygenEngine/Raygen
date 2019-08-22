#ifndef TRIANGLEMODELGEOMETRYNODE_H
#define TRIANGLEMODELGEOMETRYNODE_H

#include <memory>             
#include <string>  

#include "world/nodes/Node.h"
#include "assets/xasset/XModel.h"



namespace World
{
	class TriangleModelGeometryNode : public Node
	{
		std::shared_ptr<Assets::XModel> m_model;

	public:
		TriangleModelGeometryNode(Node* parent);
		~TriangleModelGeometryNode() = default;

		Assets::XModel* GetModel() const
		{
			auto* m = m_model.get();
			

			return m_model.get();
		}

		std::string ToString(bool verbose, uint depth) const override;

	public:
		bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;
	};
}

#endif // TRIANGLEMODELGEOMETRYNODE_H
