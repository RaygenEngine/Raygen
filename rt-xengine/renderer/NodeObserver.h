#pragma once

#include "world/nodes/Node.h"

namespace Renderer
{
	class NodeObserver
	{
		World::Node* m_nodeBase;

	public:

		NodeObserver(World::Node* node);
		virtual ~NodeObserver() = default;

		virtual void UpdateFromNode();

		World::Node* GetNodeBase() const { return m_nodeBase; }
	};

	// saves the time of writing a base observer for each new and existing node type
	template <typename RendererType, typename NodeType>
	class TypedNodeObserver : public NodeObserver
	{
	public:
		using NT = NodeType;

	protected:
		NodeType* m_node;
		RendererType* m_renderer;

	public:
		TypedNodeObserver(RendererType* renderer, NodeType* node)
			: NodeObserver(node),
			  m_node(node), 
		      m_renderer(renderer)
		{
		}

		virtual ~TypedNodeObserver() = default;

		const NodeType* GetNode() const { return m_node; }
		RendererType* GetRenderer() const { return m_renderer; }
	};
}
