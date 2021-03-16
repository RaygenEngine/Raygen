#pragma once

#if defined(near)
#	undef near
#endif

#if defined(far)
#	undef far
#endif

// SceneStructs that upload a Ubo when dirty
struct SceneStruct {
	size_t uboSize;

	bool isDirty{ true };

	SceneStruct(size_t uboSize);
};

#define SCENE_STRUCT(Child)                                                                                            \
	Child()                                                                                                            \
		: SceneStruct(sizeof(decltype(ubo)))                                                                           \
	{                                                                                                                  \
	}

struct SceneGeometry : SceneStruct {
	SCENE_STRUCT(SceneGeometry);

	struct {
		XMFLOAT4X4A transform;
		XMFLOAT4X4A prevTransform;
	} ubo{};

	GpuHandle<Mesh> mesh;
};

struct SceneAnimatedGeometry : SceneStruct { // NEW::
	SCENE_STRUCT(SceneAnimatedGeometry);

	struct {
		XMFLOAT4X4A transform;
	} ubo{};
	GpuHandle<SkinnedMesh> mesh;
	PodHandle<SkinnedMesh> meshPod;

	std::vector<glm::mat4> jointMatrices;
};


struct SceneCamera : SceneStruct {
	SCENE_STRUCT(SceneCamera);

	struct {
		XMFLOAT3A position;
		XMFLOAT4X4A view;
		XMFLOAT4X4A proj;
		XMFLOAT4X4A viewProj;
		XMFLOAT4X4A viewInv;
		XMFLOAT4X4A projInv;
		XMFLOAT4X4A viewProjInv;
	} ubo{};

	XMFLOAT4X4A prevViewProj{};
};


struct SceneDirlight : SceneStruct {
	SCENE_STRUCT(SceneDirlight);

	struct {
		XMFLOAT3A front;

		// Lightmap
		XMFLOAT4X4A viewProj;
		XMFLOAT3A color;

		float intensity;

		float maxShadowBias;
		int32 samples;
		float sampleInvSpread;
		int32 hasShadow;
	} ubo{};

	std::string name;
};


struct SceneSpotlight : SceneStruct {
	SCENE_STRUCT(SceneSpotlight);

	struct Spotlight_Ubo {
		XMFLOAT3A position;
		XMFLOAT3A front;

		// Lightmap
		XMFLOAT4X4A viewProj;
		XMFLOAT3A color;

		float intensity;

		float near;
		float far;

		float outerCutOff;
		float innerCutOff;

		float constantTerm;
		float linearTerm;
		float quadraticTerm;

		float maxShadowBias;
		int32 samples;
		float sampleInvSpread;
		int32 hasShadow;
	} ubo{};

	std::string name;
};


struct SceneQuadlight : SceneStruct {
	SCENE_STRUCT(SceneQuadlight);
	struct {
		XMFLOAT3A center;
		XMFLOAT4A orienation;

		XMFLOAT3A color;

		float intensity;
		float constantTerm;
		float linearTerm;
		float quadraticTerm;

		float cosAperture;

		float radius;

		int32 samples;
		int32 hasShadow;
	} ubo{};

	XMFLOAT4X4A transform{};
};


struct ScenePointlight : SceneStruct {
	SCENE_STRUCT(ScenePointlight);
	struct {
		XMFLOAT3A position;
		XMFLOAT3A color;

		float intensity;

		float constantTerm;
		float linearTerm;
		float quadraticTerm;

		float radius;

		int32 samples;
		int32 hasShadow;
	} ubo{};

	XMFLOAT4X4 volumeTransform{};
};

struct SceneReflprobe : public SceneStruct {
	SceneReflprobe();

	struct {
		int32 lodCount;
		float radius;
		float irradianceFactor;
		float pad;
		XMFLOAT3A position;
	} ubo{};


	int32 ptSamples{ 16 };
	int32 ptBounces{ 3 };

	int32 irrResolution{ 32 };

	int32 prefSamples{ 1024 };


	BoolFlag shouldBuild{ true };
};


struct SceneIrragrid : public SceneStruct {
	SCENE_STRUCT(SceneIrragrid);

	struct {
		int32 width;
		int32 height;
		int32 depth;
		int32 builtCount;

		XMFLOAT4A posAndDist;
	} ubo{};

	BoolFlag shouldBuild{ false };
	int32 ptSamples{ 2 };
	int32 ptBounces{ 2 };

	int32 irrResolution{ 32 };
};
