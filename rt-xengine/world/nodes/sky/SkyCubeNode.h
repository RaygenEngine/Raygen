#ifndef SKYCUBENODE_H
#define SKYCUBENODE_H

#include "world/nodes/Node.h"
#include "assets/texture/CubeMap.h"

namespace World
{
	enum MAPPING_METHOD
	{
		MM_CUBE_MAP = 0,
		MM_HDR
	};

	class SkyCubeNode : public Node
	{
		std::shared_ptr<Assets::CubeMap> m_cubeMap;

	public:
		SkyCubeNode(Node* parent);
		~SkyCubeNode() = default;

		bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;

		Assets::CubeMap* GetSkyMap() const { return m_cubeMap.get(); }

	protected:
		std::string ToString(bool verbose, uint depth) const override;
	};
}

#endif // SKYCUBENODE_H
