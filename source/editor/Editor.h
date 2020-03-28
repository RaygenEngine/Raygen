#pragma once

class Editor {
	friend class Engine_;

	static void Init();
	static void Destroy();

public:
	static void Update();
	[[nodiscard]] static bool ShouldUpdateWorld();
};
