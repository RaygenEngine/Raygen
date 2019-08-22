#ifndef ENGINEMODULE_H
#define ENGINEMODULE_H

#include "core/uuid/UUIDGenerator.h"
#include "EventListener.h" 
#include "input/Input.h"
#include "event/Event.h"

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
	class Engine;

	class EngineObject : public EventListener
	{
		Engine* m_engine;
		EngineObject* m_parentModule;

		Core::UID m_id;

	protected:

		explicit EngineObject(Engine* pModule);
		explicit EngineObject(EngineObject* pModule);
		virtual ~EngineObject() = default;

	public:

		Engine* GetEngine() const { return m_engine; }
		Assets::DiskAssetManager* GetDiskAssetManager() const;
		Renderer::Renderer* GetRenderer() const;
		World::World* GetWorld() const;
		const Input::Input& GetInput() const;
		//const Input::Mouse& GetMouse() const { return m_engine->GetInput().GetMouse(); }
		//const Input::Keyboard& GetKeyboard() const { return m_engine->GetInput().GetKeyboard(); }
		//const Input::GameController& GetGameController() const { return m_engine->GetInput().GetGameController(); }
		//const Input::OculusHMD& GetOculusHMD() const { return m_engine->GetInput().GetOculusHMD(); }

		EngineObject* GetParentModule() const { return m_parentModule; }

		void SetEventCallback(std::function<void(Event::Event&)> fn);
		void ProcessEvent(Event::Event& e);

		Core::UID GetModuleId() const { return m_id; }

		virtual void ToString(std::ostream& os) const { os << "asset-type: EngineObject, name: " << GetModuleId(); }
	};
}

#endif // ENGINEMODULE_H
