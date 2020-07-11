#include "pch.h"
#include "TerrainWalkerUserNode.h"

#include "assets/util/ParsingUtl.h"
#include "engine/Engine.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "universe/nodes/RootNode.h"
#include "assets/pods/Image.h"

float TerrainWalkerUserNode::CaclulateHeight(glm::vec3 worldPos, float deltaTime)
{
	using namespace glm;

	const bool isInBounds
		= worldPos.x > 0 && worldPos.x < m_terrainWorldScale && worldPos.z > 0 && worldPos.z < m_terrainWorldScale;

	if (!isInBounds) {
		return worldPos.y;
	}

	if (m_terrainWorldScale == 0) {
		return worldPos.y;
	}


	auto img = m_heightmap.Lock();
	auto sampleImg = [&](int w, int h) {
		int sampleW = glm::clamp(w, 0, img->width - 1);
		int sampleH = glm::clamp(h, 0, img->height - 1);

		size_t index = (sampleH * img->width + sampleW) * 4; // assumes 4 pixel size

		unsigned char data = img->data.at(index);
		float height = (data / 255.0f) * m_heightMultiplier;
		return height;
	};

	float u = worldPos.x / m_terrainWorldScale;
	float v = worldPos.z / m_terrainWorldScale;

	int sampleW = glm::clamp(math::roundToInt(u * img->width), 0, img->width - 1);
	int sampleH = glm::clamp(math::roundToInt(v * img->height), 0, img->height - 1);

	size_t index = (sampleH * img->width + sampleW) * 4; // assumes 4 pixel size

	float height = img->data.at(index) / 255.0f;
	height *= m_heightMultiplier;


	float sw = u * img->width - 0.5f;
	float sh = v * img->height - 0.5f;

	float TL = sampleImg(math::roundToInt(floor(sw)), math::roundToInt(floor(sh)));
	float TR = sampleImg(math::roundToInt(ceil(sw)), math::roundToInt(floor(sh)));
	float BL = sampleImg(math::roundToInt(floor(sw)), math::roundToInt(ceil(sh)));
	float BR = sampleImg(math::roundToInt(ceil(sw)), math::roundToInt(ceil(sh)));

	float rightPercent = fract(sw);
	float bottomPercent = fract(sh);

	float topInterpol = std::lerp(TL, TR, rightPercent);
	float botInterpol = std::lerp(BL, BR, rightPercent);

	height = std::lerp(topInterpol, botInterpol, bottomPercent);

	if (m_isInAir) {
		float jumpPos = ApplyJump(deltaTime) + m_playerHeight;
		if (height + m_playerHeight > jumpPos) {
			m_velocity = 0;
			m_isInAir = false;
		}
		else {
			return jumpPos;
		}
	}

	return height + m_playerHeight;
}

float TerrainWalkerUserNode::ApplyJump(float deltaTime)
{
	float h = GetNodePositionLCS().y - m_playerHeight;
	if (m_isInAir) {
		h += m_velocity * deltaTime;
		m_velocity += m_gravity * deltaTime;
	}
	return h;
}

void TerrainWalkerUserNode::Update(float deltaTime)
{
	using namespace glm;

	auto& input = Input;
	auto& gamepad = input.GetGamepadState();

	auto speed = m_movementSpeed;
	speed *= deltaTime;

	if (input.IsDown(Key::Shift)) {
		speed *= 2.5;
	}

	if (input.IsMouseDragging()) {
		const float yaw = -input.GetMouseDelta().x * m_turningSpeed * 0.5f;
		const float pitch = -input.GetMouseDelta().y * m_turningSpeed * 0.5f;

		RotateNodeAroundAxisWCS(GetParent()->GetNodeUpWCS(), yaw);
		RotateNodeAroundAxisWCS(GetNodeRightWCS(), pitch);
	}

	vec3 forward2d = GetNodeForwardWCS();

	forward2d.y = 0;
	forward2d = normalize(forward2d);

	if (input.IsJustPressed(Key::Space) && m_velocity <= 0) {
		m_velocity = m_jumpVelocity;
		m_isInAir = true;
	}

	if (input.AreKeysDown(Key::W /*, Key::GAMEPAD_DPAD_UP*/)) {
		AddNodePositionOffsetWCS(forward2d * speed);
	}

	if (input.AreKeysDown(Key::S /*, Key::GAMEPAD_DPAD_DOWN*/)) {
		AddNodePositionOffsetWCS((-forward2d) * speed);
	}

	if (input.AreKeysDown(Key::D /*, Key::GAMEPAD_DPAD_RIGHT*/)) {
		AddNodePositionOffsetLCS((GetNodeRightLCS()) * speed);
	}

	if (input.AreKeysDown(Key::A /*, Key::GAMEPAD_DPAD_LEFT*/)) {
		AddNodePositionOffsetLCS((-GetNodeRightLCS()) * speed);
	}


	auto pos = GetNodePositionWCS();

	pos.y = CaclulateHeight(pos, deltaTime);
	SetNodePositionWCS(pos);
}
