#pragma once

class Engine;

class EngineComponent
{
protected:
	Engine* m_engine;
public:
	EngineComponent(Engine* engine)
		: m_engine(engine) {}

	[[nodiscard]] Engine* GetEngine() const { return m_engine; }
	[[nodiscard]] Engine* GetEngine(EngineComponent*) const { return m_engine; }

	// TODO: 
	// std::vector<ComponentSubobject> 
};
