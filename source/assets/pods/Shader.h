#pragma once
#include "assets/AssetPod.h"
#include "assets/util/ShaderStageEnums.h"

struct Shader : public AssetPod {
	REFLECTED_POD(Shader)
	{
		REFLECT_ICON(FA_QRCODE);

		REFLECT_VAR(vertex);
		REFLECT_VAR(fragment);

		REFLECT_VAR(rayGen);
		REFLECT_VAR(intersect);
		REFLECT_VAR(anyHit);
		REFLECT_VAR(closestHit);
		REFLECT_VAR(miss);

		REFLECT_VAR(geometry);

		REFLECT_VAR(tessControl);
		REFLECT_VAR(tessEvaluation);


		REFLECT_VAR(callable);
		REFLECT_VAR(compute);
	}

	PodHandle<ShaderStage> vertex;
	PodHandle<ShaderStage> fragment;

	PodHandle<ShaderStage> rayGen;
	PodHandle<ShaderStage> intersect;
	PodHandle<ShaderStage> anyHit;
	PodHandle<ShaderStage> closestHit;
	PodHandle<ShaderStage> miss;

	PodHandle<ShaderStage> geometry;


	PodHandle<ShaderStage> tessControl;
	PodHandle<ShaderStage> tessEvaluation;

	PodHandle<ShaderStage> callable;
	PodHandle<ShaderStage> compute;


	PodHandle<ShaderStage> GetStage(ShaderStageType type) const
	{
		switch (type) {
			case ShaderStageType::Vertex: return vertex;
			case ShaderStageType::Fragment: return fragment;
			case ShaderStageType::RayGen: return rayGen;
			case ShaderStageType::Intersect: return intersect;
			case ShaderStageType::AnyHit: return anyHit;
			case ShaderStageType::ClosestHit: return closestHit;
			case ShaderStageType::Miss: return miss;
			case ShaderStageType::Geometry: return geometry;
			case ShaderStageType::TessControl: return tessControl;
			case ShaderStageType::TessEvaluation: return tessEvaluation;
			case ShaderStageType::Callable: return callable;
			case ShaderStageType::Compute: return compute;
		}
	}
};
