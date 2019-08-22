#include "pch.h"
#include "EngineObject.h"

#include "Engine.h"

namespace System
{
	EngineObject::EngineObject(Engine* pModule)
		: m_engine(pModule), 
		  m_parentModule(pModule), 
	      m_id(0)
	{
	}

	EngineObject::EngineObject(EngineObject* pModule)
		: m_engine(pModule->GetEngine()), m_parentModule(pModule),
		m_id(Core::UUIDGenerator::GenerateUUID())
	{
	}

	Assets::DiskAssetManager* EngineObject::GetDiskAssetManager() const
	{
		return m_engine->GetDiskAssetManager(); 
	}

	Renderer::Renderer* EngineObject::GetRenderer() const
	{
		return m_engine->GetRenderer();
	}

	World::World* EngineObject::GetWorld() const
	{
		return m_engine->GetWorld();
	}

	const Input::Input& EngineObject::GetInput() const
	{
		return m_engine->GetInput();
	}

	void EngineObject::SetEventCallback(std::function<void(Event::Event&)> fn)
	{
		m_engine->SetEventCallback(fn); 
	}

	void EngineObject::ProcessEvent(Event::Event& e)
	{
		m_engine->ProcessEvent(e);
	}
}
