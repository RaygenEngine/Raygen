#include "Assets.h"

void Assets::Init()
{
	AssetImporterManager = new AssetImporterManager_();
	AssetManager = new AssetManager_();
}

void Assets::Destroy()
{
	delete AssetManager;
	delete AssetImporterManager;
}
