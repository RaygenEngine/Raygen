#pragma once

#include "asset/AssetPod.h"

#include <imgui.h>

class World;

class SceneSave {
public:
	SceneSave();

	void OpenBrowser();

	static void SaveAs(World* world, const uri::Uri& p);
};
