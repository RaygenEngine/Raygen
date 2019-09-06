#include "pch.h"
#include "system/EngineComponent.h"
#include "system/Engine.h"

World* EngineComponent::GetWorld() const
{
	return m_engine->GetWorld();
}
