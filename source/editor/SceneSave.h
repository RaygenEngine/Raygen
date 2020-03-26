#pragma once
#include "assets/AssetPod.h"

class World;

class SceneSave {
public:
	void OpenBrowser();

	static void SaveAs(World* world, const uri::Uri& p);
};
