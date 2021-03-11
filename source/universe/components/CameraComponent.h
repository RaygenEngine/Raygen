#pragma once
#include "universe/BasicComponent.h"
#include "universe/ComponentsDb.h"
#include "universe/SceneComponentBase.h"

#if defined(near)
#	undef near
#endif

#if defined(far)
#	undef far
#endif


struct SceneCamera;

struct CCamera : CSceneBase {
	REFLECTED_SCENE_COMP(CCamera, SceneCamera)
	{
		REFLECT_ICON(FA_CAMERA);
		// REFLECT_CATEGORY("Rendering");

		REFLECT_VAR(near).Clamp(0.001f);
		REFLECT_VAR(far).Clamp(0.001f);
		REFLECT_VAR(focalLength).Clamp(0.001f);
		REFLECT_VAR(vFov, PropertyFlags::Rads).Clamp(0.001f, XM_PI);
		REFLECT_VAR(hFov, PropertyFlags::Rads).Clamp(0.001f, XM_PI);
		REFLECT_VAR(vFovOffset, PropertyFlags::Rads);
		REFLECT_VAR(hFovOffset, PropertyFlags::Rads);
		REFLECT_VAR(viewportWidth, PropertyFlags::Transient).Clamp(1.f);
		REFLECT_VAR(viewportHeight, PropertyFlags::Transient).Clamp(1.f);
	}

	CCamera();
	~CCamera();

	// distance to film plane
	float focalLength{ 1.f };

	// vertical fov (angle)
	float vFov{ XMConvertToRadians(72.f) };
	// horizontal fov depends on the vertical and the aspect ratio
	float hFov{ XMConvertToRadians(106.f) };

	float near{ 0.1f };
	float far{ 1000.f };

	float vFovOffset{ 0.f };
	float hFovOffset{ 0.f };

	int32 viewportWidth{ 1280 };
	int32 viewportHeight{ 720 };

	__declspec(align(16)) struct AlignedData {
		XMMATRIX view;
		XMMATRIX proj;
	};
	AlignedData* pData;
};
