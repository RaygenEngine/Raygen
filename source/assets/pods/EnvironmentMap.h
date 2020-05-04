#pragma once
#include "assets/AssetPod.h"
#include "assets/PodHandle.h"
#include "assets/pods/Cubemap.h"
#include "reflection/GenMacros.h"

#include <array>

struct EnvironmentMap : AssetPod {

	REFLECTED_POD(EnvironmentMap)
	{
		using namespace PropertyFlags;
		REFLECT_ICON(FA_CLOUD_MOON_RAIN);

		REFLECT_VAR(skybox);
		REFLECT_VAR(irradiance);
		REFLECT_VAR(prefiltered);
		REFLECT_VAR(brdfLut);
	}

	// TODO: TEMP: skybox - this will be part of the skymesh
	PodHandle<::Cubemap> skybox;
	// diffuse irradiance
	PodHandle<::Cubemap> irradiance;
	// specular lobe
	PodHandle<::Cubemap> prefiltered;
	// brdf look up texture
	PodHandle<::Cubemap> brdfLut;
};
