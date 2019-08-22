#ifndef ENGINEOBJECT_H
#define ENGINEOBJECT_H

#include "core/uuid/UUIDGenerator.h"
#include "EventListener.h" 
#include "input/Input.h"
#include "event/Event.h"
#include "system/Engine.h"

namespace Renderer
{
	class Renderer;
}

namespace World
{
	class World;
}

namespace Assets
{
	class DiskAssetManager;
}

namespace System
{
	class EngineObject : public EventListener
	{
		Engine* m_engine;
		EngineObject* m_parentObject;

		Core::UID m_id;

	protected:
		// Top objects (nullptr pObject) 
		explicit EngineObject(Engine* engine);
		explicit EngineObject(EngineObject* pObject);
		virtual ~EngineObject() = default;

	public:

		Engine* GetEngine() const { return m_engine; }
		Assets::DiskAssetManager* GetDiskAssetManager() const {
			return m_engine->GetDiskAssetManager();
		}

		Renderer::Renderer* GetRenderer() const { return m_engine->GetRenderer(); }
		World::World* GetWorld() const { return m_engine->GetWorld(); }
		const Input::Input& GetInput() const { return m_engine->GetInput(); }

		EngineObject* GetParentObject() const { return m_parentObject; }

		Core::UID GetObjectId() const { return m_id; }

		virtual void ToString(std::ostream& os) const { os << "asset-type: EngineObject, name: " << GetObjectId(); }

		// Part of engine object
		virtual void Update();
		virtual void WindowResize(int32 width, int32 height);
	};
}

#endif // ENGINEOBJECT_H
