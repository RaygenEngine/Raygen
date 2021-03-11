#pragma once
#include "core/math-ext/Frustum.h"
#include "rendering/scene/SceneStructs.h"

struct Camera_Ubo {
	XMFLOAT3A position;
	XMFLOAT4X4A view;
	XMFLOAT4X4A proj;
	XMFLOAT4X4A viewProj;
	XMFLOAT4X4A viewInv;
	XMFLOAT4X4A projInv;
	XMFLOAT4X4A viewProjInv;
};

struct SceneCamera : SceneStruct {
	SCENE_STRUCT(SceneCamera);
	Camera_Ubo ubo;

	XMFLOAT4X4A prevViewProj{};
	math::Frustum frustum;
};
