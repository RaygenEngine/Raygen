#ifndef TRANSFORMNODE_H
#define TRANSFORMNODE_H

#include "Node.h"

namespace World
{
	class TransformNode : public Node
	{
		// Loads from xml same as base class
	public:
		TransformNode(Node* parent);
		~TransformNode() = default;

		std::string ToString(bool verbose, uint depth) const override;
	};
}

#endif // TRANSFORMNODE_H
