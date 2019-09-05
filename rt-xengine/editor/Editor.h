#pragma once

#include "system/EngineObject.h"
#include "platform/Window.h"

namespace World
{
	class Node;
}

namespace Editor {
	class Editor : public System::EngineObject
	{
	protected:
		bool m_updateWorld;
		World::Node* m_selectedNode;
	public:
		Editor(System::Engine* engine);

		virtual ~Editor() = default;

		void UpdateEditor();

		bool ShouldUpdateWorld() const 
		{
			return m_updateWorld;
		}

		void SaveScene(const std::string& filename);

	private:
		void Outliner();
		void PropertyEditor(World::Node* activeNode);
		
	};

}
