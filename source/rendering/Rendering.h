#pragma once

class Rendering {
	friend class Engine_;
	static void Init();
	static void Destroy();

public:
	static void DrawFrame();
};
