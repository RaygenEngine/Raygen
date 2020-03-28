#pragma once

class Editor {
	friend class Engine_;

	static void Init();
	static void Destroy();

public:
	static void Update();
	static void PreBeginFrame();
	[[nodiscard]] static bool ShouldUpdateWorld();
};
