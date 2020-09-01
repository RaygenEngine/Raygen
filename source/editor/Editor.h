#pragma once
#include "universe/Entity.h"

class Editor {
	friend class Engine_;

	static void Init();
	static void Destroy();


public:
	static void Update();
	static Entity GetSelection();
};
