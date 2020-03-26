#pragma once
#include "universe/World.h"

class Universe {
	friend class S_Engine;

	static void Init();
	static void Destroy();

public:
	static World* GetMainWorld() { return MainWorld; }

}; // namespace Universe
