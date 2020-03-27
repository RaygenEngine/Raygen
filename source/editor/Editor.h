#pragma once

class Editor {
	friend class S_Engine;

	static void Init();
	static void Destroy();

public:
	static void Update();
	static void PreBeginFrame();
	[[nodiscard]] static bool ShouldUpdateWorld();
};
