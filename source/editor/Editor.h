#pragma once
#include "universe/Entity.h"

namespace vk {
class CommandBuffer;
}

using ImTextureID = void*;

class Editor {
	friend class Engine_;

	static void Init();
	static void Destroy();

public:
	static void Update();
	static Entity GetSelection();

	static void SetSelection(Entity ent);
	static void ClearSelection();

	static void BeforePlayWorld(World& world);
	static void AfterStopWorld(World& world);

	static void Draw(vk::CommandBuffer* cmdBuffer);

	static std::pair<glm::vec2, glm::vec2> GetIconUV(const char* icon);
	static ImTextureID GetFontIconTexture();
};
