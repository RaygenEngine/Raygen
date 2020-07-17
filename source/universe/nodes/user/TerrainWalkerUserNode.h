#pragma once
#include "universe/nodes/user/UserNode.h"

#include "assets/PodHandle.h"

struct Image;


class TerrainWalkerUserNode : public UserNode {
	REFLECTED_NODE(TerrainWalkerUserNode, UserNode)
	{
		REFLECT_VAR(m_heightMultiplier);
		REFLECT_VAR(m_heightmap);
		REFLECT_VAR(m_terrainWorldScale);
		REFLECT_VAR(m_playerHeight);

		REFLECT_VAR(m_gravity);
		REFLECT_VAR(m_jumpVelocity);
	}

	float m_heightMultiplier;
	float m_terrainWorldScale;
	float m_playerHeight;
	float CaclulateHeight(glm::vec3 worldPos, float deltaTime);
	PodHandle<Image> m_heightmap;

	float m_gravity{ -9.9f };
	float m_jumpVelocity{ 10.0f };

	float m_velocity{ 0 };
	bool m_isInAir{ false };

	float ApplyJump(float deltaTime);

public:
	void Update(float deltaTime) override;
};
