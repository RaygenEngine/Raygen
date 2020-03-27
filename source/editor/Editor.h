#pragma once


class Editor {
	friend class S_Engine;

	static void Init();
	static void Destroy();

public:
	static void Update();
	[[nodiscard]] static bool ShouldUpdateWorld();
};
