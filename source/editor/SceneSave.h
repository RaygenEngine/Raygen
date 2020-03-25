#pragma once
#include "asset/AssetPod.h"

class World;

class SceneSave {
public:
	void OpenBrowser();

	static void SaveAs(World* world, const uri::Uri& p);
};
