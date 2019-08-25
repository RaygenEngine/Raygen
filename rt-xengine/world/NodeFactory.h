#ifndef NODEFACTORY_H
#define NODEFACTORY_H

#include "nodes/Node.h"
#include "tinyxml2/tinyxml2.h"

namespace World
{
	class NodeFactory
	{

	public:
		// Loads all nodes from the xml code as children to 'parentNode'.
		bool LoadChildren(const tinyxml2::XMLElement* xmlData, Node* parentNode);

	protected:

		Node* LoadChildSpecificNode(const std::string& type, Node* parentNode, const tinyxml2::XMLElement* xmlChild);

		virtual Node* LoadNodeFromType(const std::string& type, Node* parentNode, const tinyxml2::XMLElement* xmlChild);
	};
}

#endif // NODEFACTORY_H
