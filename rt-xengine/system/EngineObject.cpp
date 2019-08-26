#include "pch.h"

#include "system/EngineObject.h"

namespace System
{
	EngineObject::EngineObject(Engine* engine)
		: m_engine(engine), 
		m_parentObject(nullptr), 
		m_id(Core::UUIDGenerator::GenerateUUID())
	{
	}

	EngineObject::EngineObject(EngineObject* pObject)
		: m_engine(pObject->GetEngine()), m_parentObject(pObject),
		m_id(Core::UUIDGenerator::GenerateUUID())
	{
	}

	void EngineObject::Update()
	{
	}

	void EngineObject::WindowResize(int32 width, int32 height)
	{
	}
}
