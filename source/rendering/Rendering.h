#pragma once

class Rendering {
	friend class S_Engine;
	static void Init();
	static void Destroy();

public:
	static void DrawFrame();
};
